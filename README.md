# qoiconverter

.qoi to .ppm Converter for Linux

Converts multiple files in parallel using std::async

# How to build:

    install gcc
        On Fedora: sudo dnf install gcc

    install cmake
        On Fedora: sudo dnf install cmake

    chmod +x compile.sh

    ./compile

# How to run:

    To convert all <file>.qoi images in source directory to <file>.ppm in source directory:
        ./bin/qoiconv <source_directory_path>
        

    To convert <file>.qoi image to <file>.ppm within its own directory:
        ./bin/qoiconv <source_file_path>

