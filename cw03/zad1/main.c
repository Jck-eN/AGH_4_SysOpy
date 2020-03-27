
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#define __USE_XOPEN_EXTENDED
#include <ftw.h>
#include <unistd.h>
#include <wait.h>

char* filename = NULL;


char* format_date(char* res, long t) {
    strftime(res, 36, "%Y-%m-%d %H:%M:%S", localtime(&t));
    return res;
}

char* get_type(struct stat* file_stat){
    if(S_ISREG(file_stat->st_mode)) return "file";
    else if(S_ISDIR(file_stat->st_mode)) return "dir";
    else if(S_ISCHR(file_stat->st_mode)) return "char dev";
    else if(S_ISBLK(file_stat->st_mode)) return "block dev";
    else if(S_ISFIFO(file_stat->st_mode)) return "fifo";
    else if(S_ISLNK(file_stat->st_mode)) return "slink";
    else if(S_ISSOCK(file_stat->st_mode)) return "sock";
    return "unknown";
}

int fn(char* file_path, struct stat* buf, int typeflag, struct FTW* ftwbuf){
    if(ftwbuf->level == 0) return 0;
    pid_t child_pid;
    int tmp;
    if(get_type(buf) == "dir"){
        child_pid = vfork();
        if(child_pid == 0){
            printf("%s\t ---- ",file_path);
            printf("PID: %d\n", (int)getpid());
            execlp("/bin/ls", "ls", "-l", file_path, NULL);
        }
        else{
            waitpid(child_pid, &tmp, NULL);
        }

    }
    return 0;
}



int main(int argc, char* argv[]) {

    if(argc < 2){
        printf("Usage: program <path> [OPTIONS]\n");
        printf("----\n");
        exit(0);
    }

    char abs_path[PATH_MAX];
    realpath(argv[1], abs_path);

    nftw(abs_path, fn, 10, FTW_PHYS);
    return 0;
}