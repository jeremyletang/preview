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
// c terminal utility
#include <sys/ioctl.h>
#include <stdio.h>
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
    int r,g,b;
    bool fg;
    color() = delete;
    color(float r, float g, float b, bool fg = false)
    : r(r), g(g), b(b), fg(fg) {}
};

struct clear_t {};
static clear_t clear;

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const color& c) {
    float _c = 16 + 36 * c.r + 6. * c.g + c.b;
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
    std::cout << color{5, 0, 0, true} << "error" << clear << ": "
        << "no such file " << path << ", or unsupported image file format." << std::endl;
    return std::make_pair(false, cimg_library::CImg<float>());
}

}}

int main () {
    auto img = preview::term::load_img("banana.jpg");
    if (not img.first) { return EXIT_FAILURE; }
    std::cout << preview::term::color{0,0,5} << "    " << preview::term::clear << std::endl;
}