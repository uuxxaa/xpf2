#include <array>
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <stdint.h>
#include <fstream>
#include <filesystem>
#include <sstream>

typedef uint8_t byte_t;

static void load_file(std::string_view filename, std::function<void(const byte_t* p, size_t size)>&& fn) {
    std::ifstream stream(filename.data(), std::ios::in | std::ios::binary);
    stream.exceptions(std::ios_base::badbit);

    if (!stream.is_open()) {
        printf("failed to open:\n");
        printf("%s cwd: %s \n", filename.data(), std::filesystem::current_path().string().c_str());
    } else {
        printf("embed: %s\n",filename.data());
    }

    constexpr size_t c_readBufferSize = 4096;
    std::array<byte_t, c_readBufferSize> buffer;
    while (stream.read(reinterpret_cast<char*>(buffer.data()), c_readBufferSize)) {
        fn(buffer.data(), stream.gcount());
    }

    fn(buffer.data(), stream.gcount());
}

void increment_argindex(int& i, int argc) {
    i++;
    if (i >= argc)
        throw std::runtime_error("too few args");
}

void write(std::ofstream& stream, std::string_view str) {
    stream.write(str.data(), str.length());
}

void writeln(std::ofstream& stream, std::string_view str) {
    stream.write(str.data(), str.length());
    stream.write("\n", 1);
}

void writehex(std::ofstream& ostream, byte_t b) {
    std::stringstream stream;
  stream << "0x"
         << std::setfill ('0') << std::setw(sizeof(b)*2)
         << std::hex << (int)b;
    write(ostream, stream.str());
}

int main(int argc, char* argv[]) {

    try
    {
        std::string nspace;
        std::string output_header;
        std::string output_cpp;
        std::string fname;
        std::string outfolder;

        std::ofstream hstream;
        std::ofstream cppstream;

        for (int i = 1; i < argc; i++) {
            std::string_view arg = argv[i];
            if (arg == "-ns") {
                increment_argindex(i, argc);
                nspace = std::string(argv[i]);
            } else if (arg == "-of") {
                increment_argindex(i, argc);
                outfolder = std::string(argv[i]);
            } else if (arg == "-o") {
                increment_argindex(i, argc);
                output_header = std::string(argv[i]) + ".h";
                output_cpp = std::string(argv[i]) + ".cpp";
                hstream = std::ofstream(outfolder + "/" + output_header, std::ios::out | std::ios::binary);
                writeln(hstream, "#pragma once");
                writeln(hstream, "#include <string>");
                writeln(hstream, "#include <vector>");
                writeln(hstream, "#include <stdint.h>");
                writeln(hstream, "typedef uint8_t byte_t;");
                writeln(hstream, "");
                write(hstream, "namespace "); write(hstream, nspace); writeln(hstream, " {");

                cppstream = std::ofstream(outfolder + "/" + output_cpp.data(), std::ios::out | std::ios::binary);
                write(cppstream, "#include \"");
                write(cppstream, output_header);
                writeln(cppstream, "\"");
                writeln(cppstream, "");
                write(cppstream, "namespace "); write(cppstream, nspace); writeln(cppstream, " {");

            } else if (arg == "-t" || arg == "-b") {
                bool istext = arg == "-t";
                increment_argindex(i, argc);
                fname = argv[i];
                increment_argindex(i, argc);

                if (istext) {
                    write(hstream, "std::string_view ");
                    write(hstream, fname);
                    writeln(hstream, "();");

                    write(cppstream, "std::string_view ");
                    write(cppstream, fname);
                    writeln(cppstream, "() {");
                    writeln(cppstream, "  return R\"(");

                } else {
                    write(hstream, "std::vector<byte_t> ");
                    write(hstream, fname);
                    writeln(hstream, "();");

                    write(cppstream, "std::vector<byte_t> ");
                    write(cppstream, fname);
                    writeln(cppstream, "() {");
                    writeln(cppstream, "  return {");
                }

                bool needsComma = false;
                int bcount = 0;
                for (; i < argc; i++) {
                    arg = argv[i];
                    if (arg == "-t" || arg == "-b") {
                        i--;
                        break;
                    }

                    std::string filename = argv[i];

                    if (istext) {
                        load_file(filename, [&](const byte_t* p, size_t size) {
                            cppstream.write((char*)p, size);
                        });
                    } else {
                        load_file(filename, [&](const byte_t* p, size_t size) {
                            for (size_t i = 0; i < size; i++) {
                                if (needsComma)
                                    write(cppstream, ",");

                                writehex(cppstream, *(p + i));
                                needsComma = true;
                                bcount++;
                                if (bcount > 256) {
                                    if (needsComma)
                                        write(cppstream, ",");
                                    writeln(cppstream, "");
                                    bcount = 0;
                                    needsComma = false;
                                }
                           }
                        });
                    }
                }

                if (istext) {
                    writeln(cppstream, ")\";");
                    writeln(cppstream, "}");
                    writeln(cppstream, "");
                } else {
                    writeln(cppstream, "");
                    writeln(cppstream, "  };");
                    writeln(cppstream, "}");
                    writeln(cppstream, "");
                }
            }
        }

        write(hstream, "} // "); writeln(hstream, nspace);
        write(cppstream, "} // "); writeln(cppstream, nspace);
        hstream.close();
        cppstream.close();
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    std::cout << "Done..." << std::endl;
    return 0;
}
