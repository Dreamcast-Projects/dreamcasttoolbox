#ifndef IP_H
#define IP_H

#ifdef __cplusplus
extern "C" {
#endif

#define INITIAL_PROGRAM_SIZE 0x8000

typedef struct ip_fields_t {
    char company_name[17];       // Name of company
    char software_title[129];    // Name of software
    char product_version[7];
    char release_date[17];       // (YYYYMMDD)
    char boot_filename[17];      // "1ST_READ.BIN"
} ip_fields_t;

void ip_field_create(ip_fields_t** field_data);

void ip_field_destroy(ip_fields_t* field_data);

int ip_field_get_company_name(const ip_fields_t* field_data, char* value, int value_length);

int ip_field_set_company_name(ip_fields_t* field_data, const char* value);

int ip_field_get_software_title(const ip_fields_t* field_data, char* value, int value_length);

int ip_field_set_software_title(ip_fields_t* field_data, const char* value);

int ip_field_get_product_version(const ip_fields_t* field_data, char* value, int value_length);

int ip_field_set_product_version(ip_fields_t* field_data, const char* value);

int ip_field_get_release_date(const ip_fields_t* field_data, char* value, int value_length);

int ip_field_set_release_date(ip_fields_t* field_data, const char* value);

int ip_field_get_boot_filename(const ip_fields_t* field_data, char* value, int value_length);

int ip_field_set_boot_filename(ip_fields_t* field_data, const char* value);

// Create MR file from raw data
int ip_create_from_fields(ip_fields_t* field_data, unsigned char* ip_data);

// Get width and height of image
int ip_extract_fields(unsigned char* ip_data, ip_fields_t* field_data);

// Decode MR file and produce raw data
int ip_insert_mr(unsigned char* mr_data, unsigned char* ip_data);

// Decode MR file and produce raw data
int ip_extract_mr(unsigned char* ip_data, unsigned char* mr_data, int* mr_filesize);

int ip_valid_file(unsigned char* ip_data);

#ifdef __cplusplus
}
#endif

#endif
