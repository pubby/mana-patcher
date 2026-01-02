#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct uf2_block 
{
    uint32_t magic_start[2];
    uint32_t flags;
    uint32_t target_addr;
    uint32_t payload_size;
    uint32_t block_number;
    uint32_t num_blocks;
    uint32_t file_size; // or family_id;
    uint8_t data[476];
    uint32_t magic_end;
};

int has_extension(const char *filename, const char *ext)
{
    size_t flen = strlen(filename);
    size_t elen = strlen(ext);
    if (flen < elen) 
        return 0;
    return strcmp(filename + flen - elen, ext) == 0;
}

int main(int argc, char *argv[])
{
    bool debug = false;

    if(argc < 2 || argc > 4 || !has_extension(argv[1], ".nes"))
    {
    invalid:
        fprintf(stderr, "Invalid arguments. Use:\n%s [in *.nes] [in *.uf2] [out .uf2]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char const* nes_filename = argv[1];
    char const* uf2_in_filename = "firmware.uf2";
    char* uf2_out_filename;

    if(argc >= 3)
    {
         if(!has_extension(argv[2], ".uf2"))
             goto invalid;
         uf2_in_filename = argv[2];
    }

    if(argc >= 4)
    {
         if(!has_extension(argv[3], ".uf2"))
             goto invalid;
         uf2_out_filename = argv[3];
    }
    else
    {
        unsigned len = strlen(nes_filename);
        uf2_out_filename = malloc(len + 5);
        strcpy(uf2_out_filename, nes_filename);
        strcpy(uf2_out_filename + len, ".uf2");
    }

    FILE* nes  = fopen(nes_filename, "rb");

    if(!nes) 
    {
        fprintf(stderr, "Unable to open .nes file: \"%s\"\n", nes_filename);
        return EXIT_FAILURE;
    }

    FILE* in  = fopen(uf2_in_filename, "rb");

    if(!in) 
    {
        fprintf(stderr, "Unable to open input .uf2 file: \"%s\"\n", uf2_in_filename);
        fclose(nes);
        return EXIT_FAILURE;
    }

    FILE* out = fopen(uf2_out_filename, "wb");

    if(!out) 
    {
        fprintf(stderr, "Unable to open output .uf2 file: \"%s\"\n", uf2_out_filename);
        fclose(nes);
        fclose(in);
        return EXIT_FAILURE;
    }

    unsigned bytes_written = 0;

    while(true) 
    {
        struct uf2_block block;

        if(feof(in) || fread(&block, sizeof(block), 1, in) != 1)
            break;

        if(block.magic_start[0] != 0x0A324655
        || block.magic_start[1] != 0x9E5D5157
        || block.magic_end      != 0x0AB16F30)
        {
            if(debug)
                fprintf(stderr, "Invalid magic number.\n");
            goto write;
        }

        if(block.payload_size > sizeof(block.data))
        {
            if(debug)
                fprintf(stderr, "Invalid payload size.\n");
            goto write;
        }

        if(block.flags & 1) // Not in flash.
        {
            if(debug)
                fprintf(stderr, "Not in flash.\n");
            goto write;
        }
        
        if(block.flags & 0x4000) // MD5 required.
        {
            if(debug)
                fprintf(stderr, "Requires MD5.\n");
            goto write;
        }

        if(block.target_addr < 0x10100000)
        {
            if(debug)
                fprintf(stderr, "Low address.\n");
            goto write;
        }

        unsigned offset = block.target_addr - 0x10100000;
        if(fseek(nes, offset, SEEK_SET) != 0)
            goto write;

        bytes_written += fread(block.data, 1, block.payload_size, nes);

    write:

        if(!fwrite(&block, sizeof(block), 1, out))
        {
            fprintf(stderr, "Error writing to file.\n");
            break;
        }
    }

    fclose(nes);
    fclose(in);
    fclose(out);

    if(debug)
        fprintf(stderr, "Successfully wrote %i bytes.\n", bytes_written);

    return EXIT_SUCCESS;
}
