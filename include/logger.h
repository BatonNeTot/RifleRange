//
// Created by KILLdon on 06.06.2019.
//

#ifndef RIFLERANGE_LOGGER_H
#define RIFLERANGE_LOGGER_H

#include "locator.h"

#include <iostream>

class Logger {
public:

    template <typename... Args>
    void print(const char *frmt, Args&&... args);

    void setFilename(const char *filename);
private:
    Logger();
    ~Logger();

    friend Locator;

    const char *filename = "log.txt";
    FILE *file;
};

template <typename... Args>
void Logger::print(const char *frmt, Args&&... args) {
    fprintf_s(file, frmt, std::forward<Args>(args)...);
    fprintf_s(file, "\n");
    fflush(file);
}

#endif //RIFLERANGE_LOGGER_H
