#include "revert_string.h"
#include <string.h>

void RevertString(char *str) {
	if (str == NULL)
        return;

    for (size_t i = 0, len = strlen(str); i < len / 2; i++)
    {
        
        char x = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = x;
    }
}

