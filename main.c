#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct {
    char path[PATH_MAX];
    char name[FILENAME_MAX];
    int  size;
} file_t;

char util_name[FILENAME_MAX];
file_t *f_list;
int f_list_len = 0;

int (*cmpfunc)(file_t file1, file_t file2);

int cmpname(file_t file1, file_t file2) {
    return (strcmp(file1.name, file2.name) > 0);
}

int cmpsize(file_t file1, file_t file2) {
    return ((file1.size - file2.size) < 0);
}

char* slshamnd(char* path) {
    if (path[strlen(path) - 1] != '/')
        strcat(path, "/");
    return path;
}

int dirpass(const char *dir_name) {
    DIR *directory;

    if (!(directory = opendir(dir_name))) {
        fprintf(stderr, "%s: %s: %s\n", util_name, dir_name, strerror(errno));
        return -1;
    }

    struct dirent *dir_item;
    while((dir_item = readdir(directory)) != NULL) {
        char next_item[PATH_MAX];
        strcpy(next_item, dir_name);
        strcat(next_item, dir_item->d_name);
        if(strcmp(".", dir_item->d_name) == 0 ||
            strcmp("..", dir_item->d_name) == 0)
            continue;
        struct stat statbuf;
        lstat(next_item, &statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            dirpass(slshamnd(next_item));
        } else if(S_ISREG(statbuf.st_mode)) {
            file_t file_tmp;
            strcpy(file_tmp.name, dir_item->d_name);
            strcpy(file_tmp.path, next_item);
            file_tmp.size = (int) statbuf.st_size;
            f_list = realloc(f_list, (++f_list_len)*sizeof(file_t));
            f_list[f_list_len - 1] = file_tmp;
        }
    }
    closedir(directory);
}

void flistsort(int sort_opt){
    if (sort_opt == 1)
        cmpfunc = cmpname;
    else
        cmpfunc = cmpsize;

    int i, j;
    for (i = 0; i < f_list_len; i++) {
        for (j = 0; j < f_list_len - i - 1; j++) {
            if (cmpfunc(f_list[j], f_list[j+1])) {
                file_t file_tmp = f_list[j];
                f_list[j] = f_list[j+1];
                f_list[j+1] = file_tmp;
            }
        }
    }
}

int main(int argc, char const *argv[]) {
    strcpy(util_name, (char*) basename(argv[0]));

    if (argc != 4) {
        fprintf(stderr, "%s: Wrong number of arguments\n", util_name);
        return -1;
    }

    char dist_path[PATH_MAX];
    realpath(argv[3], dist_path);
    slshamnd(dist_path);

    char sort_opt = atoi(argv[2]);
    if (sort_opt != 1 && sort_opt != 2) {
        fprintf(stderr,"%s: Wrong sort option (Must be: 1 or 2)\n", util_name);
        return -1;
    }

    char initdir[PATH_MAX];
    strcpy(initdir, argv[1]);
    dirpass(slshamnd(initdir));

    flistsort(sort_opt);

    if (mkdir(dist_path, 0777) == -1 && errno != EEXIST) {
        fprintf(stderr, "%s: %s: %s\n", util_name, dist_path, strerror(errno));
        return -1;
    }

    int i, j;
    for(i = 0; i < f_list_len; i++) {
        j = i;
        while (j++ < f_list_len &&!strcmp(f_list[i].name, f_list[j].name)) {
            char add_index[5];
            sprintf(add_index," (%d)", j-i);
            strcat(f_list[j].name, add_index);
        }

        char dist_file[PATH_MAX + 1];
        strcpy(dist_file, dist_path);
        strcat(dist_file, f_list[i].name);
        if (symlink(f_list[i].path, dist_file) == -1) {
           fprintf(stderr, "%s: %s: %s\n", util_name, dist_file, strerror(errno));
        };
    }
    return 0;
}
