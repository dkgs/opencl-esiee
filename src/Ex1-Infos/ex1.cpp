/*
 * MBDA Systems
 * Cours OpenCL ESIEE
 *
 * Exercice 1 - Informations sur OpenCL
 *
 */

#include "CL/cl.hpp"

#include <iostream>
#include <string>

// FIX ESIEE - a supprimer si sur PC personnel
#define FIX_ESIEE_NVIDIA

void check_error(int errorCode, std::string name) {
	if (errorCode != CL_SUCCESS) {
		std::cout << "Error : " << name << '\n';
		std::cout << "Error code : " << errorCode << '\n';
		getchar();
		exit(EXIT_FAILURE);
	}
}

int main(void) {

	// Variable d'erreur OpenCL
	// Permet de vérifier qu'il n'y a pas eu d'erreur durant une opération
	cl_int err;

	// Affichage de l'en-tête du programme
	std::cout << "------------------------------------------------\n";
	std::cout << "-              OPENCL INFORMATION              -\n";
	std::cout << "------------------------------------------------\n\n";

	// On récupère les différentes plateformes disponibles dans un vecteur
	std::vector<cl::Platform> platforms;
	err = cl::Platform::get(&platforms);
	check_error(err, "Platform::get");

	// On parcours le vecteurs, donc chaque plateforme
	for (auto itPlatform = std::begin(platforms); itPlatform != std::end(platforms); ++itPlatform) {

		// Pour la plateforme en cours, nous affichons quelques informations dont :
		// - le numéro de la plateforme
		// - la version
		// - le nom
		// - le vendeur
		// - les extensions disponibles (séparées par un espace)
		std::cout << "[PLATFORM : " << (itPlatform - std::begin(platforms)) << "]\n";
		std::cout << " Version    : " << itPlatform->getInfo<CL_PLATFORM_VERSION>() << '\n';
		std::cout << " Vendors    : " << itPlatform->getInfo<CL_PLATFORM_VENDOR>() << '\n';
		std::cout << " Extensions : " << itPlatform->getInfo<CL_PLATFORM_EXTENSIONS>() << '\n';

		// On créer le contexte en relation avec la plateforme
		cl_context_properties props[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(*itPlatform)(), 0};
		cl::Context context(CL_DEVICE_TYPE_ALL, props);

		// On récupère les devices rattachés a la plateforme
		// dans le cas, où il n'y a pas de device disponible, on affiche une erreur et on arrête le programme
#ifdef FIX_ESIEE_NVIDIA
	    auto devices = *(new std::vector<cl::Device>());
#else
	    std::vector<cl::Device> devices;
#endif
		err = itPlatform->getDevices(CL_DEVICE_TYPE_ALL, &devices);
		check_error(devices.size() != 0 ? CL_SUCCESS : -1, "Device not available");

		// On parcours tous les devices de la plateforme
		for (auto itDevice = std::begin(devices); itDevice != std::end(devices); ++itDevice) {

			// On affiche les informations du device en cours :
			// - son numéro
			// - si il est disponible
			// - son nom
			// - son vendeur
			// - sa version
			// - Son type (CPU || GPU)
			// - Le nombre maximal de comput units
			// - La dimension maximale pour les WorkItems
			// - La taille maximale du WorkItems selon sa dimension
			// - La taille maximale d'un WorkGroup
			// - Si le device supporte les images
			std::cout << "\n\t[DEVICE : " << (itDevice - std::begin(devices)) << "]\n";
			std::cout << "\t Available               : " << (itDevice->getInfo<CL_DEVICE_AVAILABLE>() ? "yes" : "no") << '\n';
			std::cout << "\t Vendor                  : " << itDevice->getInfo<CL_DEVICE_VENDOR>() << '\n';
			std::cout << "\t Version                 : " << itDevice->getInfo<CL_DEVICE_VERSION>() << '\n';
			std::cout << "\t Type                    : " << (itDevice->getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_CPU ? "CPU" : "GPU") << '\n';
			std::cout << "\t Max comput units        : " << itDevice->getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << '\n';
			std::cout << "\t Max WorkItems dimension : " << itDevice->getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>() << '\n';
			
			std::cout << "\t Max WorkItems Sizes     : (";
			std::cout << itDevice->getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0];
			for (unsigned int i_item = 1; i_item < itDevice->getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>(); i_item++) {
				std::cout << ", " << itDevice->getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[i_item];
			}
			std::cout << ")" << '\n';
			std::cout << "\t Max WorkGroup size      : " << itDevice->getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << '\n';
			std::cout << "\t Image Support           : " << (itDevice->getInfo<CL_DEVICE_IMAGE_SUPPORT>() ? "true" : "false") << '\n';
			std::cout << "\t FP config : DENORM      : " << ((itDevice->getInfo<CL_DEVICE_SINGLE_FP_CONFIG>() & CL_FP_DENORM) ? "true" : "false" ) << '\n';
			std::cout << "\t FP config : INF NAN     : " << ((itDevice->getInfo<CL_DEVICE_SINGLE_FP_CONFIG>() & CL_FP_INF_NAN) ? "true" : "false" ) << '\n';
			std::cout << "\t FP config : ROUND NEAR  : " << ((itDevice->getInfo<CL_DEVICE_SINGLE_FP_CONFIG>() & CL_FP_ROUND_TO_NEAREST) ? "true" : "false" ) << '\n';
			std::cout << "\t FP config : ROUND ZERO  : " << ((itDevice->getInfo<CL_DEVICE_SINGLE_FP_CONFIG>() & CL_FP_ROUND_TO_ZERO) ? "true" : "false" ) << '\n';
			std::cout << "\t FP config : ROUND INF   : " << ((itDevice->getInfo<CL_DEVICE_SINGLE_FP_CONFIG>() & CL_FP_ROUND_TO_INF) ? "true" : "false" ) << '\n';
			std::cout << "\t FP config : FMA         : " << ((itDevice->getInfo<CL_DEVICE_SINGLE_FP_CONFIG>() & CL_FP_FMA) ? "true" : "false" ) << '\n';
			std::cout << "\t FP config : SOFT FLOAT  : " << ((itDevice->getInfo<CL_DEVICE_SINGLE_FP_CONFIG>() & CL_FP_SOFT_FLOAT) ? "true" : "false" ) << '\n';

			std::cout << "\t Extensions : " << itDevice->getInfo<CL_DEVICE_EXTENSIONS>() << '\n';

		} // Fin du parcours des devices

		// On affiches des sauts de ligne pour faire jolie
		std::cout << "\n\n";

	} // Fin du parcours des plateformes

	std::cout << "Appuyer sur une touche pour continuer" << std::endl;
	getchar();

	return EXIT_SUCCESS;
}
