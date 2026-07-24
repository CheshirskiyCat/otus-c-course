#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096

#define FAILD_OPEN_FILE 1
#define FAILD_CREATE_FILE 2
#define WRONG_ARGUMENT 3
#define WRONG_CHARSET 4


void check_arguments(int argc, char** argv); 
void on_error(int error_code, char* message);
unsigned int encode(char* charset, int code); 
void write_utf8(FILE *out, unsigned int code);

const char* CP1251 = "cp1251";
const char* ISO8859 = "iso8859";
const char* KOI8 = "koi8";

static const unsigned short koi8_to_unicode[64] = {
	62, 32, 33, 54, 36, 37, 52, 35, 53, 40, 41, 42, 43, 43, 45, 46,
	47, 63, 48, 49, 50, 51, 38, 34, 60, 59, 39, 56, 61, 57, 55, 58,
	30,  0,  1, 22,  4,  5, 20,  3, 21,  8,  9, 10, 11, 12, 13, 14,
	15, 31, 16, 17, 18, 19,  6,  2, 28, 27,  7, 24, 29, 25, 23, 26
};


int main(int argc, char** argv)
{
    check_arguments(argc, argv);

    FILE* source_file = fopen(argv[2], "r");
    if(!source_file) { on_error(FAILD_OPEN_FILE, argv[2]); }

    FILE *result_file = fopen(argv[6], "wb");
    if (!result_file) { fclose(source_file); on_error(FAILD_CREATE_FILE, argv[6]); }

    int ch;

    fputc(0xEF, result_file);
    fputc(0xBB, result_file);
    fputc(0xBF, result_file);

    while ((ch = getc(source_file)) != EOF) { // Цикл до конца файла (EOF)
        unsigned int utf8_code = encode(argv[4], ch);
        write_utf8(result_file, utf8_code);
    }

    fclose(source_file);
    fclose(result_file);

    return EXIT_SUCCESS;
}

unsigned int encode(char* charset, int code)
{
    wchar_t result = code;
    if (strcmp(charset, CP1251) == 0)
    {
        if (code >= 192 && code <= 255) { result = 1040 + code - 192; }
        else if (code == 168) { result = 1025; }
        else if (code == 184) { result = 1105; }
    } 
    else if (strcmp(charset, ISO8859) == 0) 
    {
        if (code >= 176 && code <= 239) { result = 1040 + code - 176; }
        else if (code == 161) { result = 1025; }
        else if (code == 241) { result = 1105; }
    } 
    else if (strcmp(charset, KOI8) == 0) 
    {
        if(code >= 192 && code <= 255) { result = 1040 + koi8_to_unicode[code - 192]; }
        else if (code == 179) { result = 1025; }
        else if (code == 163) { result = 1105; }
    } 
    else 
    {
        on_error(WRONG_CHARSET, charset);
    }
    return result;
}

void write_utf8(FILE *out, unsigned int code)
{
    if (code <= 0x7F) {
        fputc(code, out);
    } else if (code <= 0x7FF) {
        fputc(0xC0 | ((code >> 6) & 0x1F), out);
        fputc(0x80 | (code & 0x3F), out);
    }
}

void check_arguments(int argc, char** argv)
{
    char* empty_message = "";
    
    if(argc != 7 || strcmp(argv[1], "--file") != 0 || strcmp(argv[3], "--charset") != 0 || strcmp(argv[5], "--output") != 0) 
    { 
        on_error(WRONG_ARGUMENT, empty_message); 
    }
}

void on_error(int error_code, char* message) {
    switch (error_code) {
        case FAILD_OPEN_FILE:
            fprintf(stderr, "Error: Failed to open %s for reading!\n", message);
            break;
        case FAILD_CREATE_FILE:
            fprintf(stderr, "Error: Failed to create file %s!\n", message);
            break;
        case WRONG_ARGUMENT:
            fprintf(stderr, "Error: Wrong arguments\n");
            fprintf(stderr, "Usage: encoder --file <source file> --charset <charset> --output <result file>\n");
            break;
        case WRONG_CHARSET:
            fprintf(stderr, "Error: Wrong charset\n");
            break;            
        default:
            perror("Error: unknown");
            break;
    }
    exit(EXIT_FAILURE);
}
