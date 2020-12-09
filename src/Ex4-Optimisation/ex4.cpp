/*
 * MBDA Systems
 * Cours OpenCL ESIEE
 *
 * Exercice - Optimisation progressive
 *
 */

#include "CL/cl.hpp"

#include "Timer.h"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cassert>
#include <array>

// FIX ESIEE - a supprimer si sur PC personnel
#define FIX_ESIEE_NVIDIA

// Nombre d'éléments que l'on va traiter
static const unsigned int N = 1024;
// Numéro de la plateforme choisie
static const int PLATFORM = 0;
// Type de périphérique choisie
static const int DEVICE_T = CL_DEVICE_TYPE_GPU;
// Numéro du device choisi
static const int DEVICE = 0;

// Chemin vers le fichier qui contient les kernels
const std::string KERNEL_FILEPATH = "../../kernels/ex4.cl";


void check_error(int errorCode, char * name) {
	if (errorCode != CL_SUCCESS) {
		std::cout << "Error : " << name << '\n';
		std::cout << "Error code : " << errorCode << '\n';
		getchar();
		exit(EXIT_FAILURE);
	}
}

// Fonction de référence à implémenter
std::vector<float> variance(const std::vector<float> & data) {
    // TODO
    return data;
}

int main(void) {
	
	// Compteur de temps
	CPerfCounter counter;

	// Initialisation des données
	std::vector<float> dataIn(N);
    std::vector<float> dataOut(N);

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
	err = platforms.at(PLATFORM).getDevices(DEVICE_T, &devices);
	check_error(err, "Get devices");
	for (auto itDevice = devices.begin(); itDevice != devices.end(); ++itDevice)
	{
		std::cout << "Device [" << std::distance(devices.begin(), itDevice) << "] : " << itDevice->getInfo<CL_DEVICE_NAME>() << '\n';
	}
	assert(DEVICE < devices.size());

	// Create a queue
	cl::CommandQueue queue(context, devices.at(DEVICE));

	// Kernel loading
	std::ifstream file(KERNEL_FILEPATH);
	check_error(file.is_open() ? CL_SUCCESS : -1, "Loading kernel");

	std::string prog(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length()+1));
	cl::Program program(context, source);

	err = program.build(devices);
	check_error(err, "Building program");

	cl::Kernel kernel(program, "variance", &err);
	check_error(err, "kernel");

	// Buffers allocation
    cl::Buffer dataInBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(cl_float)*N, NULL, &err);
	check_error(err, "Allocate dataInBuffer");
	cl::Buffer dataOutBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_float)*N, NULL, &err);
	check_error(err, "Allocate dataOutBuffer");

	// Copy to device
	counter.Reset();
	counter.Start();
	queue.enqueueWriteBuffer(dataInBuffer, CL_TRUE, 0, sizeof(cl_float)*N, dataIn.data());
	queue.enqueueWriteBuffer(dataOutBuffer, CL_TRUE, 0, sizeof(cl_float)*N, dataOut.data());
	queue.finish();
	counter.Stop();
	std::cout << "Transfer to device time : "  << counter.GetTotalTime() << '\n';

	// Set the arguments and run the program
	kernel.setArg(0, dataInBuffer);
	kernel.setArg(1, N);
	kernel.setArg(2, dataOutBuffer);
	counter.Reset();
	counter.Start();
	queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(N));
	queue.finish();
	counter.Stop();
	std::cout << "Execution time : "  << counter.GetTotalTime() << '\n';

	// Read the data
	counter.Reset();
	counter.Start();
	queue.enqueueReadBuffer(dataOutBuffer, CL_TRUE, 0, sizeof(cl_float)*N, dataOut.data());
	queue.finish();
	counter.Stop();
	std::cout << "Transfer from device time : "  << counter.GetTotalTime() << '\n';

	std::cout << "Appuyer sur une touche pour continuer" << std::endl;
	getchar();

	return EXIT_SUCCESS;
}
