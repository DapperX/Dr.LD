#include <sys/uio.h>
#include "ELFWriter.hpp"

ELFWriter::ELFWriter()
	: header(NULL)
{
	Elf64_Shdr section_null{
		0,
		SH_NULL,
	};
}

void ELFWriter::set_header(Elf64_Ehdr *header)
{
	ELFWriter::header = header;
}

size_t ELFWriter::add_section_header(Elf64_Shdr section_header)
{
	ELFWriter::section_header.push_back(section_header);
}

void ELFWriter::add_section_body(const slice<byte*> &section_body)
{
	ELFWriter::section_body.push_back(section_body);
}

size_t ELFWriter::write(std::string filename)
{
	FILE *file = fopen(filename.c_str(), "wb");
	int fd = fileno(file);

	fclose(file);
	return 0;
}