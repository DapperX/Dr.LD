/*
 * @Author: robertking
 * @Date:   2019-06-03 12:05:02
 * @Last Modified by:   robertking
 * @Last Modified time: 2019-06-03 15:39:31
 */

#include "Linker.hpp"

#include <cstdlib>

using namespace DrLD;

int main(int argc, char const *argv[])
{

    Linker linker;
    std::vector<std::string> names;
    if (argc <= 1) return 1;
    int numfile = atoi(argv[1]);
    for (int i = 0; i < numfile; ++i)
        names.push_back("test/code/" + std::to_string(i) + ".o");
    linker.add_files(names);
    linker("mian.exe");
    return 0;
}
