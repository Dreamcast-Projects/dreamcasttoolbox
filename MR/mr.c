#include "mr.h"

#include <string.h>
#include <stdlib.h>

#include <stdio.h>

#define PIXEL_SIZE 3 // 24bpp
#define MAX_PALETTE_COLORS 128

typedef struct mr_header_t {
    char mr[2];
    unsigned int total_size;
    unsigned int fill1;
    unsigned int data_offset;
    unsigned int width;
    unsigned int height;
    unsigned int fill2;
    unsigned int color_cnt;
} __attribute__((packed)) mr_header_t;

typedef struct mr_pal_entry_t {
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
} mr_pal_entry_t;

int mr_encode(unsigned char* indexed_data, unsigned char* compressed_data, int idata_size)
{
    int length = 0;
    int position = 0;
    int run = 0;

    while(position < idata_size)
    {
        run = 1;

        while((run < 0x17f) && (position+run < idata_size) && 
              (indexed_data[position] == indexed_data[position+run]))
            run += 1;

        if(run > 0xff)
        {
            compressed_data[length] = 0x82;
            length += 1;
            compressed_data[length] = 0x80 | (run - 0x100);
            length += 1;
            compressed_data[length] = indexed_data[position];
            length += 1;
        }
        else if(run > 0x7f)
        {
            compressed_data[length] = 0x81;
            length += 1;
            compressed_data[length] = run;
            length += 1;
            compressed_data[length] = indexed_data[position];
            length += 1;
        }  
        else if(run > 1)
        {
            compressed_data[length] = 0x80 | run;
            length += 1;
            compressed_data[length] = indexed_data[position];
            length += 1;
        }
        else 
        {
            compressed_data[length] = indexed_data[position];
            length += 1;
        } 
        
        position += run;
    }

    return length;
}
    
void mr_decode(unsigned char* compressed_data, unsigned char* indexed_data, int cdata_size, int idata_size)
{
    int i;
    int position = 0;
    int idx_position = 0;
    int run = 0;
    unsigned char first_byte;
    unsigned char second_byte;

    while(position < cdata_size)
    {
        first_byte = compressed_data[position];
        if((position+1) < cdata_size)
            second_byte = compressed_data[position+1];

        // The bytes lower than 0x80 are recopied just as they are in the Bitmap
        if(first_byte < 0x80)
        {
            run = 1;
            position += 1;
        }
        // The tag 0x81 is followed by a byte giving directly the count of points
        else if(first_byte == 0x81)
        {
            run = second_byte;
            first_byte = compressed_data[position+2];
            position += 3;
        }
        // The tag 0x82 is followed by the number of the points decoded in Run
        // By retaining only the first byte for each point
        else if(first_byte == 0x82 && second_byte >= 0x80)
        {
            run = second_byte - 0x80 + 0x100;
            first_byte = compressed_data[position+2];
            position += 3;
        }
        else 
        {
            run = first_byte - 0x80;
            first_byte = second_byte;
            position += 2; 
        }

        // Writing decompressed bytes
        for(i=0; i<run; i++)
        {
            // The additional byte (+ 1) is useless, but it always present in MR files.
            if(idx_position+i < idata_size)
                indexed_data[idx_position+i] = first_byte;
        }
            
        idx_position += run;
    }
}

// Create MR file from raw data
int mr_create_from_raw(unsigned char* raw_data, unsigned char* mr_data, int width, int height)
{
    mr_header_t header;
    mr_pal_entry_t palette[MAX_PALETTE_COLORS];

    int data_offset;
    int compressed_size;
    
    unsigned char* indexed_data = (unsigned char*)calloc(width*height, sizeof(unsigned char));
    unsigned char* compressed_data = (unsigned char*)calloc(width*height, sizeof(unsigned char));
    
    int i, found, palette_index, pixel_index;
    int color_count = 0;

    // Generate indexed_data and create the palette of the image
    memset(&palette, 0, sizeof(mr_pal_entry_t)*MAX_PALETTE_COLORS);
    for(i=0; i<(width*height); i++)
    {
        found = 0;
        palette_index = 0;

        pixel_index = i * PIXEL_SIZE;
        while(!found && palette_index < color_count) 
        {
            if(raw_data[pixel_index+0] == palette[palette_index].r &&
               raw_data[pixel_index+1] == palette[palette_index].g &&
               raw_data[pixel_index+2] == palette[palette_index].b)
                found = 1;
            else
                palette_index += 1;
        }

        // Display error message if image has more than 128 colors
        if(!found && palette_index == 128) {
            //gimp.message("Reduce the number of colors to <= 128 and try again.\n")
            printf("Image contains more than 128 colors\n");
            free(indexed_data);
            free(compressed_data);
            return 0;
        }
            
        if(!found)
        {
            palette[palette_index].r = raw_data[pixel_index+0];
            palette[palette_index].g = raw_data[pixel_index+1];
            palette[palette_index].b = raw_data[pixel_index+2];
            color_count += 1;
        }
        
        indexed_data[i] = palette_index;
    }

    // Compress indexed_data
    compressed_size = mr_encode(indexed_data, compressed_data, width*height);

    // Write header
    data_offset = sizeof(mr_header_t) + color_count*4;
    memset(&header, 0, sizeof(mr_header_t));
    strncpy(header.mr, "MR", 2);
    header.total_size = data_offset + compressed_size;
    header.data_offset = data_offset;
    header.width = width;
    header.height = height;
    header.color_cnt = color_count;
    memcpy(mr_data, &header, sizeof(mr_header_t));

    // Write palette data
    memcpy(&mr_data[sizeof(mr_header_t)], &palette, sizeof(mr_pal_entry_t)*color_count);

    // Write compressed image data  
    memcpy(&mr_data[data_offset], compressed_data, compressed_size);

    free(indexed_data);
    free(compressed_data);

    return header.total_size;
}

// Decode MR file and produce raw data
void mr_decode_to_raw(unsigned char* mr_data, unsigned char* raw_data)
{
    int i, index;
    mr_header_t header;
    mr_pal_entry_t palette[MAX_PALETTE_COLORS];
    unsigned char* indexed_data;
    unsigned char* compressed_data;
    
    // Read header
    memcpy(&header, mr_data, sizeof(mr_header_t));

    // Read Palette
    memcpy(palette, mr_data+sizeof(mr_header_t), sizeof(mr_pal_entry_t)*header.color_cnt);

    // Uncompress image data
    compressed_data = mr_data+header.data_offset;
    indexed_data = (unsigned char*)calloc(header.width*header.height, sizeof(unsigned char));
    mr_decode(compressed_data, indexed_data, header.total_size-header.data_offset, header.width*header.height);

    for(i=0; i<(header.width*header.height); i++)
    {
        index = indexed_data[i];
        raw_data[(i*3)+0] = palette[index].r;
        raw_data[(i*3)+1] = palette[index].g;
        raw_data[(i*3)+2] = palette[index].b;
    }

    free(indexed_data);
}

// Get width and height of image
void mr_decode_width_height(unsigned char* mr_data, int* width, int* height)
{
    mr_header_t header;
    memcpy(&header, mr_data, sizeof(mr_header_t));

    *width = header.width;
    *height = header.height;
}

unsigned int mr_buffer_size(int width, int height) {
    return sizeof(mr_header_t) + (sizeof(mr_pal_entry_t)*MAX_PALETTE_COLORS) + (width*height);
}
