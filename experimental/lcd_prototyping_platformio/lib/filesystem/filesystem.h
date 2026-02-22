#pragma once

#include "status.h"

#include <string>
#include <vector>

typedef struct {
    std::string name;
    unsigned long long size;
} FileInfo;

Status filesystem_init();
Status filesystem_get_file_list(std::vector<FileInfo>& file_list);
Status filesystem_open_file(const char* filename);
Status filesystem_read_file(char* buf, size_t len, size_t* actual_len);
Status filesystem_seek_file(size_t offset);
Status filesystem_close_file();