#include <iostream>
#include<memory>
#include"TextureCubeApp.h"


int main()
{
	auto app = std::make_unique<TextureCubeApp>("TextureCube");
	try {
		app->run();
	}
	catch (const AppException& ae) {
		std::cout << "Got App Exception: " << ae.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	catch (const std::exception& e) {
		std::cout << "Got Unkonw Exception: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

