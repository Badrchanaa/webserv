#ifndef __RESOURCE_HPP__
# define __RESOURCE_HPP__

#include <string>

# define ALL_PERMS (R_OK | W_OK | X_OK)
# define READ_PERM R_OK
# define WRITE_PERM W_OK
# define EXEC_PERM X_OK

class Resource
{
	public:
		typedef enum
		{
			FILE,
			DIR,
		}	resourceType;
	public:
		Resource(const char *path);
		Resource(const Resource &other);
		Resource& operator=(const Resource &other);
		Resource& operator=(const char *path);
		~Resource();

		std::string		getPath() const;

		bool			exists() const;
		bool			canRead() const;
		bool			canWrite();
		bool			canExecute() const;
		bool			hasPermission(int permissions) const;

		bool			isFile() const;
		bool			isDir() const;
		bool			validDirectory() const;

		bool			remove();

	private:
		void			_checkPermissions();
		std::string 	m_Path;
		int				m_Perm;
		bool			m_IsWriteChecked;
		resourceType	m_Type;
		Resource();
		
};

#endif