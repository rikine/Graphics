#include "colorspaces.hpp"
#include <fstream>

void PNM::input_single_file()
{
    std::ifstream fin(info.input_file, std::ios_base::binary);

    if (!fin.is_open())
        throw MyException("File aren't open");

    std::string buffer;
    fin >> buffer;
    if (buffer != "P6")
        throw MyException("File must be P6 format");

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
        fin.read((char *)&claster, 3);
        data[i].A = claster[0];
        data[i].B = claster[1];
        data[i].C = claster[2];
    }
    if (!fin.good())
        throw MyException("Invalid file", fin.gcount());
    fin.close();
}

void PNM::input_multiple_files()
{
    std::string temp1 = info.input_file;
    std::ifstream fin1(temp1.insert(temp1.size() - 4, "_1"), std::ios_base::binary);
    temp1 = info.input_file;
    std::ifstream fin2(temp1.insert(temp1.size() - 4, "_2"), std::ios_base::binary);
    temp1 = info.input_file;
    std::ifstream fin3(temp1.insert(temp1.size() - 4, "_3"), std::ios_base::binary);

    if (!fin1.is_open() || !fin2.is_open() || !fin3.is_open())
        throw MyException("Inputed files aren't open");

    std::string buffer;
    fin1 >> buffer;
    if (buffer != "P5")
        throw MyException("First file must be P5 format");
    fin2 >> buffer;
    if (buffer != "P5")
        throw MyException("Second file must be P5 format");
    fin3 >> buffer;
    if (buffer != "P5")
        throw MyException("Third file must be P5 format");

    header head2, head3;
    fin1 >> head.width >> head.height >> head.colors;
    fin2 >> head2.width >> head2.height >> head2.colors;
    fin3 >> head3.width >> head3.height >> head3.colors;
    if (!fin1.good() || !fin2.good() || !fin3.good())
        throw MyException("Invalid file(s)");

    if (head.width != head2.width || head.width != head3.width || head.height != head2.height || head.height != head3.height)
        throw MyException("Files are different");
    if (head.colors != 255 || head2.colors != 255 || head3.colors != 255)
        throw MyException("Colors must be 255 in total");

    fin1.ignore();
    fin2.ignore();
    fin3.ignore();
    uchar c = 0;
    data.resize(head.width * head.height);
    for (int i = 0; i < head.width * head.height; i++)
    {
        fin1.read((char *)&c, 1);
        data[i].A = c;
        fin2.read((char *)&c, 1);
        data[i].B = c;
        fin3.read((char *)&c, 1);
        data[i].C = c;
    }
    if (!fin1.good() || !fin2.good() || !fin3.good())
        throw MyException("Inputed files are invalid");
    fin1.close();
    fin2.close();
    fin3.close();
}

void PNM::input(int argc, char *argv[])
{
    if (argc != 11)
        throw MyException("Not enough or too much arguments", argc);

    for (int i = 1; i < argc; i++)
    {
        std::string buff = argv[i];
        if (buff == "-f")
        {
            buff = argv[++i];
            if (buff == "RGB")
            {
                info.current_cs = RGB;
                continue;
            }
            if (buff == "HSL")
            {
                info.current_cs = HSL;
                continue;
            }
            if (buff == "HSV")
            {
                info.current_cs = HSV;
                continue;
            }
            if (buff == "YCbCr.601")
            {
                info.current_cs = YCbCr_601;
                continue;
            }
            if (buff == "YCbCr.709")
            {
                info.current_cs = YCbCr_709;
                continue;
            }
            if (buff == "YCoCg")
            {
                info.current_cs = YCoCg;
                continue;
            }
            if (buff == "CMY")
            {
                info.current_cs = CMY;
                continue;
            }
            throw MyException("Wrong -f Color Space");
        }
        if (buff == "-t")
        {
            buff = argv[++i];
            if (buff == "RGB")
            {
                info.convert_to_cs = RGB;
                continue;
            }
            if (buff == "HSL")
            {
                info.convert_to_cs = HSL;
                continue;
            }
            if (buff == "HSV")
            {
                info.convert_to_cs = HSV;
                continue;
            }
            if (buff == "YCbCr.601")
            {
                info.convert_to_cs = YCbCr_601;
                continue;
            }
            if (buff == "YCbCr.709")
            {
                info.convert_to_cs = YCbCr_709;
                continue;
            }
            if (buff == "YCoCg")
            {
                info.convert_to_cs = YCoCg;
                continue;
            }
            if (buff == "CMY")
            {
                info.convert_to_cs = CMY;
                continue;
            }
            throw MyException("Wrong -t Color Space");
        }
        if (buff == "-i")
        {
            buff = argv[++i];
            if (buff == "1")
                info.input_files = false;
            else if (buff == "3")
                info.input_files = true;
            else
                throw MyException("Wrong count of input file arguments", stoi(buff));

            if (i + 1 >= argc)
                throw MyException("No input file", argc);
            info.input_file = argv[++i];
        }
        if (buff == "-o")
        {
            buff = argv[++i];
            if (buff == "1")
                info.output_files = false;
            else if (buff == "3")
                info.output_files = true;
            else
                throw MyException("Wrong count of output file arguments", stoi(buff));

            if (i + 1 >= argc)
                throw MyException("No output file", argc);
            info.output_file = argv[++i];
        }
    }

    if (info.input_files)
        input_multiple_files();
    else
        input_single_file();
}

void PNM::save_single_file()
{
    std::ofstream fout(info.output_file, std::ios_base::binary);
    fout.exceptions(std::ios_base::failbit | std::ios_base::badbit);

    if (!fout)
        throw MyException("Error while writing");
    fout << "P6" << std::endl;
    fout << head.width << " " << head.height << std::endl;
    fout << head.colors << std::endl;

    fout.write((char *)&data[0], head.width * head.height * 3);
    fout.close();
}

void PNM::save_multiple_files()
{
    std::string temp1 = info.output_file;
    std::ofstream fout1(temp1.insert(temp1.size() - 4, "_1"), std::ios_base::binary);
    temp1 = info.output_file;
    std::ofstream fout2(temp1.insert(temp1.size() - 4, "_2"), std::ios_base::binary);
    temp1 = info.output_file;
    std::ofstream fout3(temp1.insert(temp1.size() - 4, "_3"), std::ios_base::binary);
    fout1.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    fout2.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    fout3.exceptions(std::ios_base::failbit | std::ios_base::badbit);

    if (!fout1.is_open() || !fout2.is_open() || !fout3.is_open())
        throw MyException("Writing error");

    fout1 << "P5" << std::endl;
    fout2 << "P5" << std::endl;
    fout3 << "P5" << std::endl;
    fout1 << head.width << " " << head.height << std::endl
          << head.colors << std::endl;
    fout2 << head.width << " " << head.height << std::endl
          << head.colors << std::endl;
    fout3 << head.width << " " << head.height << std::endl
          << head.colors << std::endl;

    for (int i = 0; i < head.width * head.height; i++)
    {
        fout1.write((char *)&data[i].A, 1);
        fout2.write((char *)&data[i].B, 1);
        fout3.write((char *)&data[i].C, 1);
    }
}

void PNM::save()
{
    if (info.output_files)
        save_multiple_files();
    else
        save_single_file();
}

void color_spaces::make_RGB()
{
    switch (info.current_cs)
    {
    case RGB:
        break;
    case HSL:
        From_HSL();
        break;
    case HSV:
        From_HSV();
    case YCbCr_601:
        From_YCbCr_601();
        break;
    case YCbCr_709:
        From_YCbCr_709();
        break;
    case YCoCg:
        From_YCoCg();
        break;
    case CMY:
        From_CMY();
        break;
    default:
        break;
    }
}

void color_spaces::convert_to()
{
    switch (info.convert_to_cs)
    {
    case RGB:
        break;
    case HSL:
        To_HSL();
        break;
    case HSV:
        To_HSV();
    case YCbCr_601:
        To_YCbCr_601();
        break;
    case YCbCr_709:
        To_YCbCr_709();
        break;
    case YCoCg:
        To_YCoCg();
        break;
    case CMY:
        To_CMY();
        break;
    default:
        break;
    }
}

void PNM::make()
{
    if (info.current_cs == info.convert_to_cs)
        return;
    make_RGB();
    convert_to();
}

void color_spaces::To_HSL()
{
    for (size_t i = 0; i < data.size(); i++)
    {
        R = data[i].A / 255.0;
        B = data[i].B / 255.0;
        G = data[i].C / 255.0;

        MAX = std::max(R, std::max(G, B));
        MIN = std::min(R, std::min(G, B));

        double delta = MAX - MIN;
        if (delta == 0)
            H = 0;
        else if (MAX == R)
            H = 60 * (fmod((G - B) / delta, 6.0));
        else if (MAX == G)
            H = 60 * ((B - R) / delta + 2);
        else if (MAX == B)
            H = 60 * ((R - G) / delta + 4);

        S = delta == 0 ? 0 : (delta) / (1 - abs(2 * L - 1));
        L = (MAX + MIN) / 2.0;

        data[i].A = (uchar)round((H / 360.0) * 255);
        data[i].B = (uchar)round(S * 255);
        data[i].C = (uchar)round(L * 255);
    }
}

void color_spaces::From_HSL()
{
    for (size_t i = 0; i < data.size(); i++)
    {
        H = (data[i].A / 255.0) * 360.0;
        S = data[i].B / 255.0;
        L = data[i].C / 255.0;

        C = (1 - abs(2 * L - 1)) * S;
        X = C * (1 - abs(fmod(H / 60, 2) - 1));
        m = L - C / 2.0;

        if (H <= 60)
        {
            data[i].A = 255 * (C + m);
            data[i].B = 255 * (X + m);
            data[i].C = 255 * m;
        }
        else if (H <= 120)
        {
            data[i].A = 255 * (X + m);
            data[i].B = 255 * (C + m);
            data[i].C = 255 * m;
        }
        else if (H <= 180)
        {
            data[i].A = 255 * m;
            data[i].B = 255 * (C + m);
            data[i].C = 255 * (X + m);
        }
        else if (H <= 240)
        {
            data[i].A = 255 * m;
            data[i].B = 255 * (X + m);
            data[i].C = 255 * (C + m);
        }
        else if (H <= 300)
        {
            data[i].A = 255 * (X + m);
            data[i].B = 255 * m;
            data[i].C = 255 * (C + m);
        }
        else
        {
            data[i].A = 255 * (C + m);
            data[i].B = 255 * m;
            data[i].C = 255 * (X + m);
        }
    }
}

void color_spaces::To_HSV()
{
    for (size_t i = 0; i < data.size(); i++)
    {
        R = data[i].A / 255.0;
        B = data[i].B / 255.0;
        G = data[i].C / 255.0;

        MAX = std::max(R, std::max(G, B));
        MIN = std::min(R, std::min(G, B));

        double delta = MAX - MIN;
        if (delta == 0)
            H = 0;
        else if (MAX == R)
            H = 60 * (fmod((G - B) / delta, 6.0));
        else if (MAX == G)
            H = 60 * ((B - R) / delta + 2);
        else if (MAX == B)
            H = 60 * ((R - G) / delta + 4);

        S = MAX == 0 ? 0 : delta / MAX;
        V = MAX;

        data[i].A = (uchar)round((H / 360.0) * 255);
        data[i].B = (uchar)round(S * 255);
        data[i].C = (uchar)round(V * 255);
    }
}

void color_spaces::From_HSV()
{
    for (size_t i = 0; i < data.size(); i++)
    {
        H = (data[i].A / 255.0) * 360.0;
        S = data[i].B / 255.0;
        V = data[i].C / 255.0;

        C = V * S;
        X = C * (1 - abs(fmod(H / 60, 2) - 1));
        m = V - C;

        if (H <= 60)
        {
            data[i].A = 255 * (C + m);
            data[i].B = 255 * (X + m);
            data[i].C = 255 * m;
        }
        else if (H <= 120)
        {
            data[i].A = 255 * (X + m);
            data[i].B = 255 * (C + m);
            data[i].C = 255 * m;
        }
        else if (H <= 180)
        {
            data[i].A = 255 * m;
            data[i].B = 255 * (C + m);
            data[i].C = 255 * (X + m);
        }
        else if (H <= 240)
        {
            data[i].A = 255 * m;
            data[i].B = 255 * (X + m);
            data[i].C = 255 * (C + m);
        }
        else if (H <= 300)
        {
            data[i].A = 255 * (X + m);
            data[i].B = 255 * m;
            data[i].C = 255 * (C + m);
        }
        else
        {
            data[i].A = 255 * (C + m);
            data[i].B = 255 * m;
            data[i].C = 255 * (X + m);
        }
    }
}

void color_spaces::To_YCbCr_601()
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

void color_spaces::From_YCbCr_601()
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

void color_spaces::To_YCbCr_709()
{
    double Kr = 0.0722;
    double Kg = 0.2126;
    double Kb = 0.7152;
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

void color_spaces::From_YCbCr_709()
{
    double Kr = 0.0722;
    double Kg = 0.2126;
    double Kb = 0.7152;
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

void color_spaces::To_YCoCg()
{
    double Y, Co, Cg;
    for(size_t i = 0; i < data.size(); i++){
        R = data[i].A / 255.0;
		G = data[i].B / 255.0;
		B = data[i].C / 255.0;

		Y = R / 4 + G / 2 + B / 4;
		Co = R / 2 - B / 2;
		Cg = -R / 4 + G / 2 - B / 4;

		data[i].A = Y * 255.0;
		data[i].B = (Co + 0.5) * 255.0;
		data[i].C = (Cg + 0.5) * 255.0;
    }
}

void color_spaces::From_YCoCg()
{
    double Y, Co, Cg;
    for (size_t i = 0; i < data.size(); i++) {
		Y = data[i].A / 255.0;
		Co = (data[i].B / 255.0) - 0.5;
		Cg = (data[i].C / 255.0) - 0.5;

		R = Y + Co - Cg;
		G = Y + Cg;
		B = Y - Co - Cg;

		R = std::max(0.0, std::min(1.0, R));
        G = std::max(0.0, std::min(1.0, G));
        B = std::max(0.0, std::min(1.0, B));

		data[i].A = R * 255.0;
		data[i].B = G * 255.0;
		data[i].C = B * 255.0;
	}
}

void color_spaces::To_CMY()
{
    for (size_t i = 0; i < data.size(); i++)
    {
        R = data[i].A / 255.0;
		G = data[i].B / 255.0;
		B = data[i].C / 255.0;

		C = 1 - R;
		M = 1 - G;
		Y = 1 - B;

		data[i].A = C * 255.0;
		data[i].B = M  * 255.0;
		data[i].C = Y * 255.0;
    }    
}

void color_spaces::From_CMY()
{
    for (size_t i = 0; i < data.size(); i++)
    {
        C = data[i].A / 255.0;
		M = data[i].B / 255.0;
		Y = data[i].C / 255.0;

		R = 1 - C;
		G = 1 - M;
		B = 1 - Y;

		data[i].A = R * 255.0;
		data[i].B = G  * 255.0;
		data[i].C = B * 255.0;
    }    
}
