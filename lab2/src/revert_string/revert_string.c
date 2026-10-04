#include <stddef.h>
#include <string.h>

#include "revert_string.h"

void RevertString(char *str)
{
    size_t length = strlen(str);

    for (size_t left = 0; left < length / 2; ++left)
    {
        size_t right = length - 1 - left;

        char temp = str[left];
        str[left] = str[right];
        str[right] = temp;
    }
}