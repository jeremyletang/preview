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
#include <sstream>
// c terminal utility
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
// CLimg library
#include <CImg.h>
// cmake generated version
#include <version.h>

namespace preview {
namespace term {

struct size {
    unsigned int column;
    unsigned int row;
    size(unsigned int column, unsigned int row)
    : column(column), row(row) {}
};

size get_term_size() {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return {w.ws_col, w.ws_row};
}

struct color {
    uint8_t r,g,b;
    bool fg;
    color() = delete;
    color(uint8_t r, uint8_t g, uint8_t b, bool fg = false)
    : r(static_cast<uint8_t>(r) / 255. * 5.) 
    , g(static_cast<uint8_t>(g) / 255. * 5.) 
    , b(static_cast<uint8_t>(b) / 255. * 5.) 
    , fg(fg) {}
};

struct clear_t {};
static clear_t clear;

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const color& c) {
    unsigned int _c = 16 + 36 * c.r + 6 * c.g + c.b;
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
    bool v;
    arguments() = default;
    arguments(std::vector<std::string> filenames, bool h = false, bool v = false)
    : filenames(filenames), h(h), v(v) {}
    ~arguments() = default;
};

std::pair<int, arguments> parse_cmdline(int ac, char* av[]) {
    auto args = arguments{};
    int index;
    int c;
    opterr = 0;

    while ((c = getopt (ac, av, "hv")) != -1) {
        switch (c) {
        case 'h': args.h = true; break;
        case 'v': args.v = true; break;
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

void print_version() {
    std::cout << "preview version " << PREVIEW_VERSION << std::endl;
}

void print_image(cimg_library::CImg<float>& img) {
    // get rgb inside a pixel
    const auto red_value = 0;
    const auto green_value = 1;
    const auto blue_value = 2;
    // output
    auto ss = std::stringstream{};
    auto term_size = get_term_size();
    // resize the image
    img.resize(term_size.column/2, -((term_size.column/2.)/img.width()*100));
    auto x = 0;
    auto y = 0;
    auto buffer = std::string(((term_size.column - img.width()) / 2), ' ');

    while (y < img.height()) {
        x = 0;
        // add the buffer to center the image
        ss << buffer;
        while (x < img.width()) {
            auto red_fg = static_cast<uint8_t>(img(x, y, 0, red_value));
            auto green_fg = static_cast<uint8_t>(img(x, y, 0, green_value));
            auto blue_fg = static_cast<uint8_t>(img(x, y, blue_value));
            ss << color{red_fg, green_fg, blue_fg, true};
            // if we always are on the image
            if (y+1 < img.height()) {
                auto red_bg = static_cast<uint8_t>(img(x, y+1, 0, red_value));
                auto green_bg = static_cast<uint8_t>(img(x, y+1, 0, green_value));
                auto blue_bg = static_cast<uint8_t>(img(x, y+1, blue_value));
                ss << color{red_bg, green_bg, blue_bg} << "\u2580";
            } else { // use the same color thant y for y+1
                ss << " ";
            }
            ss << clear;
            x+=1;
        }
        // if there is more row to print, add \n
        if (y+2 < img.height()) {ss << "\n";}
        y+=2;
    }
    std::cout << ss.str() << std::endl;
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
    if (args.first == 0 and args.second.v == true) {
        preview::term::print_version();
        return 0;
    }
    // if no files specified
    if (args.second.filenames.empty()) {
        std::cout << preview::term::color{255, 0, 0, true} << "error" << preview::term::clear << ": "
            << "preview needs at least one input file" << std::endl;
        return EXIT_FAILURE;
    }
    auto imgs = std::vector<cimg_library::CImg<float>>{};
    // images
    for (const auto& file: args.second.filenames) {
        auto img = preview::term::load_img(file);
        if (img.first) {imgs.push_back(img.second);}
    }
    // print all images
    for (auto& i : imgs) {
        preview::term::print_image(i);
    }
}