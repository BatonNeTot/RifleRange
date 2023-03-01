//
// Created by KILLdon on 01.06.2019.
//

#ifndef RIFLERANGE_DATA_H
#define RIFLERANGE_DATA_H

template<typename T>
struct Size {
    T width;
    T height;

    Size() : width(0), height(0) {};

    Size(T width, T height) : width(width), height(height) {};
};

template<typename T>
struct Point {
    T x;
    T y;

    Point() : x(0), y(0) {};

    Point(T x, T y) : x(x), y(y) {};

    // Lua


};

#endif //RIFLERANGE_DATA_H
