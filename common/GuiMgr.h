#pragma once
#include"GuiWindow.h"
#include"Singleton.h"
#include<memory>
#include<vector>


class GuiManager : public Singleton<GuiManager> {
public:
	bool initialize();
	void render();

	GuiWindow* addWindow(GuiWindow* w);
	GuiWindow* findWindow(const std::string& title) const;
	std::unique_ptr<GuiWindow> removeWindow(const std::string& title);

protected:
	std::vector<std::unique_ptr<GuiWindow>> m_windows;
};