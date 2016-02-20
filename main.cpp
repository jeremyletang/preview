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

std::string make_buffer(const term::size& s,
                        unsigned int img_width,
                        const std::string& position) {
    if (position == "left") {
        return std::string{""};
    } else if (position == "center") {
        return std::string(((s.column - img_width) / 2), ' ');
    }
    // last possible case is right
    return std::string((s.column - img_width), ' ');
}

color make_color_from_pixel(const cimg_library::CImg<float>& img,
                            unsigned int x,
                            unsigned int y,
                            bool foreground = false) {
    // get rgb inside a pixel
    const auto red_value = 0;
    const auto green_value = 1;
    const auto blue_value = 2;

    // get pixel colors
    auto red = static_cast<uint8_t>(img(x, y, 0, red_value));
    auto green = static_cast<uint8_t>(img(x, y, 0, green_value));
    auto blue = static_cast<uint8_t>(img(x, y, blue_value));

    // make a new color
    return color{red, green, blue, foreground};
}

void print_image(cimg_library::CImg<float>& img,
                 unsigned int scale = 50,
                 const std::string& position = "left") {
    // output
    float fscale = scale/100.;
    auto ss = std::stringstream{};
    auto term_size = get_term_size();
    // resize the image
    img.resize(term_size.column*fscale, -((term_size.column*fscale)/img.width()*100));
    // buffef left|center|right
    auto buffer = make_buffer(term_size, img.width(), position);
    auto x = 0;
    auto y = 0;

    while (y < img.height()) {
        x = 0;
        // add the buffer to center the image
        ss << buffer;
        while (x < img.width()) {
            ss << make_color_from_pixel(img, x, y, true);
            // if we always are on the image
            if (y+1 < img.height()) {
                ss << make_color_from_pixel(img, x, y+1, false) << "\u2580";
            } else { // use the same color than y for y+1
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

} // term

namespace cmdline {

struct arguments {
    std::vector<std::string> filenames;
    bool h;
    bool v;
    bool s;
    bool p;
    std::string s_value;
    std::string p_value;
    arguments()
    : filenames({}), h(false), v(false), s(false)
    , p(false), s_value("50"), p_value("left") {}
    ~arguments() = default;
};

std::pair<int, arguments> parse(int ac, char* av[]) {
    auto args = arguments{};
    int index;
    int c;
    opterr = 0;

    while ((c = getopt (ac, av, "hvs:p:")) != -1) {
        switch (c) {
        case 'h': args.h = true; break;
        case 'v': args.v = true; break;
        case 's':
            args.s = true;
            args.s_value = optarg;
            break;
        case 'p':
            args.p = true;
            args.p_value = optarg;
            break;
        case '?':
            if (optopt == 's' or optopt == 'p') {
                std::cout << term::color{255, 0, 0, true} << "error" << term::clear << ": "
                    << "option -" << static_cast<char>(optopt) << " require a value" << std::endl;
            } else if (isprint(optopt)) {
                std::cout << term::color{255, 0, 0, true} << "error" << term::clear << ": "
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
    std::cout << "usage: " << program_name << " [imgs ...] -hvs" << std::endl;
    std::cout << " -h           display this help" << std::endl;
    std::cout << " -v           show preview version" << std::endl;
    std::cout << " -s [0..100]  width of the image as a percentage of the term width," << std::endl;
    std::cout << "              default to 50." << std::endl;
    std::cout << " -p pos       image position (left|center|right)" << std::endl;
}

void print_version() {
    std::cout << "preview version " << PREVIEW_VERSION << std::endl;
}

} // cmdline
} // preview

int main (int ac, char* av[]) {
    // get commandline arguments
    auto args = preview::cmdline::parse(ac, av);
    auto scale = 50;

    // validate arguments from commandline

    // an error occured, exit
    if (args.first not_eq 0) { return EXIT_FAILURE; }
    // if h == true display help
    if (args.first == 0 and args.second.h == true) {
        preview::cmdline::print_help(av[0]);
        return 0;
    }
    // if v == true display version
    if (args.first == 0 and args.second.v == true) {
        preview::cmdline::print_version();
        return 0;
    }
    // if s == true use custom scale
    if (args.first == 0 and args.second.s == true) try {
        scale = std::stoi(args.second.s_value);
        if (scale <= 0 or scale > 100) {
            std::cout << preview::term::color{255, 0, 0, true}
                << "error" << preview::term::clear <<
                ": scale should be between 0..100." << "" << std::endl;
            return EXIT_FAILURE;
        }
    } catch (...) {
        std::cout << preview::term::color{255, 0, 0, true}
            << "error" << preview::term::clear
            << ": scale should be between 0..100." << "" << std::endl;
            return EXIT_FAILURE;
    }
    // if p == true custom position
    if (args.first == 0 and args.second.p == true) {
        if (args.second.p_value not_eq "left" and args.second.p_value not_eq "center" and
            args.second.p_value not_eq "right") {
            std::cout << "error" << preview::term::clear
                << ": position should be left|center|right" << "" << std::endl;
            return EXIT_FAILURE;
        }
    }

    // if no files specified
    if (args.second.filenames.empty()) {
        std::cout << preview::term::color{255, 0, 0, true} << "error" << preview::term::clear << ": "
            << "preview needs at least one input file" << std::endl;
        return EXIT_FAILURE;
    }

    // load images

    auto imgs = std::vector<cimg_library::CImg<float>>{};
    // images
    for (const auto& file: args.second.filenames) {
        auto img = preview::term::load_img(file);
        if (img.first) {imgs.push_back(img.second);}
    }

    // print images
    for (auto& i : imgs) {
        preview::term::print_image(i, scale, args.second.p_value);
    }

    return 0;
}