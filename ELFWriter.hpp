#ifndef _ELFWRITER_HPP_
#define _ELFWRITER_HPP_

#include <cstdint>
#include <vector>
#include <string>

namespace DrLD{

class ELFWriter{
public:
	ELFWriter();

	void set_header(Elf64_Ehdr *header);
	size_t add_section_header(Elf64_Shdr section_header);
	void add_section_body(const slice<byte*> &section_body);
	size_t write(std::string filename);

private:
	Elf64_Ehdr *header;
	std::vector<Elf64_Shdr> section_header;
	std::vector<slice<byte*>> section_body;
};

};

#endif // _ELFWRITER_HPP_