#include "twiddler.h"

#include <stdlib.h>

struct Twiddler 
{
  int width;
  int height;
  int* indexes;
};

static int twiddle(int* indexes, int stride, int x, int y, int blocksize, int seq);

Twiddler* twiddler_create(int width, int height) 
{
    Twiddler* twiddler = (Twiddler*)malloc(sizeof(Twiddler));
    twiddler->width = width;
    twiddler->height = height;
    twiddler->indexes = (int*)malloc(width*height*sizeof(int));

    int index = 0;
    int x, y;

    if(width < height) 
    {
        for (y=0;y<height;y+=width) 
        {
            index += twiddle(twiddler->indexes, width, 0, y, width, index);
        }
    }
    else 
    {
        for (x=0;x<width;x+=height) 
        {
            index += twiddle(twiddler->indexes, width, x, 0, height, index);
        } 
    }

    return twiddler;
}

void twiddler_destroy(Twiddler* twiddler) 
{
    if(twiddler == NULL)
        return;

    if(twiddler->indexes != NULL)
    {
        free(twiddler->indexes);
        twiddler->indexes = NULL;
    }

    free(twiddler);
    twiddler = NULL;
}

int twiddler_index(Twiddler* twiddler, int i) 
{
    return twiddler->indexes[i];
}

int twiddler_index_xy(Twiddler* twiddler, int x, int y) 
{
    return twiddler->indexes[y * twiddler->width + x];
}

static int twiddle(int* indexes, int stride, int x, int y, int blocksize, int seq) 
{
    int before = seq;

    if (blocksize == 1) 
    {
        indexes[seq] = y * stride + x;
        seq++;
    } 
    else 
    {
        blocksize = blocksize >> 1;
        seq += twiddle(indexes, stride, x, y, blocksize, seq);
        seq += twiddle(indexes, stride, x, y + blocksize, blocksize, seq);
        seq += twiddle(indexes, stride, x + blocksize, y, blocksize, seq);
        seq += twiddle(indexes, stride, x + blocksize, y + blocksize, blocksize, seq);
    }
     
    return (seq - before);
}
