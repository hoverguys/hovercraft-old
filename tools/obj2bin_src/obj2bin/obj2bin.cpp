// obj2bin.cpp : Defines the entry point for the console application.
//

#include "obj2bin.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <iostream>
#include <cstdio>
#include <iterator>
using namespace std;

Assimp::Importer importer;
aiScene* scene = NULL;

// Prototyping
bool loadObjmodel(string file);
bool saveBinfile(string file);

int main(int argc, char* argv[]) {
	string inFilePath, outFilePath;
	try {
		po::options_description desc("Valid arguments");
		desc.add_options()
			("help", "produce help message")
			("input,i", po::value<string>(), "input obj file")
			("output,o", po::value<string>(), "output bin file")
			;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Help
		if (vm.count("help")) {
			cout << desc << "\n";
			return 0;
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

	if (!loadObjmodel(inFilePath))
		return 1;
	if (!saveBinfile(outFilePath))
		return 1;


	return 0;
}

bool loadObjmodel(string file) {
	scene = (aiScene*)importer.ReadFile(file,
										aiProcess_Triangulate |
										aiProcess_JoinIdenticalVertices
										);

	if (scene == NULL || scene->mNumMeshes != 1) {
		cerr << "Error reading file, might have more then 1 mesh?" << std::endl;
		return false;
	}

	cout << "loaded " << file << "\n";
	cout << "meshes: " << scene->mNumMeshes << "\n";

	return true;
}

bool saveBinfile(string file) {
	// Make object to save
	binheader_t		binHeader;
	bindata_t		bindData;

	aiMesh* mesh = scene->mMeshes[0];

	// Counting time
	memset(&binHeader, 0, sizeof(binheader_t));
	binHeader.vcount = mesh->mNumVertices;
	binHeader.ncount = mesh->mNumVertices;
	binHeader.vtcount = mesh->mNumVertices;
	binHeader.fcount = mesh->mNumFaces;

	// Allocate memory
	bindData.positions = new float[binHeader.vcount * 3];
	bindData.normals = new float[binHeader.ncount * 3];
	bindData.texcoords = new float[binHeader.vtcount * 2];
	bindData.indices = new unsigned short[binHeader.fcount * 3];

	cout << "positions: " << binHeader.vcount << "\n";
	cout << "normals: " << binHeader.ncount << "\n";
	cout << "texcoords: " << binHeader.vtcount << "\n";
	cout << "faces: " << binHeader.fcount << "\n";


		// Fix endian
	binheader_t binHeaderEndian;
	binHeaderEndian.vcount = EndianFixInt(binHeader.vcount);
	binHeaderEndian.ncount = EndianFixInt(binHeader.ncount);
	binHeaderEndian.vtcount = EndianFixInt(binHeader.vtcount);
	binHeaderEndian.fcount = EndianFixInt(binHeader.fcount);

	// Dump file
	FILE *outFile = fopen(file.c_str(), "wb");

	if (outFile == NULL) {
		cout << "Error, unable to open output file\n";
		return 1;
	}

	fwrite(&binHeaderEndian, sizeof(binheader_t), 1, outFile);

	// Vertices
	for (unsigned int vi = 0; vi < binHeader.vcount; vi++) {
		aiVector3D& v = mesh->mVertices[vi];
		const unsigned int fi = vi * 3;
		bindData.positions[fi + 0] = EndianFixFloat(v.x);
		bindData.positions[fi + 1] = EndianFixFloat(v.y);
		bindData.positions[fi + 2] = EndianFixFloat(v.z);
	}
	// Normals
	for (unsigned int ni = 0; ni < binHeader.ncount; ni++) {
		aiVector3D& v = mesh->mNormals[ni];
		const unsigned int fi = ni * 3;
		bindData.normals[fi + 0] = EndianFixFloat(v.x);
		bindData.normals[fi + 1] = EndianFixFloat(v.y);
		bindData.normals[fi + 2] = EndianFixFloat(v.z);
	}
	// Texture Coordinates
	for (unsigned int ti = 0; ti < binHeader.vtcount; ti++) {
		aiVector3D& v = mesh->mTextureCoords[0][ti];
		const unsigned int fi = ti * 2;
		bindData.texcoords[fi + 0] = EndianFixFloat(v.x);
		bindData.texcoords[fi + 1] = EndianFixFloat((1.0f - v.y));
	}
	// Indices
	for (unsigned int ii = 0; ii < mesh->mNumFaces; ii++) {
		aiFace& f = mesh->mFaces[ii];
		const unsigned int si = ii * 3;
		bindData.indices[si + 0] = EndianFixShort((unsigned short)f.mIndices[0]);
		bindData.indices[si + 1] = EndianFixShort((unsigned short)f.mIndices[1]);
		bindData.indices[si + 2] = EndianFixShort((unsigned short)f.mIndices[2]);
	}

	fwrite(bindData.positions, sizeof(float), binHeader.vcount * 3, outFile);
	fwrite(bindData.normals, sizeof(float), binHeader.ncount * 3, outFile);
	fwrite(bindData.texcoords, sizeof(float), binHeader.vtcount * 2, outFile);
	fwrite(bindData.indices, sizeof(unsigned short), binHeader.fcount * 3, outFile);

	// Close file
	fclose(outFile);

	// Destroy memory
	delete[] bindData.positions;
	delete[] bindData.normals;
	delete[] bindData.texcoords;
	delete[] bindData.indices;

	return true;
}
