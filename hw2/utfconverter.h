
    #ifndef UTFCONVERTER_H
    #define UTFCONVERTER_H
    #include <stdlib.h>
    #include <stdio.h>
    #include <stdbool.h>
    #include <unistd.h>
    #include <string.h>
    #include <sys/stat.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <stdint.h>
    #include "utfconverter.h"

    /* Constants for validate_args return values. */
    #define VALID_ARGS 0
    #define SAME_FILE  1
    #define FILE_DNE   2
    #define FAILED     3

    #define UTF8_4_BYTE 0xF0
    #define UTF8_3_BYTE 0xE0
    #define UTF8_2_BYTE 0xC0
    #define UTF8_CONT   0x80

    /* # of bytes a UTF-16 codepoint takes up */
    #define CODE_UNIT_SIZE 1

    #define SURROGATE_PAIR 0x10000

    #define SAFE_PARAM 0x0FA47E10

    /**
     * Checks to make sure the input arguments meet the following constraints.
     * 1. input_path is a path to an existing file.
     * 2. output_path is not the same path as the input path.
     * 3. output_format is a correct format as accepted by the program.
     * @param input_path Path to the input file being converted.
     * @param output_path Path to where the output file should be created.
     * @return Returns 0 if there was no errors, 1 if they are the same file, 2
     *         if the input file doesn't exist, 3 if something went wrong.
     */
    int validate_args(const char *input_path, const char *output_path);

    /**
     * Converts the input file UTF-8 file to UTF-16LE.
     * @param input_fd The input files file descriptor.
     * @param output_fd The output files file descriptor.
     * @return Returns true if the conversion was a success else false.
     */
    bool convert(const int input_fd, const int output_fd,int vflag,int eflag);
    bool convertBig(const int input_fd, const int output_fd);
    int reverseInt(int i);

    /**
     * Writes bytes to output_fd and reports the success of the operation.
     * @param value Value to be written to file.
     * @param size Size of the value in bytes to write.
     * @return Returns true if the write was a success, else false.
     */
    bool safe_write(int output_fd, void *value, size_t size);

    /**
     * Print out the program usage string
     */
    #define USAGE(name) do {                                                                                                \
        fprintf(stderr,                                                                                                     \
            "\n%s [-h] [-v | -vv | -vvv] -e OUTPUT_ENCODING INPUT_FILE OUTPUT_FILE \n"                                                                           \
            "\n"                                                                                                            \
            "\ncommand line utility for converting files to and from UTF-8, UTF-16LE, or UTF-16BE\n"                          \
            "\n"                                                                                                            \
            "\n"                                                                                                            \
            "Option arguments:\n\n"                                                                                         \
            "-h                             Displays this usage menu.\n"                                                    \
            "-v                             Enable verbose output.\n"                                                    \
            "                               This argument can be uesd up to three times for a noticeable effect.\n"                                                    \
            "-e OUTPUT_ENCODING             Format to encide the output file.\n"                                                    \
            "                               Accepted values:\n"                                                    \
            "                                               UTF-8\n"                                                    \
            "                                               UTF-16LE\n"                                                    \
            "                                               UTF-16BE\n"                                                    \
            "                                               if this flag is not provided or an invalid value is given the program should exit with EXIT_FAILURE return code.\n"                                                    \
            "\nPositional arguments:\n\n"                                                                                   \
            "INPUT_FILE                     File to convert. Must contain a\n"                                              \
            "                               valid BOM. If it does not contain a\n"                                          \
            "                               valid BOM the program should exit\n"                                            \
            "                               with the EXIT_FAILURE return code.\n"                                           \
            "\n"                                                                                                            \
            "OUTPUT_FILE                    Output file to create. If the file\n"                                           \
            "                               already exists and its not the input\n"                                         \
            "                               file, it should be overwritten. If\n"                                           \
            "                               the OUTPUT_FILE is the same as the\n"                                           \
            "                               INPUT_FILE the program should exit\n"                                           \
            "                               with the EXIT_FAILURE return code.\n"                                           \
            ,(name)                                                                                                         \
        );                                                                                                                  \
    } while(0)

    #endif
