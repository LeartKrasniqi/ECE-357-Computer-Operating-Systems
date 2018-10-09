#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <langinfo.h>


int read_direc(char *direc);
int getInfo(struct dirent *entry, struct stat statbuf);
int firstRun = 0;
int main(int argc, char **argv)
{
	// when run it will look like: ./a.out [starting directory] so max argc is 2
	if (argc > 2)
	{
		printf("Error: Too many arguments.  Expected format is:\n./read_direct [starting_directory]\n");
		exit(EXIT_FAILURE);
	}

	char *direc;
	if (argc == 2)
		direc = argv[1];
	else 
		direc = "./";

	read_direc(direc);

	return 0;
}

int read_direc(char * direc)
{
	DIR *dir;
  	struct dirent *entry;
  	struct stat statbuf;
  	char newPath[2048] = {0};


  	if ((dir = opendir(direc)) == NULL)
  	{
    	fprintf(stderr, "Error opening directory %s: %s \n", direc, strerror(errno));
    	return -1;
  	}

  	while ((entry = readdir(dir)) != NULL)
  	{
  		
  		strcpy(newPath, direc);
  		strcat(newPath, "/");
  		strcat(newPath, entry->d_name);
  		if ((lstat(newPath, &statbuf)) < 0)
  		{
  			fprintf(stderr, "Error getting information about file %s: %s \n", newPath, strerror(errno));
  			return -1;
  		}

  		if (S_ISLNK(statbuf.st_mode))
  		{
  			char buf[1024];
  			ssize_t len;
  			getInfo(entry, statbuf);
  			if ((len = readlink(newPath, buf, sizeof(buf)-1)) == -1)
    		{
    			fprintf(stderr, "Error reading contents of %s: %s \n", newPath, strerror(errno));
    			return -1;
    		}
    		else	
    			buf[len] = '\0';

    		printf("%s \t", newPath);
    		printf("-> %s \n", buf);

  		}
  		else if (S_ISREG(statbuf.st_mode))
  		{
  			getInfo(entry, statbuf);
    		printf("%s \n", newPath);
  		}
  		else if (S_ISDIR(statbuf.st_mode))
  		{
        if ((strcmp(entry->d_name, ".") == 0) && (!firstRun))
        {
          getInfo(entry, statbuf);
          firstRun++;
          printf("%s \n", direc);
          continue;
        }
        else if ((strcmp(entry->d_name, ".") == 0) || ((strcmp(entry->d_name, "..")) == 0))
          continue;
  			else
  			{
  				getInfo(entry, statbuf);
  				printf("%s \n", newPath);
  				read_direc(newPath);
  			}
  		}
  	}

  	return 0;
}

int getInfo(struct dirent *entry, struct stat statbuf)
{
	struct passwd *pwd;
	struct group *group;
	struct tm *time;
	char datestring[256];
	errno = 0;		// Preclear 

	printf("%10llu \t", statbuf.st_ino);				// Print inode #
	printf("%8llu \t", (statbuf.st_blocks));		// Print disk usage 
	// Print permissions
	printf( (S_ISDIR(statbuf.st_mode)) ? "d" : "-");	
    printf( (statbuf.st_mode & S_IRUSR) ? "r" : "-");
    printf( (statbuf.st_mode & S_IWUSR) ? "w" : "-");
    printf( (statbuf.st_mode & S_IXUSR) ? "x" : "-");
    printf( (statbuf.st_mode & S_IRGRP) ? "r" : "-");
    printf( (statbuf.st_mode & S_IWGRP) ? "w" : "-");
    printf( (statbuf.st_mode & S_IXGRP) ? "x" : "-");
    printf( (statbuf.st_mode & S_IROTH) ? "r" : "-");
    printf( (statbuf.st_mode & S_IWOTH) ? "w" : "-");
    printf( (statbuf.st_mode & S_IXOTH) ? "x" : "-");
	printf("%4d \t", statbuf.st_nlink);				// Print number of links
	if ((pwd = getpwuid(statbuf.st_uid)) == NULL)	// Find user id
    {
    	if(errno)
    	{
    		fprintf(stderr, "Error getting user id for %s: %s\n", entry->d_name, strerror(errno));
    		return -1;
    	} 
    	else
    		printf("%8d \t", statbuf.st_uid);	
    }      
    else
        printf("%8s \t", pwd->pw_name);


    if ((group = getgrgid(statbuf.st_gid)) == NULL)	// Find group id
    {
        if(errno)
        {
        	fprintf(stderr, "Error getting group id for %s: %s\n", entry->d_name, strerror(errno));
        	return -1;
        }
        else
        	printf("%8d \t", statbuf.st_gid);
    }
    else       
    	printf("%8s \t", group->gr_name);


    printf("%9ld \t", (long int)statbuf.st_size);	// Print file size in bytes

    // Prints mtime in human-readable format
    time = localtime(&statbuf.st_mtime);
    strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), time);
    printf("%s \t", datestring);

    return 0;
}
