#include <limits.h>
#include <stdio.h>

#define FOR(i, a, n) for (int i = a; i < n; i++)

FILE* urandom;

static inline unsigned long _rand()
{
    if (!urandom)
        urandom = fopen("/dev/urandom", "rb");
    unsigned long buf;
    fread(&buf, sizeof(unsigned long), 1, urandom);
    return buf;
}

static inline int randint(int a, int b)
{
    return a + _rand() % (b - a + 1);
}

static inline double randdbl(double a, double b)
{
    return a + (double) _rand() / ULONG_MAX * (b - a);
}

int main()
{
    int N = 1000, D = 100;
    FOR (i, 0, N) {
        int res = randint(0, 1);
        if (res == 0)
            res--;

        printf("%d ", res);
        FOR (j, 1, D + 1) {
            int cont = randint(0, 1);
            if (cont)
                continue;
            printf("%d:%f ", j, randdbl(0, 100));
        }
        puts("");
    }
    return 0;
}
