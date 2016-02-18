// The MIT License (MIT)
// 
// Copyright (c) 2016 Jeremy Letang
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <utility>
#include <vector>
#include <string>
// c terminal utility
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
// CLimg library
#include <CImg.h>

namespace preview {
namespace term {

struct size {
    unsigned int column;
    unsigned int raw;
    size(unsigned int column, unsigned int raw)
    : column(column), raw(raw) {}
};

size dimension() {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return {w.ws_row, w.ws_col};
}

struct color {
    float r,g,b;
    bool fg;
    color() = delete;
    color(uint8_t r, uint8_t g, uint8_t b, bool fg = false)
    : r(static_cast<float>(r) / 255. * 5.) 
    , g(static_cast<float>(g) / 255. * 5.) 
    , b(static_cast<float>(b) / 255. * 5.) 
    , fg(fg) {}
    static color from_float(float r_, float g_, float b_, bool fg_ = false) {
        auto this_ = color{0, 0, 0};
        this_.r = r_;
        this_.g = g_;
        this_.b = b_;
        this_.fg = fg_;
        return this_;
    }
};

struct clear_t {};
static clear_t clear;

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const color& c) {
    unsigned int _c = 16. + 36. * c.r + 6. * c.g + c.b;
    if (not c.fg) {os << "\x1b[48;5;" << _c << "m";}
    else {os << "\x1b[38;5;" << _c << "m";}
    return os;
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const clear_t&) {
    os << "\x1b[39;49;00m";
    return os;
}

std::pair<bool, cimg_library::CImg<float>> load_img(const std::string& path) try {
    auto i = cimg_library::CImg<float>(path.c_str());
    return std::make_pair(true, i);
} catch (...) {
    std::cout << color{255, 0, 0, true} << "error" << clear << ": "
        << "no such file " << path << ", or unsupported image file format." << std::endl;
    return std::make_pair(false, cimg_library::CImg<float>());
}

struct arguments {
    std::vector<std::string> filenames;
    bool h;
    arguments() = default;
    arguments(std::vector<std::string> filenames, bool h = false)
    : filenames(filenames), h(h) {}
    ~arguments() = default;
};

std::pair<int, arguments> parse_cmdline(int ac, char* av[]) {
    auto args = arguments{};
    int index;
    int c;
    opterr = 0;

    while ((c = getopt (ac, av, "h")) != -1) {
        switch (c) {
        case 'h': args.h = true; break;
        case '?':
            if (isprint(optopt)) {
                std::cout << color{255, 0, 0, true} << "error" << clear << ": "
                    << "unknown option -" << static_cast<char>(optopt) << std::endl;
            }
            return std::make_pair(1, args);
        default: return std::make_pair(0, args);
        }
    }

    for (index = optind; index < ac; index++) {
        args.filenames.push_back(av[index]);
    }

    return std::make_pair(0, args);
}

void print_help(const std::string& program_name) {
    std::cout << "usage: " << program_name << " [imgs ...] -h" << std::endl;
}

}}

int main (int ac, char* av[]) {
    // get commandline arguments
    auto args = preview::term::parse_cmdline(ac, av);
    // an error occured, exit
    if (args.first not_eq 0) { return EXIT_FAILURE; }
    // if h == true display help
    if (args.first == 0 and args.second.h == true) {
        preview::term::print_help(av[0]);
        return 0;
    }
    // if no files specified
    if (args.second.filenames.empty()) {
        std::cout << preview::term::color{255, 0, 0, true} << "error" << preview::term::clear << ": "
            << "preview needs at least one input file" << std::endl;
        return EXIT_FAILURE;
    }
    auto img = preview::term::load_img(args.second.filenames[0]);
    if (not img.first) { return EXIT_FAILURE; }
    std::cout << preview::term::color{0, 0, 255} << "  " << preview::term::clear << std::endl;
}