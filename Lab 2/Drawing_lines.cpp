#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <conio.h>
using namespace std;

typedef unsigned char uchar;
typedef pair<double, double> pdd;

struct header {
	string type;
	int width;
	int height;
	int colors;
};

class LineDraw {
private:
	header head;
	uchar** bytes;

	inline double FPart(double x) {
		return x - (int)x;
	}

	void save(char *argv[]) {
		ofstream fout(argv[2], ios::binary);
		fout << head.type << '\n';
		fout << head.width << ' ' << head.height << '\n' << head.colors << '\n';
		for (int i = 0; i < head.height; i++)
			fout.write((char*)bytes[i], head.width);
		fout.close();
	}

	void new_point(int y, int x, double koef, uchar clr) {
		if (y < 0 || x < 0 || x >= head.width || y >= head.height)
			return;
		koef = max(min(koef, 1.0), 0.0);
		double clr_to_lin = clr / 255.0;
		double lightofpic_to_lin = bytes[x][y] / 255.0;
		double light_of_lin = lightofpic_to_lin <= 0.04045 ? lightofpic_to_lin / 12.92 : pow((lightofpic_to_lin + 0.055) / 1.055, 2.4);
		double c = (1 - koef)*clr_to_lin + koef * light_of_lin;
		double srgb = c <= 0.0031308 ? 12.92 * c : 1.055 * pow(c, 1 / 2.4) - 0.055;
		bytes[x][y] = srgb * 255;
	}

	double dist(double x1, double y1, double x, double y) {
		return sqrt((x1 - x)*(x1 - x) + (y1 - y)*(y1 - y));
	}

	void make_line(char *argv[]) {
		uchar clr = stoi(argv[3]);
		double thinkness = stof(argv[4]);
		pair<double, double> start = make_pair(stof(argv[5]), stof(argv[6]));
		pair<double, double> end = make_pair(stof(argv[7]), stof(argv[8]));

		bool steep = abs(start.second - end.second) > abs(start.first - end.first);

		if (steep) {
			swap(start.first, start.second);
			swap(end.first, end.second);
		}
		if (start.first > end.first) {
			swap(start.first, end.first);
			swap(start.second, end.second);
		}

		double dx = end.first - start.first;
		double dy = end.second - start.second;
		double grad = dy / dx;

		int xpxl1 = start.first;
		int xpxl2 = end.first;
		double intersectY = start.second + grad * (round(start.first) - start.first);

		for (int x = xpxl1; x <= xpxl2; x++) {
			for (int j = (int)(intersectY - (thinkness - 1) / 2); j <= (int)(intersectY - (thinkness - 1) / 2 + thinkness); j++)
			{
				if (steep)
					new_point(j, x, 1 - min(1.0, (thinkness + 1.0) / 2.0 - abs(intersectY - j)), clr);
				else
					new_point(x, j, 1 - min(1.0, (thinkness + 1.0) / 2.0 - abs(intersectY - j)), clr);
			}
			intersectY += grad;
		}

		for (int x = round(start.first) - thinkness / 2; x < round(start.first); x++) {
			intersectY = start.second;
			for (int y = int(intersectY - (thinkness - 1) / 2.0); y <= int(intersectY - (thinkness - 1) / 2.0 + thinkness); y++) {
				if(steep)
					new_point(y, x, 1 - min(1.0, (thinkness + 0.5) / 2.0 - dist(y, x, round(start.second), round(start.first))), clr);
				else
					new_point(x, y, 1 - min(1.0, (thinkness + 0.5) / 2.0 - dist(x, y, round(start.first), round(start.second))), clr);
			}
		}

		for (int x = round(end.first) + 1; x <= round(end.first) + thinkness / 2; x++) {
			intersectY = end.second;
			for (int y = int(intersectY - (thinkness - 1) / 2.0); y <= int(intersectY - (thinkness - 1) / 2.0 + thinkness); y++) {
				if (steep)
					new_point(y, x, 1 - min(1.0, (thinkness + 0.5) / 2.0 - dist(y, x, round(end.second), round(end.first))), clr);
				else
					new_point(x, y, 1 - min(1.0, (thinkness + 0.5) / 2.0 - dist(x, y, round(end.first), round(end.second))), clr);
			}
		}
	}

public:
	LineDraw(header &header, ifstream &fin) {
		bytes = new uchar*[header.height];
		for (int i = 0; i < header.height; i++)
			bytes[i] = new uchar[header.width];
		head = header;

		fin.ignore(1);
		for (int i = 0; i < header.height; i++)
			fin.read((char*)bytes[i], header.width);
		
		if (!fin) {
			cout << "Error: file is invalid";
			fin.close();
			_getch();
			exit(0);
		}
	}

	void make(char *argv[]) {
		make_line(argv);
		save(argv);
	}

	~LineDraw() {
		for(int i=0;i<head.height;i++)
			delete[] bytes[i];
		delete[] bytes;
	}
};

int main(int argc, char *argv[]) {
	header header;
	ifstream fin(argv[1], ios::binary);
	if (argc < 9 || argc > 9) {
		cout << "Not enought arguments";
		_getch();
		return 0;
	}
	if (!fin) {
		cout << "File isn't found";
		_getch();
		return 0;
	}

	fin >> header.type;
	if (header.type[0] != 'P' && header.type[1] != '5' && header.type.size() != 2) {
		cout << "Invalid file";
		_getch();
		return 0;
	}

	fin >> header.width >> header.height >> header.colors;
	
	LineDraw picture(header, fin);
	picture.make(argv);

	fin.close();
	return 0;
}
