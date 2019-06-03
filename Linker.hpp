#ifndef _LINKER_HPP_
#define _LINKER_HPP_

#include "base.h"
#include "ELFFile.hpp"

#include <map>

namespace DrLD {

class Linker {
public:
    Linker();
    ~Linker();

    void add_files(const std::vector<std::string> &);
    // void add_files(const std::string &);

    void operator()(const char *filename);

private:
    static bool replace(const Elf64_Sym &old_hdr, const Elf64_Sym &new_hdr);

private:
    std::vector<ELFFile> files;

    // helper structs
    struct SectionCollection
    {
        size_t offset, bytes;
        size_t section_id;
        std::vector<std::pair<const ELFFile *, Elf64_Shdr> > headers;
        void add(const ELFFile *file, const Elf64_Shdr &sec)
        {
            headers.push_back(std::make_pair(file, sec));
            assert(headers[0].second.sh_type == headers.back().second.sh_type);
            bytes += sec.sh_size;
        }
    };
    // <<file, sec_id>, new_section_offset>
    using SectionIndex = std::map<std::pair<ELFFile*, size_t>, std::pair<size_t, size_t>>;
    /*struct SymbolTableCollection
    {
        struct Bundle {
            ELFFile *file;
            Elf32_Shdr symtab;
            Elf32_Shdr strtab;
        };
        size_t symtab_offset, symtab_bytes;
        size_t strtab_offset, strtab_bytes;
        std::vector<Bundle> headers;
        void add(const ELFFile *file, Elf32_Shdr &symtab, Elf32_Shdr &strtab)
        {
            size_t offset = headers.size() ? headers.back().end_offset() : 0;
            headers.push_back(Bundle { .file = file, .symtab = symtab, .strtab = strtab, })
            symtab_bytes += symtab.sh_size;
            strtab_bytes += strtab.sh_size;
        }
    };*/
};

}

#endif /* _LINKER_HPP_ */
