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

#ifndef _LIBAAN_CRYPTO_HASH_HH_
#define _LIBAAN_CRYPTO_HASH_HH_

#include <openssl/evp.h>
#include <string>

namespace libaan {
namespace crypto {
class hash {
public:
    hash()
    {
        OpenSSL_add_all_algorithms();
    }
    bool sha1(const std::string &in, std::string &out) const;
    bool sha1_hmac(const std::string &cipher_text_in, const std::string &key,
                   std::string &hmac_out) const;

private:
    bool do_hash(const EVP_MD *md, const std::string &in,
                        std::string &out) const;
    bool do_hmac(const EVP_MD *md, const std::string &cipher_text_in,
                 const std::string &key, std::string &hmac_out) const;
};
}
}

#include <openssl/hmac.h>
#include <openssl/sha.h>
// https://groups.google.com/forum/#!topic/mailing.openssl.users/QjC9p14dOGI
// https://www.openssl.org/docs/crypto/EVP_DigestInit.html#EXAMPLE
inline bool libaan::crypto::hash::sha1(const std::string &in,
                                       std::string &out) const
{
    //out.resize(SHA_DIGEST_LENGTH);
    out.resize(EVP_MAX_MD_SIZE);
    return do_hash(EVP_sha1(), in, out);
}

// http://security.stackexchange.com/a/20301
inline bool libaan::crypto::hash::sha1_hmac(const std::string &cipher_text_in,
                                            const std::string &key,
                                            std::string &hmac_out) const
{
    hmac_out.resize(EVP_MAX_MD_SIZE);
    return do_hmac(EVP_sha1(), cipher_text_in, key, hmac_out);
}
inline bool libaan::crypto::hash::do_hmac(const EVP_MD *md,
                                          const std::string &cipher_text_in,
                                          const std::string &key,
                                          std::string &hmac_out) const
{
    if(!md)
        return false;

    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);

    if(HMAC_Init_ex(&ctx, key.data(), key.length(), md, nullptr) != 1) {
        std::cerr << "Error: HMAC_INIT_ex failed.\n";
        return false;
    }

    if(HMAC_Update(&ctx, reinterpret_cast<const unsigned char *>(cipher_text_in.data()),
                   cipher_text_in.length()) != 1) {
        std::cerr << "Error: HMAC_Update failed.\n";
        return false;
    }

    unsigned int len = hmac_out.length();
    if(HMAC_Final(&ctx, reinterpret_cast<unsigned char *>(&hmac_out[0]), &len) != 1) {
        std::cerr << "Error: HMAC_Final failed.\n";
        return false;
    }
    hmac_out.resize(len);

    HMAC_CTX_cleanup(&ctx);

    return true;
}

inline bool libaan::crypto::hash::do_hash(const EVP_MD *md,
                                          const std::string &in,
                                          std::string &out) const
{
    if(!md)
        return false;

    EVP_MD_CTX mdctx;
    EVP_MD_CTX_init(&mdctx);
    EVP_DigestInit_ex(&mdctx, md, nullptr);
    EVP_DigestUpdate(&mdctx,
                     reinterpret_cast<const unsigned char *>(in.data()),
                     in.length());
    unsigned int md_len = -1;
    EVP_DigestFinal_ex(&mdctx, reinterpret_cast<unsigned char *>(&out[0]),
                       &md_len);
    EVP_MD_CTX_cleanup(&mdctx);

    out.resize(md_len);

    return true;
}

#endif
