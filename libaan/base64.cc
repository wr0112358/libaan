#include "base64.hh"

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

size_t libaan::base64decode(const void* in, const size_t in_len, char* out, const size_t out_len)
{
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *mem = BIO_new(BIO_s_mem());
    const auto w = BIO_write(mem, in, in_len);
    assert(w > 0 && (size_t)w == in_len);
    BIO_push(b64, mem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    const auto r = BIO_read(b64, out, out_len);
    BIO_free_all(b64);
    return r > 0 ? r : 0;
}

size_t libaan::base64encode(const void* in, const size_t in_len, char* out, const size_t out_len)
{
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *mem = BIO_new(BIO_s_mem());
    mem = BIO_push(b64, mem);
    BIO_set_flags(mem, BIO_FLAGS_BASE64_NO_NL);
    const auto w = BIO_write(mem, in, in_len);
    assert(w > 0 && (size_t)w == in_len);
    (void)BIO_flush(mem);

    BUF_MEM *buf;
    BIO_get_mem_ptr(mem, &buf);
    const auto r = std::min(buf->length, out_len);
    memcpy(out, buf->data, r);
    BIO_free_all(mem);
    return r > 0 ? r : 0;
}
