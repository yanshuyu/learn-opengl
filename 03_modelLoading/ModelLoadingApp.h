#pragma once
#include<common/GLApplication.h>


class ModelLoadingApp : public GLApplication {
public:
	ModelLoadingApp(const std::string& wndTitle, int wndWidth = 1920, int wndHeight = 1080);

private:
	bool initailize() override;
	void render() override;

};