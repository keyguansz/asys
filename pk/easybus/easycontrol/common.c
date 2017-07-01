#include "common.h"
#include "easy_common.h"


static int hex2num(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static int hex2byte(const char *hex)
{
    int a, b;
    a = hex2num(*hex++);
    if (a < 0)
        return -1;
    b = hex2num(*hex++);
    if (b < 0)
        return -1;
    return (a << 4) | b;
}

/**
 * hexstr2bin - Convert ASCII hex string into binary data
 * @hex: ASCII hex string (e.g., "01ab")
 * @buf: Buffer for the binary data
 * @len: Length of the text to convert in bytes (of buf); hex will be double
 * this size
 * Returns: 0 on success, -1 on failure (invalid hex string)
 */
int hexstr2bin(const char *hex, unsigned char *buf, size_t len)
{
    size_t i;
    int a;
    const char *ipos = hex;
    unsigned char *opos = buf;

    for (i = 0; i < len; i++) {
        a = hex2byte(ipos);
        if (a < 0)
            return -1;
        *opos++ = a;
        ipos += 2;
    }
    return 0;
}

static easy_msg_cb_func easy_msg_cb = NULL;

void easy_msg_register_cb(easy_msg_cb_func func)
{
    easy_msg_cb = func;
}

void easy_msg(void *ctx, char *msgtype, const char *buf, size_t len)
{
    if (ctx == NULL || msgtype == NULL || buf == NULL) {
        return;
    }
    
    EB_LOGD("easy_msg buf = %s\n", buf);
    if (easy_msg_cb) {
        easy_msg_cb(ctx, msgtype, buf, len);
    }
}

static inline int _easy_snprintf_hex(char *buf, size_t buf_size, const unsigned char *data,
                    size_t len, int uppercase)
{
    size_t i;
    char *pos = buf, *end = buf + buf_size;
    int ret;
    if (buf_size == 0)
        return 0;
    for (i = 0; i < len; i++) {
        ret = snprintf(pos, end - pos, uppercase ? "%02X" : "%02x",
                  data[i]);
        if (ret < 0 || ret >= end - pos) {
            end[-1] = '\0';
            return pos - buf;
        }
        pos += ret;
    }
    end[-1] = '\0';
    return pos - buf;
}

/**
 * easy_easy_snprintf_hex - Print data as a hex string into a buffer
 * @buf: Memory area to use as the output buffer
 * @buf_size: Maximum buffer size in bytes (should be at least 2 * len + 1)
 * @data: Data to be printed
 * @len: Length of data in bytes
 * Returns: Number of bytes written
 */
int easy_snprintf_hex(char *buf, size_t buf_size, const unsigned char *data, size_t len)
{
    return _easy_snprintf_hex(buf, buf_size, data, len, 0);
}

int get_time(struct easy_time *t)
{
    int res;
    struct timeval tv;
    res = gettimeofday(&tv, NULL);
    t->sec = tv.tv_sec;
    t->usec = tv.tv_usec;
    return res;
}

