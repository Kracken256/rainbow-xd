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

/*
 * Introducing rainbow-xd: Unveiling the Vibrant Spectrum of Data Analysis!
 *
 * Are you weary of sifting through mundane monocolor hexdumps? Are you craving
 * a revolutionary solution that harnesses the power of pattern recognition? Look
 * no further! rainbow-xd is the ultimate tool that will unlock the true potential
 * of your data. Whether you're a cryptoanalyst searching for repeating sequences
 * or a data enthusiast seeking sequential patterns, rainbow-xd is your answer!
 *
 * With rainbow-xd, the tedium of manual color assignment becomes a thing of the
 * past. Our cutting-edge technology employs advanced algorithms, meticulously
 * crafted over several hours, to autonomously discover and deterministically
 * assign colors to patterns of any length, ranging from 2 to 32 bytes. Brace
 * yourself for a mesmerizing display of 256 distinct hues that effortlessly
 * reveal the inner workings of your binary files.
 *
 * Unlock the power of rainbow-xd and immerse yourself in a world of vivid
 * data analysis. See your information come alive as our tool seamlessly identifies
 * contiguous and sequential patterns, providing invaluable insights in the
 * blink of an eye. Embrace the excitement of discovery and the joy of effort
 * less comprehension as you navigate the colorful landscape of your data with
 * rainbow-xd.
 *
 * Experience the future of data analysis today with rainbow-xd. Revolutionize
 * your workflow, enhance your efficiency, and unlock hidden patterns with ease.
 * Say goodbye to monotonous hexdumps and say hello to the vibrant spectrum of
 * rainbow-xd!
 *
 * rainbow-xd is now available for FREE!!!
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
#include <cstring>
#include <sys/ioctl.h>

/*
 * If you need to extract the binary data from the colored output of this program,
 * you can use the following steps:
 * 1. Save the colored output as 'rainbow-input.txt'.
 * 2. Prepare the target file 'output.bin' to store the extracted binary data.
 * 3. Install the 'hex' utility from the basez package and the 'ansi2txt' utility
 *  from an additional package (package name not specified).

 * 4. Run the following command to extract the binary data:
 *    $ touch output.bin; truncate -s0 output.bin; for LINE in $(cat rainbow-input.txt | ansi2txt | cut -d ' ' -f 1); do echo -n $LINE | head -c 128 | hex -d >> output.bin; done

 * Note: The above command will process each line of the 'rainbow-input.txt' file,
 * extract the first 128 characters (64 bytes decoded), convert them from hex to binary, and append
 * the result to 'output.bin'.

 * If you want to view the colored output using the 'less' pager, you can add
 * the '-R' flag to preserve the colors. Otherwise, the output will be illegible
 * without color support.
*/

#ifdef _WIN32
// It does not work with windows 16 color console.
static_assert(false, "Windows terminal is not supported yet");
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h> // for read()
#endif

#define VERSION_ID "0.3.0"
#define CHUNK_SIZE 1024

// lowercase hex. Undefine to use uppercase hex.

std::vector<uint8_t> colors_list;
typedef uint8_t byte;
typedef std::vector<byte> byte_vector;
std::map<byte_vector, uint16_t> patterns_dict;
uint16_t bytes_per_row = 32;
uint64_t color_pallet = 0;

static const uint64_t crc64_table[256] = {
    UINT64_C(0x0000000000000000),
    UINT64_C(0x7ad870c830358979),
    UINT64_C(0xf5b0e190606b12f2),
    UINT64_C(0x8f689158505e9b8b),
    UINT64_C(0xc038e5739841b68f),
    UINT64_C(0xbae095bba8743ff6),
    UINT64_C(0x358804e3f82aa47d),
    UINT64_C(0x4f50742bc81f2d04),
    UINT64_C(0xab28ecb46814fe75),
    UINT64_C(0xd1f09c7c5821770c),
    UINT64_C(0x5e980d24087fec87),
    UINT64_C(0x24407dec384a65fe),
    UINT64_C(0x6b1009c7f05548fa),
    UINT64_C(0x11c8790fc060c183),
    UINT64_C(0x9ea0e857903e5a08),
    UINT64_C(0xe478989fa00bd371),
    UINT64_C(0x7d08ff3b88be6f81),
    UINT64_C(0x07d08ff3b88be6f8),
    UINT64_C(0x88b81eabe8d57d73),
    UINT64_C(0xf2606e63d8e0f40a),
    UINT64_C(0xbd301a4810ffd90e),
    UINT64_C(0xc7e86a8020ca5077),
    UINT64_C(0x4880fbd87094cbfc),
    UINT64_C(0x32588b1040a14285),
    UINT64_C(0xd620138fe0aa91f4),
    UINT64_C(0xacf86347d09f188d),
    UINT64_C(0x2390f21f80c18306),
    UINT64_C(0x594882d7b0f40a7f),
    UINT64_C(0x1618f6fc78eb277b),
    UINT64_C(0x6cc0863448deae02),
    UINT64_C(0xe3a8176c18803589),
    UINT64_C(0x997067a428b5bcf0),
    UINT64_C(0xfa11fe77117cdf02),
    UINT64_C(0x80c98ebf2149567b),
    UINT64_C(0x0fa11fe77117cdf0),
    UINT64_C(0x75796f2f41224489),
    UINT64_C(0x3a291b04893d698d),
    UINT64_C(0x40f16bccb908e0f4),
    UINT64_C(0xcf99fa94e9567b7f),
    UINT64_C(0xb5418a5cd963f206),
    UINT64_C(0x513912c379682177),
    UINT64_C(0x2be1620b495da80e),
    UINT64_C(0xa489f35319033385),
    UINT64_C(0xde51839b2936bafc),
    UINT64_C(0x9101f7b0e12997f8),
    UINT64_C(0xebd98778d11c1e81),
    UINT64_C(0x64b116208142850a),
    UINT64_C(0x1e6966e8b1770c73),
    UINT64_C(0x8719014c99c2b083),
    UINT64_C(0xfdc17184a9f739fa),
    UINT64_C(0x72a9e0dcf9a9a271),
    UINT64_C(0x08719014c99c2b08),
    UINT64_C(0x4721e43f0183060c),
    UINT64_C(0x3df994f731b68f75),
    UINT64_C(0xb29105af61e814fe),
    UINT64_C(0xc849756751dd9d87),
    UINT64_C(0x2c31edf8f1d64ef6),
    UINT64_C(0x56e99d30c1e3c78f),
    UINT64_C(0xd9810c6891bd5c04),
    UINT64_C(0xa3597ca0a188d57d),
    UINT64_C(0xec09088b6997f879),
    UINT64_C(0x96d1784359a27100),
    UINT64_C(0x19b9e91b09fcea8b),
    UINT64_C(0x636199d339c963f2),
    UINT64_C(0xdf7adabd7a6e2d6f),
    UINT64_C(0xa5a2aa754a5ba416),
    UINT64_C(0x2aca3b2d1a053f9d),
    UINT64_C(0x50124be52a30b6e4),
    UINT64_C(0x1f423fcee22f9be0),
    UINT64_C(0x659a4f06d21a1299),
    UINT64_C(0xeaf2de5e82448912),
    UINT64_C(0x902aae96b271006b),
    UINT64_C(0x74523609127ad31a),
    UINT64_C(0x0e8a46c1224f5a63),
    UINT64_C(0x81e2d7997211c1e8),
    UINT64_C(0xfb3aa75142244891),
    UINT64_C(0xb46ad37a8a3b6595),
    UINT64_C(0xceb2a3b2ba0eecec),
    UINT64_C(0x41da32eaea507767),
    UINT64_C(0x3b024222da65fe1e),
    UINT64_C(0xa2722586f2d042ee),
    UINT64_C(0xd8aa554ec2e5cb97),
    UINT64_C(0x57c2c41692bb501c),
    UINT64_C(0x2d1ab4dea28ed965),
    UINT64_C(0x624ac0f56a91f461),
    UINT64_C(0x1892b03d5aa47d18),
    UINT64_C(0x97fa21650afae693),
    UINT64_C(0xed2251ad3acf6fea),
    UINT64_C(0x095ac9329ac4bc9b),
    UINT64_C(0x7382b9faaaf135e2),
    UINT64_C(0xfcea28a2faafae69),
    UINT64_C(0x8632586aca9a2710),
    UINT64_C(0xc9622c4102850a14),
    UINT64_C(0xb3ba5c8932b0836d),
    UINT64_C(0x3cd2cdd162ee18e6),
    UINT64_C(0x460abd1952db919f),
    UINT64_C(0x256b24ca6b12f26d),
    UINT64_C(0x5fb354025b277b14),
    UINT64_C(0xd0dbc55a0b79e09f),
    UINT64_C(0xaa03b5923b4c69e6),
    UINT64_C(0xe553c1b9f35344e2),
    UINT64_C(0x9f8bb171c366cd9b),
    UINT64_C(0x10e3202993385610),
    UINT64_C(0x6a3b50e1a30ddf69),
    UINT64_C(0x8e43c87e03060c18),
    UINT64_C(0xf49bb8b633338561),
    UINT64_C(0x7bf329ee636d1eea),
    UINT64_C(0x012b592653589793),
    UINT64_C(0x4e7b2d0d9b47ba97),
    UINT64_C(0x34a35dc5ab7233ee),
    UINT64_C(0xbbcbcc9dfb2ca865),
    UINT64_C(0xc113bc55cb19211c),
    UINT64_C(0x5863dbf1e3ac9dec),
    UINT64_C(0x22bbab39d3991495),
    UINT64_C(0xadd33a6183c78f1e),
    UINT64_C(0xd70b4aa9b3f20667),
    UINT64_C(0x985b3e827bed2b63),
    UINT64_C(0xe2834e4a4bd8a21a),
    UINT64_C(0x6debdf121b863991),
    UINT64_C(0x1733afda2bb3b0e8),
    UINT64_C(0xf34b37458bb86399),
    UINT64_C(0x8993478dbb8deae0),
    UINT64_C(0x06fbd6d5ebd3716b),
    UINT64_C(0x7c23a61ddbe6f812),
    UINT64_C(0x3373d23613f9d516),
    UINT64_C(0x49aba2fe23cc5c6f),
    UINT64_C(0xc6c333a67392c7e4),
    UINT64_C(0xbc1b436e43a74e9d),
    UINT64_C(0x95ac9329ac4bc9b5),
    UINT64_C(0xef74e3e19c7e40cc),
    UINT64_C(0x601c72b9cc20db47),
    UINT64_C(0x1ac40271fc15523e),
    UINT64_C(0x5594765a340a7f3a),
    UINT64_C(0x2f4c0692043ff643),
    UINT64_C(0xa02497ca54616dc8),
    UINT64_C(0xdafce7026454e4b1),
    UINT64_C(0x3e847f9dc45f37c0),
    UINT64_C(0x445c0f55f46abeb9),
    UINT64_C(0xcb349e0da4342532),
    UINT64_C(0xb1eceec59401ac4b),
    UINT64_C(0xfebc9aee5c1e814f),
    UINT64_C(0x8464ea266c2b0836),
    UINT64_C(0x0b0c7b7e3c7593bd),
    UINT64_C(0x71d40bb60c401ac4),
    UINT64_C(0xe8a46c1224f5a634),
    UINT64_C(0x927c1cda14c02f4d),
    UINT64_C(0x1d148d82449eb4c6),
    UINT64_C(0x67ccfd4a74ab3dbf),
    UINT64_C(0x289c8961bcb410bb),
    UINT64_C(0x5244f9a98c8199c2),
    UINT64_C(0xdd2c68f1dcdf0249),
    UINT64_C(0xa7f41839ecea8b30),
    UINT64_C(0x438c80a64ce15841),
    UINT64_C(0x3954f06e7cd4d138),
    UINT64_C(0xb63c61362c8a4ab3),
    UINT64_C(0xcce411fe1cbfc3ca),
    UINT64_C(0x83b465d5d4a0eece),
    UINT64_C(0xf96c151de49567b7),
    UINT64_C(0x76048445b4cbfc3c),
    UINT64_C(0x0cdcf48d84fe7545),
    UINT64_C(0x6fbd6d5ebd3716b7),
    UINT64_C(0x15651d968d029fce),
    UINT64_C(0x9a0d8ccedd5c0445),
    UINT64_C(0xe0d5fc06ed698d3c),
    UINT64_C(0xaf85882d2576a038),
    UINT64_C(0xd55df8e515432941),
    UINT64_C(0x5a3569bd451db2ca),
    UINT64_C(0x20ed197575283bb3),
    UINT64_C(0xc49581ead523e8c2),
    UINT64_C(0xbe4df122e51661bb),
    UINT64_C(0x3125607ab548fa30),
    UINT64_C(0x4bfd10b2857d7349),
    UINT64_C(0x04ad64994d625e4d),
    UINT64_C(0x7e7514517d57d734),
    UINT64_C(0xf11d85092d094cbf),
    UINT64_C(0x8bc5f5c11d3cc5c6),
    UINT64_C(0x12b5926535897936),
    UINT64_C(0x686de2ad05bcf04f),
    UINT64_C(0xe70573f555e26bc4),
    UINT64_C(0x9ddd033d65d7e2bd),
    UINT64_C(0xd28d7716adc8cfb9),
    UINT64_C(0xa85507de9dfd46c0),
    UINT64_C(0x273d9686cda3dd4b),
    UINT64_C(0x5de5e64efd965432),
    UINT64_C(0xb99d7ed15d9d8743),
    UINT64_C(0xc3450e196da80e3a),
    UINT64_C(0x4c2d9f413df695b1),
    UINT64_C(0x36f5ef890dc31cc8),
    UINT64_C(0x79a59ba2c5dc31cc),
    UINT64_C(0x037deb6af5e9b8b5),
    UINT64_C(0x8c157a32a5b7233e),
    UINT64_C(0xf6cd0afa9582aa47),
    UINT64_C(0x4ad64994d625e4da),
    UINT64_C(0x300e395ce6106da3),
    UINT64_C(0xbf66a804b64ef628),
    UINT64_C(0xc5bed8cc867b7f51),
    UINT64_C(0x8aeeace74e645255),
    UINT64_C(0xf036dc2f7e51db2c),
    UINT64_C(0x7f5e4d772e0f40a7),
    UINT64_C(0x05863dbf1e3ac9de),
    UINT64_C(0xe1fea520be311aaf),
    UINT64_C(0x9b26d5e88e0493d6),
    UINT64_C(0x144e44b0de5a085d),
    UINT64_C(0x6e963478ee6f8124),
    UINT64_C(0x21c640532670ac20),
    UINT64_C(0x5b1e309b16452559),
    UINT64_C(0xd476a1c3461bbed2),
    UINT64_C(0xaeaed10b762e37ab),
    UINT64_C(0x37deb6af5e9b8b5b),
    UINT64_C(0x4d06c6676eae0222),
    UINT64_C(0xc26e573f3ef099a9),
    UINT64_C(0xb8b627f70ec510d0),
    UINT64_C(0xf7e653dcc6da3dd4),
    UINT64_C(0x8d3e2314f6efb4ad),
    UINT64_C(0x0256b24ca6b12f26),
    UINT64_C(0x788ec2849684a65f),
    UINT64_C(0x9cf65a1b368f752e),
    UINT64_C(0xe62e2ad306bafc57),
    UINT64_C(0x6946bb8b56e467dc),
    UINT64_C(0x139ecb4366d1eea5),
    UINT64_C(0x5ccebf68aecec3a1),
    UINT64_C(0x2616cfa09efb4ad8),
    UINT64_C(0xa97e5ef8cea5d153),
    UINT64_C(0xd3a62e30fe90582a),
    UINT64_C(0xb0c7b7e3c7593bd8),
    UINT64_C(0xca1fc72bf76cb2a1),
    UINT64_C(0x45775673a732292a),
    UINT64_C(0x3faf26bb9707a053),
    UINT64_C(0x70ff52905f188d57),
    UINT64_C(0x0a2722586f2d042e),
    UINT64_C(0x854fb3003f739fa5),
    UINT64_C(0xff97c3c80f4616dc),
    UINT64_C(0x1bef5b57af4dc5ad),
    UINT64_C(0x61372b9f9f784cd4),
    UINT64_C(0xee5fbac7cf26d75f),
    UINT64_C(0x9487ca0fff135e26),
    UINT64_C(0xdbd7be24370c7322),
    UINT64_C(0xa10fceec0739fa5b),
    UINT64_C(0x2e675fb4576761d0),
    UINT64_C(0x54bf2f7c6752e8a9),
    UINT64_C(0xcdcf48d84fe75459),
    UINT64_C(0xb71738107fd2dd20),
    UINT64_C(0x387fa9482f8c46ab),
    UINT64_C(0x42a7d9801fb9cfd2),
    UINT64_C(0x0df7adabd7a6e2d6),
    UINT64_C(0x772fdd63e7936baf),
    UINT64_C(0xf8474c3bb7cdf024),
    UINT64_C(0x829f3cf387f8795d),
    UINT64_C(0x66e7a46c27f3aa2c),
    UINT64_C(0x1c3fd4a417c62355),
    UINT64_C(0x935745fc4798b8de),
    UINT64_C(0xe98f353477ad31a7),
    UINT64_C(0xa6df411fbfb21ca3),
    UINT64_C(0xdc0731d78f8795da),
    UINT64_C(0x536fa08fdfd90e51),
    UINT64_C(0x29b7d047efec8728),
};

static const char *hex_chars_lookup_table_lower = "0123456789abcdef";

static const char *hex_chars_lookup_table_upper = "0123456789ABCDEF";

static char const *hex_chars_lookup_table = (char const *)hex_chars_lookup_table_lower;

inline static uint64_t crc64(const byte *data, size_t length)
{
    uint64_t crc = 0xFFFFFFFFFFFFFFFFULL;

    for (size_t i = 0; i < length; ++i)
    {
        byte index = crc ^ data[i];
        crc = crc64_table[index] ^ (crc >> 8);
    }

    return crc ^ 0xFFFFFFFFFFFFFFFFULL;
}

#define ROTRIGHT(a, b) (((a) >> (b)) | ((a) << (64 - (b))))

// My own hash/checksum algorithm fn1
#define permute_box1(a, b, c, d)                                          \
    *a ^= ROTRIGHT(((*a ^ *b) & ~*c) + 1, 20) << (ROTRIGHT(*d, 12) % 14); \
    *b ^= ROTRIGHT(((*b ^ *c) & ~*d) + 1, 26) << (ROTRIGHT(*a, 17) % 14); \
    *c ^= ROTRIGHT(((*c ^ *d) & ~*a) + 1, 15) << (ROTRIGHT(*b, 29) % 14); \
    *d ^= ROTRIGHT(((*d ^ *a) & ~*b) + 1, 37) << (ROTRIGHT(*c, 47) % 14);

// My own hash/checksum algorithm fn2
#define permute_box2(a, b, c, d)                               \
    *a ^= ROTRIGHT(((*a & *b) ^ ~*c), (*d % 64)) >> (*a % 16); \
    *b ^= ROTRIGHT(((*b & *c) ^ ~*d), (*c % 63)) >> (*b % 17); \
    *c ^= ROTRIGHT(((*c & *d) ^ ~*a), (*b % 62)) >> (*c % 18); \
    *d ^= ROTRIGHT(((*d & *a) ^ ~*b), (*a % 61)) >> (*d % 19);

inline static byte get_color(byte_vector &pattern)
{
    // Note: i tried just using crc64 but it wasn't doing well for short patterns and low entropy `color_pallet` values.
    // // This works now as is.
    // My propietary algorithm for awesome color selection.
    // Compute color based on pattern bytes.
    // Get entropy from this data (make output more random/higher quality)
    uint64_t Iv = color_pallet;
    uint64_t Si = crc64(pattern.data(), pattern.size());
    uint64_t Sj = crc64(pattern.data(), (pattern.size() / 2) + 1);
    uint64_t Sk = crc64(pattern.data() + pattern.size() / 2, (pattern.size() / 2) + 1);

    // Call my own hash/checksum algorithms
    permute_box1(&Iv, &Si, &Sj, &Sk);
    permute_box2(&Iv, &Si, &Sj, &Sk);

    // Squach it down to 64 bits
    uint64_t checksum = Iv ^ Si ^ Sj ^ Sk;

    // Xor with my birthday (i think that is the right number)!
    checksum ^= 0x40f3b534;

    // Finally, modulo reduce it to the size of the color pallet (256).
    uint64_t theGrandColor = colors_list[checksum % colors_list.size()];

    // Return the color
    return theGrandColor;
}

inline static std::string make_ascii_dump(const byte_vector &input)
{
    std::string output;
    size_t input_size = input.size();
    output.resize(input_size + 19);
    output = "\033[38;5;250m |";

    for (size_t i = 0; i < input_size; i++)
    {
        if (input[i] >= 0x20 && input[i] <= 0x7E)
        {
            output += input[i];
        }
        else
        {
            output += '.';
        }
    }
    output += "|\033[0m\n";

    return output;
}

int rainbow_fd_dump(FILE *file)
{
    int file_fd = fileno(file);
    std::array<byte, CHUNK_SIZE> current_chunk;
    patterns_dict.clear();

    ssize_t bytes_read_stat = read(file_fd, current_chunk.data(), CHUNK_SIZE);

    if (bytes_read_stat < 0)
    {
        return -1;
    }
    size_t bytes_read = bytes_read_stat;

    std::string bytes_buffer = "";
    bytes_buffer.reserve(3 * CHUNK_SIZE);
    size_t current_pos = 0;
    size_t offset = 0; // Initialize the offset to 0
    while (bytes_read > 0)
    {
        for (size_t i = 0; i < bytes_read; i++)
        {
            size_t max_j = std::min(bytes_read - i, static_cast<size_t>(33));

            for (size_t j = 3; j <= max_j; j++)
            {
                byte_vector pattern_bytes(current_chunk.begin() + i, current_chunk.begin() + i + j);

                std::map<byte_vector, uint16_t>::iterator it = patterns_dict.find(pattern_bytes);
                if (it != patterns_dict.end())
                {
                    // add of not max
                    if (it->second < 65535)
                    {
                        it->second++;
                    }
                }
                else
                {
                    patterns_dict[pattern_bytes] = 1;
                }
            }
        }

        bytes_buffer.clear();
        size_t i = 0;
        size_t current_line_len = 0;
        std::stringstream ss1, ss2;
        while (i < bytes_read)
        {
            bool found_pattern = false;

            for (size_t j = 32; j > 2; j--)
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
                            if (current_line_len == 0)
                            {
                                // Print the offset at the beginning of the line
                                std::stringstream ss3;
                                ss3 << "\033[38;5;250m";
                                if (hex_chars_lookup_table == hex_chars_lookup_table_upper)
                                {
                                    ss3 << std::uppercase;
                                }
                                ss3 << std::setw(8) << std::setfill('0') << std::hex << offset;
                                ss3 << "\033[0m";
                                bytes_buffer += ss3.str();
                                bytes_buffer += "  ";
                            }

                            size_t j = std::min(pattern_bytes.size() - k, bytes_per_row - current_line_len);
                            int endPos = k + j;
                            ss1.str(std::string());
                            ss1 << "\033[38;5;" << (int)color << "m";
                            for (int l = k; l < endPos; l++)
                            {
                                ss1 << hex_chars_lookup_table[pattern_bytes[l] >> 4];
                                ss1 << hex_chars_lookup_table[pattern_bytes[l] & 0x0F];
                            }
                            ss1 << "\033[0m";
                            bytes_buffer += ss1.str();

                            current_line_len += j;
                            if (current_line_len == bytes_per_row)
                            {
                                // Print ascii
                                bytes_buffer += make_ascii_dump(byte_vector(current_chunk.begin() + i + j - current_line_len, current_chunk.begin() + i + j));

                                current_line_len = 0;
                            }

                            k += j;
                        }
                        i += j;
                        offset += j;
                        found_pattern = true;
                        break;
                    }
                }
            }
            if (!found_pattern)
            {
                if (current_line_len == 0)
                {
                    // Print the offset at the beginning of the line
                    std::stringstream ss3;
                    ss3 << "\033[38;5;250m";
                    ss3 << std::setw(8) << std::setfill('0') << std::hex << offset;
                    ss3 << "\033[0m";
                    bytes_buffer += ss3.str();
                    bytes_buffer += "  ";
                }
                ss2.str(std::string());
                ss2 << hex_chars_lookup_table[current_chunk[i] >> 4];
                ss2 << hex_chars_lookup_table[current_chunk[i] & 0x0F];

                bytes_buffer += ss2.str();
                current_line_len += 1;
                if (current_line_len == bytes_per_row)
                {
                    // Print ascii
                    bytes_buffer += make_ascii_dump(byte_vector(current_chunk.begin() + i - current_line_len, current_chunk.begin() + i));
                    current_line_len = 0;
                }
                offset++;
                i++;
            }
        }
        // Print ascii last line
        if (current_line_len > 0)
        {
            std::stringstream ss3;
            ss3 << "\033[38;5;250m";
            if (hex_chars_lookup_table == hex_chars_lookup_table_upper)
            {
                ss3 << std::uppercase;
            }
            ss3 << std::setw(8) << std::setfill('0') << std::hex << offset << ' ';
            ss3 << "\033[0m";
            // bytes_buffer.insert(bytes_buffer.find_last_of('\n') + 1, ss3.str());
            bytes_buffer.replace(bytes_buffer.find_last_of('\n') + 1, 24, ss3.str());

            bytes_buffer += std::string((bytes_per_row - current_line_len) * 2, ' ');
            bytes_buffer += make_ascii_dump(byte_vector(current_chunk.begin() + i - current_line_len, current_chunk.begin() + i));
            bytes_read = bytes_read_stat;
        }
        std::cout << bytes_buffer;
        current_pos += bytes_read;
        bytes_read_stat = read(file_fd, current_chunk.data(), CHUNK_SIZE);
        if (bytes_read_stat < 0)
        {
            return -1;
        }
        bytes_read = bytes_read_stat;
    }
    return 0;
}

// Initialize the colors list lookup table
inline void init_colors(void)
{
    colors_list.clear();
    colors_list.resize(256);
    for (int i = 0; i < 256; i++)
    {
        colors_list[i] = i;
    }
}

inline void println(std::string s)
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
    println("  -h             Print this help message.");
    println("  -v (--version) Print version information.");
    println("  -U             Prints in uppercase hexadecimals.");
    println("  '-'            Read from stdin.");
    println("");
    println("Note:");
    println("  Many more aliases are available for all the options. For example, -h == -? == --help.");
    println("  The parameters have been idiot proofed.");
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
    println(std::string("  rainbow-xd version ") + VERSION_ID);
    println("");
    println("Author:");
    println("  Written by Wesley Charles Jones.");
    println("");
    println("COPYRIGHT:");
    println("  Copyright © 2023 Wesley Charles Jones.  License GPLv2+: GNU GPL version 2 or later <https://gnu.org/licenses/gpl.html>.");
    println("  This is free software: you are free to change and redistribute it.  There is NO WARRANTY, to the extent permitted by law.");
    println("");
    println("SEE ALSO:");
    println("   https://github.com/Kracken256");
    println("");
}

int main(int argc, char **argv)
{
    std::vector<std::string> arguments(argv + 1, argv + argc);
    std::string exename = argv[0];
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
            // get 64 bit color code
            if (i + 1 < arguments.size())
            {
                std::string color_code = arguments[i + 1];
                try
                {
                    uint64_t color = std::stoull(color_code, nullptr, 10);

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
                    uint64_t cols = (uint64_t)std::stoull(num_cols);
                    if (cols < 1 || cols > 0xFFFF)
                    {
                        std::cout << "Invalid number of columns: " << num_cols << std::endl;
                        std::cout << "Must be between 1 and 65535" << std::endl;
                        should_exit = true;
                    }
                    else
                    {
                        bytes_per_row = (uint16_t)cols;
                        i++;
                    }
                }
                catch (std::exception &e)
                {
                    std::cout << "Invalid number of columns: " << num_cols << std::endl;
                    std::cout << "The number of columns must be a number between 1 and 65535" << std::endl;
                    should_exit = true;
                }
            }
            else
            {
                std::cout << "You did not specify the number of columns after '-c'" << std::endl;
                should_exit = true;
            }
        }
        else if (s == "-v" || s == "--version" || s == "-version" || s == "-V")
        {
            should_print_version = true;
            should_exit = true;
        }
        else if (s == "-U" || s == "--uppercase" || s == "--upper")
        {
            hex_chars_lookup_table = (char const *)hex_chars_lookup_table_upper;
        }
        // help. HELP ME GOD DAMN IT!!!
        // This is idiot proof. I don't want to deal with people who don't know how to use a command line.
        else if (s == "-h" || s == "--help" || s == "-?" || s == "/?" || s == "/h" || s == "/help" || s == "/h" || s == "-help")
        {
            should_print_help = true;
            should_exit = true;
        }
        else
        {
            std::cout << "Unknown argument: " << s << std::endl;
            std::cout << "Try '" << exename << " --help' for more information." << std::endl;
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
        FILE *file = fopen(filepath.c_str(), "rb");
        if (file == NULL)
        {
            std::cout << "Error opening file: " << filepath << std::endl;
            return 1;
        }
        int err = rainbow_fd_dump(file);
        fclose(file);
        return err;
    }
    // This is also idiot proof.
    // Check if stdin is sending data
    // Why don't all cli tools do this?
    int count;
    ioctl(fileno(stdin), FIONREAD, &count);

    if (count > 0)
    {
        return rainbow_fd_dump(stdin);
    }
    else
    {
        println("!![ERROR]!! No data to available read. Specify a file or pass data to STDIN\n");
        print_help();
        return 0;
    }
}