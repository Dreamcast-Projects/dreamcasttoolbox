#ifndef MR_H
#define MR_H

// Create MR file from raw data
int mr_create_from_raw(unsigned char* raw_data, unsigned char* mr_data, int width, int height);

// Decode MR file and produce raw data
void mr_decode_to_raw(unsigned char* mr_data, unsigned char* raw_data);

// Get width and height of image
void mr_decode_width_height(unsigned char* mr_data, int* width, int* height);

// Returns max size of buffer that is needed to fit a mr image given width and height
unsigned int mr_buffer_size(int width, int height);

#endif
