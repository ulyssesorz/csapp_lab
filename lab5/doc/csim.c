#include "cachelab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdint.h>
#include <assert.h>
char *path;
char str[1024];

int hit = 0;
int miss = 0;
int evict = 0;

int s, E, b, S;

typedef struct
{
    int valid;
    int tag;
    int stamp;
} Line;

Line **cache = NULL;

void unitCache(int S, int E)
{
    cache = (Line **)malloc(sizeof(Line *) * S);
    for (int i = 0; i < S; i++)
    {
        cache[i] = (Line *)malloc(sizeof(Line) * E);
        for (int j = 0; j < E; j++)
        {
            cache[i][j].valid = 0;
            cache[i][j].tag = -1;
            cache[i][j].stamp = -1;
        }
    }
}

void freeCache(int S)
{
    for (int i = 0; i < S; i++)
    {
        free(cache[i]);
    }
    free(cache);
}

void updateStamp(int s, int E)
{
    for (int i = 0; i < 1 << s; i++)
    {
        for (int j = 0; j < E; j++)
        {
            if (cache[i][j].valid == 1)
                cache[i][j].stamp++;
        }
    }
}

int getTag(uint64_t addr, int s, int b)
{
    return addr >> (s + b);
}

int getIndex(uint64_t addr, int s, int b)
{
    return (addr >> b) & ((1 << s) - 1);
}

void updateCache(uint64_t addr, int s, int b, int E)
{
    int tag = getTag(addr, s, b);
    int index = getIndex(addr, s, b);
    // tag匹配说明命中
    for (int i = 0; i < E; i++)
    {
        if (cache[index][i].tag == tag)
        {
            cache[index][i].stamp = 0;
            hit++;
            return;
        }
    }
    //未命中，搜索valid=0的空闲块
    for (int i = 0; i < E; i++)
    {
        if (cache[index][i].valid == 0)
        {
            cache[index][i].valid = 1;
            cache[index][i].tag = tag;
            cache[index][i].stamp = 0;
            miss++;
            return;
        }
    }
    //无空闲块，找最大stamp
    int max_stamp = INT_MIN;
    int max_idx = -1;
    for (int i = 0; i < E; ++i)
    {
        if (cache[index][i].stamp > max_stamp)
        {
            max_stamp = cache[index][i].stamp;
            max_idx = i;
        }
    }
    cache[index][max_idx].tag = tag;
    cache[index][max_idx].stamp = 0;
    miss++;
    evict++;
    return;
}

void printSummary(int hits, int misses, int evictions)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    FILE *output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
}

int char2int(char c)
{
    return c - '0';
}

int main(int argc, char *argv[])
{
    int s, E, b, size;
    char op;
    uint64_t address;
    FILE *fp;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            char tag = argv[i][1];
            switch (tag)
            {
            case 's': //提取s和S
                s = char2int(argv[++i][0]);
                break;
            case 'E':
                E = char2int(argv[++i][0]);
                break;
            case 'b':
                b = char2int(argv[++i][0]);
                break;
            case 't':
                path = argv[++i];
                break;
            default:
                break;
            }
        }
        if (i > argc)
            break;
    }
    unitCache(1 << s, E);
    fp = fopen(path, "r");
    if (fp == NULL)
    {
        printf("open error");
        exit(-1);
    }
    while (fgets(str, 100, fp) != NULL)
    {
        sscanf(str, " %c %lx,%d", &op, &address, &size); //处理我们读入的每一行每一列。
        switch (op)
        {
        case 'I':
            continue;
        case 'L':
        case 'S':
            updateCache(address, s, b, E);
            break;
        case 'M':
            updateCache(address, s, b, E);
            updateCache(address, s, b, E);
            break;
        }
        updateStamp(s, E);
    }
    fclose(fp);
    freeCache(1 >> s);
    printSummary(hit, miss, evict);
    return 0;
}