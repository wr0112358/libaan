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

#ifndef _LIBAAN_CRYPTO_FILE_HH_
#define _LIBAAN_CRYPTO_FILE_HH_

#include "file_util.hh"
#include "crypto_camellia.hh"

#include <fstream>

namespace libaan {
namespace crypto {
namespace file {

const size_t HEADER_SIZE = 128;
const std::string MAGIC = {'\x13', '\x12', '\x11', '\x13'};
// VERSION_0010:
//   HEADER_SIZE = 64
const size_t OLD_HEADER_SIZE_0010 = 64;
const std::string OLD_MAGIC_0010 = {'\x13', '\x13', '\x13', '\x13',
                                    '\x13', '\x13', '\x13', '\x13'};
const std::string VERSION_0010 = {'\x0', '\x0', '\x1', '\x0'};
// VERSION_0020:
//   reduce size of magic number and use a value, that makes sense
//   add hmac to unencrypted header.
//   add timestamp to unencrypted header.
//   HEADER_SIZE = 128
const std::string VERSION_0020 = {'\x0', '\x0', '\x2', '\x0'};

class crypto_file
{
public:
    enum error_type {
#ifdef NO_GOOD
    #undef NO_ERROR
#endif
        NO_ERROR,
        NO_HEADER_IN_FILE,
        CIPHER_ERROR_FILE_LENGTH,
        INTERNAL_CIPHER_ERROR
        // NO_SUCH_KEY if decrypt fails
        // HMAC_FAILED if authenticity check fails
    };

/*
TODO
- memset(0) before/after use
- mlock on decrypted string buffer?
- de/serialization:
  when writing convert all non-ascii/not-byte-array header elements to network byte order
  when reading convert them back to host byte order
*/
public:
    crypto_file(const std::string &file_name /*, cipher_type type*/)
        : total_file_length(0), dirty(false), filename(file_name) {}

    ~crypto_file()
    {
        // Overwrite memory.
        clear_buffers();
    }

    // read specified file in buffer.
    // password is not saved.
    error_type read(const std::string & password);

    // write (possibly modified) buffer to associated file.
    error_type write(const std::string & password);

    // set all internal data buffers to 0.
    void clear_buffers()
    {
        std::fill(file_header.begin(), file_header.end(), 0);
        std::fill(decrypted_buffer.begin(), decrypted_buffer.end(), 0);
        std::fill(encrypted_file.begin(), encrypted_file.end(), 0);
    }

    // use this function to modify the decrypted buffer.
    std::string &get_decrypted_buffer() { return decrypted_buffer; }
    void set_dirty() { dirty = true; }
    bool is_dirty() const { return dirty; }

    error_type get_last_error() const { return last_error; } 
    static std::string error_string(error_type err)
    {
        switch(err) {
        case NO_ERROR: return "NO_ERROR";
        case NO_HEADER_IN_FILE: return "NO_HEADER_IN_FILE";
        case CIPHER_ERROR_FILE_LENGTH: return "CIPHER_ERROR_FILE_LENGTH";
        case INTERNAL_CIPHER_ERROR: return "INTERNAL_CIPHER_ERROR";
        }
        return "UNKNOWN ERROR";
    }
    bool read_and_parse_old_version_0010(const std::string &password);

private:
    error_type error(error_type e) { last_error = e; return last_error; }

    // parse file_header, fill iv/salt
    bool parse_header();

    // create file_header buffer from salt/iv etc
    void build_header_from_buffers();

    bool parse_header_old_version_0010();

private:
    // filesize including header
    size_t total_file_length;
    // fixed size header containing db information. unencrypted
    std::string file_header;
    std::string salt;
    std::string iv;
    std::string hmac;
    std::string timestamp;

    std::string encrypted_file;
    std::string decrypted_buffer;
    // decrypted_buffer changed. encrypted_file must be updated.
    bool dirty;

    const std::string filename;
    error_type last_error;
    const std::string version = VERSION_0020;
};
}
}
}

// Implementation

#include "chrono_util.hh"
#include "crypto_camellia.hh"
#include "crypto_hash.hh"

inline bool libaan::crypto::file::crypto_file::parse_header()
{
    size_t off = 0;
    std::string magic_tmp = file_header.substr(off, MAGIC.length());
    if(magic_tmp != MAGIC) {
        std::cerr << "parse_header ERROR: magic number wrong.\n";
        return false;
    }

    off += MAGIC.length();

    std::string version_tmp
        = file_header.substr(off, version.length());

    if(version_tmp != version) {
        std::cerr << "parse_header ERROR: invalid version number.\n";
        return false;
    }
    off += version.length();

    salt = file_header.substr(
        off, camellia::camellia_256::SALT_SIZE);
    off += camellia::camellia_256::BLOCK_SIZE;

    iv = file_header.substr(off, camellia::camellia_256::BLOCK_SIZE);
    off += camellia::camellia_256::BLOCK_SIZE;
    hmac = file_header.substr(off, hash::SHA1_HASHLENGTH);
    off += hash::SHA1_HASHLENGTH;

    timestamp = file_header.substr(off, sizeof(int64_t));
    return true;
}

inline void
libaan::crypto::file::crypto_file::build_header_from_buffers()
{
    file_header.resize(HEADER_SIZE);
    std::fill(file_header.begin(), file_header.end(), 0);
    size_t off = 0;
    file_header.replace(file_header.begin() + off,
                        file_header.begin() + off + MAGIC.length(), MAGIC);
    off += MAGIC.length();

    file_header.replace(file_header.begin() + off,
                        file_header.begin() + off + version.length(),
                        version);
    off += version.length();

    std::string salt_tmp(camellia::camellia_256::SALT_SIZE, 0);
    salt_tmp.replace(salt_tmp.begin(), salt_tmp.end(), salt);
    file_header.replace(file_header.begin() + off,
                        file_header.begin() + off + salt_tmp.length(),
                        salt_tmp);
    off += salt_tmp.length();

    std::string iv_tmp(camellia::camellia_256::BLOCK_SIZE, 0);
    iv_tmp.replace(iv_tmp.begin(), iv_tmp.end(), iv);
    file_header.replace(file_header.begin() + off,
                        file_header.begin() + off + iv_tmp.length(), iv_tmp);
    off += iv_tmp.length();

    std::string hmac_tmp(hash::SHA1_HASHLENGTH, 0);
    hmac_tmp.replace(hmac_tmp.begin(), hmac_tmp.end(), hmac);
    file_header.replace(file_header.begin() + off,
                        file_header.begin() + off + hmac_tmp.length(), hmac_tmp);
    off += hmac_tmp.length();

    std::string timestamp_tmp(sizeof(int64_t), 0);
    timestamp_tmp.replace(timestamp_tmp.begin(), timestamp_tmp.end(), timestamp);
    file_header.replace(file_header.begin() + off,
                        file_header.begin() + off + timestamp_tmp.length(), timestamp_tmp);
}

inline libaan::crypto::file::crypto_file::error_type
libaan::crypto::file::crypto_file::read(
    const std::string &password)
{
    clear_buffers();
    dirty = false;

    // possible cases
    // o File does not exist
    //   + generate iv/salt via camellia api and write header to file.
    //   (-> return error if file io fails)
    // o File exists
    //   + header valid. contains iv and salt. -> read and decrypt
    //   + header invalid. return error

    camellia::camellia_256 cipher;
    std::ifstream fp(filename, std::ios_base::in | std::ios_base::binary);
    total_file_length = libaan::util::file::get_file_length(fp);

    if (total_file_length < HEADER_SIZE) {
        // No header/empty file. Create new header.
        if(!cipher.init()) {
            return INTERNAL_CIPHER_ERROR;
        }
        iv = cipher.iv;
        salt = cipher.salt;
        set_dirty();
        // Clearing dst-file not necessary, since dirty flag is set.
    } else {
        // File contains something. Read header and encrypted contents
        // in buffers.
        file_header.resize(HEADER_SIZE);
        char *begin = &*file_header.begin();
        // read header
        fp.read(begin, HEADER_SIZE);
        if(!parse_header()) {
            std::cerr << "Parsing header failed.\n";
            return NO_HEADER_IN_FILE;
        }

        const size_t encrypted_file_length = total_file_length - HEADER_SIZE;
        // if(!encrypted_file_length)
        //     std::cerr << "crypto_file::read -> header exists, file empty\n";
        encrypted_file.resize(encrypted_file_length);
        begin = &*encrypted_file.begin();

        // read encrypted contents
        fp.read(begin, encrypted_file_length);

        // TODO: after parsing, before decryption, the hmac should be calculated
        //       and checked with the stored one
        hash h;
        std::string hmac_tmp;
        if(!h.sha1_hmac(timestamp + encrypted_file, password, hmac_tmp)) {
            std::cerr << "crypto_file::write(): hmac generation failed.\n";
            return INTERNAL_CIPHER_ERROR;
        }
        if(hmac_tmp != hmac) {
            std::cerr << "HMAC check failed. File integrity not ensured.\n";
            return INTERNAL_CIPHER_ERROR;
        }

        if(!cipher.init(salt, iv)) {
            return INTERNAL_CIPHER_ERROR;
        }
        decrypted_buffer.resize(encrypted_file_length);

        if(!cipher.decrypt(password, encrypted_file, decrypted_buffer)) {
            std::cerr << "crypto_file::read -> cipher.decrypt() failed.\n";
            return INTERNAL_CIPHER_ERROR;
        }
    }
    // at this point file_header and decrypted_buffer are filled

    return error(NO_ERROR);
}

inline libaan::crypto::file::crypto_file::error_type
libaan::crypto::file::crypto_file::write(
    const std::string &password)
{
    camellia::camellia_256 cipher;
    if(!cipher.init(salt, iv)) {
        std::cerr << "crypto_file::read: cipher.init() failed.\n";
        return INTERNAL_CIPHER_ERROR;
    }

    if(!cipher.new_random_iv())
        return INTERNAL_CIPHER_ERROR;
    iv = cipher.iv;

    if(!cipher.encrypt(password, decrypted_buffer, encrypted_file)) {
        std::cerr << "crypto_file::write(): camellia_256::encrypt() failed\n";
        return INTERNAL_CIPHER_ERROR;
    }

    timestamp = libaan::util::storable_time_point_now();
    hash h;
    if(!h.sha1_hmac(timestamp + encrypted_file, password, hmac)) {
        std::cerr << "crypto_file::write(): hmac generation failed.\n";
        return INTERNAL_CIPHER_ERROR;
    }
    // TODO: create hmac from encrypted buffer and timestamp
    build_header_from_buffers();

    // write buffers to file. binary mode to avoid problems with different line
    // endings under windows. truncate mode to overwrite the file everytime.
    std::ofstream fp(filename, std::ios_base::out | std::ios_base::binary
                               | std::ios_base::trunc);
    char *begin = &*file_header.begin();
    fp.write(begin, file_header.length());
    begin = &*encrypted_file.begin();
    fp.write(begin, encrypted_file.length());
    fp.close();
    dirty = false;

    return error(NO_ERROR);
}


inline bool libaan::crypto::file::crypto_file::parse_header_old_version_0010()
{
    size_t off = 0;
    std::string magic_tmp = file_header.substr(off, OLD_MAGIC_0010.length());
    if(magic_tmp != OLD_MAGIC_0010) {
        std::cerr << "parse_header ERROR: magic number wrong.\n";
        return false;
    }
    off += OLD_MAGIC_0010.length();
    std::string version_0010_tmp
        = file_header.substr(off, VERSION_0010.length());
    if(version_0010_tmp != VERSION_0010) {
        std::cerr << "parse_header ERROR: version_0010 number wrong.\n";
        return false;
    }
    off += VERSION_0010.length();
    salt = file_header.substr(
        off, camellia::camellia_256::SALT_SIZE);
    off += camellia::camellia_256::BLOCK_SIZE;
    iv = file_header.substr(off, camellia::camellia_256::BLOCK_SIZE);
    return true;
}

inline bool libaan::crypto::file::crypto_file::read_and_parse_old_version_0010(
    const std::string &password)
{
    clear_buffers();
    dirty = false;

    camellia::camellia_256 cipher;
    std::ifstream fp(filename, std::ios_base::in | std::ios_base::binary);
    total_file_length = libaan::util::file::get_file_length(fp);
    if (total_file_length < OLD_HEADER_SIZE_0010) {
        // No header/empty file. Create new header.
        if(!cipher.init()) {
            return INTERNAL_CIPHER_ERROR;
        }
        iv = cipher.iv;
        salt = cipher.salt;
        // Clearing dst-file not necessary, since dirty flag is set.
    } else {
        // File contains something. Read header and encrypted contents
        // in buffers.
        file_header.resize(OLD_HEADER_SIZE_0010);
        char *begin = &*file_header.begin();
        fp.read(begin, OLD_HEADER_SIZE_0010);
        if(!parse_header_old_version_0010()) {
            std::cerr << "Parsing header failed.\n";
            return NO_HEADER_IN_FILE;
        }
        const size_t encrypted_file_length = total_file_length - OLD_HEADER_SIZE_0010;

        encrypted_file.resize(encrypted_file_length);
        begin = &*encrypted_file.begin();
        // read encrypted contents
        fp.read(begin, encrypted_file_length);
        if(!cipher.init(salt, iv)) {
            return INTERNAL_CIPHER_ERROR;
        }
        decrypted_buffer.resize(encrypted_file_length);
        if(!cipher.decrypt(password, encrypted_file, decrypted_buffer)) {
            std::cerr << "crypto_file::read -> cipher.decrypt() failed.\n";
            return INTERNAL_CIPHER_ERROR;
        }
    }

    // force rewrite
    set_dirty();
    return true;
}

#endif
