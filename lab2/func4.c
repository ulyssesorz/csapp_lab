#include<stdio.h>

int func4(int target, int a, int b)
{
    int ret = b - a;
    int k = (unsigned)(ret) >> 31;
    ret = (ret + k) >> 1;
    k = ret + a;
    if(k > target)
        return 2 * func4(target, a, k - 1);
    ret = 0;
    if(k < target)    
        return 2 * func4(target, k + 1, b) + 1;
    return ret;
}
int main()
{
    for(int i = 0; i <= 14; i++)
    {
        if(func4(i, 0, 14) == 0)
            printf("%d\n", i);
    }
    return 0;
}