#include "Linker.hpp"
#include "ELFFile.hpp"
#include "Trie.hpp"
#include "ELFWriter.hpp"

#include <cassert>
#include <chrono>
#include <iostream>

// #include <omp.h>

namespace DrLD
{
Linker::Linker() {}
Linker::~Linker() {}

void Linker::add_files(const std::vector<std::string> &filenames)
{
    const size_t num_files = filenames.size();
    // #pragma omp parallel for
    for (size_t i = 0; i < num_files; ++i) {
        ELFFile file(filenames[i]);
        files.push_back(std::move(file));
    }
}

using namespace std::chrono;
high_resolution_clock::time_point _last_time;

void tic(high_resolution_clock::time_point *last_time = &_last_time)
{
    *last_time = high_resolution_clock::now();
}

void toc(const char *name, high_resolution_clock::time_point *last_time = &_last_time)
{
    const auto now = high_resolution_clock::now();
    const auto duration = duration_cast<nanoseconds>(now - *last_time).count();
    std::cerr << name << " takes " << duration << "ns." << std::endl;
}

void Linker::operator()(const char *filename)
{
    high_resolution_clock::time_point all_time;
    tic(&all_time);
    // output file header
    // TODO: initialize
    Elf64_Ehdr new_header;
    // <sec_name, [<file_idx, sec_hdr>]>
    std::map<std::string_view, SectionCollection> sections;

    const size_t num_files = files.size();

    // tic();
    // Extract sections info
    SectionIndex index;
    size_t section_counter = 0;
    std::vector<SectionCollection *> section_vec;
    for (auto &f : files) {
        const Elf64_Ehdr header = f.get_header();
        const slice<char *> shstrtab = f.get_strtbl(header.e_shstrndx);

        auto secs = f.get_section();
        for (auto it = secs.begin(), end = secs.end(); it != end; ++it) {
            if (it->sh_type == SHT_NULL || it->sh_type == SHT_SYMTAB ||
                it->sh_type == SHT_RELA)
                continue;
            auto name = std::string_view(&*(shstrtab.begin() + it->sh_name));
            auto &sec_coll = sections[name];
            if (sec_coll.headers.size() == 0) {
                section_vec.push_back(&sec_coll);
                sec_coll.section_id = section_vec.size();
            }
            index.emplace(
                std::make_pair(&f, it - secs.begin()),
                std::make_pair(sec_coll.section_id, sec_coll.bytes));
            sec_coll.add(&f, *it);
        }
    }
    // toc("Extract sections info");

    // tic();
    // Sum up section sizes & offsets except symtab & rela
    size_t size_sum = sizeof(Elf64_Ehdr);  // initial offset
    // TODO: add (vec.size() + ?) * sizeof(Elf64_Phdr)
    for (auto &sec : section_vec) {
        sec->offset = size_sum;
        size_sum += sec->bytes;
    }
    // toc("Sum up");
    // TODO: fill in resized output

    // tic();
    // Combine symtab in parallel
    Trie<Elf64_Sym> symbol_table;
// #pragma omp parallel for num_threads(8)
    for (size_t i = 0; i < num_files; ++i) {
        auto &f = files[i];
        const auto secs = f.get_section();
        for (auto it = secs.begin(), end = secs.end(); it != end; ++it) {
            const auto &sec = *it;
            if (it->sh_type != SHT_SYMTAB) continue;
            size_t cur_sec_id = it - secs.begin();
            // slice<Elf64_Sym*> symtab = f.get_table<Elf64_Sym>(*it);
            slice<Elf64_Sym *> symtab = f.get_symtbl(cur_sec_id);
            slice<char *> strtab = f.get_strtbl(it->sh_link);
            // auto sec_info = index.find(std::make_pair(&f, sec_id));
            auto str_info = index.find(std::make_pair(&f, it->sh_link));
            // assert(sec_info != index.end());
            assert(str_info != index.end());

            for (auto sym_it = symtab.begin(), end = symtab.end();
                 sym_it != end; ++sym_it) {
                size_t symtab_size = symtab.size();
                size_t sym_id = sym_it - symtab.begin();
                auto &sym = *sym_it;
                // TODO: handle ABS
                if (sym.st_shndx == SHN_ABS) continue;
                if (sym.st_shndx == STN_UNDEF) continue;
                // std::cerr << "sym.st_shndx " << sym.st_shndx << std::endl;
                // std::cerr << "sym name \"" << (const u8*)&*(strtab.begin() +
                // sym.st_name) << "\"" << std::endl;
                if (sym.st_size == 0) continue;  // do not save empty str
                Elf64_Sym new_sym = sym;
                auto sec_info = index.find(std::make_pair(&f, sym.st_shndx));
                assert(sec_info != index.end());
                const size_t old_name = sym.st_name;
                sym.st_name += str_info->second.second;
                sym.st_shndx = sec_info->second.first;
                sym.st_value += sec_info->second.second;
                symbol_table.put((const u8 *)&*(strtab.begin() + old_name),
                                 std::move(sym), &replace);
            }
        }
    }
    symbol_table.put((const u8 *)"", Elf64_Sym(), &replace);
    // toc("Combine symtab in parallel");

    // tic();
// #pragma omp parallel for num_threads(8)
    for (size_t i = 0; i < num_files; ++i) {
        auto &f = files[i];
        const auto secs = f.get_section();
        for (auto it = secs.begin(), end = secs.end(); it != end; ++it) {
            if (it->sh_type != SHT_SYMTAB) continue;
            slice<Elf64_Sym *> symtab = f.get_table<Elf64_Sym>(*it);
            const slice<char *> strtab = f.get_strtbl(it->sh_link);
            /*auto sec_info = index.find(std::make_pair(&f, sec_id));
            auto str_info = index.find(std::make_pair(&f, it->sh_link));
            assert(sec_info != index.end());
            assert(str_info != index.end());*/

            for (auto &sym : symtab) {
                if (sym.st_shndx != STN_UNDEF) continue;
                auto res = symbol_table.get((const u8 *)&*(strtab.begin() + sym.st_name));
                if (res == nullptr) {
                    // TODO: add missing symbol back to symbol table
                    // All symbols exist in the test case.
                    std::cerr << "Symbol \"" << &*(strtab.begin() + sym.st_name) << "\" not found." << std::endl;
                }
            }
        }
    }
    // toc("Handle UNDEF");

    // tic();
    // #pragma omp parallel for num_threads(8)
    for (size_t i = 0; i < num_files; ++i) {
        auto &f = files[i];
        const auto secs = f.get_section();
        for (auto rela_sec = secs.begin(), end = secs.end(); rela_sec != end; ++rela_sec) {
            if (rela_sec->sh_type != SHT_RELA) continue;

            slice<Elf64_Rela*> relatab = f.get_table<Elf64_Rela>(*rela_sec, true);
            slice<Elf64_Sym*>  symtab = f.get_symtbl(rela_sec->sh_link);
            slice<char *> strtab = f.get_strtbl((secs.begin() + rela_sec->sh_link)->sh_link);

            slice<u8*> target = f.get_table<u8>(rela_sec->sh_info, false);

            for (auto &rela : relatab) {
                auto old_sym = symtab.begin() + ELF64_R_SYM(rela.r_info);
                // if (ELF64_ST_TYPE(old_sym->st_info) == STT_SECTION)
                if (old_sym->st_shndx == SHN_ABS || old_sym->st_shndx == SHN_COMMON ||
                    old_sym->st_shndx == SHN_UNDEF)
                    continue;

                // auto sec_addr = (secs.begin() + old_sym->st_shndx)->sh_addr;

                auto new_sym = symbol_table.get((const u8*)&*(strtab.begin() + old_sym->st_name));
                if (new_sym == nullptr) {
                    // std::cerr << "Symbol \"" << &*(strtab.begin() + old_sym->st_name) << "\" not found." << std::endl;
                    continue;
                }

                Elf64_Addr ref_offset = section_vec[new_sym->st_shndx - 1]->offset + new_sym->st_value;

                switch (ELF64_R_TYPE(rela.r_info)) {
                case R_X86_64_32:
                    *(Elf64_Addr*)&*(target.begin() + rela.r_offset) = ref_offset + rela.r_addend;
                    break;
                case R_X86_64_PC32:
                    {
                        auto target_sec = index.find(std::make_pair(&f, old_sym->st_shndx));
                        assert(target_sec != index.end());
                        Elf64_Addr target_offset = section_vec[target_sec->second.first - 1]->offset + target_sec->second.second + rela.r_offset;

                        *(Elf64_Addr*)&*(target.begin() + rela.r_offset) = ref_offset - target_offset + rela.r_addend;
                    }
                    break;
                default:
                    std::cerr << "Relocation type not supported: " << ELF64_R_TYPE(rela.r_info) << std::endl;
                    continue;
                };

            }
        }
    }
    // toc("Handle rela");

    ELFWriter writer;
    Elf64_Ehdr new_hdr = files[0].get_header();
    new_hdr.e_shnum = section_vec.size() + 1;
    new_hdr.e_shoff = sizeof(Elf64_Ehdr);
    new_hdr.e_ehsize = sizeof(Elf64_Ehdr);
    writer.set_header(new_hdr);
    // tic();
    for (auto &sec : section_vec) {
        Elf64_Shdr new_hdr = sec->headers[0].second;
        new_hdr.sh_offset = sec->offset;
        new_hdr.sh_size = sec->bytes;
        auto new_link = index.find(std::make_pair(sec->headers[0].first, (size_t)new_hdr.sh_link));
        auto new_info = index.find(std::make_pair(sec->headers[0].first, (size_t)new_hdr.sh_info));
        assert(new_link == index.end());
        assert(new_info == index.end());
        new_hdr.sh_link = new_link->second.first;
        new_hdr.sh_info = new_info->second.first;

        writer.add_section_header(new_hdr);
        for (auto &hdr : sec->headers) {
            writer.add_section_body(slice(hdr.first->get_data() + hdr.second.sh_offset, hdr.second.sh_size));
        }
    }
    // TODO: add symtable
    // toc("Output");
    writer.write(filename);
    toc("All time", &all_time);
}

bool Linker::replace(const Elf64_Sym &old_hdr, const Elf64_Sym &new_hdr)
{
    const unsigned char old_bind = ELF64_ST_BIND(old_hdr.st_info);
    const unsigned char new_bind = ELF64_ST_BIND(new_hdr.st_info);
    if (old_bind == new_bind) {
        if (old_bind != STB_WEAK) {
            std::cerr << "TWO STRONG" << std::endl;
            throw "TOO STRONG";
        }
        return false;
    }
    if (old_bind == STB_WEAK) return true;
    if (new_bind == STB_WEAK) return false;
    return false;  // should not reach
}

}  // namespace DrLD
