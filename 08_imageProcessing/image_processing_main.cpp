#include"ImageProcessingApp.h"


int main()
{
	auto app = std::unique_ptr<ImageProcessingApp>(new ImageProcessingApp("Image Processing", 1920, 1080, 4, 3));

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