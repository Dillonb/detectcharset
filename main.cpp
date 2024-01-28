#include <chardet.h>
#include <ctype.h>
#include <iostream>
#include <string>
#include <vector>

#include "cflags.h"

int process(const char* str, size_t len) {
    DetectObj *obj;
    if ((obj = detect_obj_init()) == NULL) {
        fprintf(stderr, "Memory Allocation failed\n");
        return CHARDET_MEM_ALLOCATED_FAIL;
    }

    switch (detect_r(str, len, &obj)) {
        case CHARDET_OUT_OF_MEMORY:
            fprintf(stderr, "Error - out of memory when detecting charset\n");
            detect_obj_free(&obj);
            return CHARDET_OUT_OF_MEMORY;
        case CHARDET_NULL_OBJECT:
            fprintf(stderr, "Error - null object\n");
            return CHARDET_NULL_OBJECT;
        default:
            break;
    }

    printf("encoding: %s, confidence: %f, has BOM: %d\n", obj->encoding, obj->confidence, obj->bom);
    detect_obj_free(&obj);
    return 0;
}

int process_hex(const char* str, std::vector<char>& output) {
    size_t len = strlen(str);
    if (len % 2 != 0) {
        printf("Hex string was not an even number of characters\n");
        return -1;
    }
    size_t num_bytes = len / 2;
    for (int i = 0; i < num_bytes; i++) {
        char tmp[3];
        tmp[0] = str[i * 2];
        tmp[1] = str[i * 2 + 1];
        tmp[2] = '\0';

        output.emplace_back(strtol(tmp, NULL, 16));
    }
    return 0;
}

typedef enum chardet_format {
    CHARDET_FORMAT_UNKNOWN,
    CHARDET_FORMAT_TEXT,
    CHARDET_FORMAT_HEX
} chardet_format_t;

bool streqi(const char* s1, const char* s2) {
    int len = strlen(s1);
    if (len != strlen(s2)) {
        return false;
    }

    for (int i = 0; i < len; i++) {
        if (tolower(s1[i]) != tolower(s2[i])) {
            return false;
        }
    }
    return true;
}

int process_input_chunk(std::string chunk, chardet_format_t format, std::vector<char>& output) {
    int res = 0;
    switch (format) {
        case CHARDET_FORMAT_UNKNOWN:
        case CHARDET_FORMAT_TEXT:
            for (int j = 0; j < chunk.size(); j++) {
                output.emplace_back(chunk.at(j));
            }
            break;
        case CHARDET_FORMAT_HEX:
            res = process_hex(chunk.c_str(), output);
            break;
    }
    return res;
}

int main(int argc, char** argv) {
    cflags_t *flags = cflags_init();

    bool help = false;
    cflags_add_bool(flags, 'h', "help", &help, "Print this text and exit");

    const char* format_arg = "text";
    cflags_add_string(flags, 'f', "format", &format_arg, "Format to use: text (default) or hex (without 0x)");

    if (!cflags_parse(flags, argc, argv) || help) {
        cflags_print_usage(flags, "[OPTION]... [ARG]...", "Detects charsets using libchardet. Pass text either as positional arguments, or from stdin.", "");
        return 0;
    }

    bool from_stdin = flags->argc == 1;

    chardet_format_t format = CHARDET_FORMAT_UNKNOWN;
    if (streqi("text", format_arg)) {
        format = CHARDET_FORMAT_TEXT;
    } else if (streqi("hex", format_arg)) {
        format = CHARDET_FORMAT_HEX;
    }

    if (format == CHARDET_FORMAT_UNKNOWN) {
        cflags_free(flags);
        printf("Unknown format: %s\n", format_arg);
        return -1;
    }

    std::vector<char> input;

    if (from_stdin) {
        std::string line;
        while (std::getline(std::cin, line)) {
            int res = process_input_chunk(line, format, input);
            if (res != 0) {
                cflags_free(flags);
                return res;
            }
        }
    } else {
        for (int i = 1; i < flags->argc; i++) {
            int res = process_input_chunk(flags->argv[i], format, input);
            if (res != 0) {
                cflags_free(flags);
                return res;
            }
        }
    }

    cflags_free(flags);

    return process(input.data(), input.size());
}
