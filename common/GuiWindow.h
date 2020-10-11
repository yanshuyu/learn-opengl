#pragma once
#include<string>

class GuiWindow {
public:
	GuiWindow(const std::string& title = "");
	virtual ~GuiWindow() {}

	virtual bool initialize() {
		return true;
	}

	virtual void render() = 0;

	inline void setTitle(const std::string& title) {
		m_title = title;
	}

	inline std::string getTitle() const {
		return m_title;
	}

protected:
	std::string m_title;
};
