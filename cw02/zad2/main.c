
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#define __USE_XOPEN_EXTENDED
#include <ftw.h>

int mtime_min = -1;
int mtime_max = -1;
int atime_min = -1;
int atime_max = -1;
int max_depth = -1;
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

int is_time_between(int time_min_days, int time_max_days, long file_time){
    long diff = time(NULL)-file_time;
    diff/=(24*3600);

    if(time_max_days!=-1 && diff > time_max_days){
        return 0;
    }
    if(time_min_days!=-1 && diff < time_min_days){
        return 0;
    }
    return 1;
}

void search_dir(char* dir_path, char* filename, int mtime_min, int mtime_max, int atime_min, int atime_max,
                int max_depth, int depth){
    if(max_depth != -1 && depth > max_depth) return;
    DIR* dir = opendir(dir_path);
    if(!dir){
        printf("Error opening directory!");
        exit(1);
    }
    char tmp_str1[36];
    char tmp_str2[36];


    char new_file_path[PATH_MAX];
    struct dirent *dir_ent;
    struct stat* buf = calloc(1, sizeof(struct stat));
    while(dir_ent = readdir(dir)){
        if(strcmp(dir_ent->d_name,".")==0 || strcmp(dir_ent->d_name,"..")==0) continue;
        strcpy(new_file_path, dir_path);
        strcat(new_file_path, "/");
        strcat(new_file_path, dir_ent->d_name);
        lstat(new_file_path, buf);

        if(
                is_time_between(mtime_min, mtime_max, buf->st_mtim.tv_sec) &&
                is_time_between(atime_min, atime_max, buf->st_atim.tv_sec) &&
                (filename == NULL || strcmp(filename, dir_ent->d_name)==0)
            ){
            printf("%s\t%d\t%s\t%d\t%s\t%s\n", new_file_path, buf->st_nlink, get_type(buf), buf->st_size,
                    format_date(tmp_str1, buf->st_atim.tv_sec), format_date(tmp_str2, buf->st_mtim.tv_sec));
        }


        if(strcmp(get_type(buf), "dir")==0){
            search_dir(new_file_path, filename, mtime_min, mtime_max, atime_min, atime_max, max_depth, depth+1);
        }

    }

    if(closedir(dir) == -1){
        printf("Error closing directory!");
        exit(1);
    }
    free(buf);



}

int fn(char* file_path, struct stat* buf, int typeflag, struct FTW* ftwbuf){
    char tmp_str1[36];
    char tmp_str2[36];
    if(     ftwbuf->level<= max_depth &&
            is_time_between(mtime_min, mtime_max, buf->st_mtim.tv_sec) &&
            is_time_between(atime_min, atime_max, buf->st_atim.tv_sec))
            {
        printf("%s\t%d\t%s\t%d\t%s\t%s\n", file_path, buf->st_nlink, get_type(buf), buf->st_size,
               format_date(tmp_str1, buf->st_atim.tv_sec), format_date(tmp_str2, buf->st_mtim.tv_sec));
    }
    return 0;
}



int main(int argc, char* argv[]) {

    if(argc < 2){
        printf("Usage: program <path> [OPTIONS]\n");
        printf("----\n");
        printf("The following options can be used:\n");
        printf("-maxdepth <n> \t - maximum recursion in folder tree\n");
        printf("-atime [+/-]<days> \t max/min time in days of last file access.\n");
        printf("-mtime [+/-]<days> \t max/min time in days of last file modification.\n");
        exit(0);
    }
    printf("Path:\t\t\t\t\t Links:\t Type:\tSize:\tLast access:\tLast_modification:\n");
    printf("----------------------------------------------------------------------------------------------------------\n");

    char abs_path[PATH_MAX];
    realpath(argv[1], abs_path);
    int i=2;
    while(i < argc){
        if(strcmp(argv[i], "-mtime")==0){
            i++;
            if(argv[i]== NULL){
                printf("Not enought arguments!");
                exit(1);
            }
            char* mtime = argv[i];
            if(mtime[0] == '-'){
                mtime_max = (-1) * atoi(mtime);
            }
            else if(mtime[0] == '+'){
                mtime_min = atoi(mtime);
            }
            else {
                mtime_min = mtime_max = atoi(mtime);
            }
        }
        else if(strcmp(argv[i], "-atime")==0){
            i++;
            if(argv[i]== NULL){
                printf("Not enought arguments!");
                exit(1);
            }
            char* mtime = argv[i];
            if(mtime[0] == '-'){
                atime_max = (-1) * atoi(mtime);
            }
            else if(mtime[0] == '+'){
                atime_min = atoi(mtime);
            }
            else {
                atime_min = atime_max = atoi(mtime);
            }
        }
        else if(strcmp(argv[i], "-maxdepth")==0){
            i++;
            if(argv[i]== NULL){
                printf("Not enought arguments!");
                exit(1);
            }
            max_depth = atoi(argv[i]);
        }
        i++;
    }
    realpath(argv[1], abs_path);
    printf("--------------\t DIR & STAT:\t --------------\n");
    search_dir(abs_path, filename, mtime_min,mtime_max,atime_min,atime_max,max_depth, 1);
    printf("--------------\t NFTW:\t --------------\n");
    nftw(abs_path, fn, 10, FTW_PHYS);
    return 0;
}
