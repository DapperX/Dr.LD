#ifndef _SYMBOL_HPP_
#define _SYMBOL_HPP_

#include "elf.h"
#include "ELFFile.hpp"

class Symbol
{
public:
    Symbol();
    ~Symbol();

private:
    const ELFFile &file;
    Elf64_Sym symbol;
}

#endif /* _SYMBOL_HPP_ */
