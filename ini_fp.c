#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <grp.h>
#include <pwd.h>

char dir[10000][1000];
int bnyk_dir = 1;

static const char *dirpath = "/home/ramrom";
static const char *mpath = "/home/ramrom/Music";

char cek[4] = ".mp3";


void list_dir(char* path){
	struct dirent *de;
    DIR *dr = opendir(path); 
  
    if (dr == NULL) 
    { 
        return; 
    } 
  

    while ((de = readdir(dr)) != NULL){
    	if(strcmp(de->d_name,".") == 0 || strcmp(de->d_name,"..") == 0 ){
    		continue;
		}
		sprintf(dir[bnyk_dir],"%s/%s",path,de->d_name);
    	++bnyk_dir;
	}

  
    closedir(dr);     
    return; 
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
  int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",mpath,path);
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=mpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",mpath,path);
	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		int j;
		char namafile[1000];
		strcpy(namafile,de->d_name);
		int pnj = strlen(namafile);		

		for(j=4;j>0;j--){
			if(namafile[pnj-j] != cek[4-j]){
				break;
			}
		}
		if(j==0){
			res = (filler(buf, de->d_name, &st, 0));
				if(res!=0) break;
		}
	}

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=mpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",mpath,path);
	int res = 0;
  int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

void kumpulDir(){
	strcpy(dir[0],dirpath);
	int i,j;
	
	for(i=0;i<bnyk_dir;i++){
		list_dir(dir[i]);
		int pnj = strlen(dir[i]);
		for(j=4;j>0;j--){
			if(dir[i][pnj-j] != cek[4-j]){
				break;
			}
		}
		if(j==0){
			pid_t child;
			child=fork();

			if(child == 0){
				char sis[1000];
				sprintf(sis,"%s/Music",dirpath);
				char* argv[] = {"mv", dir[i], sis, NULL};
				execv("/bin/mv",argv);
			}
		}
	}
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
};

int main(int argc, char *argv[])
{
	kumpulDir();
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
