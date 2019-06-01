#ifndef _LINKER_HPP_
#define _LINKER_HPP_

#include <map>
#include "base.h"
#include "Symbol.hpp"

namespace DrLD {

class Linker {
public:
    Linker();
    ~Linker();

    void add(const std::vector<std::string> &);
    void add(const std::string &);

    const file_content &operator()();

private:
    std::vector<ELFFile> files;
    std::map<std::string_view, Symbol> symbol_table;

    file_content result;
};

}

#endif /* _LINKER_HPP_ */
