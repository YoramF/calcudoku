#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct __digit
{
    char digit;
    char count;
} _digit;

typedef struct __combHdr
{
    unsigned short mask;
    int nDigits;
} _combHdr;

typedef struct __comb
{
    _combHdr hdr;
    _digit Digits[1];
} _comb;

void simpleSort(int *digits, int m)
{
    int i, j, k, t;

    for (i = 0; i < m - 1; i++)
    {
        k = i;
        for (j = i + 1; j < m; j++)
        {
            if (digits[k] > digits[j])
                k = j;
        }

        if (k > i)
        {
            t = digits[k];
            digits[k] = digits[i];
            digits[i] = t;
        }
    }
}

int main ()
{
    int a[50];
    int i;
    time_t t;

    long long *lp1, *lp2;
    int *ip1, *ip2;
    short *sp1, *sp2;
    char *cp1, *cp2;

    _comb *cPtr1, *cPtr2;

    srand((unsigned)time(&t));

    for (i = 0; i < 50; i++)
    {
        a[i] = rand() % 50;
    }

    for (i = 0; i < 50; i++)
        printf("[%d] %d\n",i, a[i]);

    simpleSort(a, 50);

    printf("sorting....\n");
    for (i = 0; i < 50; i++)
        printf("[%d] %d\n", i, a[i]);

    lp1 = malloc(sizeof(long long));
    lp2 = malloc(sizeof(long long));
    ip1 = malloc(sizeof(int));
    ip2 = malloc(sizeof(int));
    sp1 = malloc(sizeof(short));
    sp2 = malloc(sizeof(short));
    cp1 = malloc(sizeof(char));
    cp2 = malloc(sizeof(char));

    printf("long long: %d %d\n", sizeof(long long),  (long long)lp2 - (long long)lp1);
    printf("int: %d %d\n", sizeof(int), (long long)ip2 - (long long)ip1);
    printf("short: %d %d\n", sizeof(short), (long long)sp2 - (long long)sp1);
    printf("char: %d %d\n", sizeof(char), (long long)cp2 - (long long)cp1);

    cPtr1 = malloc(sizeof(_combHdr)+sizeof(_digit)*12);
    cPtr2 = malloc(sizeof(_combHdr) + sizeof(_digit) * 6);
    cPtr1->hdr.mask = 10;
    cPtr1->hdr.nDigits = 12;
    cPtr2->hdr.mask = 20;
    cPtr2->hdr.nDigits = 6;

    for (i=0; i < 12; i++)
    {
        cPtr1->Digits[i].count = i + 1;
        cPtr1->Digits[i].digit = i;
    }

    for (i = 0; i < 6; i++)
    {
        cPtr2->Digits[i].count = 2*i + 1;
        cPtr2->Digits[i].digit = i;
    }

    printf("cPtr1: %d, %d\n", cPtr1->hdr.mask, cPtr1->hdr.nDigits);
    for (i=0; i<cPtr1->hdr.nDigits; i++)
        printf("[%d], [%d]\n", cPtr1->Digits[i].digit, cPtr1->Digits[i].count);

    printf("cPtr2: %d, %d\n", cPtr2->hdr.mask, cPtr2->hdr.nDigits);
    for (i = 0; i < cPtr2->hdr.nDigits; i++)
        printf("[%d], [%d]\n", cPtr2->Digits[i].digit, cPtr2->Digits[i].count);
}

