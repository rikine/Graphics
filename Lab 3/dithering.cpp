#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <ctime>
using namespace std;

typedef unsigned char uchar;

class IPicture {
protected:
	vector<uchar> data;

	struct header {
		string tag;
		int height;
		int width;
		int colors;
	};

	enum dithering_type
	{
		NONE, ORDERED, RANDOM, FLOYD_STEINBER, JARVIS_JUDICE_NINKE,
		SIERRA, ATKINSON, HALFTONE
	};

	struct info {
		string input_file;
		string output_file;
		bool is_gradient;
		dithering_type dt;
		char bitrate;
		double gamma;

		virtual void check() {
			if (input_file == "")
				throw exception("Bad input file");
			if (input_file == "")
				throw exception("Bad output file");
			if (is_gradient != 0 && is_gradient != 1)
				throw exception("Wrong gradient");
			if (dt < 0 || dt > 7)
				throw exception("Wrong dithering type");
			if (bitrate < 1 || bitrate > 8)
				throw exception("Wrong bitrate");
		}
	};

	info info;
	header header;

	virtual inline const size_t index(int i, int j) const {
		return i * header.width + j;
	}

	double gamma_correction(double value) const{
		value = value / 255;
		if (!info.gamma)
		{
			value = value <= 0.0031308 ? 12.92 * value : 1.055 * pow(value, 1 / 2.4) - 0.055;
		}
		else
		{
			value = pow(value, info.gamma);
		}
		return 255 * value;
	}

	double rev_gamma_correction(double value) const{
		value = value / 255;
		if (!info.gamma)
		{
			value = value <= 0.04045 ? value / 12.92 : pow((value + 0.055) / 1.055, 2.4);
		}
		else
		{
			value = pow(value, 1 / info.gamma);
		}
		return 255 * value;
	}

public:
	virtual void input(int argc, char *argv[]) = 0;
	virtual void make() = 0;
	virtual void save() = 0;
};

class Picture_Dithering : public IPicture{
	int value;

	const double Ordered[8][8] = {
		{0.0 / 64.0,  48.0 / 64.0, 12.0 / 64.0, 60.0 / 64.0, 3.0 / 64.0,  51.0 / 64.0, 15.0 / 64.0, 63.0 / 64.0},
		{32.0 / 64.0, 16.0 / 64.0, 44.0 / 64.0, 28.0 / 64.0, 35.0 / 64.0, 19.0 / 64.0, 47.0 / 64.0, 31.0 / 64.0},
		{8.0 / 64.0,  56.0 / 64.0, 4.0 / 64.0,  52.0 / 64.0, 11.0 / 64.0, 59.0 / 64.0, 7.0 / 64.0,  55.0 / 64.0},
		{40.0 / 64.0, 24.0 / 64.0, 36.0 / 64.0, 20.0 / 64.0, 43.0 / 64.0, 27.0 / 64.0, 39.0 / 64.0, 23.0 / 64.0},
		{2.0 / 64.0,  50.0 / 64.0, 14.0 / 64.0, 62.0 / 64.0, 1.0 / 64.0,  49.0 / 64.0, 13.0 / 64.0, 61.0 / 64.0},
		{34.0 / 64.0, 18.0 / 64.0, 46.0 / 64.0, 30.0 / 64.0, 33.0 / 64.0, 17.0 / 64.0, 45.0 / 64.0, 29.0 / 64.0},
		{10.0 / 64.0, 58.0 / 64.0, 6.0 / 64.0,  54.0 / 64.0, 9.0 / 64.0,  57.0 / 64.0, 5.0 / 64.0,  53.0 / 64.0},
		{42.0 / 64.0, 26.0 / 64.0, 38.0 / 64.0, 22.0 / 64.0, 41.0 / 64.0, 25.0 / 64.0, 37.0 / 64.0, 21.0 / 64.0},
	};
	const double JarvisJudiceNinke[3][5] = { {0, 0, 0, 7, 5},
											{3, 5, 7, 5, 3},
											{1, 3, 5, 3, 1} };
	const double Sierra3[3][5] = { {0, 0, 0, 5, 3},
								  {2, 4, 5, 4, 2},
								  {0, 2, 3, 2, 0} };
	const double Atkinson[3][5] = { {0, 0, 0, 1, 1},
								   {0, 1, 1, 1, 0},
								   {0, 0, 1, 0, 0} };
	const double Halftone[4][4] = {
			{13.0 / 16.0, 11.0 / 16.0, 4.0 / 16.0,  8.0 / 16.0},
			{6.0 / 16.0,  0,           3.0 / 16.0,  15.0 / 16.0},
			{14.0 / 16.0, 1.0 / 16.0,  2.0 / 16.0,  7.0 / 16.0},
			{9.0 / 16.0,  5.0 / 16.0,  10.0 / 16.0, 12.0 / 16.0},
	};

public:

	void set_value() {
		value = (int)pow(2, info.bitrate);
	}

	void no_dither() {
		for (int i = 0; i < header.height*header.width; i++) {
			double temp = rev_gamma_correction(data[i]) / 255.0;
			temp *= value - 1;
			temp = round(temp);
			data[i] = (uchar)round(gamma_correction(temp * (255.0 / (value - 1))));
		}
	}

	void ordered() {
		for (int i = 0; i < header.height; ++i)
		{
			for (int j = 0; j < header.width; ++j)
			{
				double buffer = (rev_gamma_correction(data[index(i, j)]) +
					(255.0 / (info.bitrate)) * (Ordered[i % 8][j % 8] - 0.5)) / 255.0;
				buffer = buffer < 0 ? 0 : buffer;
				buffer *= value - 1;
				buffer = round(buffer);
				data[index(i, j)] = (uchar)round((gamma_correction(buffer * (255.0 / (value - 1)))));
			}
		}
	}
	
	void random() {
		srand((unsigned int)(time(0)));
		for (int i = 0; i < header.width * header.height; ++i)
		{
			double buffer = (rev_gamma_correction(data[i]) +
				(255.0 / (info.bitrate)) * ((double)rand() / RAND_MAX - 0.75)) / 255.0;
			buffer = buffer < 0 ? 0 : buffer;
			buffer *= value;
			buffer = round(buffer);
			data[i] = (uchar)round((int)(gamma_correction(buffer * (255.0 / (value - 1)))));
		}
	}

	void halftone() {
		for (int i = 0; i < header.height; ++i)
		{
			for (int j = 0; j < header.width; ++j)
			{
				double buffer = (rev_gamma_correction(data[index(i, j)]) +
					(255.0 / info.bitrate) * (Halftone[i % 4][j % 4] - 0.75)) / 255.0;
				buffer = buffer < 0 ? 0 : buffer;
				buffer *= value;
				buffer = round(buffer);
				data[index(i, j)] = (uchar)round(gamma_correction(buffer * (255.0 / (value - 1))));
			}
		}
	}

	void floyd_steinber() {
		vector <int> error(header.height * header.width);
		for (int i = 0; i < header.height; ++i)
		{
			for (int j = 0; j < header.width; ++j)
			{
				double buffer = (rev_gamma_correction(data[index(i, j)]) + error[index(i, j)]) / 255.0;
				buffer = buffer < 0 ? 0 : buffer;
				buffer *= value - 1;
				buffer = round(buffer);
				buffer *= 255.0 / (value - 1);

				int CurrentErrorValue = data[index(i, j)] + error[index(i, j)] - (int)buffer;
				data[index(i, j)] = (uchar)round(gamma_correction(buffer));

				if (j + 1 < header.width)
					error[index(i, j)] += int(CurrentErrorValue * (7.0 / 16.0));

				if (i + 1 < header.height){
					if (j + 1 < header.width)
						error[index(i + 1, j + 1)] += int(CurrentErrorValue * (1.0 / 16.0));

					error[index(i + 1, j)] += int(CurrentErrorValue * (5.0 / 16.0));

					if ((i - 1 > 0) && (j - 1 > 0))
						error[index(i + 1, j - 1)] += int(CurrentErrorValue * (3.0 / 16.0));
				}
			}
		}
	}

	void jarvis_judice_ninke() {
		vector <int> error(header.height * header.width);
		for (int i = 0; i < header.height; ++i){
			for (int j = 0; j < header.width; ++j){
				double buffer = (rev_gamma_correction(data[index(i, j)]) + error[index(i, j)]) / 255.0;
				buffer *= (value - 1);
				buffer = round(buffer);
				buffer *= 255.0 / (value - 1);
				int CurrentErrorValue = data[index(i, j)] + error[index(i, j)] - (int)buffer;
				data[index(i, j)] = (uchar)round(gamma_correction(buffer));
				for (int k = 0; k <= 2; ++k){
					for (int l = -2; l <= 2; ++l){
						if (i + k < header.height){
							if ((k == 0) && (l > 0)){
								if (j + l < header.width)
									error[index(i + k, j + l)] += int(CurrentErrorValue * JarvisJudiceNinke[k][2 + l] / 48.0);
							}
							else{
								if ((j + l < header.width) && (j + l > 0))
									error[index(i + k, j + l)] += int(CurrentErrorValue * JarvisJudiceNinke[k][2 + l] / 48.0);
							}
						}
					}
				}
			}
		}
	}

	void atkinson() {
		vector <int> error(header.height * header.width);
		for (int i = 0; i < header.height; ++i){
			for (int j = 0; j < header.width; ++j){
				double buffer = (rev_gamma_correction(data[index(i, j)]) + error[index(i, j)]) / 255.0;
				buffer *= (value - 1);
				buffer = round(buffer);
				buffer *= 255.0 / (value - 1);
				int CurrentErrorValue = data[index(i, j)] + error[index(i, j)] - (int)buffer;
				data[index(i, j)] = (uchar)round(gamma_correction(buffer));
				for (int k = 0; k <= 2; ++k){
					for (int l = -2; l <= 2; ++l){
						if (i + k < header.height){
							if ((k == 0) && (l > 0)){
								if (j + l < header.width)
									error[index(i + k, j + l)] += int(CurrentErrorValue * Atkinson[k][2 + l] / 8.0);
							}
							else{
								if ((j + l < header.width) && (j + l > 0))
									error[index(i + k, j + l)] += int(CurrentErrorValue * Atkinson[k][2 + l] / 8.0);
							}
						}
					}
				}
			}
		}
	}

	void sierra() {
		vector <int> error(header.height * header.width);
		for (int i = 0; i < header.height; ++i)
		{
			for (int j = 0; j < header.width; ++j)
			{
				double buffer = (rev_gamma_correction(data[index(i, j)]) + error[index(i, j)]) / 255.0;
				buffer *= (value - 1);
				buffer = round(buffer);
				buffer *= 255.0 / (value - 1);
				int CurrentErrorValue = data[index(i, j)] + error[index(i, j)] - (int)buffer;
				data[index(i, j)] = (uchar)round(gamma_correction(buffer));
				for (int k = 0; k <= 2; ++k){
					for (int l = -2; l <= 2; ++l){
						if (i + k < header.height){
							if ((k == 0) && (l > 0)){
								if (j + l < header.width)
									error[index(i + k, j + l)] += int(CurrentErrorValue * Sierra3[k][2 + l] / 32.0);
							}
							else{
								if ((j + l < header.width) && (j + l > 0))
									error[index(i + k, j + l)] += int(CurrentErrorValue * Sierra3[k][2 + l] / 32.0);
							}
						}
					}
				}
			}
		}
	}
};

class PGM : public Picture_Dithering{
public:
	PGM() = default;
	~PGM() = default;

	void input(int argc, char *argv[]) override {
		if (argc != 7) {
			throw exception("Not enought or too much arguments");
		}
		info.input_file = string(argv[1]);
		info.output_file = string(argv[2]);
		info.is_gradient = stoi(argv[3]);
		info.dt = (dithering_type)stoi(argv[4]);
		info.bitrate = stoi(argv[5]);
		info.gamma = stod(argv[6]);
		info.check();

		ifstream fin(info.input_file, ios_base::binary);
		if (!fin.is_open())
			throw exception("File isn't open");
		fin >> header.tag;
		if (header.tag != "P5")
			throw exception("File's tag is damaged");
		fin >> header.width >> header.height >> header.colors;
		if (!header.width || !header.height || !header.colors)
			throw exception("File's header is damaged");
		
		fin.ignore();
		data.resize(header.height * header.width);
		fin.read((char*)&data[0], header.width * header.height);
		if (!fin) {
			cerr << fin.tellg();
			throw exception("File is damaged");
		}
		fin.close();
	}

	void save() override {
		ofstream fout(info.output_file, ios_base::binary);
		fout << header.tag << '\n' << header.width << ' ' << header.height;
		fout << '\n' << header.colors << '\n';
		fout.write((char*)&data[0], header.height * header.width);
		if (!fout)
			throw exception("Something wrong with writing");
		fout.close();
	}

	void make() override {
		if (info.is_gradient)
			make_grad();

		set_value();
		switch (info.dt)
		{
		case NONE:
			no_dither();
			break;
		case ORDERED:
			ordered();
			break;
		case ATKINSON:
			atkinson();
			break;
		case FLOYD_STEINBER:
			floyd_steinber();
			break;
		case JARVIS_JUDICE_NINKE:
			jarvis_judice_ninke();
			break;
		case RANDOM:
			random();
			break;
		case SIERRA:
			sierra();
			break;
		case HALFTONE:
			halftone();
			break;
		default:
			throw exception("wtf");
			break;
		}

	}

	void make_grad() {
		for (int i = 0; i < header.height; i++)
			for (int j = 0; j < header.width; j++)
				data[index(i, j)] = (uchar)gamma_correction((double)j / header.width * 255);
	}
};


int main(int argc, char *argv[]) {
	IPicture* a = new PGM;
	try {
		a->input(argc, argv);
		a->make();
		a->save();
	}
	catch(exception &ex){
		cerr << "Problem: " << ex.what();
		delete a;
		system("pause");
		return 1;
	}
	catch (...) {
		cerr << "It scares... nobody knows about it";
		delete a;
		return 1;
	}
	delete a;
	return 0;
}
