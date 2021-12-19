#include <stdio.h> // fopen, fread, ...
#include <stdlib.h> // malloc, free, ...
#include <string.h> // for memcmp
#include <stdint.h> // for uint8_t


typedef uint8_t ubyte;

struct Pixel_arr {
    ubyte* data;
    size_t width;
    size_t height;
    ubyte format;
    size_t colmap_width;
    size_t colmap_size;
    ubyte* color_data;
};

typedef union PixelInfo
{
    struct
    {
        ubyte R, G, B, A;
    };
} *PPixelInfo;

typedef struct Pixel_arr PixelArray;

int main()
{
    char c; // 1 byte
    unsigned char u; // 1 byte

    FILE* fp = fopen("fern.tga", "rb");
    FILE* outfile = NULL;
    if (fp == NULL) {
        fprintf(stderr, "cannot open file\n");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp); //get last position in file

    fseek(fp, 0, SEEK_SET);
    ubyte* data = (ubyte*)malloc(fsize); //size of ubyte is 1 btw
    const ubyte tga_sig[3] = {0, 0, 2};
    const ubyte tga_color_sig[3] = {0, 1, 2};//for non color tga
    const size_t tga_header_size = 18;
    PixelArray pix;
    pix.data = NULL;

    int res = fread(data, 1, fsize, fp); //this is where data get inputt
    //test
    //for(int i=0; i<19; i++){
        //printf("%d=%d ", i, data[i]);
   // }
    printf("\n");
    //for(int i=19; i<10000; i++){
       // printf("%d ", data[i]);
   // }
    if (res < fsize) {
        fprintf(stderr, "invalid tga file\n");
        goto done;
    }

    if (data[1] = 0){
        printf(" no color-map data is included with this image\n");
        res = memcmp(tga_sig, data, sizeof(tga_sig));
    }
    else if (data[1] = 1){
        printf("a color-map is included with this image\n");
        res = memcmp(tga_color_sig, data, sizeof(tga_sig));
    }
    else{
        fprintf(stderr, "invalid tga file\n");
        goto done;
    }

    if (res != 0) {
        fprintf(stderr, "invalid tga file\n");
        goto done;
    }

    pix.colmap_width = data[5] + (data[6] << 8);
    pix.colmap_size = data[7];
    pix.width = data[12] + (data[13] << 8);
    pix.height = data[14] + (data[15] << 8);
    pix.format = data[16];
    if (pix.format != 24 && pix.format != 32){
        fprintf(stderr, "unsupported tga format\n");
        goto done;
    }

    size_t image_size = pix.width * pix.height * pix.format / 8;
    if ((tga_header_size + image_size) > fsize) {
        fprintf(stderr, "invalid tga file size\n");
        goto done;
    }

    pix.data = (ubyte*)malloc(image_size);
    memcpy(pix.data, data + 18, image_size);

    ubyte buffer[18] = {};
    memcpy(buffer, tga_sig, sizeof(tga_sig));
    buffer[12] = pix.width;
    buffer[13] = pix.width >> 8; // = pix.width/(2^8)
    buffer[14] = pix.height;
    buffer[15] = pix.height >> 8;
    buffer[16] = pix.format;
    //เอาจิงๆ pix.data คือ เริ่มตัว 19 แล้ว เดี๋ยวมาแก้
    ubyte* color_data = (ubyte*)malloc(sizeof(pix.data));
    PPixelInfo Pixel = {0};
    int CurrentByte = 0;
    size_t CurrentPixel = 0;
    ubyte ChunkHeader = {0};
    int BytesPerPixel = data[16]/ 8;

    while(CurrentPixel < (pix.colmap_width * pix.colmap_size)){
        if(ChunkHeader < 128)
        {
            ++ChunkHeader;
            fread(ChunkHeader, 1, pix.colmap_width, fp);
            for(int I = 0; I < ChunkHeader; ++I, ++CurrentPixel)
            {
                fread(Pixel, BytesPerPixel, 1, fp);
                color_data[CurrentByte++] = Pixel->B;
                color_data[CurrentByte++] = Pixel->G;
                color_data[CurrentByte++] = Pixel->R;
                if (data[16] > 24) color_data[CurrentByte++] = Pixel->A;
            }
        }
        else
        {
            ChunkHeader -= 127;
            for(int I = 0; I < ChunkHeader; ++I, ++CurrentPixel)
            {
                fread(Pixel, BytesPerPixel, 1, fp);
                color_data[CurrentByte++] = Pixel->B;
                color_data[CurrentByte++] = Pixel->G;
                color_data[CurrentByte++] = Pixel->R;
                if (data[16]  > 24) color_data[CurrentByte++] = Pixel->A;
            }
        }
    } ;


    outfile = fopen("done.tga", "wb");
    fwrite(buffer, 1, sizeof(buffer), outfile);
    fwrite(pix.data, 1, image_size, outfile);

done:
    fclose(outfile);
    free(pix.data);
    free(data);
    fclose(fp);
    return 0;
}
