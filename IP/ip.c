#include "ip.h"
#include "iptmpl.h"

#include "mr.h"

#include <string.h>
#include <stdio.h>

typedef struct ip_meta_t {
    char hardware_id[16];        // Always "SEGA SEGAKATANA "
    char maker_id[16];           // "SEGA ENTERPRISES"
    char device_info[16];
    char area_symbols[8];        // JUE
    char peripherals[8];
    char product_number[10];
    char product_version[6];
    char release_date[16];       // (YYYYMMDD)
    char boot_filename[16];      // "1ST_READ.BIN"
    char company_name[16];       // Name of company that produced the disc
    char software_title[128];    // Name of software
} __attribute__((packed)) ip_meta_t;

#define MR_OFFSET 0x3820

int calculate_crc(const unsigned char *buf, int size)
{
    int i, c, n = 0xffff;
    for (i = 0; i < size; i++)
    {
        n ^= (buf[i]<<8);
        for (c = 0; c < 8; c++) {
            if (n & 0x8000)
                n = (n << 1) ^ 4129;
            else
                n = (n << 1);
        }
    }
    return n & 0xffff;
}

// Create MR file from raw data
int ip_create_from_fields(ip_fields_t* field_data, unsigned char* ip_data)
{
    ip_meta_t meta = 
    {
        "SEGA SEGAKATANA ",
        "SEGA ENTERPRISES",
        "0000 CD-ROM1/1  ",
        "JUE     ",
        "E000F10 ",
        "T-00000   ",
        "V1.000",
        "20200706        ",
        "1ST_READ.BIN    ",
        "INDIE DEV       ",
        "MY INCREDIBLE GAME                                                                                                              "
    };
    int field_length = 0;

    field_length = strlen(field_data->company_name);
    if(field_length > 0)
    {
        if(field_length > sizeof(meta.company_name))
        {
            printf("Company name too large");
            return -1;
        }

        memset(meta.company_name, ' ', sizeof(meta.company_name));
        memcpy(meta.company_name, field_data->company_name, field_length);
    }

    field_length = strlen(field_data->software_title);
    if(field_length > 0)
    {
        if(field_length > sizeof(meta.software_title))
        {
            printf("Software title too large");
            return -1;
        }

        memset(meta.software_title, ' ', sizeof(meta.software_title));
        memcpy(meta.software_title, field_data->software_title, field_length);
    }
    
    field_length = strlen(field_data->product_version);
    if(field_length > 0)
    {
        if(field_length > sizeof(meta.product_version))
        {
            printf("Product Version too large");
            return -1;
        }

        memset(meta.product_version, ' ', sizeof(meta.product_version));
        memcpy(meta.product_version, field_data->product_version, field_length);
    }

    field_length = strlen(field_data->release_date);
    if(field_length > 0)
    {
        if(field_length > sizeof(meta.release_date))
        {
            printf("Release Date too large");
            return -1;
        }

        memset(meta.release_date, ' ', sizeof(meta.release_date));
        memcpy(meta.release_date, field_data->release_date, field_length);
    }

    field_length = strlen(field_data->boot_filename);
    if(field_length > 0)
    {
        if(field_length > sizeof(meta.boot_filename))
        {
            printf("Product Version too large");
            return -1;
        }

        memset(meta.boot_filename, ' ', sizeof(meta.boot_filename));
        memcpy(meta.boot_filename, field_data->boot_filename, field_length);
    }

    int crc = calculate_crc((unsigned char *)(meta.product_number), 16);
    memcpy(meta.device_info, &crc, sizeof(int));

    // Copy Ip.BIN template then memcpy over with configurable meta data
    memcpy(ip_data, default_ip_data, INITIAL_PROGRAM_SIZE);
    memcpy(ip_data, &meta, sizeof(meta));

    return 0;
}

// Decode MR file and produce raw data
int ip_insert_mr(unsigned char* mr_data, unsigned char* ip_data)
{
    int filesize = 0;

    if(ip_valid_file(ip_data) < 0)
        return -1;

    if(mr_valid_file(mr_data) != 0)
       return -1;

    mr_decode_filesize(mr_data, &filesize);
    memcpy(ip_data+MR_OFFSET, mr_data, filesize);

    return 0;
}

// Decode MR file and produce raw data
int ip_extract_mr(unsigned char* ip_data, unsigned char* mr_data, int* mr_filesize)
{
    if(ip_valid_file(ip_data) < 0)
        return -1;

    mr_decode_filesize(ip_data+MR_OFFSET, mr_filesize);
    memcpy(mr_data, ip_data+MR_OFFSET, *mr_filesize);

    return 0;
}

// Get width and height of image
int ip_extract_fields(unsigned char* ip_data, ip_fields_t* field_data)
{
    ip_meta_t meta;
    memcpy(&meta, ip_data, sizeof(ip_meta_t));

    if(ip_valid_file(ip_data) < 0)
        return -1;

    memcpy(field_data->company_name, meta.company_name, sizeof(meta.company_name));
    field_data->company_name[sizeof(meta.company_name)] = '\0';
    memcpy(field_data->software_title, meta.software_title, sizeof(meta.software_title));
    field_data->software_title[sizeof(meta.software_title)] = '\0';
    memcpy(field_data->product_version, meta.product_version, sizeof(meta.product_version));
    field_data->product_version[sizeof(meta.product_version)] = '\0';
    memcpy(field_data->release_date, meta.release_date, sizeof(meta.release_date));
    field_data->release_date[sizeof(meta.release_date)] = '\0';
    memcpy(field_data->boot_filename, meta.boot_filename, sizeof(meta.boot_filename));
    field_data->boot_filename[sizeof(meta.boot_filename)] = '\0';

    return 0;
}

int ip_valid_file(unsigned char* ip_data) 
{
    ip_meta_t meta;
    memcpy(&meta, ip_data, sizeof(ip_meta_t));

    // Check hardware_id
    if(strncmp(meta.hardware_id, "SEGA SEGAKATANA ", 16) != 0)
    {
        printf("HardwareID is incorrect\n");
        return -1;
    }
    
    // Check maker_id
    if(strncmp(meta.maker_id, "SEGA ENTERPRISES", 16) != 0)
    {
        printf("MakerID is incorrect\n");
        return -1;
    }
    
    // Check boot_filename is not empty(just spaces)
    if(strncmp(meta.boot_filename, " ", 1) == 0)
    {
        printf("Boot file name is incorrect\n");
        return -1;
    }

    return 0;
}
