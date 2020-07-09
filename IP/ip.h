#ifndef IP_H
#define IP_H

#define INITIAL_PROGRAM_SIZE 0x8000

typedef struct ip_fields_t {
    char company_name[17];       // Name of company
    char software_title[129];    // Name of software
    char product_version[7];
    char release_date[17];       // (YYYYMMDD)
    char boot_filename[17];      // "1ST_READ.BIN"
} ip_fields_t;

// Create MR file from raw data
int ip_create_from_fields(ip_fields_t* field_data, unsigned char* ip_data);

// Get width and height of image
int ip_extract_fields(unsigned char* ip_data, ip_fields_t* field_data);

// Decode MR file and produce raw data
int ip_insert_mr(unsigned char* mr_data, unsigned char* ip_data);

// Decode MR file and produce raw data
int ip_extract_mr(unsigned char* ip_data, unsigned char* mr_data, int* mr_filesize);

int ip_valid_file(unsigned char* ip_data);

#endif
