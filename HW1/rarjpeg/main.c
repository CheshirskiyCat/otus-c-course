#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define FAILD_OPEN_FILE 1
#define WRONG_ARGUMENT 2
#define EMPTY_FILE 3
#define WRONG_CD_SIGNATURE 4

long get_file_size(FILE* file);
void check_arguments(int argc); 
void on_error(int error_code, char* message);

struct CentralDirectoryFileHeader
{
    uint32_t signature;
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

    check_arguments(argc);

    FILE* file = fopen(argv[1], "rb");
    if(!file) { on_error(FAILD_OPEN_FILE, argv[1]); }

    long file_size = get_file_size(file);
    int has_signature = 1;
    long int file_offset = 0;


    for (size_t offset = file_size - sizeof(struct EOCD); offset != 0; --offset)
    {
        uint32_t signature = 0;

        fseek(file, offset, SEEK_SET);
        fread((char *) &signature, sizeof(signature), 1, file);

        if (0x06054b50 == signature)
        {
            file_offset = offset;
            has_signature = 0;
            break;
        }
    }   

    printf("offset: %ld.\n", file_offset);
    printf("file_size: %ld.\n", file_size);

    if (has_signature == 0){
        printf("A ZIP archive is contained at the end of the image file.\n");
    } else {
        printf("The image file contains no attachments.\n");
        fclose(file);
        return EXIT_SUCCESS;
    }

    struct EOCD eocd;
    fread((char *) &eocd, sizeof(eocd), 1, file);

    printf("Comment: %d.\n", eocd.commentLength);
    printf("num: %d.\n", eocd.numberCentralDirectoryRecord);
    printf("total: %d.\n", eocd.totalCentralDirectoryRecord);
    printf("size of CD: %d.\n", eocd.sizeOfCentralDirectory);
    printf("offset: %d.\n", eocd.centralDirectoryOffset);

    if (eocd.commentLength)
    {
        uint8_t comment[eocd.commentLength + 1];
        uint8_t *comment_ptr = comment;

        fread((char *) comment_ptr, eocd.commentLength, SEEK_CUR, file);

        comment_ptr[eocd.commentLength] = 0;
        printf("Comment: %s.\n", comment_ptr);

    }

    fseek(file, eocd.centralDirectoryOffset, SEEK_SET);

    for (uint16_t i = 0; i < eocd.numberCentralDirectoryRecord; ++i)
    {
        struct CentralDirectoryFileHeader cdfh;

        fread((char *) &cdfh, sizeof(cdfh), 1, file);
        printf("cdfh.signature: %d .\n", cdfh.signature);  

        // if (0x504b0102 != cdfh.signature) { fclose(file); on_error(WRONG_CD_SIGNATURE, ""); }
        if (0x02014b50 != cdfh.signature) { fclose(file); on_error(WRONG_CD_SIGNATURE, ""); }

        if (cdfh.filenameLength)
        {
            uint8_t file_name[cdfh.filenameLength + 1];
            uint8_t *file_name_ptr = file_name;
            fread((char *) file_name_ptr, cdfh.filenameLength, 1, file);
    
            file_name_ptr[cdfh.filenameLength] = 0;  
            printf("File: %s .\n", file_name_ptr);  
        }
    }

    fclose(file);

    return EXIT_SUCCESS;
}

long get_file_size(FILE* file)
{
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        on_error(EMPTY_FILE, "");
    }
    return ftell(file);
}

void check_arguments(int argc)
{
    char* empty_message = "";
    
    if(argc != 2) 
    { 
        on_error(WRONG_ARGUMENT, empty_message); 
    }
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
    exit(EXIT_FAILURE);
}