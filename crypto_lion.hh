// Dont use this. Only here to show usage of the api.
#ifndef _LIBAAN_CRYPTO_LION_HH_
#define _LIBAAN_CRYPTO_LION_HH_

#include <openssl/rc4.h>
#include <openssl/sha.h>

namespace libaan {
namespace crypto {
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
}
}

// Implementation


inline bool libaan::crypto::lion::check_file_size(size_t file_size)
{
//    if(file_size % CHUNK_SIZE)
//        return false;
    if(file_size < HASH_SIZE)
        return false;
    return true;
}

inline void libaan::crypto::lion::encrypt(const unsigned char *input_buffer,
                 unsigned char *output_buffer, size_t block_length, const unsigned char *key)
{
    int tmp[HASH_WORDS];
    RC4_KEY k;
    const unsigned char *key_data = reinterpret_cast<const unsigned char *>(&tmp[0]);

    /* Round 1: R = R ^ RC4(L ^ K1) */
    for (int i = 0; i < HASH_WORDS; i++)
        tmp[i] = ((int *)input_buffer)[i] ^ ((int *)key)[i];
    RC4_set_key(&k, HASH_SIZE, key_data);
    RC4(&k, block_length - HASH_SIZE, input_buffer + HASH_SIZE,
        output_buffer + HASH_SIZE);

    /* Round 2: L = L ^ SHA1(R) */
    SHA1(output_buffer + HASH_SIZE, block_length - HASH_SIZE, output_buffer);
    for (int i = 0; i < HASH_WORDS; i++)
        ((int *)output_buffer)[i] ^= ((int *)input_buffer)[i];

    /* Round 3: R = R ^ RC4(L ^ K2) */
    for (int i = 0; i < HASH_WORDS; i++)
        tmp[i] = ((int *)output_buffer)[i] ^ ((int *)key)[i + HASH_WORDS];
    RC4_set_key(&k, HASH_SIZE, key_data);
    RC4(&k, block_length - HASH_SIZE, output_buffer + HASH_SIZE,
        output_buffer + HASH_SIZE);
}

inline void libaan::crypto::lion::decrypt(const unsigned char *input_buffer,
                 unsigned char *output_buffer, size_t block_length, const unsigned char *key)
{
    int tmp[HASH_WORDS];
    RC4_KEY k;
    const unsigned char *key_data = reinterpret_cast<const unsigned char *>(&tmp[0]);

    for (int i = 0; i < HASH_WORDS; i++)
        tmp[i] = ((int *)input_buffer)[i] ^ ((int *)key)[i + HASH_WORDS];
    RC4_set_key(&k, HASH_SIZE, key_data);
    RC4(&k, block_length - HASH_SIZE, input_buffer + HASH_SIZE,
        output_buffer + HASH_SIZE);
    SHA1(output_buffer + HASH_SIZE, block_length - HASH_SIZE, output_buffer);

    for (int i = 0; i < HASH_WORDS; i++) {
        ((int *)output_buffer)[i] ^= ((int *)input_buffer)[i];
        tmp[i] = ((int *)output_buffer)[i] ^ ((int *)key)[i];
    }
    RC4_set_key(&k, HASH_SIZE, key_data);
    RC4(&k, block_length - HASH_SIZE, output_buffer + HASH_SIZE,
        output_buffer + HASH_SIZE);
}


#endif
