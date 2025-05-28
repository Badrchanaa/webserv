#include "Resource.hpp"
#include <iostream>
#include <string>
#include <cstdio>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

Resource::Resource(void) {}

Resource::Resource(const char *path, int perms): m_Path(path), m_Exist(false), m_Perm(0), m_CheckedPerms(0), m_Type(Resource::UNKNOWN)
{
	std::cout << "NEW RESOURCE: " << m_Path << std::endl;
	_checkPermissions(perms);
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
	return m_Exist;
}

bool			Resource::canRead()
{
	if (!exists())
		return false;
	if (m_CheckedPerms & READ_PERM)
		return m_Perm & READ_PERM;
	m_CheckedPerms |= READ_PERM;
	if (access(m_Path.c_str(), R_OK) == 0)
		m_Perm |= READ_PERM;
	return m_Perm & READ_PERM;
}

bool			Resource::canWrite()
{
	if (!exists())
		return false;
	if (m_CheckedPerms & WRITE_PERM)
		return m_Perm & WRITE_PERM;
	m_CheckedPerms |= WRITE_PERM;
	if (access(m_Path.c_str(), W_OK) == 0)
		m_Perm |= WRITE_PERM;
	return m_Perm & WRITE_PERM;
}

bool			Resource::canExecute()
{
	if (!exists())
		return false;
	if (m_CheckedPerms & EXEC_PERM)
		return m_Perm & EXEC_PERM;
	m_CheckedPerms |= EXEC_PERM;
	if (access(m_Path.c_str(), X_OK) == 0)
		m_Perm |= EXEC_PERM;
	return m_Perm & EXEC_PERM;
}

void		Resource::_checkPermissions(int perms)
{
	std::cout << "ENTER CHECK PERMISSIONS" << std::endl;
	struct stat fileStat;
	const char	*path = m_Path.c_str();

	// int deniedPerms = access(path, F_OK);
	if (access(path, F_OK) != 0)
	{
		m_CheckedPerms = ALL_PERMS;
		return;
	}
	m_Exist = true;
	if (perms & WRITE_PERM && access(path, WRITE_PERM) == 0)
		m_Perm |= WRITE_PERM;
	if (perms & READ_PERM && access(path, READ_PERM) == 0)
		m_Perm |= READ_PERM;
	if (perms & EXEC_PERM && access(path, EXEC_PERM) == 0)
		m_Perm |= EXEC_PERM;
	m_CheckedPerms |= perms;
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