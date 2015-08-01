
#include "crypto_file.hh"
#include "crypto.hh"
#include "file.hh"
#include "time.hh"

#include <fstream>

#include <iostream> // TODO: kill this

/*


Questions:
Is it ok to reuse the iv?
   No.
-> http://en.wikipedia.org/wiki/Block_cipher_modes_of_operation#Initialization_vector_.28IV.29
   "...in most cases, it is important that an initialization vector is
   never reused under the same key.
   For CBC and CFB, reusing an IV leaks some information about the
   first block of plaintext, and about any common prefix shared by the
   two messages.
   For OFB and CTR, reusing an IV completely destroys security.[6]
   This can be seen because both modes effectively create a bitstream
   that is XORed with the plaintext, and this bitstream is dependent
   on the password and IV only.
   Reusing a bitstream destroys security.[8] In CBC mode, the IV must,
   in addition, be unpredictable at encryption time; in particular,
   the (previously) common practice of re-using the last ciphertext
   block of a message as the IV for the next message is insecure (for
   example, this method was used by SSL 2.0).
   If an attacker knows the IV (or the previous block of ciphertext)
   before he specifies the next plaintext, he can check his guess
   about plaintext of some block that was encrypted with the same key
   before (this is known as the TLS CBC IV attack)."

Is it ok to reuse the salt?


Possible improvements:
Generate master_key with pbkdf2_pkcs5 alorithm using stored constant salt+password as input.
salt can be reused.
-> how to get unique iv from this master_key?
http://security.stackexchange.com/a/31544
http://security.stackexchange.com/a/31542

Use a different cpher mode like GCM or EAX: IV can be a counter, no padding necessary.
-> would make file protocoll/io easier.
-> openssl support?
http://security.stackexchange.com/a/49034

provide integrity with a MAC
  "CBC does not ensure integrity. Usually, when you need to encrypt
  (for confidentiality), you also need to reliably detect hostile
  alterations. For that, you need a MAC. Assembling a MAC and
  encryption is tricky."
using GCM or EAX mode would make this unnecessary:
http://crypto.stackexchange.com/questions/202/should-we-mac-then-encrypt-or-encrypt-then-mac

http://security.stackexchange.com/a/20301
"An HMAC by itself does not provide message integrity. It can be one
of the components in a protocol that provides integrity.(...)One
possible way to provide storage integrity in this scenario would be to
include the file name and a version number as part of the data whose
MAC is computed; Alice would need to remember the latest version
number of each file so as to verify that she is not given stale
data. Another way to ensure integrity would be for Alice to remember
the MAC of each file"
  - HMAC provides authenticity
  - for integrity a version number/date of last encryption should be
    appended to file header, or in the encrypted ćontent. If it is in
    the file header, it must be included in hmac construction.
    Integrity is given when user is shown the time of last encryption
    and checks if it is valid. Integrity is in this case only
    verified after decryption. Authenticity before decryption.

*/

libaan::crypto_file::crypto_file(const std::string &file_name /*, cipher_type type*/)
    : total_file_length(0), dirty(false), filename(file_name)
{
    OpenSSL_add_all_algorithms();
}

libaan::crypto_file::~crypto_file()
{
    // Overwrite memory.
    clear_buffers();
    std::cout << "~crypto_file\n";
    EVP_cleanup();
}


std::string libaan::crypto_file::time_of_last_write() const
{
    return to_string(deserialize_time_point<libaan::time_point_t>(timestamp), false);
}

bool libaan::crypto_file::parse_header()
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

    salt = file_header.substr(off, camellia_256::SALT_SIZE);
    off += camellia_256::BLOCK_SIZE;

    iv = file_header.substr(off, camellia_256::BLOCK_SIZE);
    off += camellia_256::BLOCK_SIZE;
    hmac = file_header.substr(off, hash::SHA1_HASHLENGTH);
    off += hash::SHA1_HASHLENGTH;

    timestamp = file_header.substr(off, sizeof(int64_t));
    return true;
}

void libaan::crypto_file::build_header_from_buffers()
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

    std::string salt_tmp(camellia_256::SALT_SIZE, 0);
    salt_tmp.replace(salt_tmp.begin(), salt_tmp.end(), salt);
    file_header.replace(file_header.begin() + off,
                        file_header.begin() + off + salt_tmp.length(),
                        salt_tmp);
    off += salt_tmp.length();

    std::string iv_tmp(camellia_256::BLOCK_SIZE, 0);
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

libaan::crypto_file::error_type
libaan::crypto_file::read(
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

    camellia_256 cipher;
    std::ifstream fp(filename, std::ios_base::in | std::ios_base::binary);
    total_file_length = libaan::get_file_length(fp);

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

libaan::crypto_file::error_type
libaan::crypto_file::write(
    const std::string &password)
{
    camellia_256 cipher;
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

    timestamp = storable_time_point_now_bin<libaan::time_point_t>();

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


bool libaan::crypto_file::parse_header_old_version_0010()
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
    salt = file_header.substr(off, camellia_256::SALT_SIZE);
    off += camellia_256::BLOCK_SIZE;
    iv = file_header.substr(off, camellia_256::BLOCK_SIZE);
    return true;
}

bool libaan::crypto_file::read_and_parse_old_version_0010(
    const std::string &password)
{
    clear_buffers();
    dirty = false;

    camellia_256 cipher;
    std::ifstream fp(filename, std::ios_base::in | std::ios_base::binary);
    total_file_length = libaan::get_file_length(fp);
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
