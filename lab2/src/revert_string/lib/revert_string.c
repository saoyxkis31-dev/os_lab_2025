
#include "revert_string.h"
#include <string.h>

void RevertString(char *str)
{
    if (str == NULL)
        return;

    size_t len = strlen(str);
    if (len < 2)
        return;

    char *left = str;
    char *right = str + len - 1;

    while (left < right)
    {
        char tmp = *left;
        *left = *right;
        *right = tmp;
        left++;
        right--;
    }
}