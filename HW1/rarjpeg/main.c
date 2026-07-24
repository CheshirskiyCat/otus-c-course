#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define FAILD_OPEN_FILE 1
#define WRONG_ARGUMENT 2
#define EMPTY_FILE 3
#define WRONG_CD_SIGNATURE 4

int check_arguments(int argc); 
void on_error(int error_code, char* message);
void dump_hex(const void* data, size_t size);

struct CentralDirectoryFileHeader
{
    uint16_t versionMadeBy;
    uint16_t versionToExtract;
    uint16_t generalPurposeBitFlag;
    uint16_t compressionMethod;
    uint16_t modificationTime;
    uint16_t modificationDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t filenameLength;
    uint16_t extraFieldLength;
    uint16_t fileCommentLength;
    uint16_t diskNumber;
    uint16_t internalFileAttributes;
    uint32_t externalFileAttributes;
    uint32_t localFileHeaderOffset;

} __attribute__((packed));

struct EOCD 
{
    uint16_t diskNumber;
    uint16_t startDiskNumber;
    uint16_t numberCentralDirectoryRecord;
    uint16_t totalCentralDirectoryRecord;
    uint32_t sizeOfCentralDirectory;
    uint32_t centralDirectoryOffset;
    uint16_t commentLength;

} __attribute__((packed));


typedef enum { ZIP_STORED = 0, ZIP_DEFLATED = 8 } method_t;

typedef struct zipmemb_t zipmemb_t;

int main(int argc, char *argv[]) {

    int check_result = check_arguments(argc);
    if (check_result == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    FILE* file = fopen(argv[1], "rb");
    if(!file) { on_error(FAILD_OPEN_FILE, argv[1]); return EXIT_FAILURE; }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        on_error(EMPTY_FILE, "");
        return EXIT_FAILURE;
    }
    size_t file_size = ftell(file);

    size_t eocd_offset = 0;


    for (size_t offset = file_size - sizeof(struct EOCD); offset != 0; --offset)
    {
        uint32_t signature = 0;

        fseek(file, offset, SEEK_SET);
        fread((char *) &signature, sizeof(signature), 1, file);

        if (0x06054b50 == signature)
        {
            eocd_offset = offset;
            break;
        }
    }   

    printf("eocd_offset: %ld.\n", eocd_offset);
    printf("file_size: %ld.\n", file_size);

    if (eocd_offset != 0){
        printf("A ZIP archive is contained at the end of the image file.\n");
    } else {
        printf("The image file contains no attachments.\n");
        fclose(file);
        return EXIT_SUCCESS;
    }

    struct EOCD eocd;
    fread((char *) &eocd, sizeof(eocd), 1, file);
    size_t cd_offset = eocd_offset - eocd.sizeOfCentralDirectory;

    printf("total: %d.\n", eocd.totalCentralDirectoryRecord);
    printf("Files: \n");

    for (size_t offset = cd_offset; offset < eocd_offset; ++offset)
    {
        uint32_t signature = 0;

        fseek(file, offset, SEEK_SET);
        fread((char *) &signature, sizeof(signature), 1, file);

        if (0x02014b50 == signature)
        {
           
            struct CentralDirectoryFileHeader fileheader;
            fread((char *) &fileheader, sizeof(fileheader), 1, file);

            if (fileheader.filenameLength)
            {
                uint8_t file_name[fileheader.filenameLength + 1];
                uint8_t *file_name_ptr = file_name;
                fread((char *) file_name_ptr, fileheader.filenameLength, 1, file);
        
                file_name_ptr[fileheader.filenameLength] = 0;  
                printf("File: %s\n", file_name_ptr);  
            }
        }
    }   

    fclose(file);

    return EXIT_SUCCESS;
}

int check_arguments(int argc)
{
    char* empty_message = "";
    
    if(argc != 2) 
    { 
        on_error(WRONG_ARGUMENT, empty_message); 
        return 1;
    }
    return 0;
}

void on_error(int error_code, char* message) {
    switch (error_code) {
        case FAILD_OPEN_FILE:
            fprintf(stderr, "Error: Failed to open %s for reading!\n", message);
            break;
        case WRONG_ARGUMENT:
            fprintf(stderr, "Error: Wrong arguments\n");
            fprintf(stderr, "Usage: solution <file path>\n");
            break;       
        case EMPTY_FILE:
            fprintf(stderr, "Error: Empty file\n");
            break;          
        case WRONG_CD_SIGNATURE:
            fprintf(stderr, "Error: File has no central directory file header signature\n");
            break;                 
        default:
            perror("Error: unknown");
            break;
    }
}
