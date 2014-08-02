// obj2bin.cpp : Defines the entry point for the console application.
//

#include "raw2bin.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <iostream>
#include <iterator>
using namespace std;

// Prototyping
template<typename T>
bool convertRaw(string inFile, string outFile);

int main(int argc, char* argv[]) {
	string inFilePath, outFilePath;
	unsigned char bpp = 16;

	try {
		po::options_description desc("Valid arguments");
		desc.add_options()
			("help", "produce help message")
			("input,i", po::value<string>(), "input raw file")
			("output,o", po::value<string>(), "output raw file")
			("bits,b", po::value<unsigned char>(), "bits per pixel")
			;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Help
		if (vm.count("help")) {
			cout << desc << "\n";
			return 0;
		}

		// BBP
		if (vm.count("bits")) {
			bpp = vm["bits"].as<unsigned char>();
			if (bpp != 8 && bpp != 16 && bpp != 32) {
				cout << "ERROR:\n  Wrong number used for bits.\nPlease use either 8, 16 or 32.\n";
				return 1;
			}
		}

		// Arguments
		if (vm.count("input")) {
			inFilePath = vm["input"].as<string>();
		} else {
			cout << "ERROR:\n  Missing input argument.\n";
			cout << desc << "\n";
			return 1;
		}

		if (vm.count("output")) {
			outFilePath = vm["output"].as<string>();
		} else {
			cout << "ERROR:\n  Missing output argument.\n";
			cout << desc << "\n";
			return 1;
		}
	}
	catch (exception& e) {
		cerr << "error: " << e.what() << "\n";
		return 1;
	}
	catch (...) {
		cerr << "Exception of unknown type!\n";
	}

	bool result = false;
	switch (bpp) {
	case 8:
		result = convertRaw<unsigned char>(inFilePath, outFilePath);
		break;
	case 16:
		result = convertRaw<unsigned short>(inFilePath, outFilePath);
		break;
	case 32:
		result = convertRaw<unsigned int>(inFilePath, outFilePath);
		break;

	}

	if (!result)
		return 1;

	return 0;
}

template<typename T>
bool convertRaw(string inFile, string outFile) {
	FILE *outFileHandle = NULL, *inFileHandle = NULL;

	// Open and check
	fopen_s(&inFileHandle, inFile.c_str(), "rb");
	if (inFileHandle == NULL) {
		cout << "Error input file [" << inFile << "] could not be opened\n";
		return false;
	}
	fopen_s(&outFileHandle, outFile.c_str(), "wb");
	if (outFileHandle == NULL) {
		cout << "Error output file [" << outFile << "] could not be opened\n";
		fclose(inFileHandle);
		return false;
	}

	T readBuffer, writeBuffer;
	while (!feof(inFileHandle)) {
		if (fread(&readBuffer, sizeof(T), 1, inFileHandle) == 1) {
			writeBuffer = swapEndian<T>(readBuffer);

			fwrite(&writeBuffer, sizeof(T), 1, outFileHandle);
		} else {
			break;
		}
	}

	// Close file
	fclose(inFileHandle);
	fclose(outFileHandle);
	return true;
}

template<class T>
inline T swapEndian(T input);

// BPP 8
template<>
inline unsigned char swapEndian(unsigned char input) {
	return input;
}

// BPP 16
template<>
inline unsigned short swapEndian(unsigned short input) {
	return SwapShort(input);
}

// BPP 32
template<>
inline unsigned int swapEndian(unsigned int input) {
	return SwapInt(input);
}