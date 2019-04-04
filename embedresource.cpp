#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char** argv)
{
    if (argc < 3) {
        fprintf(stderr, "USAGE: %s {sym} {rsrc}\n\n"
                        "  Creates {sym}.cpp from the contents of {rsrc}\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    string dst{argv[1]};
    string src{argv[2]};

    string sym = src;
    replace(sym.begin(), sym.end(), '.', '_');
    replace(sym.begin(), sym.end(), '-', '_');
    replace(sym.begin(), sym.end(), '/', '_');
    replace(sym.begin(), sym.end(), '\\', '_');

    //create_directories(dst.parent_path());

    ofstream ofs(dst);

    ifstream ifs(src);

    ofs << "#include <stdlib.h>" << endl;
    ofs << "char _resource_" << sym << "[] = {" << endl;

    size_t lineCount = 0;
    char c;
    while (ifs.get(c))
    {
        ofs << "0x" << hex << (c&0xff) << ", ";
        if (++lineCount == 10) {
            ofs << endl;
            lineCount = 0;
        }
    }
    ofs << "0x00 ";


    ofs << "};" << endl;
    ofs << "size_t _resource_" << sym << "_len = sizeof(_resource_" << sym << ");";

    ifs.close();
    ofs.close();

    return EXIT_SUCCESS;
}
