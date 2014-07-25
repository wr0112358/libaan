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

// TODO: sha1 must be replaced

#ifndef _LIBAAN_CRYPTO_PBKDF2_PKCS5_HH_
#define _LIBAAN_CRYPTO_PBKDF2_PKCS5_HH_

#include <string>

namespace libaan {
namespace crypto {
// Generate a key from a password.
namespace pbkdf2_pkcs5 {

// http://www.ietf.org/rfc/rfc2898.txt
// This value needs to be the output size of your pseudo-random function (PRF)
const size_t PRF_OUTPUT_LENGTH = 20;

/* This is an implementation of the PKCS#5 PBKDF2 PRF using HMAC-SHA1.
 * always gives 20-byte outputs.
 */

/* The first three functions are internal helper functions. */
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
bool pbkdf2(const std::string &pw, const std::string &salt,
            unsigned int iteration_count, std::string &derived_key);

}
}
}

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h> /* for htonl */
#include <cstdint>
#include <cstring>

inline void libaan::crypto::pbkdf2_pkcs5::pkcs5_initial_prf(
    const unsigned char *password, size_t password_length,
    const unsigned char *salt, size_t salt_length, size_t i, unsigned char *out,
    size_t *outlen)
{
    size_t swapped_i;
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init(&ctx, password, password_length, EVP_sha1());
    HMAC_Update(&ctx, salt, salt_length);
    swapped_i = htonl(i);
    HMAC_Update(&ctx, (unsigned char *)&swapped_i, 4);
    HMAC_Final(&ctx, out, (unsigned int *)outlen);
}

inline void libaan::crypto::pbkdf2_pkcs5::pkcs5_subsequent_prf(
    const unsigned char *password, size_t password_length, unsigned char *v,
    size_t vlen, unsigned char *o, size_t *olen)
{
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init(&ctx, password, password_length, EVP_sha1());
    HMAC_Update(&ctx, v, vlen);
    HMAC_Final(&ctx, o, (unsigned int *)olen);
}

inline void libaan::crypto::pbkdf2_pkcs5::pkcs5_F(
    const unsigned char *password, size_t password_length,
    const unsigned char *salt, size_t salt_length, size_t ic, size_t bix,
    unsigned char *out)
{
    size_t i = 1, j, outlen;
    unsigned char ulast[PRF_OUTPUT_LENGTH];
    memset(out, 0, PRF_OUTPUT_LENGTH);
    pkcs5_initial_prf(password, password_length, salt, salt_length, bix, ulast, &outlen);
    while (i++ <= ic) {
        for (j = 0; j < PRF_OUTPUT_LENGTH; j++)
            out[j] ^= ulast[j];
        pkcs5_subsequent_prf(password, password_length, ulast, PRF_OUTPUT_LENGTH, ulast, &outlen);
    }
    for (j = 0; j < PRF_OUTPUT_LENGTH; j++)
        out[j] ^= ulast[j];
}

inline bool libaan::crypto::pbkdf2_pkcs5::pbkdf2(
    const unsigned char *password, unsigned int password_length,
    const unsigned char *salt, uint64_t salt_length,
    unsigned int iteration_count, unsigned char *derived_key,
    uint64_t derived_key_length)
{
    unsigned long i, l, r;
    unsigned char final[PRF_OUTPUT_LENGTH] = { 0, };
    if (derived_key_length >
        ((((uint64_t)1) << 32) - 1) * PRF_OUTPUT_LENGTH)
        return false;

    l = derived_key_length / PRF_OUTPUT_LENGTH;
    r = derived_key_length % PRF_OUTPUT_LENGTH;
    for (i = 1; i <= l; i++)
        pkcs5_F(password, password_length, salt, salt_length, iteration_count,
                i, derived_key + (i - 1) * PRF_OUTPUT_LENGTH);
    if (r) {
        pkcs5_F(password, password_length, salt, salt_length, iteration_count,
                i, final);
        for (l = 0; l < r; l++)
            *(derived_key + (i - 1) *PRF_OUTPUT_LENGTH + l) = final[l];
    }
    return true;
}

inline bool libaan::crypto::pbkdf2_pkcs5::pbkdf2(
    const std::string &pw, const std::string &salt,
    unsigned int iteration_count, std::string &derived_key)
{
    return pbkdf2(
        reinterpret_cast<const unsigned char *>(pw.c_str()), pw.length(),
        reinterpret_cast<const unsigned char *>(salt.c_str()), salt.length(),
        iteration_count, reinterpret_cast<unsigned char *>(&derived_key[0]),
        derived_key.length());
}

#endif
