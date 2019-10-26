#include <stdint.h>
#include <stdio.h>
#include <malloc.h>

#include <mbr.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("must provide output filename\n");
        return 1;
    }

    const char *output_filename = argv[1];

    MBR mbr = {};

    return 0;
}