/*
 * MBDA Systems
 * Cours OpenCL ESIEE
 *
 * Exercice 2 - Addition de vecteur
 *
 */

#include "CL/cl.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

// FIX ESIEE - a supprimer si sur PC personnel
#define FIX_ESIEE_NVIDIA

// Nombre d'éléments que l'on va traiter
const int N = 8;
// Numéro de la plateforme choisie
const int PLATFORM = 0;
// Type de périphérique choisie
const int DEVICE_T = CL_DEVICE_TYPE_GPU;

// Chemin vers le fichier qui contient les kernels
const std::string KERNEL_FILEPATH = "../../kernels/ex2.cl";

void check_error(int errorCode, char * name) {
	if (errorCode != CL_SUCCESS) {
		std::cout << "Error : " << name << '\n';
		std::cout << "Error code : " << errorCode << '\n';
		getchar();
		exit(EXIT_FAILURE);
	}
}

struct UniqueNumber {
	int _current;
	UniqueNumber() : _current(0) {}
	int operator()() { return ++_current; }
};

int main(void) {
	
	// Initialisation des données
	std::vector<float> data1(N);
	std::vector<float> data2(N);
	std::vector<float> dataOut(N);
	
	// 1, 2, 3, ...
	std::generate(std::begin(data1), std::end(data1), UniqueNumber());
	std::generate(std::begin(data2), std::end(data2), UniqueNumber());
	// 0, 0, 0, ...
	std::generate(std::begin(dataOut), std::end(dataOut), [&]() -> float { return 0.0f; });

	// Code d'erreur OpenCL
	cl_int err;

	// Initialisation de l'environnement OpenCL
	// Récupération des plateformes
	std::vector<cl::Platform> platforms;
	err = cl::Platform::get(&platforms);
	check_error(err, "Platform::get");

	std::cout << "Running on the following platform : " << platforms[PLATFORM].getInfo<CL_PLATFORM_NAME>() << '\n';
	
	// Create the context
	cl_context_properties props[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[PLATFORM](), 0};
	cl::Context context(DEVICE_T, props);

	// Get the devices
#ifdef FIX_ESIEE_NVIDIA
	auto devices = *(new std::vector<cl::Device>());
#else
	std::vector<cl::Device> devices;
#endif
	err = platforms[PLATFORM].getDevices(DEVICE_T, &devices);
	check_error(err, "Get devices");
	std::cout << "Device : " << devices[0].getInfo<CL_DEVICE_NAME>() << '\n';

	// Create a queue
	cl::CommandQueue queue(context, devices[0]);

	// Kernel loading
	std::ifstream file(KERNEL_FILEPATH);
	check_error(file.is_open() ? CL_SUCCESS : -1, "Loading kernel");

	std::string prog(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length()+1));
	cl::Program program(context, source);

	err = program.build(devices);
	check_error(err, "Building program");

	cl::Kernel addKernel(program, "add", &err);
	check_error(err, "addKernel");

	// Buffers allocation
	cl::Buffer data1Buffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(float)*N, NULL, &err);
	check_error(err, "Allocate data1Buffer");
	cl::Buffer dataOutBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float)*N, NULL, &err);
	check_error(err, "Allocate dataOutBuffer");

	// Copy to device
	queue.enqueueWriteBuffer(data1Buffer, CL_TRUE, 0, sizeof(float)*N, data1.data());
	queue.enqueueWriteBuffer(dataOutBuffer, CL_TRUE, 0, sizeof(float)*N, dataOut.data());
	queue.finish();


	// Set the arguments and run the program 1
	addKernel.setArg(0, data1Buffer);
	addKernel.setArg(1, dataOutBuffer);
	queue.enqueueNDRangeKernel(addKernel, cl::NullRange, cl::NDRange(N));
	queue.finish();

	// Read the matrix
	queue.enqueueReadBuffer(dataOutBuffer, CL_TRUE, 0, sizeof(float)*N, dataOut.data());
	queue.finish();

	// Affichage au maximum des 10 premières valeurs
	std::cout << "Resultat du calcul :\n";
	for (int i = 0; i < std::min(N, 10); ++i)
	{
		std::cout << dataOut[i] << "\t";
	}
	std::cout << '\n';

	std::cout << "Appuyer sur une touche pour continuer" << std::endl;
	getchar();

	return EXIT_SUCCESS;
}
