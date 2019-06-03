/*
* @Author: robertking
* @Date:   2019-06-03 12:05:02
* @Last Modified by:   robertking
* @Last Modified time: 2019-06-03 12:14:32
*/

#include "Linker.hpp"

using namespace DrLD;

int main()
{
    Linker linker;
    std::vector<std::string> names;
    names.push_back("test/lib.o");
    names.push_back("test/main.o");
    linker.add_files(names);
    linker("mian.exe");
    return 0;
}
