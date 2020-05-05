#include <exception>
#include <string>
#include <vector>
#include <cmath>
#pragma once

typedef unsigned char uchar;

class MyException : public std::exception{
    std::string msg;
    int extra;
public:
    MyException(std::string msg, int extra = 0) : msg(msg), extra(extra){}

    const std::string whats() const noexcept{
        return msg;
    }

    int extra_info() const noexcept{
        return extra;
    }
};

class Picture{
protected:
    struct Pixel{
        uchar A;
        uchar B;
        uchar C;
    };

    enum ColorSpace{
        RGB,
	    HSL,
	    HSV,
	    YCbCr_601,
	    YCbCr_709,
	    YCoCg,
	    CMY
    };

    struct info{
        std::string input_file;
        std::string output_file;
        ColorSpace current_cs;
        ColorSpace convert_to_cs;
        bool input_files;
        bool output_files;
    } info;

    struct header{
        int width;
        int height;
        int colors;
    } head;

    std::vector<Pixel> data;
    
public:
    virtual void input(int argc, char* argv[]) = 0;
    virtual void make() = 0;
    virtual void save() = 0;

    virtual ~Picture() = default;
};

class color_spaces: public Picture{
    double R, G, B;
    double MAX, MIN;
    double C, M, Y;
    double Co, Cg;
    double U, V;
    double H, S, X, m;
    double L;
    
protected:
    void make_RGB();
    void convert_to();

    void To_HSL();
    void From_HSL();
    void To_HSV();
    void From_HSV();

    void To_YCbCr_601();
    void From_YCbCr_601();
    void To_YCbCr_709();
    void From_YCbCr_709();

    void To_YCoCg();
    void From_YCoCg();

    void To_CMY();
    void From_CMY();
};

class PNM: public color_spaces{
    void input_single_file();
    void input_multiple_files();
    void input(int argc, char* argv[]) override;

    void make();

    void save_single_file();
    void save_multiple_files();
    void save();
};
