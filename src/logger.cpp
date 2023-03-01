//
// Created by KILLdon on 06.06.2019.
//

#include "logger.h"

#include <cstring>

Logger::Logger() {
    file = fopen(filename, "wt");
    if (!file) {
        std::cerr << "FATAL LOG ERROR: Can't open logfile" << std::endl;
    }
}
Logger::~Logger() {
    fclose(file);
}

void Logger::setFilename(const char *filename) {
    if (strcmpi(this->filename, filename) != 0) {
        FILE *file = fopen(filename, "wt");
        if (!file) {
            std::cerr << "ERROR LOG ERROR: Can't open logfile" << std::endl;
        } else {
            fclose(this->file);
            this->file = file;
            this->filename = filename;
        }
    }
}
