/*
    # ================================================================================================= #
    #       ___           ___                       ___           ___           ___           ___       #
    #      /\  \         /\  \          ___        /\__\         /\  \         /\  \         /\__\      #
    #     /::\  \       /::\  \        /\  \      /::|  |       /::\  \       /::\  \       /:/ _/_     #
    #    /:/\:\  \     /:/\:\  \       \:\  \    /:|:|  |      /:/\:\  \     /:/\:\  \     /:/ /\__\    #
    #   /::\~\:\  \   /::\~\:\  \      /::\__\  /:/|:|  |__   /::\~\:\__\   /:/  \:\  \   /:/ /:/ _/_   #
    #  /:/\:\ \:\__\ /:/\:\ \:\__\  __/:/\/__/ /:/ |:| /\__\ /:/\:\ \:|__| /:/__/ \:\__\ /:/_/:/ /\__\  #
    #  \/_|::\/:/  / \/__\:\/:/  / /\/:/  /    \/__|:|/:/  / \:\~\:\/:/  / \:\  \ /:/  / \:\/:/ /:/  /  #
    #     |:|::/  /       \::/  /  \::/__/         |:/:/  /   \:\ \::/  /   \:\  /:/  /   \::/_/:/  /   #
    #     |:|\/__/        /:/  /    \:\__\         |::/  /     \:\/:/  /     \:\/:/  /     \:\/:/  /    #
    #     |:|  |         /:/  /      \/__/         /:/  /       \::/__/       \::/  /       \::/  /     #
    #      \|__|         \/__/                     \/__/         ~~            \/__/         \/__/      #
    #                                 Created by: Wesley Jones                                          #
    # ================================================================================================= #
*/

#include <map>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdio.h>

// If you want, for some reason, to extract the binary data from the colored output of this program use:
// Where rainbow-input.txt is the output of this program
// and output.bin is the binary file you want to extract
// $ touch output.bin; truncate -s0 output.bin; for LINE in $(cat rainbow-input.txt | ansi2txt | cut -d ' ' -f 1); do echo -n $LINE | head -c 128 | hex -d >> output.bin; done

// If you want to display in 'less' pager you can use the '-R' flag to display the colors, else it will be illegible.

#ifdef _WIN32

// Is not working with windows 16 color console
static_assert(false, "Windows terminal is not supported yet");
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h> // for read()
#endif

#define VERSION_ID "0.2.0"

#define CHUNK_SIZE 4096

std::vector<uint8_t> colors_list;
typedef uint8_t byte;
typedef std::vector<byte> byte_vector;
std::map<byte_vector, uint32_t> patterns_dict;
uint16_t bytes_per_row = 64;
uint64_t color_pallet = 0;

// I dont want to reinvent my wheel again
#define ROTRIGHT(a, b) (((a) >> (b)) | ((a) << (64 - (b))))

// These functions/algorithms are mine and subject to their own license. They may be used in this programs binary "AS IS" freely though. See applicable license('s) at https://github.com/Kracken256/NinoHash.
// They can be swapped with any other function that does the same thing. Which is mixing bytes to produce a color.
// I used mine instead of something like crc32 because crc32 is 64 bits and I wanted to use all 64 bits of the color.
// I also did not want other 3rd party code in my code. I wanted to keep it as simple as possible.
// Anything cryptographic is overkill and wasteful for this purpose.

// BEGIN OTHER COPYRIGHTED CODE
#define permute_box1(a, b, c, d)                  \
    *d ^= (((*a) << 32) & ~((*d) >> 32)) ^ ~(*a); \
    *c ^= (((*b) << 48) & ~((*c) >> 16)) ^ ~(*b); \
    *b ^= (((*c) << 32) & ~((*b) >> 32)) ^ ~(*c); \
    *a ^= (((*d) << 16) & ~((*a) >> 48)) ^ ~(*d);

#define permute_box2(a, b, c, d)                                      \
    *a ^= ROTRIGHT(((*a ^ *b) & ~*c), 20) << (ROTRIGHT(*d, 12) % 14); \
    *b ^= ROTRIGHT(((*b ^ *c) & ~*d), 26) << (ROTRIGHT(*a, 17) % 14); \
    *c ^= ROTRIGHT(((*c ^ *d) & ~*a), 15) << (ROTRIGHT(*b, 29) % 14); \
    *d ^= ROTRIGHT(((*d ^ *a) & ~*b), 37) << (ROTRIGHT(*c, 47) % 14);

#define permute_box3(a, b, c, d)                               \
    *a ^= ROTRIGHT(((*a & *b) ^ ~*c), (*d % 64)) >> (*a % 16); \
    *b ^= ROTRIGHT(((*b & *c) ^ ~*d), (*c % 63)) >> (*b % 17); \
    *c ^= ROTRIGHT(((*c & *d) ^ ~*a), (*b % 62)) >> (*c % 18); \
    *d ^= ROTRIGHT(((*d & *a) ^ ~*b), (*a % 61)) >> (*d % 19);

// END OTHER COPYRIGHTED CODE

uint8_t get_color(byte_vector pattern)
{

    // compute color based on pattern bytes. Checksum based for non-seq colors
    uint64_t a = color_pallet;
    uint64_t b = 0;
    uint64_t c = 0;
    uint64_t d = 0;
    uint64_t color = 0;
    // I just cant help myself
    for (size_t i = 0; i < pattern.size(); i++)
    {
        // Slower but need the diffusion. For the colors!!!
        // Else its not a rainbow!
        // 2 rounds of permutation for maximum diffusion and uniformity.
        // If this won't make a rainbow, I dont know what will.
        a ^= pattern[i];
        permute_box1(&a, &b, &c, &d);
        b ^= pattern[i];
        permute_box2(&a, &b, &c, &d);
        c ^= pattern[i];
        permute_box3(&a, &b, &c, &d);
        d ^= pattern[i];
        permute_box1(&a, &b, &c, &d);
        permute_box2(&a, &b, &c, &d);
        permute_box3(&a, &b, &c, &d);
    }
    color = a ^ b ^ c ^ d;

    color = colors_list[color % colors_list.size()];
    // I am not racist, but I dont like black color on black terminal.
    if (color == 0)
    {
        color = 1;
    }

    return color;
}

byte_vector translate_ascii_dump(byte_vector input)
{
    byte_vector output;
    for (size_t i = 0; i < input.size(); i++)
    {
        if (input[i] >= 0x20 && input[i] <= 0x7E)
        {
            output.push_back(input[i]);
        }
        else
        {
            output.push_back('.');
        }
    }
    return output;
}

int do_file(std::string filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cout << "Failed to open file: " << filepath << std::endl;
        return 1;
    }
    patterns_dict.clear();
    byte_vector current_chunk;
    current_chunk.resize(CHUNK_SIZE);

    size_t bytes_read = file.readsome((char *)current_chunk.data(), CHUNK_SIZE);

    while (bytes_read > 0)
    {

        for (size_t i = 0; i < bytes_read; i++)
        {
            for (size_t j = 2; j < 36; j++)
            {
                if (i + j <= bytes_read)
                {
                    byte_vector pattern_bytes(current_chunk.begin() + i, current_chunk.begin() + i + j);
                    if (patterns_dict.find(pattern_bytes) != patterns_dict.end())
                    {
                        patterns_dict[pattern_bytes] += 1;
                    }
                    else
                    {
                        patterns_dict[pattern_bytes] = 1;
                    }
                }
            }
        }

        size_t i = 0;
        std::string bytes_buffer;
        size_t current_line_len = 0;
        while (i < bytes_read)
        {
            bool found_pattern = false;

            for (size_t j = 35; j > 1; j--)
            {
                if (i + j <= bytes_read)
                {
                    byte_vector pattern_bytes(current_chunk.begin() + i, current_chunk.begin() + i + j);
                    if (patterns_dict.find(pattern_bytes) != patterns_dict.end() && patterns_dict[pattern_bytes] > 1)
                    {
                        uint8_t color = get_color(pattern_bytes);
                        size_t k = 0;
                        while (k < pattern_bytes.size())
                        {
                            size_t j = std::min(pattern_bytes.size() - k, bytes_per_row - current_line_len);
                            std::stringstream ss;
                            ss << "\033[38;5;" << (int)color << "m";
                            for (size_t l = k; l < (k + j); l++)
                            {
                                ss << std::hex << std::setfill('0') << std::setw(2) << (int)pattern_bytes[l];
                            }
                            ss << "\033[0m";

                            bytes_buffer += ss.str();
                            current_line_len += j;
                            if (current_line_len == bytes_per_row)
                            {
                                // Print ascii
                                bytes_buffer += " |";
                                byte_vector ascii_dump = translate_ascii_dump(byte_vector(current_chunk.begin() + i + j - current_line_len, current_chunk.begin() + i + j));
                                bytes_buffer += std::string(ascii_dump.begin(), ascii_dump.end());
                                bytes_buffer += "|\n";
                                current_line_len = 0;
                            }
                            k += j;
                        }
                        i += j;
                        found_pattern = true;
                        break;
                    }
                }
            }
            if (!found_pattern)
            {
                std::stringstream ss;
                ss << "\033[38;5;245m";
                ss << std::hex << std::setfill('0') << std::setw(2) << (int)current_chunk[i];
                ss << "\033[0m";
                bytes_buffer += ss.str();
                current_line_len += 1;
                if (current_line_len == bytes_per_row)
                {
                    // Print ascii
                    bytes_buffer += " |";
                    byte_vector ascii_dump = translate_ascii_dump(byte_vector(current_chunk.begin() + i + 1 - current_line_len, current_chunk.begin() + i + 1));
                    bytes_buffer += std::string(ascii_dump.begin(), ascii_dump.end());
                    bytes_buffer += "|\n";
                    current_line_len = 0;
                }
                i++;
            }
        }
        // Print ascii last line
        if (current_line_len > 0)
        {
            bytes_buffer += std::string((bytes_per_row - current_line_len) * 2, ' ');
            bytes_buffer += " |";
            byte_vector ascii_dump = translate_ascii_dump(byte_vector(current_chunk.begin() + i - current_line_len, current_chunk.begin() + i));
            bytes_buffer += std::string(ascii_dump.begin(), ascii_dump.end());
            bytes_buffer += "|\n";
        }
        std::cout << bytes_buffer;
        bytes_read = file.readsome((char *)current_chunk.data(), CHUNK_SIZE);
    }
    file.close();
    return 0;
}

int do_stdin()
{
    // should not have duplicate code here
    int file = fileno(stdin);

    byte_vector current_chunk;
    patterns_dict.clear();
    current_chunk.resize(CHUNK_SIZE);

    size_t bytes_read = read(file, current_chunk.data(), CHUNK_SIZE);

    while (bytes_read > 0)
    {

        for (size_t i = 0; i < bytes_read; i++)
        {
            for (size_t j = 2; j < 36; j++)
            {
                if (i + j <= bytes_read)
                {
                    byte_vector pattern_bytes(current_chunk.begin() + i, current_chunk.begin() + i + j);
                    if (patterns_dict.find(pattern_bytes) != patterns_dict.end())
                    {
                        patterns_dict[pattern_bytes] += 1;
                    }
                    else
                    {
                        patterns_dict[pattern_bytes] = 1;
                    }
                }
            }
        }

        size_t i = 0;
        std::string bytes_buffer;
        size_t current_line_len = 0;
        while (i < bytes_read)
        {
            bool found_pattern = false;

            for (size_t j = 35; j > 1; j--)
            {
                if (i + j <= bytes_read)
                {
                    byte_vector pattern_bytes(current_chunk.begin() + i, current_chunk.begin() + i + j);
                    if (patterns_dict.find(pattern_bytes) != patterns_dict.end() && patterns_dict[pattern_bytes] > 1)
                    {
                        uint8_t color = get_color(pattern_bytes);
                        size_t k = 0;
                        while (k < pattern_bytes.size())
                        {
                            size_t j = std::min(pattern_bytes.size() - k, bytes_per_row - current_line_len);
                            std::stringstream ss;
                            ss << "\033[38;5;" << (int)color << "m";
                            for (size_t l = k; l < (k + j); l++)
                            {
                                ss << std::hex << std::setfill('0') << std::setw(2) << (int)pattern_bytes[l];
                            }
                            ss << "\033[0m";

                            bytes_buffer += ss.str();
                            current_line_len += j;
                            if (current_line_len == bytes_per_row)
                            {
                                // Print ascii
                                bytes_buffer += " |";
                                byte_vector ascii_dump = translate_ascii_dump(byte_vector(current_chunk.begin() + i + j - current_line_len, current_chunk.begin() + i + j));
                                bytes_buffer += std::string(ascii_dump.begin(), ascii_dump.end());
                                bytes_buffer += "|\n";
                                current_line_len = 0;
                            }
                            k += j;
                        }
                        i += j;
                        found_pattern = true;
                        break;
                    }
                }
            }
            if (!found_pattern)
            {
                std::stringstream ss;
                ss << "\033[38;5;245m";
                ss << std::hex << std::setfill('0') << std::setw(2) << (int)current_chunk[i];
                ss << "\033[0m";
                bytes_buffer += ss.str();
                current_line_len += 1;
                if (current_line_len == bytes_per_row)
                {
                    // Print ascii
                    bytes_buffer += " |";
                    byte_vector ascii_dump = translate_ascii_dump(byte_vector(current_chunk.begin() + i - current_line_len, current_chunk.begin() + i));
                    bytes_buffer += std::string(ascii_dump.begin(), ascii_dump.end());
                    bytes_buffer += "|\n";
                    current_line_len = 0;
                }
                i++;
            }
        }
        // Print ascii last line
        // TODO: Fix this formula. It is wrong if length if data is < bytes_per_row
        if (current_line_len > 0)
        {
            bytes_buffer += std::string((bytes_per_row - current_line_len) * 2, ' ');
            bytes_buffer += " |";
            byte_vector ascii_dump = translate_ascii_dump(byte_vector(current_chunk.begin() + i - current_line_len, current_chunk.begin() + i));
            bytes_buffer += std::string(ascii_dump.begin(), ascii_dump.end());
            bytes_buffer += "|\n";
        }
        std::cout << bytes_buffer;
        bytes_read = read(file, current_chunk.data(), CHUNK_SIZE);
    }
    close(file);
    return 0;
}

void init_colors(void)
{
    for (int i = 0; i < 256; i++)
    {
        colors_list.push_back(i);
    }
}

void println(std::string s)
{
    std::cout << s << std::endl;
}

void print_help(void)
{
    println("Usage: rainbow [OPTIONS] [FILE]");
    println("Prints a hexdump of the given file with colored patterns.");
    println("If no file is given, stdin is used.");
    println("");
    println("Options:");
    println("  -m <color iv>  Set a initialization vector/nonce for the color pattet. <color> a 64 bit integer.");
    println("  -c <cols>      Set the number of bytes per row. Default is 32.");
    println("  -h (many more) Print this help message.");
    println("  -v (--version) Print version information.");
    println("  '-'           Read from stdin.");
    println("");
    println("Colors pattern codes:");
    println("  I made this as simple as possible. Just pick a number (default of 0) and that will");
    println("  make your color palette. There are 256 colors on an xterm-256color terminal. The");
    println("  colors are numbered from 0 to 255. The 64-bit number is permuted with a given pattern");
    println("  to produce 64 bits of entropy dissimilar to the original number. The entropy is then");
    println("  reduced modulo 256 to get a number between 0 and 255.  This means that assuming the");
    println("  permutation is distributed uniformly, (which it is not), the colors will be");
    println("  distributed uniformly for any given number. And by extension, there are 2^64 ");
    println("  possible color palettes to choose from. There are only 256 colors, but because the");
    println("  color depends on the pattern, this is useful. The downside is that you can't know");
    println("  what the colors will be without trying them out. But that's the fun part!");
    println("");
    println("VERSION:");
    std::cout << "  rainbow-xd version " << VERSION_ID << std::endl;
    println("");
    println("Author:");
    println("  Written by Wesley Charles Jones.");
    println("");
    println("COPYRIGHT:");
    println("  Copyright © 2023 Wesley Charles Jones.  License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.");
    println("  This is free software: you are free to change and redistribute it.  There is NO WARRANTY, to the extent permitted by law.");
    println("");
    println("SEE ALSO:");
    println("   https://github.com/Kracken256");
    println("");
}

int main(int argc, char **argv)
{
    std::vector<std::string> arguments(argv + 1, argv + argc);

    std::string filepath;
    bool mode_stdin = false;
    bool should_exit = false;
    bool should_print_help = false;
    bool should_print_version = false;
    // find filepath
    size_t i = 0;
    while (i < arguments.size())
    {
        std::string s = arguments[i];
        if (s == "-")
        {
            mode_stdin = true;
        }
        else if (std::filesystem::exists(s) && !mode_stdin)
        {
            filepath = s;
            mode_stdin = false;
        }
        else if (s == "-m")
        {
            // get 16 bit color code
            if (i + 1 < arguments.size())
            {
                std::string color_code = arguments[i + 1];
                try
                {
                    uint64_t color = std::stol(color_code, nullptr, 16);

                    color_pallet = color;
                    i++;
                }
                catch (std::exception &e)
                {
                    std::cout << "Invalid color code: " << color_code << std::endl;
                    should_exit = true;
                }
            }
            else
            {
                std::cout << "Missing color code" << std::endl;
                should_exit = true;
            }
        }
        else if (s == "-c")
        {
            // set cols
            if (i + 1 < arguments.size())
            {
                std::string num_cols = arguments[i + 1];
                try
                {
                    uint16_t cols = (uint16_t)std::stoull(num_cols);
                    if (cols < 1 || cols > 0xFFFF)
                    {
                        std::cout << "Invalid number of columns: " << num_cols << std::endl;
                        should_exit = true;
                    }
                    bytes_per_row = cols;
                    i++;
                }
                catch (std::exception &e)
                {
                    std::cout << "Invalid number of columns: " << num_cols << std::endl;
                    should_exit = true;
                }
            }
            else
            {
                std::cout << "Missing number of columns" << std::endl;
                should_exit = true;
            }
        }
        else if (s == "-v" || s == "--version")
        {
            should_print_version = true;
            should_exit = true;
        }
        // help. HELP ME GOD DAMN IT!!!
        else if (s == "-h" || s == "--help" || s == "-?" || s == "/?" || s == "/h" || s == "/help" || s == "/h" || s == "-help")
        {
            should_print_help = true;
            should_exit = true;
        }
        else
        {
            std::cout << "Unknown argument: " << s << std::endl;
            should_exit = true;
        }
        i++;
    }
    if (should_print_version)
    {
        println(std::string("rainbow version ") + VERSION_ID);
    }
    if (should_print_help)
    {
        print_help();
    }

    if (should_exit)
        return 0;

    // init color vector
    init_colors();
    if (std::filesystem::exists(filepath))
    {
        return do_file(filepath);
    }
    return do_stdin();
}