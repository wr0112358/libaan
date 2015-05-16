/* File encryption for small files that are rewritten completely when changed.


VERSION 0010
file format:
64bytes unencrypted header
variable number of camellia 256 encrypted blocks in cbc mode

header format:
8 bytes magic string
4 bytes version string
camelia256::saltsize byte salt
camelia256::blocksize bytes iv
size = 8 + 4 + 16 + 16 = 44

TODO: VERSION 0020



VERSION 0020
file format:
128bytes unencrypted header
variable number of camellia 256 encrypted blocks in cbc mode

header format:
4 bytes magic string
4 bytes version string
camelia256::saltsize(16) byte salt
camelia256::blocksize(16) bytes iv
sha1_hashlength(20) bytes hmac
8byte timestamp
size = 8 + 4 + 16 + 16 + 20 + 8 = 72

*/

#ifndef _LIBAAN_CRYPTO_FILE_HH_
#define _LIBAAN_CRYPTO_FILE_HH_

#include <cstddef>
#include <string>

namespace libaan {

const size_t HEADER_SIZE = 128;
const std::string MAGIC = {'\x13', '\x12', '\x11', '\x13'};
// VERSION_0010:
//   HEADER_SIZE = 64
const size_t OLD_HEADER_SIZE_0010 = 64;
const std::string OLD_MAGIC_0010 = {'\x13', '\x13', '\x13', '\x13',
                                    '\x13', '\x13', '\x13', '\x13'};
const std::string VERSION_0010 = {'\x0', '\x0', '\x1', '\x0'};
// VERSION_0020:
//   o 0010 only 20 bytes left, sha1 HMAC fits exactly, no space for 64bit timestamp
//   o reduce size of magic number and use a value, that makes sense
//   o add hmac to unencrypted header.
//   o add timestamp to unencrypted header.
//   o HEADER_SIZE = 128
const std::string VERSION_0020 = {'\x0', '\x0', '\x2', '\x0'};

class crypto_file {
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
    crypto_file(const std::string &file_name /*, cipher_type type*/);

    ~crypto_file();

    /* read specified file in buffer, password is not saved.
       1. read existing iv/salt from header
       2. take user provided password and salt from file header to generate a
          key of length camelia256::keysize with pbkdf2_pkcs5 algorithm
       3. read the whole encrypted file in a buffer and decrypt using key and iv
    */
    error_type read(const std::string & password);

    /* write (possibly modified) buffer to associated file.
       create file if necessary:
         1. fill salt/iv with rng from /dev/random
         2. write header to file

       write file:
         1. read existing iv/salt from header
         2. take user provided password and salt from file header to generate a
            key of length camelia256::keysize with pbkdf2_pkcs5 algorithm
         3. get new iv from /dev/random
         4. encrypt provided buffer using key and the iv from step 3.
            update and write header and all blocks to file.
    */
    error_type write(const std::string & password);

    // set all internal data buffers to 0.
    void clear_buffers()
    {
        std::fill(file_header.begin(), file_header.end(), 0);
        std::fill(decrypted_buffer.begin(), decrypted_buffer.end(), 0);
        std::fill(encrypted_file.begin(), encrypted_file.end(), 0);
    }

    // Return time of last write. should only be called, after a
    // successfull read().
    // Provides integrity: user must remember when he last modified
    // the file. If the returned date fits and hmac was verified
    // successfully for authenticity.
    std::string time_of_last_write() const;

    // use this function to modify the decrypted buffer.
    std::string &get_decrypted_buffer() { return decrypted_buffer; }
    const std::string &get_decrypted_buffer() const { return decrypted_buffer; }

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

#endif
