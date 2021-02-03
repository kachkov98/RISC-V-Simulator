#include <stdio.h>

int main()
{
    unsigned long long a1 = 1, a2 = 1;
    for (unsigned i = 0; i < 10000000; ++i)
    {
        unsigned tmp = a1;
        a1 = a2;
        a2 += tmp;
    }
    printf("%llu\n", a2);
    return 0;
}
