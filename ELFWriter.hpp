#ifndef _ELFWRITER_HPP_
#define _ELFWRITER_HPP_

#include <cstdint>
#include <vector>
#include <string>
#include "slice.hpp"

namespace DrLD{

class ELFWriter{
public:
	ELFWriter();

	Elf64_Ehdr& get_header();
	void set_header(const Elf64_Ehdr &header);
	size_t add_section_header(const Elf64_Shdr &section_header);
	size_t add_section_body(const slice<byte*> &section_body);
	size_t add_section_body(const slice<byte*> &section_body, size_t pos);
	size_t write(std::string filename);

private:
	struct loose_section{
		Elf64_Shdr header;
		std::vector<slice<byte*>> body;
	};

	Elf64_Ehdr header;
	std::vector<loose_section> section;
};

};

#endif // _ELFWRITER_HPP_