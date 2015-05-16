#ifndef _LIBAAN_CRYPTO_HH_
#define _LIBAAN_CRYPTO_HH_

#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <random>
#include <string>

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

namespace libaan {

bool read_random_bytes_noblock(size_t count, std::string & bytes);
bool read_random_bytes(size_t count, std::string &bytes);
bool read_random_ascii_set(const size_t count, const std::string &set,
                           std::string &bytes);

/* Usage:
   {
       const password_from_stdin pw(6);
       if(pw) {}
   }
*/
struct password_from_stdin {
    password_from_stdin(size_t pw_minlength = 1,
                        const std::string &prompt = "Enter password: ");
    ~password_from_stdin();

    operator std::string() const { return password; }
    operator bool() const { return have_password; }

    bool have_password;
    std::string password;
};


class hmac {
public:
    static const std::size_t SIZE;

    hmac(const std::string &key, std::string &hmac_out);
    bool update(const std::string &cipher_text_in);

    // hmac_out is only written in destructor. It is empty on failure
    ~hmac();

    bool state{false};
private:
    HMAC_CTX ctx;
    std::string &out;
};

class hash {
public:
    const static std::size_t SHA1_HASHLENGTH = SHA_DIGEST_LENGTH;

public:
    bool sha1(const std::string &in, std::string &out) const;
    bool sha1_hmac(const std::string &cipher_text_in, const std::string &key,
                   std::string &hmac_out) const;

private:
    bool do_hash(const EVP_MD *md, const std::string &in,
                        std::string &out) const;
    bool do_hmac(const EVP_MD *md, const std::string &cipher_text_in,
                 const std::string &key, std::string &hmac_out) const;
};


// Generate a key from a password.
//
// TODO: sha1 must be replaced
//
// http://www.ietf.org/rfc/rfc2898.txt
// This value needs to be the output size of your pseudo-random function (PRF)
const size_t PRF_OUTPUT_LENGTH = 20;

/* This is an implementation of the PKCS#5 PBKDF2 PRF using HMAC-SHA1.
 * always gives 20-byte outputs.
 */

// internal helper functions.
void pkcs5_initial_prf(const unsigned char *password, size_t password_length,
                       const unsigned char *salt, size_t salt_length, size_t i,
                       unsigned char *out, size_t *outlen);

/* The PRF doesn't *really* change in subsequent calls, but above we handled the
 * concatenation of the salt and i within the function, instead of external to
 * it,
 * because the implementation is easier that way.
 */
void pkcs5_subsequent_prf(const unsigned char *password, size_t password_length,
                          unsigned char *v, size_t vlen, unsigned char *o,
                          size_t *olen);

void pkcs5_F(const unsigned char *password, size_t password_length,
             const unsigned char *salt, size_t salt_length, size_t ic,
             size_t bix, unsigned char *out);

bool pbkdf2(const unsigned char *password, unsigned int password_length,
            const unsigned char *salt, uint64_t salt_length,
            unsigned int iteration_count, unsigned char *derived_key,
            uint64_t derived_key_length);

// c++ wrapper for pbkdf2, derived_key should be resized to wanted size
inline bool pbkdf2(const std::string &pw, const std::string &salt,
                   unsigned int iteration_count, std::string &derived_key)
{
    return pbkdf2(
reinterpret_cast<const unsigned char *>(pw.c_str()), pw.length(),
        reinterpret_cast<const unsigned char *>(salt.c_str()), salt.length(),
        iteration_count, reinterpret_cast<unsigned char *>(&derived_key[0]),
        derived_key.length());
}



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

#ifdef LION_ENABLED
// Dont use this. Only here to show usage of the api.

// Lion block cipher combines a stream cipher and a cryptographic hash function.
// "LION, which is a construction that takes a stream cipher and hash function
// and turns them into a block cipher that has an arbitrary block size.
// Essentially, LION turns those constructs into a single block cipher that has
// a variable block length, and you use the cipher in ECB mode."

// -> Advantage for file encryption is: no nonces/IVs for every 8192 byte
//    chunk needed.
// Uses the stream cipher twice with independent keys and the cryptographic hash
// once.// RC4 stream cipher: 128bit/16byte key
// Alternatives for file encryption are:
// - block cipher in CBC mode with random IV per block(8192)
namespace lion {

// LION requires stream cipher to be secure against related-key attacks

// Secure Programming Cookbook for C and C++: 5.15
const int HASH_SIZE = SHA_DIGEST_LENGTH;
const int HASH_WORDS = (HASH_SIZE / sizeof(int));
//const size_t CHUNK_SIZE = 8192;

bool check_file_size(size_t file_size);
// Input file must be longer than the output size of the message digest function(20 bytes for SHA1).
void encrypt(const unsigned char *input_buffer, unsigned char *output_buffer,
             size_t block_length, const unsigned char *key);
void decrypt(const unsigned char *input_buffer, unsigned char *output_buffer,
             size_t block_length, const unsigned char *key);
}
#endif

template<typename T>
// create random float in range [a, b]
T ranf(T a, T b)
{
    std::random_device r_dev;
    std::mt19937 engine(r_dev());
    std::uniform_real_distribution<> dist(a, b);
    return dist(engine);
}

template<typename container_type>
container_type ranf(size_t count, typename container_type::value_type a, typename container_type::value_type b)
{
    std::random_device r_dev;
    std::mt19937 engine(r_dev());
    std::uniform_real_distribution<> dist(a, b);
//    container_type c(count);
//    std::generate_n(std::begin(c), std::end(c), dist(engine));
    container_type c;
    c.reserve(count);
    for(size_t i = 0; i < count; i++)
        c.push_back(dist(engine));

    return c;
}

}

#endif
