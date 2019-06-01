#ifndef _ELFFILE_HPP_
#define _ELFFILE_HPP_

#include <string>

namespace DrLD {

class ELFFile
{
public:
    ELFFile();
    ELFFile(const ELFFile &rhs) = delete;
    ELFFile(ELFFile &&rhs);
    ~ELFFile();

    /* class ReltabIterator {
    public:
        ReltabIterator();
        ~ReltabIterator();

    private:
        ELFFile *file;
    };

    ReltabIterator reltab_begin();
    ReltabIterator reltab_end(); */

    void reltab_iter(Func &&f)
    {
        for () f();
    }

private:
    file_content content;
};

}

#endif /* _ELFFILE_HPP_ */
