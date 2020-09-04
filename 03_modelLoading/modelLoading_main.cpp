#include"ModelLoadingApp.h"
#include<memory>



int main() {
	auto app = std::unique_ptr<ModelLoadingApp>(new ModelLoadingApp("Model Loading"));

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

	return 0;
}