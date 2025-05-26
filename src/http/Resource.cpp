#include "Resource.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstdio>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

Resource::Resource(void) {}

Resource::Resource(const char *path): m_Path(path)
{
	std::cout << "NEW RESOURCE: " << m_Path << std::endl;
	_checkPermissions();
}

std::string	Resource::getPath() const
{
	return m_Path;
}

bool			Resource::hasPermission(int permissions) const
{
	return (m_Perm & permissions) == permissions;
}


bool			Resource::isFile() const
{
	return m_Type == Resource::FILE;
}

bool			Resource::isDir() const
{
	return m_Type == Resource::DIR;
}

bool			Resource::validDirectory() const
{
	return isDir() && hasPermission(READ_PERM | EXEC_PERM);
}

bool			Resource::exists() const
{
	return m_Perm & F_OK;
}

bool			Resource::canRead() const
{
	return exists() && m_Perm & R_OK;
}

bool			Resource::canWrite()
{
	if (!exists())
		return false;
	if (m_IsWriteChecked)
		return m_Perm & WRITE_PERM;
	m_IsWriteChecked = true;
	if (access(m_Path.c_str(), W_OK) == 0)
		m_Perm |= WRITE_PERM;
	return m_Perm & WRITE_PERM;
}

bool			Resource::canExecute() const
{
	return exists() && m_Perm & X_OK;
}

void		Resource::_checkPermissions()
{
	std::cout << "ENTER CHECK PERMISSIONS" << std::endl;
	struct stat fileStat;
	const char	*path = m_Path.c_str();

	// int deniedPerms = access(path, F_OK);
	if (access(path, F_OK) == 0)
		m_Perm |= F_OK;
	if (access(path, R_OK) == 0)
		m_Perm |= R_OK;
	if (access(path, X_OK) == 0)
		m_Perm |= X_OK;
	if (stat(path, &fileStat) == -1)
	{
		std::cout << "ERRNO: " << strerror(errno) << std::endl;
		std::cout << "PATH: " << path << std::endl;
		m_Perm = 0;
		return ;
	}
  	if (S_ISREG(fileStat.st_mode))
		m_Type = Resource::FILE;
	else if (S_ISDIR(fileStat.st_mode))
		m_Type = Resource::DIR;
	std::cout<< "CHECKING PERMISSION FOR: " << m_Path << std::endl;
	std::cout << "\tExists: ";
	if (exists())
		std::cout << "true" << std::endl;
	else
		std::cout << "false" << std::endl;
	if (isFile())
		std::cout << "\tType: File" << std::endl;
	else if (isDir())
		std::cout << "\tType: Directory" << std::endl;
	std::cout << "\tPermissions: ";
	if (canRead())
		std::cout << "READ ";
	if (canWrite())
		std::cout << "WRITE ";
	if (canExecute())
		std::cout << "EXECUTE ";
	std::cout << std::endl;
}

bool	Resource::remove()
{
	if (!exists())
		return false;
	return std::remove(m_Path.c_str()) == 0;
}

Resource::Resource(const Resource &other)
{
	*this = other;	
}

Resource& Resource::operator=(const Resource &other)
{	
	if (this == &other)
		return *this;
	m_Path = other.m_Path;
	m_Type = other.m_Type;
	m_Perm = other.m_Perm;
	return *this;
}

Resource& Resource::operator=(const char *path)
{	
	*this = Resource(path);
	return *this;
}

Resource::~Resource(void)
{	
}