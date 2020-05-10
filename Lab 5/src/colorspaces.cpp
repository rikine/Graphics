#include "colorspaces.hpp"
#include <fstream>
#include <string>

void PNM::input_single_file()
{
    std::ifstream fin(info.input_file, std::ios_base::binary);

    if (!fin.is_open())
        throw MyException("File aren't open");

    std::string buffer;
    fin >> buffer;
    if (buffer == "P5")
        info.colorful = false;
    else if (buffer == "P6")
        info.colorful = true;
    else
        throw MyException("File must be P5 or P6 format");

    fin >> head.width >> head.height >> head.colors;
    if (!fin.good())
        throw MyException("Invalid file");
    if (head.colors != 255)
        throw MyException("Colors must be 255 in total");

    fin.ignore();
    data.resize(head.height * head.width);
    uchar claster[3];
    for (int i = 0; i < head.width * head.height; i++)
    {
        if (info.colorful)
        {
            fin.read((char *)&claster, 3);
            data[i].A = claster[0];
            data[i].B = claster[1];
            data[i].C = claster[2];
        }
        else
        {
            fin.read((char *)&claster, 1);
            data[i].A = data[i].B = data[i].C = claster[0];
        }
    }
    if (!fin.good())
        throw MyException("Invalid file", fin.gcount());
    fin.close();
}

void PNM::input(int argc, char *argv[])
{
    if (argc != 4 && argc != 6)
        throw MyException("Wrong count of arguments", argc);

    info.input_file = std::string(argv[1]);
    info.output_file = std::string(argv[2]);

    int transformation = std::stoi(argv[3]);
    if (transformation >= 0 && transformation <= 5)
        info.ac = (Auto_Correction)transformation;
    else
        throw MyException("Wrong transformation", transformation);

    if (info.ac == Auto_Correction::To_RGB || info.ac == Auto_Correction::To_YCbCr_601)
    {
        if (argc != 6)
            throw MyException("Wrong count of arguments", argc);
        info.offset = std::stoi(argv[4]);
        info.multiplier = std::stod(argv[5]);
    }
    input_single_file();
}

void PNM::save_single_file()
{
    std::ofstream fout(info.output_file, std::ios_base::binary);
    fout.exceptions(std::ios_base::failbit | std::ios_base::badbit | std::ios_base::eofbit);

    if (!fout.is_open())
        throw MyException("Writing error");

    if (info.colorful)
        fout << "P6" << std::endl;
    else
        fout << "P5" << std::endl;
    fout << head.width << " " << head.height << std::endl
         << head.colors << std::endl;

    if (info.colorful)
    {
        fout.write((char *)&data[0], head.height * head.width * 3);
    }
    else
        for (int i = 0; i < head.width * head.height; i++)
        {
            fout.write((char *)&data[i].A, 1);
        }
    fout.close();
}

void PNM::save()
{
    save_single_file();
}

void PNM::make()
{
    switch (info.ac)
    {
    case Auto_Correction::To_RGB:
        to_YCbCr_601();
    case Auto_Correction::To_YCbCr_601:
        auto_correct(info.offset, info.multiplier);
        break;
    case Auto_Correction::To_RGB_Min_Max:
        to_YCbCr_601();
    case Auto_Correction::To_YCbCr_601_Min_Max:
        auto_correct_min_max();
        break;
    case Auto_Correction::To_RGB_Min_Max_39:
        to_YCbCr_601();
    case Auto_Correction::To_YCbCr_601_Min_Max_39:
        auto_correct_min_max_39();
        break;
    default:
        break;
    }

    switch (info.ac)
    {
    case Auto_Correction::To_RGB:
        from_YCbCr_601();
        break;
    case Auto_Correction::To_RGB_Min_Max:
        from_YCbCr_601();
        break;
    case Auto_Correction::To_RGB_Min_Max_39:
        from_YCbCr_601();
        break;
    default:
        break;
    }
}

void auto_correction::auto_correct(int offset, double multiplier)
{
    for (size_t i = 0; i < data.size(); i++)
    {
        double res = (data[i].A - offset) * multiplier;
        res = std::max(0.0, std::min(255.0, res));
        data[i].A = (uchar)round(res);
    }
}

void auto_correction::auto_correct_min_max()
{
    uchar min = 0, max = 255;
    for(size_t i = 0; i < data.size(); i++){
        min = std::min(min, data[i].A);
        max = std::max(max, data[i].A);
    }
    uchar res = 1;
    if(min != max)
        res = 255.0 / (max - min);
    auto_correct(min, res);
}

void auto_correction::auto_correct_min_max_39()
{
    uchar min, max;
    uchar colors[256];
    for(size_t i = 0; i < data.size(); i++)
        colors[data[i].A]++;
    
    size_t to_del = data.size() * 0.0039, deleted = 0, pos_min = 0, pos_max = 255;
    while(deleted < to_del){
        if(deleted % 2){
            while(colors[pos_min] == 0)
            {
                pos_min++;
            }
            colors[pos_min]--;
            deleted++;
        }
        else{
            while (colors[pos_max] == 0)
            {
                pos_max--;
            }
            colors[pos_max]--;
            deleted++;
        }
    }
    int i = 0;
    while(i < 256 && colors[i] == 0)
        i++;
    min = colors[i];

    i = 255;
    while (i >= 0 && colors[i] == 0)
        i--;
    max = colors[i];

    if(min > max)
        std::swap(min, max);

    uchar res = 1;
    if (min != max)
        res = 255.0 / (max - min);
    auto_correct(min, res);
}

void color_spaces::to_YCbCr_601()
{
    double Kr = 0.299;
    double Kg = 0.587;
    double Kb = 0.114;
    for (size_t i = 0; i < data.size(); i++)
    {
        R = data[i].A / 255.0;
        G = data[i].B / 255.0;
        B = data[i].C / 255.0;

        Y = Kr * R + Kg * G + Kb * B;
        U = 0.5 * ((B - Y) / (1.0 - Kb));
        V = 0.5 * ((R - Y) / (1.0 - Kr));

        data[i].A = (uchar)round(Y * 255.0);
        data[i].B = (uchar)round((U + 0.5) * 255.0);
        data[i].C = (uchar)round((V + 0.5) * 255.0);
    }
}

void color_spaces::from_YCbCr_601()
{
    double Kr = 0.299;
    double Kg = 0.587;
    double Kb = 0.114;
    for (size_t i = 0; i < data.size(); i++)
    {
        Y = data[i].A / 255.0;
        U = data[i].B / 255.0 - 0.5;
        V = data[i].C / 255.0 - 0.5;
        R = (Y + V * (2 - 2 * Kr));
        G = (Y - (Kb / Kg) * (2 - 2 * Kb) * U - (Kr / Kg) * (2 - 2 * Kr) * V);
        B = (Y + (2 - 2 * Kb) * U);

        R = std::max(0.0, std::min(1.0, R));
        G = std::max(0.0, std::min(1.0, G));
        B = std::max(0.0, std::min(1.0, B));

        data[i].A = (uchar)round(R * 255);
        data[i].B = (uchar)round(G * 255);
        data[i].C = (uchar)round(B * 255);
    }
}