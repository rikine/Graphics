#include <exception>
#include <string>
#include <vector>
#include <cmath>
#pragma once

typedef unsigned char uchar;

class MyException : public std::exception
{
    std::string msg;
    int extra;

public:
    MyException(std::string msg, int extra = 0) : msg(msg), extra(extra) {}

    const std::string whats() const noexcept
    {
        return msg;
    }

    int extra_info() const noexcept
    {
        return extra;
    }
};

class Picture
{
protected:
    struct Pixel
    {
        uchar A;
        uchar B;
        uchar C;
    };

    enum Auto_Correction
    {
        To_RGB,
        To_YCbCr_601,
        To_RGB_Min_Max,
        To_YCbCr_601_Min_Max,
        To_RGB_Min_Max_39,
        To_YCbCr_601_Min_Max_39,
    };

    struct info
    {
        std::string input_file;
        std::string output_file;
        Auto_Correction ac;
        bool colorful;
        double multiplier;
        int offset;
    } info;

    struct header
    {
        int width;
        int height;
        int colors;
    } head;

    std::vector<Pixel> data;

public:
    virtual void input(int argc, char *argv[]) = 0;
    virtual void make() = 0;
    virtual void save() = 0;

    virtual ~Picture() = default;
};

class color_spaces : virtual public Picture
{
    double R, G, B, Y, U, V;
protected:
    void to_YCbCr_601();
    void from_YCbCr_601();
};

class auto_correction : virtual public Picture
{
protected:
    void auto_correct(int offset, double multiplier);
    void auto_correct_min_max();
    void auto_correct_min_max_39();
};

class PNM : public auto_correction, public color_spaces
{
    void input_single_file();
    void input(int argc, char *argv[]) override;

    void make();

    void save_single_file();
    void save();
};
