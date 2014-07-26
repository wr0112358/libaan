/*
Copyright (C) 2014 Reiter Wolfgang wr0112358@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef _LIBAAN_CRYPTO_UTIL_CAMELLIA_HH_
#define _LIBAAN_CRYPTO_UTIL_CAMELLIA_HH_

#include "crypto_pbkdf2_pkcs5.hh"

#include <openssl/err.h>
#include <openssl/evp.h>

namespace libaan {
namespace crypto {
namespace camellia {

void dump_evp_info(const EVP_CIPHER *ctx, const std::string &prefix = "")
{
    std::string mode_string;
    switch(EVP_CIPHER_mode(ctx)) {
    case EVP_CIPH_ECB_MODE:
        mode_string = "EVP_CIPH_ECB_MODE";
        break;
    case EVP_CIPH_CBC_MODE:
        mode_string = "EVP_CIPH_CBC_MODE";
        break;
    case EVP_CIPH_CFB_MODE:
        mode_string = "EVP_CIPH_CFB_MODE";
        break;
    case EVP_CIPH_OFB_MODE:
        mode_string = "EVP_CIPH_OFB_MODE";
        break;
    case EVP_CIPH_STREAM_CIPHER:
        mode_string = "EVP_CIPH_STREAM_CIPHER";
        break;
    default:
        mode_string = "UNKNOWN_CIPHER_MODE";
    }
    std::cout << prefix << "\n\tmode = " << mode_string << "\n";
}

/*
TODO
- EVP_DecryptUpdate has problems reading cipher texts padded to minimal size(BLOCK_SIZE)
*/

// De-/Encryption of variable length strings with camellia block cipher in CBC mode.
class camellia_256 {
public:
    static const uint8_t KEY_SIZE = 32; //256bit camellia
    static const uint8_t BLOCK_SIZE = 16;
    static const uint8_t SALT_SIZE = BLOCK_SIZE;

public:
    camellia_256() {}
    // generates iv and salt
    bool init();
    // use existing salt and iv.
    bool init(const std::string &existing_salt, const std::string &existing_iv);

    bool encrypt(const std::string &pw, const std::string &plain,
                 std::string &cipher);
    bool decrypt(const std::string &pw, const std::string &cipher,
                 std::string &plain);

private:
    bool generate_key(const std::string &pw, std::string &key);
    bool do_encrypt(EVP_CIPHER_CTX *ctx, const std::string &plain_in,
                    std::string &cipher_out);
    bool do_decrypt(EVP_CIPHER_CTX *ctx, const std::string &cipher_in,
                    std::string &plain_out);

//private:
public:
    std::string iv;
    std::string salt;
};

}
}
}

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

inline void ascii(const std::string &s, const std::string &prefix = "")
{
    std::istringstream iss(s);
    std::string line;
    while(std::getline(iss, line))
        std::cout << prefix << line;
}

inline void hex(const std::string &s, const std::string &prefix = "",
         size_t newline_after_doublenibbles = 24)
{
    size_t count = 0;
    bool newline_started = true;
    for(char c: s) {
        if(newline_started) {
            std::cout << prefix;
            newline_started = false;
        }
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)((unsigned char)c);
        ++count;
        if(count % newline_after_doublenibbles)
            std::cout << " ";
        else {
            std::cout << "\n";
            newline_started = true;
        }
    }
}

inline void dump_last_written(const std::string &buff, size_t offset, size_t count,
                       bool print_hex = true,
                       const std::string &descr = "DUMP",
                       const std::string &prefix = "",
                       size_t newline_after_doublenibbles = 24)
{
    std::cout << descr << " " << std::to_string(count) << ":\n";
    std::string curr(buff, offset, count);
    if(print_hex)
        hex(curr, prefix, newline_after_doublenibbles);
    else
        ascii(curr, prefix);
    std::cout << "\n";
}

inline bool read_random_bytes(size_t count, std::string & bytes)
{
    bytes.resize(count);

#ifndef NO_GOOD
    const std::string REAL_RANDOM_NUMBERS = "/dev/random";
    const std::string PSEUDO_RANDOM_NUMBERS = "/dev/urandom";

    std::ifstream f(PSEUDO_RANDOM_NUMBERS,
                    std::ios_base::in | std::ios_base::binary);
    f.read(&bytes[0], count);
    return f;
#else
    std::fill(bytes.begin(), bytes.end(), 0);
    // TODO:
    // http://msdn.microsoft.com/en-us/library/aa382048.aspx
    // http://msdn.microsoft.com/en-us/library/aa379942.aspx
    return true;
#endif
}

// TODO: provide 2nd version for incremental encryption(see opennssl book s.186 "incremental_ecrypt(...)"
inline bool libaan::crypto::camellia::camellia_256::do_encrypt(
    EVP_CIPHER_CTX *ctx, const std::string &plain_in, std::string &cipher_out)
{
    //std::cout << "encrypt iv:"; hex(iv, "\t");
    //std::cout << "encrypt salt:"; hex(salt, "\t");
    if(!plain_in.length()
       || (cipher_out.length() != plain_in.length() + BLOCK_SIZE)) {
        std::cerr << "camellia_256::do_encrypt: wrong buffersizes specified:\n"
                  << "\tplain_in.length() = " << plain_in.length() << "\n"
                  << "\tcipher_out.length() = " << cipher_out.length() << "\n"
                  << "\tBLOCK_SIZE = " << BLOCK_SIZE << "\n";
        return false;
    }

    unsigned char *data =
        reinterpret_cast<unsigned char *>(const_cast<char *>(&plain_in[0]));
    unsigned char *out = reinterpret_cast<unsigned char *>(&cipher_out[0]);
    size_t ol = 0;
    size_t input_offset = 0;
    const int BYTES_PER_LOOP = (100 <= plain_in.length()) ? 100 : plain_in.length();
    int write_this_turn = BYTES_PER_LOOP;
    while(true) {
        int written = 0;
        // TODO: bug is most likely here
        if(!EVP_EncryptUpdate(ctx, &out[ol], &written, &data[input_offset], write_this_turn)) {
            std::cout << "EVP_EncryptUpdate failed\n";
            return false;
        }

        input_offset += write_this_turn;
        if((input_offset + BYTES_PER_LOOP) > plain_in.length())
            write_this_turn = plain_in.length() - input_offset;

        //dump_last_written(cipher_out, ol, written, true, "encryption-loop -> written", "\t");

        ol += written;
        //std::cout << "encrypt-loop: (total-written = " << ol
        //          << ", written = " << written
        //          << ", input_offset = " << input_offset
        //          << ", write_this_turn = " << write_this_turn << ")\n";

        if(input_offset == plain_in.length())
            break;
        if(input_offset > plain_in.length()) {
            std::cout
                << "Encrypt: Fatal Error(input_offset > plain_in.length())\n";
            break;
        }
    }

    int written = 0;
    if(!EVP_EncryptFinal(ctx, &out[ol], &written)) {
        std::cout << "EVP_EncryptFinal failed\n";
        return false;
    }

    //dump_last_written(cipher_out, ol, written, true, "encryption-final -> written", "\t");
    ol += written;
    //std::cout << "encrypt-final: (total-written = " << ol << ", written = " << written << ")\n";
    cipher_out.resize(ol);

    //std::cout << "encrypt: (in.size = " << plain_in.length()
    //          << ", out.size = " << cipher_out.length() << ")\n";
    return true;
}

inline bool libaan::crypto::camellia::camellia_256::do_decrypt(
    EVP_CIPHER_CTX *ctx, const std::string &cipher_in, std::string &plain_out)
{
    //std::cout << "decrypt iv:"; hex(iv, "\t");
    //std::cout << "decrypt salt:"; hex(salt, "\t");
    if (!cipher_in.length() ||
        (plain_out.length() != (cipher_in.length() + BLOCK_SIZE + 1))) {
        std::cout << "camellia_256::decrypt failed: wrong buffer sizes\n";
        return false;
    }

    unsigned char *data =
        reinterpret_cast<unsigned char *>(const_cast<char *>(&cipher_in[0]));
    unsigned char *out = reinterpret_cast<unsigned char *>(&plain_out[0]);
    int ol = 0;
    int cipher_in_length = static_cast<int>(cipher_in.length());
    if(!EVP_DecryptUpdate(ctx, out, &ol, data, cipher_in_length)) {
            std::cout << "EVP_DecryptUpdate failed\n";
            return false;
    }
    if(!ol) {
        plain_out.resize(0);
        std::cout << "camellia_256::decrypt: no cipher data. Exiting.\n";
        return true;
    }
    //std::cout << "decrypt: (written = " << ol << ")\n";
    //dump_last_written(plain_out, 0, ol, false, "decryption -> written", "\t");


        std::cerr << "\tbuffersizes:\n"
                  << "\tcipher_in.length() = " << cipher_in.length() << "\n"
                  << "\tplain_out.length() = " << plain_out.length() << "\n"
                  << "\tBLOCK_SIZE = " << BLOCK_SIZE << "\n";

        ERR_load_crypto_strings();
    int written = 0;
    if(!EVP_DecryptFinal(ctx, &out[ol], &written)) {
        unsigned long err = ERR_get_error();
        std::cout << "EVP_DecryptFinal failed:\n\toffset = " << ol
                  << "\n\twritten = " << written << "\n";
        ERR_print_errors_fp(stdout);
        std::cout << "\n\"" << ERR_error_string(err, nullptr) << "\"\n";
        return false;
    }

        std::cout << "EVP_DecryptFinal SUCCESS:\n\toffset = " << ol
                  << "\n\twritten = " << written << "\n";

    //dump_last_written(plain_out, ol, written, false, "decryption-final -> written", "\t");
    ol += written;
    //std::cout << "decrypt-final: (total-written = " << ol << ", written = " << written << ")\n";
    plain_out.resize(ol);
    //std::cout << "decrypt-3.plain("<<plain_out.length()<<"): \"" << plain_out << "\"\n";
    //std::cout << "decrypt: (in.size = " << cipher_in.length()
    //          << ", out.size = " << plain_out.length() << ")\n";
    return true;
}

inline bool libaan::crypto::camellia::camellia_256::generate_key(
    const std::string &pw, std::string &key)
{
    const size_t iteration_count = 1000;
    if(!salt.length())
        return false;

    key.resize(KEY_SIZE);

    if(!libaan::crypto::pbkdf2_pkcs5::pbkdf2(pw, salt, iteration_count, key)) {
        std::cout << "pbkdf2 key generation failed.\n";
        return false;
    }

    return true; 
}

inline bool libaan::crypto::camellia::camellia_256::init()
{
    if(!read_random_bytes(16, salt)) {// 128 bit salt
        std::cout << "read from /dev/random failed\n";
        return false;
    }

    if(!read_random_bytes(BLOCK_SIZE, iv)) {// iv with block size
        std::cout << "read from /dev/random failed\n";
        return false;
    }
    return true;
}

inline bool
libaan::crypto::camellia::camellia_256::init(const std::string &existing_salt,
                                                     const std::string &existing_iv)
{
    if(existing_salt.length() != BLOCK_SIZE)
        std::cout << "camellia_256::init: Warning salt length(" << salt.length()
                  << ") differs from default(" << BLOCK_SIZE
                  << "). Continueing.\n";
    if(existing_iv.length() != BLOCK_SIZE) {
        std::cout << "camellia_256::init: Fatal Error: iv length differs from "
                     "default(" << BLOCK_SIZE << "). Aborting.\n";
        return false;
    }

    salt = existing_salt;
    iv = existing_iv;

    return true;
}

inline bool libaan::crypto::camellia::camellia_256::encrypt(
    const std::string &pw, const std::string &plain, std::string &cipher)
{
    if(iv.length() != BLOCK_SIZE)
        return false;

    std::string key;
    if(!generate_key(pw, key))
        return false;

    EVP_CIPHER_CTX ctx;
    if(!EVP_EncryptInit(&ctx, EVP_camellia_256_cbc(),
                        reinterpret_cast<unsigned char *>(&key[0]),
                        reinterpret_cast<unsigned char *>(&iv[0]))) {
        std::cout << "EVP_EncryptInit failed\n";
        return false;
    }

    cipher.resize(plain.length() + BLOCK_SIZE);
    bool ret = do_encrypt(&ctx, plain, cipher);

    //dump_evp_info(EVP_camellia_256_cbc(), "camellia: ");
    return ret;
}

inline bool libaan::crypto::camellia::camellia_256::decrypt(
    const std::string &pw, const std::string &cipher, std::string &plain)
{
    if(iv.length() != BLOCK_SIZE)
        return false;

    std::string key;
    if(!generate_key(pw, key))
        return false;

    EVP_CIPHER_CTX ctx;
    if(!EVP_DecryptInit(&ctx, EVP_camellia_256_cbc(),
                        reinterpret_cast<unsigned char *>(&key[0]),
                        reinterpret_cast<unsigned char *>(&iv[0]))) {
        std::cout << "EVP_EncryptInit failed\n";
        return false;
    }
    plain.resize(cipher.length() + BLOCK_SIZE + 1);

    //dump_evp_info(EVP_camellia_256_cbc(), "camellia: ");
    return do_decrypt(&ctx, cipher, plain);
}

#endif
