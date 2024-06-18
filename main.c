#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SV_IMPLEMENTATION
#include "sv.h"

#define LINE_INIT_CAP 64

typedef struct lines {
  cstring_view* lines;
  size_t count;
  size_t cap;
} lines;

int line_append(lines* lines, cstring_view value);
void extract_lines(lines* lines, cstring_view input);

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "Expected 2 arguments, got %d.\n", argc - 1);
    fprintf(stderr, "Usage: %s INFILE OUTFILE\n", argv[0]);
    return 1;
  }

  const char* infile  = argv[1];
  const char* outfile = argv[2];

  cstring_view infile_data;
  if (!sv_read_file(infile, &infile_data)) {
    printf("Failed to read '%s': %s\n", infile, strerror(errno));
    return 1;
  }

  lines lines = {0};
  extract_lines(&lines, infile_data);

  // pick random line
  srand(time(NULL));
  int line = rand() % lines.count;

  printf(sv_fmt "\n", sv_arg(lines.lines[line]));

  FILE* f = fopen(outfile, "w");
  if (f == NULL) {
    printf("Failed to open '%s': %s\n", outfile, strerror(errno));
    free(lines.lines);
    sv_read_file_free(infile_data);
    return 1;
  }

  // write back to same file without random line
  for (int i = 0; i < lines.count; i++) {
    if (i != line)
      fprintf(f, sv_fmt "\n", sv_arg(lines.lines[i]));
  }

  fclose(f);

  free(lines.lines);
  sv_read_file_free(infile_data);

  return 0;
}

int line_append(lines* lines, cstring_view value) {
  if (lines->count >= lines->cap) {
    lines->cap   = lines->cap == 0 ? LINE_INIT_CAP : lines->cap * 2;
    lines->lines = realloc(lines->lines, lines->cap * sizeof(*lines->lines));
  }

  lines->lines[lines->count] = value;
  lines->count++;

  return 1;
}

// split by \n, \r, \r\n
void extract_lines(lines* lines, cstring_view input) {
  cstring_view sv_line;
  sv_index_t pos;

  do {
    char c = sv_find_first_of_switch(input, svl("\n\r"), 0, &pos);
    if (c == '\n') {
      sv_line = sv_substr(input, 0, pos);
      input   = sv_remove_prefix(input, pos + 1);
    } else if (c == '\r') {
      sv_line = sv_substr(input, 0, pos);
      input   = sv_remove_prefix(input, pos + 1);
      if (input.length >= 1 && sv_front(input) == '\n') {
        input = sv_remove_prefix(input, 1);
      }
    } else if (c == '\0') { // not found
      sv_line = input;
    }

    line_append(lines, sv_line);

  } while (pos != SV_NPOS && !sv_is_empty(input));
}