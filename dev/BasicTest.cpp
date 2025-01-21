
#include <iostream>
#include <msctf.h>
#include <string>
#include <windows.h>

void fun1(int *out, int in1)
{
    *out += in1;
}

void fun2(char in1, char *out)
{
    int len      = lstrlenA(out);
    out[len]     = in1;
    out[len + 1] = '\0';
}

int main()
{
    int a = 1;
    int b = 0x5ffff;
    fun1(&a, b);
    char c[10] = {"nihao"};
    fun2('5', c);
    printf("out %d\n", a);
    printf("fun2 out %s\n", c);
    return 0;
}