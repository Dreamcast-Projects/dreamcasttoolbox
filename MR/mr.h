#ifndef MR_H
#define MR_H

#ifdef __cplusplus
extern "C" {
#endif

// Create MR file from raw data
int mr_create_from_raw(unsigned char* raw_data, int width, int height, unsigned char* mr_data);

// Decode MR file and produce raw data
int mr_decode_to_raw(unsigned char* mr_data, unsigned char* raw_data);

// Get width and height of image
int mr_decode_width_height(unsigned char* mr_data, int* width, int* height);

// Returns 0, stores filesize
int mr_decode_filesize(unsigned char* mr_data, int* filesize);

// Returns 0 if valid file
int mr_valid_file(unsigned char* mr_data);

// Returns max size of buffer that is needed to fit a mr image given width and height
unsigned int mr_buffer_size(int width, int height);

#ifdef __cplusplus
}
#endif

#endif
