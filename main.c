#include <chardet.h>
#include <ctype.h>

#include "cflags.h"

int process(const char* str, size_t len) {
  DetectObj *obj;
  if ((obj = detect_obj_init()) == NULL) {
    fprintf(stderr, "Memory Allocation failed\n");
    return CHARDET_MEM_ALLOCATED_FAIL;
  }

#ifndef CHARDET_BINARY_SAFE
  #error libchardet version must be >= 1.0.5
#else
  switch (detect_r(str, strlen(str), &obj))
#endif
  {
  case CHARDET_OUT_OF_MEMORY:
    fprintf(stderr, "Error - out of memory when detecting charset\n");
    detect_obj_free(&obj);
    return CHARDET_OUT_OF_MEMORY;
  case CHARDET_NULL_OBJECT:
    fprintf(stderr, "Error - null object\n");
    return CHARDET_NULL_OBJECT;
  }

#ifndef CHARDET_BOM_CHECK
  printf("encoding: %s, confidence: %f\n", obj->encodiNg, obj->confidence);
#else
  // from 1.0.6 support return whether exists BOM
  printf("encoding: %s, confidence: %f, exist BOM: %d\n", obj->encoding,
         obj->confidence, obj->bom);
#endif
  detect_obj_free(&obj);
  return 0;
}

int process_text(const char* str) {
    return process(str, strlen(str));
}

int process_hex(const char* str) {
    size_t len = strlen(str);
    if (len % 2 != 0) {
        printf("Hex string was not an even number of characters\n");
        return -1;
    }
    size_t num_bytes = len / 2;
    char* result = malloc(num_bytes);
    for (int i = 0; i < num_bytes; i++) {
        char tmp[3];
        tmp[0] = str[i * 2];
        tmp[1] = str[i * 2 + 1];
        tmp[2] = '\0';

        result[i] = strtol(tmp, NULL, 16);
    }
    return process(result, num_bytes);
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

int main(int argc, char** argv) {
  cflags_t *flags = cflags_init();

  bool help = false;
  cflags_add_bool(flags, 'h', "help", &help, "print this text and exit");

  const char* format_arg = "text";
  cflags_add_string(flags, 'f', "format", &format_arg, "Format to use: text (default) or hex (without 0x)");

  if (!cflags_parse(flags, argc, argv) || help || flags->argc == 1) {
    cflags_print_usage(
        flags, "[OPTION]... [ARG]...", "Detects charsets using libchardet.", "");
    return 0;
  }

  chardet_format_t format = CHARDET_FORMAT_UNKNOWN;
  if (streqi("text", format_arg)) {
    format = CHARDET_FORMAT_TEXT;
  } else if (streqi("hex", format_arg)) {
    format = CHARDET_FORMAT_HEX;
  }

  if (format == CHARDET_FORMAT_UNKNOWN) {
    printf("Unknown format: %s\n", format_arg);
    return -1;
  }

  for (int i = 1; i < flags->argc; i++) {
    int res;
    switch (format) {
      case CHARDET_FORMAT_UNKNOWN:
      case CHARDET_FORMAT_TEXT:
        res = process_text(flags->argv[i]);
        break;
      case CHARDET_FORMAT_HEX:
        res = process_hex(flags->argv[i]);
        break;
    }
    if (res != 0) {
      return res;
    }
  }
  return 0;
}