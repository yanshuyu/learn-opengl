#pragma once
#include<filesystem>

namespace fs = std::filesystem;

class FileSystem {
public:
	enum class FileType {
		Directory,
		File,
		SymLink,
		Other,
	};

	static FileSystem Default;

public:
	fs::path currentWorkingDirectory(bool canonical = true);
	void setCurrentWorkingDirectory(const fs::path& path);

	bool pathExist(const fs::path& path);
	FileType fileType(const fs::path& path);

	inline void setHomeDirectory(const fs::path& home) {
		m_home = home;
	}

	inline fs::path getHomeDirectory() const {
		return m_home;
	}

protected:
	fs::path m_home;
};