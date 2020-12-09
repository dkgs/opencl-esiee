/*
 * MBDA Systems
 * Cours OpenCL ESIEE
 *
 * Exercice 3 - Traitement simple sur image
 *
 */

#include <fstream>
#include <iostream>
#include <string>

#include "CL/cl.hpp"
#include "FreeImage.h"

// FIX ESIEE - a supprimer si sur PC personnel
#define FIX_ESIEE_NVIDIA

// Nombre d'éléments que l'on va traiter
const int N = 8;
// Numéro de la plateforme choisie
const int PLATFORM = 1;
// Type de périphérique choisie
const cl_device_type DEVICE_T = CL_DEVICE_TYPE_ALL;
const int DEVICE = 0;

// Chemin vers le fichier qui contient les kernels
const std::string KERNEL_FILEPATH = "../../kernels/ex3.cl";

// Chemin vers le fichier image à charger
const std::string IMAGE_INPUT_FILEPATH = "../../resources/ex3.jpg";
const std::string IMAGE_OUTPUT_FILEPATH = "../../resources/ex3_out.jpg";

// Angle de rotation de l'image (en radian)
const float ANGLE = 45.0f * 3.1415f / 180.0f;

void check_error(int errorCode, char * name) {
	if (errorCode != CL_SUCCESS) {
		std::cout << "Error : " << name << '\n';
		std::cout << "Error code : " << errorCode << '\n';
		getchar();
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char * argv[]) {

	//// Gestion de l'image
	// Chargement de l'image
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(IMAGE_INPUT_FILEPATH.c_str(), 0);
	std::unique_ptr<FIBITMAP,  void(__stdcall *)(FIBITMAP*)> image(FreeImage_Load(format, IMAGE_INPUT_FILEPATH.c_str()), FreeImage_Unload);

	// Conversion en 32bits
	image.reset(FreeImage_ConvertTo32Bits(image.get()));

	// Récupération de la taille
	const unsigned int width = FreeImage_GetWidth(image.get());
	const unsigned int height = FreeImage_GetHeight(image.get());

	std::cout << "Image (" << width << ", " << height << ")\n";
	std::cout << "Pitch : " << FreeImage_GetPitch(image.get()) << '\n';
	std::cout << "BPP : " << FreeImage_GetBPP(image.get()) << '\n';
	std::cout << "ImageType : " << FreeImage_GetImageType(image.get()) << '\n';
	
	//// Partie OpenCL
	// Error managment
	cl_int err;

	// First we are going to set up the environment
	// Get the platform
	std::vector<cl::Platform> platforms;
	err = cl::Platform::get(&platforms);
	check_error(err, "Platform::get");

	std::cout << "Running on the following platform : " << platforms[PLATFORM].getInfo<CL_PLATFORM_NAME>() << '\n';
	
	// Get the devices
#ifdef FIX_ESIEE_NVIDIA
	auto devices = *(new std::vector<cl::Device>());
#else
	std::vector<cl::Device> devices;
#endif
	err = platforms[PLATFORM].getDevices(DEVICE_T, &devices);
	check_error(err, "Get devices");
	check_error(devices.size() != 0 ? CL_SUCCESS : -1, "No device available");
	check_error(devices.size() > DEVICE ? CL_SUCCESS : -1, "Specified device not available");
	std::cout << "Device : " << devices[DEVICE].getInfo<CL_DEVICE_NAME>() << '\n';

	// Create the context
	cl_context_properties props[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[PLATFORM](), 0};
	cl::Context context(devices, props);

	// Create a queue
	cl::CommandQueue queue(context, devices[0], 0);
	
	// Kernel loading
	std::ifstream file(KERNEL_FILEPATH);
	check_error(file.is_open() ? CL_SUCCESS : -1, "Loading kernel");

	std::string prog(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length()+1));
	cl::Program program(context, source);

	err = program.build(devices);
	check_error(err, "Building program");

	cl::Kernel matrixKernel(program, "modify", &err);
	check_error(err, "Kernel");

	// Create buffers
	cl::ImageFormat clImageFormat(CL_RGBA, CL_UNORM_INT8);
	cl::Image2D imageInBuffer(context, CL_MEM_READ_ONLY, clImageFormat, width, height, 0, nullptr, &err);
	check_error(err, "Image Buffer Creation");
	cl::Image2D imageOutBuffer(context, CL_MEM_WRITE_ONLY, clImageFormat, width, height, 0, nullptr, &err);
	check_error(err, "Image out Buffer Creation");

	// Image size
	cl::size_t<3> origin;
	origin.push_back(0);
	origin.push_back(0);
	origin.push_back(0);
	cl::size_t<3> region;
	region.push_back(width);
	region.push_back(height);
	region.push_back(1);

	// Copy data to buffer
	queue.enqueueWriteImage(imageInBuffer, CL_TRUE, origin, region, 0, 0, FreeImage_GetBits(image.get()));

	// Set the arguments and run the program 2
	matrixKernel.setArg(0, imageInBuffer);
	matrixKernel.setArg(1, imageOutBuffer);
	matrixKernel.setArg(2, width);
	matrixKernel.setArg(3, height);
	err = queue.enqueueNDRangeKernel(matrixKernel, cl::NullRange, cl::NDRange(width, height));
	queue.finish();
	check_error(err, "Execution");
	
	// Read the image back
	std::vector<char> bufferOut(width*height*4);

	err = queue.enqueueReadImage(imageOutBuffer, CL_TRUE, origin, region, 0, 0, bufferOut.data());
	queue.finish();
	check_error(err, "Read Image from device");

	// Write the image
	auto formatOut = FreeImage_GetFIFFromFilename(IMAGE_OUTPUT_FILEPATH.c_str());
	
	std::unique_ptr<FIBITMAP,  void(__stdcall *)(FIBITMAP*)> imageOut(FreeImage_ConvertFromRawBits((BYTE*) bufferOut.data(), width, height, width*4, 32, 0xFF000000, 0x00FF0000, 0x0000FF00), FreeImage_Unload);
	imageOut.reset(FreeImage_ConvertTo24Bits(imageOut.get()));
	bool success = FreeImage_Save(formatOut, imageOut.get(), IMAGE_OUTPUT_FILEPATH.c_str());

	std::cout << (success ? "success" : "fail") << '\n';
	
	std::cout << "Appuyer sur une touche pour continuer" << std::endl;
	getchar();

	return EXIT_SUCCESS;
}
