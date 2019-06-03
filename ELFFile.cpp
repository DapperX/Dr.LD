#include <fstream>
#include <iterator>
#include <cstring>
#include "elf.h"
#include "ELFFile.hpp"

namespace DrLD {

ELFFile::type_identity ELFFile::id_now;

ELFFile::ELFFile(const std::string &filename)
	: identity(gen_identity()), filename(filename)
{
	std::ifstream file_input(filename, std::ifstream::binary);
	auto input_buffer = file_input.rdbuf();
	size = input_buffer->pubseekoff(0, file_input.end);
	input_buffer->pubseekoff(0, file_input.beg);
	// content.resize(size);
	// input_buffer->sgetn((char*)content.data(), size);
	content = new byte[size];
	input_buffer->sgetn((char*)content, size);
	file_input.close();
}

ELFFile::ELFFile(ELFFile &&rhs)
	: filename(std::move(rhs.filename)), identity(rhs.identity),
	size(rhs.size), content(rhs.content)
{
	rhs.size = 0;
	rhs.identity = 0;
	rhs.content = nullptr;
}

ELFFile::~ELFFile()
{
	delete []content;
}

const std::string& ELFFile::get_filename() const
{
	return filename;
}

size_t ELFFile::get_size() const
{
	return size;
}

ELFFile::type_identity ELFFile::get_identity() const
{
	return identity;
}

ELFFile::type_identity ELFFile::gen_identity()
{
	return ++id_now;
}

byte *ELFFile::get_data()
{
	return content;
}

Elf64_Ehdr& ELFFile::get_header()
{
	return *(Elf64_Ehdr*)content;
}

slice<Elf64_Shdr*> ELFFile::get_section()
{
	const Elf64_Ehdr& header = get_header();
	Elf64_Shdr *section = (Elf64_Shdr*)&content[header.e_shoff];
	return slice<Elf64_Shdr*>(section, header.e_shnum);
}

slice<char*> ELFFile::get_strtbl(size_t section_id)
{
	return get_table<char>(section_id, false);
}

slice<Elf64_Sym*> ELFFile::get_symtbl(size_t section_id)
{
	return get_table<Elf64_Sym>(section_id, true);
}

slice<Elf32_Rel*> ELFFile::get_reltbl(size_t section_id)
{
	return get_table<Elf32_Rel>(section_id, true);
}

typename slice<Elf64_Shdr*>::iterator ELFFile::find_section(int sh_type, slice<Elf64_Shdr*>::iterator start)
{
	const auto& sections = get_section();
	for(auto i=start; i!=sections.end(); ++i)
	{
		if(i->sh_type==sh_type) return i;
	}
	return sections.end();
}

typename slice<Elf64_Shdr*>::iterator ELFFile::find_section(std::string name, slice<Elf64_Shdr*>::iterator start)
{
	auto shstrtab = get_strtbl(get_header().e_shstrndx);
	const char *raw_str = shstrtab.raw();
	const auto& sections = get_section();
	for(auto i=start; i!=sections.end(); ++i)
	{
		if(!strcmp(&raw_str[i->sh_name], name.c_str())) return i;
	}
	return sections.end();
}

template<typename type_table>
slice<type_table*> ELFFile::get_table(size_t section_id, bool spec_entsize)
{
	const auto &section = get_section()[section_id];
	return get_table<type_table>(section, spec_entsize);
}

template<typename type_table>
slice<type_table*> ELFFile::get_table(const Elf64_Shdr &section, bool spec_entsize)
{
	auto base = (type_table*)&content[section.sh_offset];
	size_t entsize = spec_entsize?section.sh_entsize:sizeof(type_table);
	return slice<type_table*>(base, section.sh_size/entsize, entsize);
}

} // namespace DrLD

/*
// upper align `val` to multiple of 2**`bit`
static inline u32 align(u32 val,u32 bit)
{
	register u32 mask=(1u<<bit)-1;
	return (val+mask)&(~mask);
}
*/

/*
void elf_init_LMA(byte *const buffer)
{
	// following define may be used in the future for supportnig more segment attributes
	#define SEGMENT_RO 0
	#define SEGMENT_RW 1

	#define TO_TYPE_SEGMENT(type_section) ((type_section)&SHF_WRITE)

	#define CNT_TYPE 2
	u32 offset[CNT_TYPE]={0};
	u32 base[CNT_TYPE];

	Elf32_Ehdr *header=(Elf32_Ehdr*)buffer;
	kprintf("e_shentsize: %d\n",header->e_shentsize);
	kprintf("e_shnum: %d\n",header->e_shnum);
	Elf32_Shdr *section=(Elf32_Shdr*)&buffer[header->e_shoff];
	for(u32 i=0;i<header->e_shnum;++i)
	{
		if(!(section[i].sh_flags&SHF_ALLOC)) continue;
		// for now there are only two types of segment: ReadOnly and Read/Write
		// NoeXecute attribute of page is unsupported yet
		// the code is designed for more types of segment
		u32 type=TO_TYPE_SEGMENT(section[i].sh_flags);
		section[i].sh_addr=align(offset[type],section[i].sh_addralign);
		offset[type]=section[i].sh_addr+section[i].sh_size;
	}
	for(u32 i=0;i<CNT_TYPE;++i)
	{
		u32 cnt=align12(offset[i])>>12;
		base[i]=(u32)get_page_free(cnt,i<<1);
	}
	for(u32 i=0;i<header->e_shnum;++i)
		if(section[i].sh_flags&SHF_ALLOC)
		{
			section[i].sh_addr+=base[TO_TYPE_SEGMENT(section[i].sh_flags)];
			kprintf("section[%d].addr:%p\n",i,section[i].sh_addr);
		}
	#undef CNT_TYPE
	#undef TO_TYPE_SEGMENT
	#undef SEGMENT_RW
	#undef SEGMENT_RO
}
*/
/*
void elf_relocate(byte *const buffer)
{
	Elf32_Ehdr *header=(Elf32_Ehdr*)buffer;
	Elf32_Shdr *section=(Elf32_Shdr*)&buffer[header->e_shoff];
	char *shstrtab=(char*)&buffer[section[header->e_shstrndx].sh_offset];

	// relocatation
	for(u32 i=0;i<header->e_shnum;++i)
	{
		if(section[i].sh_type!=SHT_REL) continue;

		u32 cnt_reltab=section[i].sh_size/section[i].sh_entsize;
		Elf32_Rel *reltab=(Elf32_Rel*)&buffer[section[i].sh_offset];
		Elf32_Word id_symtab=section[i].sh_link;
		Elf32_Word dest=section[i].sh_info;
	//	kprintf("#%s symtab:%d dest:%d\n",&shstrtab[section[i].sh_name],id_symtab,dest);

		Elf32_Sym *symtab=(Elf32_Sym*)&buffer[section[id_symtab].sh_offset];
		char *strtab=(char*)&buffer[section[section[id_symtab].sh_link].sh_offset];
		byte *section_dest=(byte*)&buffer[section[dest].sh_offset];
		byte *addr_section_dest=(byte*)section[dest].sh_addr;

		for(u32 j=0;j<cnt_reltab;++j)
		{
			Elf32_Addr offset=reltab[j].r_offset;
			Elf32_Sym *symbol=&symtab[reltab[j].r_info>>8];

			kprintf("@symtab[%d]: ",reltab[j].r_info>>8);
			if((symbol->st_info&0xf)==STT_SECTION)
				kputs(&shstrtab[section[symbol->st_shndx].sh_name]);
			else
				kputs(&strtab[symbol->st_name]);
			kprintf("offset: %x\n",offset);

			byte *addr_section_sym=NULL;
			switch(symbol->st_shndx)
			{
				case SHN_ABS:
				case SHN_COMMON:
				case SHN_UNDEF:
					DEBUG_BREAKPOINT; // ignore these pseudo sections, for now
					break;
				default:
					addr_section_sym=(byte*)section[symbol->st_shndx].sh_addr;
			}
			if(!addr_section_sym)
			{
				kprintf("unsupported symbol section %d\n",symbol->st_shndx);
				continue;
			}

			if((reltab[j].r_info&0xff)==R_386_32) // absolute location
				*(int*)(section_dest+offset)+=(int)((u32)(addr_section_sym+symbol->st_value)-ADDR_LOW_MEMORY+ADDR_HIGH_MEMORY);
			else // relative location(R_386_PC32)
				*(int*)(section_dest+offset)+=(int)((u32)(addr_section_sym+symbol->st_value)-(u32)(addr_section_dest+offset));
		}
	}
	kprintf("buffer: %p\n",buffer);
}
*/