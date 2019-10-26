#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>

#include <mbr.h>

uint8_t *read_file_into_memory(const char *filename, size_t *outFileSize)
{
    FILE *file = fopen(filename, "rb");
    if (file == 0)
    {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t *result = (uint8_t *)malloc(fileSize);
    fread(result, 1, fileSize, file);
    fclose(file);
    if (outFileSize != 0)
    {
        *outFileSize = fileSize;
    }
    return result;
}

int32_t main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("must provide output filename\n");
        return -1;
    }

    if (argc < 3)
    {
        printf("must provide boot loader filename\n");
        return -1;
    }

    const char *output_filename = argv[1];
    const char *bootloader_filename = argv[2];

    size_t bootloader_code_size;
    uint8_t *bootloader_code = read_file_into_memory(bootloader_filename, &bootloader_code_size);
    if (!bootloader_code)
    {
        printf("could not open bootloader_code: %s\n", bootloader_filename);
        return -1;
    }

    printf("bootloader_code_size: %dbytes\n", (int32_t)bootloader_code_size);
    assert(bootloader_code_size <= 440);

    MBR mbr = {};
    memcpy(&mbr.bootstrap_code, bootloader_code, bootloader_code_size);
    mbr.valid_bootsector_signature = MBR_VALID_BOOTSECTOR_SIGNATURE;

    FILE *file = fopen(output_filename, "wb");
    if (file == 0)
    {
        printf("could not open file for writing: %s\n", output_filename);
        return -1;
    }

    fwrite(&mbr, 1, sizeof(MBR), file);
    fclose(file);

    return 0;
}