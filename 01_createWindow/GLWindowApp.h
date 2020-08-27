#pragma once
#include<GLApplication.h>


class GLWindowApp : public GLApplication {
public:
	GLWindowApp(const std::string& name);
	
	bool initailize() override;
	void render() override;
};