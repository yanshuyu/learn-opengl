#include"FileSystem.h"

namespace fs = std::filesystem;

FileSystem FileSystem::Default;

fs::path FileSystem::currentWorkingDirectory(bool canonical) {
	if (canonical)
		return fs::canonical(fs::current_path());
	
	return fs::current_path();
}

void FileSystem::setCurrentWorkingDirectory(const fs::path& path) {
	fs::current_path(path);
}


bool FileSystem::pathExist(const fs::path& path) {
	return fs::exists(path);
}


FileSystem::FileType FileSystem::fileType(const fs::path& path) {
	if (fs::is_directory(path))
		return FileType::Directory;

	if (fs::is_regular_file(path))
		return FileType::File;

	if (fs::is_symlink(path))
		return FileType::SymLink;

	return FileType::Other;
}
