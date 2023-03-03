#include "printk.h"
#include "sbi.h"

// 使用0x01进行system call， 输出一个ascii字符
#define putchar(c) sbi_ecall(0x01, 0x00, (c), 0, 0, 0, 0, 0)



void puts(char *s)
{
    // 输出每个字符
    while (*s)
    {
        putchar(*s);
        ++s;
    }
}

void puti(int x)
{
    // 记录数字的缓冲区
    static char num_buf[32];
    int i = 0;
    int neg_flag = 0;
    if (!x)
        putchar('0'); // 0 的特殊处理
    if (x < 0)
    {
        neg_flag = 1; // 负号处理
        x = -x;
    }
    // 提取每一位
    while (x)
    {
        num_buf[i++] = '0' + (x % 10);
        x /= 10;
    }

    if (neg_flag)
        putchar('-');
    i -= 1;
    // 倒序输出
    for (; i >= 0; i--)
        putchar(num_buf[i]);
    putchar('\n');
}

void putc(char c)
{
    sbi_ecall(SBI_PUTCHAR, 0, c, 0, 0, 0, 0, 0);
}

static int vprintfmt(void (*putch)(char), const char *fmt, va_list vl)
{
    int in_format = 0, longarg = 0;
    size_t pos = 0;
    for (; *fmt; fmt++)
    {
        if (in_format)
        {
            switch (*fmt)
            {
            case 'l':
            {
                longarg = 1;
                break;
            }

            case 'x':
            {
                long num = longarg ? va_arg(vl,
                                            long)
                                   : va_arg(vl,
                                            int);

                int hexdigits = 2 * (longarg ? sizeof(long) : sizeof(int)) - 1;
                for (int halfbyte = hexdigits; halfbyte >= 0; halfbyte--)
                {
                    int hex = (num >> (4 * halfbyte)) & 0xF;
                    char hexchar = (hex < 10 ? '0' + hex : 'a' + hex - 10);
                    putch(hexchar);
                    pos++;
                }
                longarg = 0;
                in_format = 0;
                break;
            }

            case 'd':
            {
                long num = longarg ? va_arg(vl,
                                            long)
                                   : va_arg(vl,
                                            int);
                if (num < 0)
                {
                    num = -num;
                    putch('-');
                    pos++;
                }
                int bits = 0;
                char decchar[25] = {'0', 0};
                for (long tmp = num; tmp; bits++)
                {
                    decchar[bits] = (tmp % 10) + '0';
                    tmp /= 10;
                }

                for (int i = bits; i >= 0; i--)
                {
                    putch(decchar[i]);
                }
                pos += bits + 1;
                longarg = 0;
                in_format = 0;
                break;
            }

            case 'u':
            {
                unsigned long num = longarg ? va_arg(vl,
                                                     long)
                                            : va_arg(vl,
                                                     int);
                int bits = 0;
                char decchar[25] = {'0', 0};
                for (long tmp = num; tmp; bits++)
                {
                    decchar[bits] = (tmp % 10) + '0';
                    tmp /= 10;
                }

                for (int i = bits; i >= 0; i--)
                {
                    putch(decchar[i]);
                }
                pos += bits + 1;
                longarg = 0;
                in_format = 0;
                break;
            }

            case 's':
            {
                const char *str = va_arg(vl,
                                         const char *);
                while (*str)
                {
                    putch(*str);
                    pos++;
                    str++;
                }
                longarg = 0;
                in_format = 0;
                break;
            }

            case 'c':
            {
                char ch = (char)va_arg(vl,
                                       int);
                putch(ch);
                pos++;
                longarg = 0;
                in_format = 0;
                break;
            }
            default:
                break;
            }
        }
        else if (*fmt == '%')
        {
            in_format = 1;
        }
        else
        {
            putch(*fmt);
            pos++;
        }
    }
    return pos;
}

int printk(const char *s, ...)
{
    int res = 0;
    va_list vl;
    va_start(vl, s);
    res = vprintfmt(putc, s, vl);
    va_end(vl);
    return res;
}
