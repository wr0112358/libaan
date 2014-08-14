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
bool read_random_bytes(size_t count, std::string &bytes);
bool read_random_ascii_set(size_t count, const std::string &set,
                           std::string &bytes);
namespace camellia {

// De-/Encryption of variable length strings with camellia block cipher in
// CBC mode. Blocks are padded automatically by openssl.
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

    bool new_random_iv();
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

// Implementation

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef NO_GOOD
#include <windows.h>
#include <wincrypt.h>

inline std::string err_string(DWORD error)
{
    if (error == 0)
        return "No Error.";

    std::string buf;
    buf.resize(256);

    ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    reinterpret_cast<TCHAR *>(&buf[0]), buf.length(), nullptr);
    return buf;
}

inline void win_err(const std::string &msg) {
    const auto err = GetLastError();
    std::cerr << "\"" << msg << "\"\n"
              << "Error(" << err << "): " << err_string(err);
}
#endif

/*
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
        std::cout << std::setw(2) << std::setfill('0')
                  << std::hex << (int)((unsigned char)c);
        ++count;
        if(count % newline_after_doublenibbles)
            std::cout << " ";
        else {
            std::cout << "\n";
            newline_started = true;
        }
    }
}
*/

inline bool libaan::crypto::read_random_bytes(size_t count, std::string & bytes)
{
    bytes.resize(count);

#ifndef NO_GOOD
    const std::string REAL_RANDOM_NUMBERS = "/dev/random";
    const std::string PSEUDO_RANDOM_NUMBERS = "/dev/urandom";

    std::ifstream f(REAL_RANDOM_NUMBERS,
                    std::ios_base::in | std::ios_base::binary);
    f.read(&bytes[0], count);
    return f;
#else
    HCRYPTPROV hCryptProv;
    // CRYPT_SILENT?
    //    if(!CryptAcquireContext(&hCryptProv, nullptr, nullptr, PROV_RSA_FULL,
    //    0)) {
    if(!CryptAcquireContext(&hCryptProv, nullptr, nullptr, PROV_RSA_FULL,
                            CRYPT_VERIFYCONTEXT)) {
        // GetLastError() == ERROR_BUSY -> retry?
        //  If ERROR_BUSY is a problem, creating a new key container and passing
        //  it in pszContainer might solve the problem.
        // GetLastError() == NTE_BAD_KEYSET
        // might indicate missing access rights to the default key container ->
        // create our own? See:
        // http://msdn.microsoft.com/en-us/library/aa379886.aspx
        win_err("CryptAcquireContext failed");
        return false;
    }

    if(!CryptGenRandom(hCryptProv, count,
                       reinterpret_cast<BYTE *>(&bytes[0]))) {
        win_err("CryptGenRandom failed");
        return false;
    }
    if(!CryptReleaseContext(hCryptProv, 0)) {
        win_err("CryptReleaseContext failed");
        return false;
    }

    return true;
#endif
}

inline bool libaan::crypto::read_random_ascii_set(size_t count,
                                                  const std::string &set,
                                                  std::string &bytes)
{
    bytes.resize(count);
    // possible optimization: read more than one at a time and cache it.
    // windows: create context at program start and reuse it
#ifndef NO_GOOD
    const std::string REAL_RANDOM_NUMBERS = "/dev/random";
    const std::string PSEUDO_RANDOM_NUMBERS = "/dev/urandom";

    // pseudo random good enough for password?
    std::ifstream f(PSEUDO_RANDOM_NUMBERS,
                    std::ios_base::in | std::ios_base::binary);
    std::size_t read = 0;
    do {
        f.read(&bytes[read], 1);
        if(set.find(bytes[read]) != std::string::npos)
            read++;
    } while(read < count);
    return f;
#else
    HCRYPTPROV hCryptProv;
    if(!CryptAcquireContext(&hCryptProv, nullptr, nullptr, PROV_RSA_FULL,
                            CRYPT_VERIFYCONTEXT)) {
        // GetLastError() == ERROR_BUSY -> retry?
        //  If ERROR_BUSY is a problem, creating a new key container and passing
        //  it in pszContainer might solve the problem.
        // GetLastError() == NTE_BAD_KEYSET
        // might indicate missing access rights to the default key container ->
        // create our own? See:
        // http://msdn.microsoft.com/en-us/library/aa379886.aspx
        win_err("CryptAcquireContext failed");
        return false;
    }

    std::size_t read = 0;
    do {
        if(!CryptGenRandom(hCryptProv, 1,
                           reinterpret_cast<BYTE *>(&bytes[read]))) {
            win_err("CryptGenRandom failed");
            return false;
        }

        if(set.find(bytes[read]) != std::string::npos)
            read++;
    } while(read < count);

    if(!CryptReleaseContext(hCryptProv, 0)) {
        win_err("CryptReleaseContext failed");
        return false;
    }

    return true;
#endif
}

// TODO: version for incremental encryption(see Viega p.186 "incremental_..."
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
    const int BYTES_PER_LOOP = (100 <= plain_in.length()) ? 100
                                                          : plain_in.length();
    int write_this_turn = BYTES_PER_LOOP;
    while(true) {
        int written = 0;
        if(!EVP_EncryptUpdate(ctx, &out[ol], &written, &data[input_offset],
                              write_this_turn)) {
            std::cout << "EVP_EncryptUpdate failed\n";
            return false;
        }

        input_offset += write_this_turn;
        if((input_offset + BYTES_PER_LOOP) > plain_in.length())
            write_this_turn = plain_in.length() - input_offset;

        ol += written;

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

    ol += written;
    cipher_out.resize(ol);

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

    int written = 0;
    if(!EVP_DecryptFinal(ctx, &out[ol], &written)) {
        unsigned long err = ERR_get_error();
        // TODO: should be moved to some central init function
        ERR_load_crypto_strings();
        std::cout << "EVP_DecryptFinal failed:\n\toffset = " << ol
                  << "\n\twritten = " << written << "\n";
        std::cout << "\n\"" << ERR_error_string(err, nullptr) << "\"\n";
        return false;
    }

    ol += written;
    plain_out.resize(ol);

    return true;
}

inline bool libaan::crypto::camellia::camellia_256::generate_key(
    const std::string &pw, std::string &key)
{
    const size_t iteration_count = 1000;
    if(!salt.length())
        return false;

    key.resize(KEY_SIZE);

    if(!pbkdf2_pkcs5::pbkdf2(pw, salt, iteration_count, key)) {
        std::cout << "pbkdf2 key generation failed.\n";
        return false;
    }

    return true; 
}

inline bool libaan::crypto::camellia::camellia_256::new_random_iv()
{
   if(!read_random_bytes(BLOCK_SIZE, iv)) {// iv with block size
        std::cout << "read from /dev/random failed\n";
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

    return new_random_iv();
}

inline bool
libaan::crypto::camellia::camellia_256::init(const std::string &existing_salt,
                                             const std::string &existing_iv)
{
    if(existing_salt.length() != BLOCK_SIZE)
        std::cout << "camellia_256::init: Warning salt length("
                  << salt.length() << ") differs from default("
                  << BLOCK_SIZE << "). Continueing.\n";
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
    // TODO: get new iv at this point? or provide api extension?
    if(iv.length() != BLOCK_SIZE)
        return false;

    std::string key;
    if(!generate_key(pw, key))
        return false;

    if(!plain.length()) {
        cipher.resize(0);
        std::cerr << "camellia_256::encrypt: skipping encryption. empty input.\n";
        return true;
    }
    EVP_CIPHER_CTX ctx;
    if(!EVP_EncryptInit(&ctx, EVP_camellia_256_cbc(),
                        reinterpret_cast<unsigned char *>(&key[0]),
                        reinterpret_cast<unsigned char *>(&iv[0]))) {
        std::cout << "EVP_EncryptInit failed\n";
        return false;
    }

    cipher.resize(plain.length() + BLOCK_SIZE);

    bool ret = do_encrypt(&ctx, plain, cipher);

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

    if(!cipher.length()) {
        plain.resize(0);
        std::cerr << "camellia_256::decrypt: skipping decryption. empty input.\n";
        return true;
    }

    EVP_CIPHER_CTX ctx;
    if(!EVP_DecryptInit(&ctx, EVP_camellia_256_cbc(),
                        reinterpret_cast<unsigned char *>(&key[0]),
                        reinterpret_cast<unsigned char *>(&iv[0]))) {
        std::cout << "EVP_DecryptInit failed\n";
        return false;
    }
    plain.resize(cipher.length() + BLOCK_SIZE + 1);

    return do_decrypt(&ctx, cipher, plain);
}

#endif
