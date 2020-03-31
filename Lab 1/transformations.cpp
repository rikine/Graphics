#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

enum Operations {
	invertion,
	gorizontal,
	vertical,
	clockwize_on,
	clockwize_off
};

struct Header {
	string head;
	int width;
	int height;
	int total;
};

class P5 {
private:
	char **data;
	char **temp;

public:
	P5(Header &header, ifstream &fin) {
		data = new char*[header.height];
		for (int i = 0; i < header.height; i++)
			data[i] = new char[header.width];

		char p;
		fin.read(&p, 1);
		for (int i = 0; i < header.height; i++)
			fin.read(data[i], header.width);
	}

	void invertion_f(Header &header) {
		for (int i = 0; i < header.height; i++)
			for (int j = 0; j < header.width; j++)
				data[i][j] = abs(data[i][j] - header.total);
	}

	void goriz(Header &header) {
		for (int i = 0; i < header.height; i++)
			for (int j = 0; j < header.width / 2; j++)
				swap(data[i][j], data[i][header.width - j - 1]);
	}

	void vert(Header &header) {
		for (int i = 0; i < header.width; i++)
			for (int j = 0; j < header.height / 2; j++)
				swap(data[j][i], data[header.height - j - 1][i]);
	}

	void povpo(Header &header) {
		temp = new char*[header.width];
		for (int i = 0; i < header.width; i++)
			temp[i] = new char[header.height];

		for (int i = 0; i < header.width; i++)
			for (int j = 0; j < header.height; j++)
				temp[i][j] = data[header.height - j - 1][i];
	}

	void povpro(Header &header) {
		temp = new char*[header.width];
		for (int i = 0; i < header.width; i++)
			temp[i] = new char[header.height];

		for (int i = 0; i < header.width; i++)
			for (int j = 0; j < header.height; j++)
				temp[i][j] = data[j][header.width - i - 1];
	}

	void save(Header &header, char *argv[]) {
		ofstream fout(string(argv[2]) + ".pgm", ios::binary);
		if (argv[3][0] - '0' == invertion || argv[3][0] - '0' == gorizontal || argv[3][0] - '0' == vertical) {
			fout << "P5" << '\n';
			fout << header.width << ' ' << header.height << '\n';
			fout << header.total << '\n';
			for (int i = 0; i < header.height; i++)
				fout.write(data[i], header.width);

			for (int i = 0; i < header.height; i++)
				delete[] data[i];
		}
		else if (argv[3][0] - '0' == clockwize_on || argv[3][0] - '0' == clockwize_off) {
			fout << "P5" << '\n';
			fout << header.height << ' ' << header.width << '\n';
			fout << header.total << '\n';
			for (int i = 0; i < header.width; i++)
				fout.write(temp[i], header.height);

			for (int i = 0; i < header.width; i++)
				delete[] temp[i];
		}
		fout.close();
	}
};

class P6 {
private:
	char **data;
	char **temp;
public:

	P6(const Header &header, ifstream &fin) {
		data = new char*[header.height];
		for (int i = 0; i < header.height; i++)
			data[i] = new char[header.width * 3];

		char p;
		fin.read(&p, 1);
		for (int i = 0; i < header.height; i++) {
			if (fin.eof()) {
				cout << "Not correct image";
				exit(0);
			}
			fin.read(data[i], header.width * 3);
		}
	}

	void invertion_f(Header &header) {
		for (int i = 0; i < header.height; i++)
			for (int j = 0; j < header.width * 3; j++)
				data[i][j] = abs(data[i][j] - header.total);
	}

	void goriz(Header &header) {
		for (int i = 0; i < header.height; i++)
			for (int j = 0; j < 3 * header.width / 2; j += 3) {
				swap(data[i][j], data[i][header.width * 3 - j - 3]);
				swap(data[i][j + 1], data[i][header.width * 3 - j - 2]);
				swap(data[i][j + 2], data[i][header.width * 3 - j - 1]);
			}
	}

	void vert(Header &header) {
		for (int i = 0; i < 3 * header.width; i++)
			for (int j = 0; j < header.height / 2; j++) {
				swap(data[j][i], data[header.height - j - 1][i]);
			}
	}

	void povpo(Header &header) {
		temp = new char*[header.width];
		for (int i = 0; i < header.width; i++)
			temp[i] = new char[header.height * 3];

		int k = 1;
		for (int i = 0; i < header.width; i++)
			for (int j = 0; j < header.height * 3; j += 3) {
				if (j == 0)
					k = 0;
				temp[i][j] = data[header.height + k - 1][i * 3];
				temp[i][j + 1] = data[header.height + k - 1][i * 3 + 1];
				temp[i][j + 2] = data[header.height + k - 1][i * 3 + 2];
				k--;
			}
	}

	void povpro(Header &header) {
		temp = new char*[header.width];
		for (int i = 0; i < header.width; i++)
			temp[i] = new char[header.height * 3];

		int k = 1;
		for (int i = 0; i < header.width; i++)
			for (int j = 0; j < header.height * 3; j += 3) {
				if (j == 0)
					k = 0;
				temp[i][j] = data[k][header.width * 3 - 3 * i - 3];
				temp[i][j + 1] = data[k][header.width * 3 - 3 * i - 2];
				temp[i][j + 2] = data[k][header.width * 3 - 3 * i - 1];
				k++;
			}
	}

	void save(Header &header, char *argv[]) {
		ofstream fout(string(argv[2]) + ".ppm", ios::binary);
		if (argv[3][0] - '0' == invertion || argv[3][0] - '0' == gorizontal || argv[3][0] - '0' == vertical) {
			fout << "P6" << '\n';
			fout << header.width << ' ' << header.height << '\n';
			fout << header.total << '\n';
			for (int i = 0; i < header.height; i++)
				fout.write(data[i], header.width * 3);

			for (int i = 0; i < header.height; i++)
				delete[] data[i];
		}
		else if (argv[3][0] - '0' == clockwize_on || argv[3][0] - '0' == clockwize_off) {
			fout << "P6" << '\n';
			fout << header.height << ' ' << header.width << '\n';
			fout << header.total << '\n';
			for (int i = 0; i < header.width; i++)
				fout.write(temp[i], header.height * 3);

			for (int i = 0; i < header.width; i++)
				delete[] temp[i];
		}
		fout.close();
	}
};

int main(int argc, char *argv[]) {
	if (argc < 4) {
		cout << "Not enought arguments";
		return 0;
	}
	
	Header header;
	string file = argv[1];
	int size = file.size() - 1;

	ifstream fin(file, ios::binary);

	if (!fin) {
		cout << "File aren't found";
		return 0;
	}

	else if (!(file[size] == 'm' 
		&& (file[size - 1] == 'p' || file[size - 1] == 'g') 
		&& file[size - 2] == 'p' && file[size - 3] == '.')) {
		cout << "Invalid format";
		return 0;
	}

	fin >> header.head;
	if (!(header.head == "P5" || header.head == "P6")) {
		cout << "BItyi file";
		system("pause");
		exit(0);
	}
	fin >> header.width >> header.height >> header.total;

	//P5 FILE
	if (header.head[1] == '5') {
		P5 file(header, fin);

		if (argv[3][0] - '0' == invertion)
			file.invertion_f(header);
		else if (argv[3][0] - '0' == gorizontal)
			file.goriz(header);
		else if (argv[3][0] - '0' == vertical)
			file.vert(header);
		else if (argv[3][0] - '0' == clockwize_on)
			file.povpo(header);
		else if (argv[3][0] - '0' == clockwize_off)
			file.povpro(header);

		file.save(header, argv);
	}

	//P6 FILE
	else if (header.head[1] == '6') {
		P6 file(header, fin);

		if (argv[3][0] - '0' == invertion)
			file.invertion_f(header);
		else if (argv[3][0] - '0' == gorizontal)
			file.goriz(header);
		else if (argv[3][0] - '0' == vertical)
			file.vert(header);
		else if (argv[3][0] - '0' == clockwize_on)
			file.povpo(header);
		else if (argv[3][0] - '0' == clockwize_off)

			file.povpro(header);

		file.save(header, argv);
	}
	
	fin.close();
	return 0;
}
