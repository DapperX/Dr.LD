#ifndef _ELFFILE_HPP_
#define _ELFFILE_HPP_

#include <string>
#include "elf.h"
#include "base.h"
#include "slice.hpp"

namespace DrLD {

class ELFFile
{
public:
	typedef u32 type_identity;

    ELFFile(const std::string &filename);
    ELFFile(const ELFFile &rhs) = delete;
    ELFFile(ELFFile &&rhs);
    ~ELFFile();

	const std::string& get_filename() const;
	size_t get_size() const;
	type_identity get_identity() const;

	byte* get_data();
	Elf64_Ehdr& get_header();
	slice<Elf64_Shdr*> get_section();
	slice<char*> get_strtbl(size_t section_id);
	slice<Elf64_Sym*> get_symtbl(size_t section_id);
	slice<Elf32_Rel*> get_reltbl(size_t section_id);

	template<typename type_table>
	slice<type_table*> get_table(size_t section_id, bool spec_entsize=false);
	template<typename type_table>
	slice<type_table*> get_table(const Elf64_Shdr &section, bool spec_entsize=false);

	slice<Elf64_Shdr*>::iterator find_section(int sh_type, slice<Elf64_Shdr*>::iterator start);
	slice<Elf64_Shdr*>::iterator find_section(std::string name, slice<Elf64_Shdr*>::iterator start);

private:
	std::string filename;
	type_identity identity;
	size_t size;
	byte* content;

	static type_identity id_now;
	static type_identity gen_identity();
};

}

#endif /* _ELFFILE_HPP_ */
