#ifndef __RESOURCE_HPP__
# define __RESOURCE_HPP__

#include <string>
#include <unistd.h>

# define ALL_PERMS (F_OK | R_OK | W_OK | X_OK)
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
			UNKNOWN,
		}	resourceType;
	public:
		Resource(const char *path, int perms=(READ_PERM | EXEC_PERM));
		Resource(const Resource &other);
		Resource& operator=(const Resource &other);
		Resource& operator=(const char *path);
		~Resource();

		std::string		getPath() const;

		bool			exists() const;
		bool			canRead();
		bool			canWrite();
		bool			canExecute();
		bool			hasPermission(int permissions) const;

		bool			isFile() const;
		bool			isDir() const;
		bool			validDirectory() const;

		bool			remove();

	private:
		void			_checkPermissions(int perms);
		std::string 	m_Path;
		bool			m_Exist;
		int				m_Perm;
		int				m_CheckedPerms;			
		resourceType	m_Type;
		Resource();
		
};

#endif