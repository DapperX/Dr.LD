#include "Linker.hpp"
#include "ELFParser.hpp"

void Linker::add_files(const std::vector<std::string> &filenames)
{
    const size_t num_files = filenames.size();
    // #pragma omp parallel for
    for (size_t i = 0; i < num_files; ++i) {
        Arc file = Arc(new ELFFile(filenames[i]));
        files.push_back(file);
    }
}

const file_content &Linker::operator()()
{
    if (result.size()) return result;

    size_t num_files;
    // #pragma omp parallel for
    for (size_t i = 0; i < num_files; ++i) {
        for (auto symbol : files[i]) {
            symbol_table[symbol] = Symbol;
        }
    }

    for (each section)
        for (size_t i = 0; i < num_files; ++i) {
            result += output;
        }

    return result;
}
