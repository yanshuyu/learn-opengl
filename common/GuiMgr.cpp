#include"GuiMgr.h"


bool GuiManager::initialize() {
	bool ok = true;
	for (auto& wnd : m_windows) {
		ok = wnd->initialize();
		if (!ok)
			return false;
	}

	return ok;
}

void GuiManager::render() {
	for (auto& wnd : m_windows) {
		wnd->render();
	}
}

GuiWindow* GuiManager::addWindow(GuiWindow* w) {
	m_windows.emplace_back(w);
	return w;
}

GuiWindow* GuiManager::findWindow(const std::string& title) const {
	for (auto& wnd : m_windows) {
		if (wnd->getTitle() == title)
			return wnd.get();
	}

	return nullptr;
}

std::unique_ptr<GuiWindow> GuiManager::removeWindow(const std::string& title) {
	for (auto pos = m_windows.begin(); pos != m_windows.end(); pos++) {
		if (pos->get()->getTitle() == title) {
			m_windows.erase(pos);
			return std::move(*pos);
		}
	}

	return nullptr;
}