#include "crypto.hh"

#include <openssl/err.h>

//#include <cassert>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>


#ifdef LION_ENABLED
#include <openssl/rc4.h>
#endif

#ifndef NO_GOOD
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h> /* for htonl */

#else

#include <windows.h>
#include <wincrypt.h>
#include <winsock2.h>

namespace {

// since getpass(3) is deprecated, maybe use something self written.
// or read_passphrase(...) from this truecrypt fork looks good at first glance.
// https://github.com/bwalex/tc-play/blob/39125738741a43ee5888b862ab998854ba5ccf3b/io.c#L393
//
// or openbsd/signify/readpasphrase.c

char *getpass(const char *prompt)
{
    static char buffer[255];
    memset(buffer, 0, 255);

    fprintf(stderr, "%s", prompt);

    // Disable character echoing and line buffering
    HANDLE hstdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;

    if(!GetConsoleMode(hstdin, &mode))
        return 0;
    if(hstdin == INVALID_HANDLE_VALUE
       || !(SetConsoleMode(hstdin,
                           mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT))))
        return 0; // Failed to disable buffering

    DWORD dwread;
    std::size_t i = 0;
    unsigned char c = 0;
    while(ReadConsoleA(hstdin, &c, 1, &dwread, nullptr)) {
        if(c == '\n' || c == '\r' || i == 254) {
            buffer[i] = '\0';
            break;
        }
        if(c == (char)8 && i) {
            --i;
            putchar('\b');
        }
        buffer[i] = c;
        putchar('*');

        ++i;
    }
    if(buffer[0])
        puts("");

    if (!SetConsoleMode(hstdin, mode))
        return 0;

    return buffer;
}

std::string err_string(DWORD error)
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

void win_err(const std::string &msg)
{
    const auto err = GetLastError();
    std::cerr << "\"" << msg << "\"\n"
              << "Error(" << err << "): " << err_string(err);
}

}

#endif

libaan::password_from_stdin::password_from_stdin
(size_t pw_minlength, const std::string &prompt)
{
    have_password = false;
    // points to static pw buffer after reading the input.
    char * pw = getpass(prompt.c_str());
    if(!pw || (std::strlen(pw) < pw_minlength))
        return;
    password = pw;
    for(char * p = pw; *p;)
        *p++ = '\0';

    have_password = true;
}

libaan::password_from_stdin::~password_from_stdin()
{
    std::fill(std::begin(password), std::end(password), '\0');
}

// https://groups.google.com/forum/#!topic/mailing.openssl.users/QjC9p14dOGI
// https://www.openssl.org/docs/crypto/EVP_DigestInit.html#EXAMPLE
bool libaan::hash::sha1(const std::string &in,
                        std::string &out) const
{
    //out.resize(SHA_DIGEST_LENGTH);
    out.resize(EVP_MAX_MD_SIZE);
    return do_hash(EVP_sha1(), in, out);
}

// http://security.stackexchange.com/a/20301
bool libaan::hash::sha1_hmac(const std::string &cipher_text_in,
                                            const std::string &key,
                                            std::string &hmac_out) const
{
    hmac_out.resize(EVP_MAX_MD_SIZE);
    return do_hmac(EVP_sha1(), cipher_text_in, key, hmac_out);
}

bool libaan::hash::do_hmac(const EVP_MD *md,
                                          const std::string &cipher_text_in,
                                          const std::string &key,
                                          std::string &hmac_out) const
{
    if(!md)
        return false;

    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);

    if(HMAC_Init_ex(&ctx, key.data(), key.length(), md, nullptr) != 1)
        return false;

    if(HMAC_Update(&ctx, reinterpret_cast<const unsigned char *>(cipher_text_in.data()),
                   cipher_text_in.length()) != 1)
        return false;

    unsigned int len = hmac_out.length();
    if(HMAC_Final(&ctx, reinterpret_cast<unsigned char *>(&hmac_out[0]), &len) != 1)
        return false;
    hmac_out.resize(len);

    HMAC_CTX_cleanup(&ctx);

    return true;
}

bool libaan::hash::do_hash(const EVP_MD *md,
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

// sha1 can still be used for hmac construction:
// http://tools.ietf.org/html/rfc2104#section-6
const std::size_t libaan::hmac::SIZE { SHA_DIGEST_LENGTH };

libaan::hmac::hmac(const std::string &key, std::string &hmac_out)
    : out(hmac_out)
{
    out.resize(SIZE);
    HMAC_CTX_init(&ctx);

    if(HMAC_Init_ex(&ctx, key.data(), key.length(), EVP_sha1(), nullptr) != 1)
        return;
    state = true;
}

libaan::hmac::~hmac()
{
    unsigned int len = out.length();
    if(!state
       || (HMAC_Final(&ctx, reinterpret_cast<unsigned char *>(&out[0]),
                      &len) != 1))
        out.resize(0);
    else
        out.resize(len);

    HMAC_CTX_cleanup(&ctx);
}

bool libaan::hmac::update(const std::string &cipher_text_in)
{
    if(!state)
        return false;

    if(HMAC_Update(&ctx, reinterpret_cast<const unsigned char *>
                   (cipher_text_in.data()), cipher_text_in.length()) != 1) {
        state = false;
        return false;
    }
    return true;
}

void libaan::pkcs5_initial_prf(
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

void libaan::pkcs5_subsequent_prf(
    const unsigned char *password, size_t password_length, unsigned char *v,
    size_t vlen, unsigned char *o, size_t *olen)
{
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init(&ctx, password, password_length, EVP_sha1());
    HMAC_Update(&ctx, v, vlen);
    HMAC_Final(&ctx, o, (unsigned int *)olen);
}

void libaan::pkcs5_F(const unsigned char *password,
                     size_t password_length,
                     const unsigned char *salt,
                     size_t salt_length, size_t ic,
                     size_t bix,
                     unsigned char *out)
{
    size_t i = 1, j, outlen;
    unsigned char ulast[PRF_OUTPUT_LENGTH];
    memset(out, 0, PRF_OUTPUT_LENGTH);
    pkcs5_initial_prf(password, password_length, salt, salt_length, bix, ulast,
                      &outlen);
    while (i++ <= ic) {
        for (j = 0; j < PRF_OUTPUT_LENGTH; j++)
            out[j] ^= ulast[j];
        pkcs5_subsequent_prf(password, password_length, ulast,
                             PRF_OUTPUT_LENGTH, ulast, &outlen);
    }
    for (j = 0; j < PRF_OUTPUT_LENGTH; j++)
        out[j] ^= ulast[j];
}

bool libaan::pbkdf2(const unsigned char *password,
                    unsigned int password_length,
                    const unsigned char *salt,
                    uint64_t salt_length,
                    unsigned int iteration_count,
                    unsigned char *derived_key,
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

#ifndef NO_GOOD
bool libaan::read_random_bytes_noblock(size_t count, std::string & bytes)
{
    bytes.resize(count);

    std::ifstream f("/dev/urandom",
                    std::ios_base::in | std::ios_base::binary);
    f.read(&bytes[0], count);
    return !!f;
}
#endif

bool libaan::read_random_bytes(size_t count, std::string & bytes)
{
    bytes.resize(count);

#ifndef NO_GOOD
    const std::string REAL_RANDOM_NUMBERS = "/dev/random";
    const std::string PSEUDO_RANDOM_NUMBERS = "/dev/urandom";

    std::ifstream f(REAL_RANDOM_NUMBERS,
                    std::ios_base::in | std::ios_base::binary);
    f.read(&bytes[0], count);
    return !!f;
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

bool libaan::read_random_ascii_set(const size_t count, const std::string &set,
                                   std::string &bytes)
{
    if(count == 0 || set.empty())
        return true;
    bytes.resize(count);
    size_t fill = 0;
    while(fill < count) {
        const auto rand = ranf<std::string>(count * 2,
                                            *std::min_element(std::begin(set), std::end(set)),
                                            *std::max_element(std::begin(set), std::end(set)));

        const auto size = std::min(count - fill, rand.size());

        for(size_t i = 0; i < size; i++) {
            if(set.find(rand[i]) != std::string::npos) {
                bytes[fill] = rand[i];
                fill++;
            }
        }

        // std::cout << "read_random_ascii_set: " << fill << " < " << count << "\n";
    }

    // std::cout << "read_random_ascii_set(\"" << set << "\") -> \"" << bytes << "\", count=" << count << ", b.size=" << bytes.size() << "\n";
    return true;
/*
    bytes.resize(count);
    // possible optimization: read more than one at a time and cache it.
    // windows: create context at program start and reuse it
#ifndef NO_GOOD
    const std::string REAL_RANDOM_NUMBERS = "/dev/random";
    const std::string PSEUDO_RANDOM_NUMBERS = "/dev/urandom";

    std::ifstream f(PSEUDO_RANDOM_NUMBERS,
                    std::ios_base::in | std::ios_base::binary);
    std::size_t read = 0;
    do {
        f.read(&bytes[read], 1);
        if(set.find(bytes[read]) != std::string::npos)
            read++;
    } while(read < count);
    return !!f;
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
*/
}

// TODO: version for incremental encryption(see Viega p.186 "incremental_..."
bool libaan::camellia_256::do_encrypt(
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

bool libaan::camellia_256::do_decrypt(EVP_CIPHER_CTX *ctx,
                                      const std::string &cipher_in,
                                      std::string &plain_out)
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

bool libaan::camellia_256::generate_key(const std::string &pw, std::string &key)
{
    const size_t iteration_count = 1000;
    if(!salt.length())
        return false;

    key.resize(KEY_SIZE);

    if(!pbkdf2(pw, salt, iteration_count, key)) {
        std::cout << "pbkdf2 key generation failed.\n";
        return false;
    }

    return true; 
}

bool libaan::camellia_256::new_random_iv()
{
   if(!read_random_bytes(BLOCK_SIZE, iv)) {// iv with block size
        std::cout << "read from /dev/random failed\n";
        return false;
    }
    return true;
}

bool libaan::camellia_256::init()
{
    if(!read_random_bytes(16, salt)) {// 128 bit salt
        std::cout << "read from /dev/random failed\n";
        return false;
    }

    return new_random_iv();
}

bool libaan::camellia_256::init(const std::string &existing_salt,
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

bool libaan::camellia_256::encrypt(const std::string &pw, const std::string &plain,
                                   std::string &cipher)
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

bool libaan::camellia_256::decrypt(const std::string &pw, const std::string &cipher,
                                   std::string &plain)
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


#ifdef LION_ENABLED

inline bool libaan::lion::check_file_size(size_t file_size)
{
//    if(file_size % CHUNK_SIZE)
//        return false;
    if(file_size < HASH_SIZE)
        return false;
    return true;
}

void libaan::lion::encrypt(const unsigned char *input_buffer,
                           unsigned char *output_buffer, size_t block_length,
                           const unsigned char *key)
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

void libaan::lion::decrypt(const unsigned char *input_buffer,
                           unsigned char *output_buffer, size_t block_length,
                           const unsigned char *key)
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
