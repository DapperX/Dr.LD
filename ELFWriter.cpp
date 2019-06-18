#include <cstring>
#include <sys/uio.h>
#include <memory>
#include <iostream>
#include "elf.h"
#include "ELFWriter.hpp"

namespace DrLD{

ELFWriter::ELFWriter()
{
	memset(&header, 0, sizeof(header));
	Elf64_Shdr section_null{0, SHT_NULL};
	add_section_header(section_null);
}

Elf64_Ehdr& ELFWriter::get_header()
{
	return header;
}

void ELFWriter::set_header(const Elf64_Ehdr &header)
{
	ELFWriter::header = header;
}

size_t ELFWriter::add_section_header(const Elf64_Shdr &section_header)
{
	section.push_back(loose_section{.header=section_header});
	return section.size();
}

size_t ELFWriter::add_section_body(const slice<byte*> &section_body)
{
	return add_section_body(section_body, section.size()-1);
}

size_t ELFWriter::add_section_body(const slice<byte*> &section_body, size_t pos)
{
	section[pos].body.push_back(section_body);
	return pos;
}

size_t ELFWriter::write(std::string filename)
{
	FILE *file = fopen(filename.c_str(), "wb");
	int fd = fileno(file);
	fwrite(&header, sizeof(header), 1, file);
	size_t offset = header.e_ehsize;
	fseek(file, offset, SEEK_SET);

	std::vector<iovec> buffer;
	buffer.reserve(section.size());
	for(auto &k : section)
	{
		buffer.emplace_back((iovec){&k.header, sizeof(k.header)});
		for(auto &t : k.body)
			buffer.emplace_back((iovec){t.raw(), t.size()*t.byte_each()});
	}
	for (size_t off = 0; off < buffer.size(); off += IOV_MAX)
		writev(fd, &*buffer.begin() + off, std::min(buffer.size() - off, (size_t)IOV_MAX));
	fclose(file);
	return 0;
}

}