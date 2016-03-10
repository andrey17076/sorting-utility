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

char utility_name[FILENAME_MAX];
file_t *file_list;
int file_list_length = 0;

int (*cmpfunction)(file_t file1, file_t file2);

int cmpname(file_t file1, file_t file2) {
    return (strcmp(file1.name, file2.name) > 0);
}

int cmpsize(file_t file1, file_t file2) {
    return ((file1.size - file2.size) < 0);
}

int filesrec(const char *dir_name) {
    DIR *directory;

    if (!(directory = opendir(dir_name))) {
        fprintf(stderr, "%s: %s: %s\n", utility_name, dir_name, strerror(errno));
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
        stat(next_item, &statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            filesrec(next_item);
        } else {
            file_t file_tmp;
            strcpy(file_tmp.name, dir_item->d_name);
            strcpy(file_tmp.path, next_item);
            file_tmp.size = (int) statbuf.st_size;
            file_list = realloc(file_list, (++file_list_length)*sizeof(file_t));
            file_list[file_list_length - 1] = file_tmp;
        }
    }
    closedir(directory);
}

int main(int argc, char const *argv[]) {
    strcpy(utility_name, (char*) basename(argv[0]));

    if (argc != 4) {
        fprintf(stderr, "%s: Wrong number of arguments\n", utility_name);
        return -1;
    }

    char dist_dir[PATH_MAX];
    realpath(argv[3], dist_dir);
    //if (dist_dir[strlen(dist_dir) - 1] != '/')
    //    strcat(dist_dir, "/");

    char sort_option = atoi(argv[2]);
    if (sort_option != 1 && sort_option != 2) {
        fprintf(stderr,"%s: Wrong sorting option (Must be: 1 or 2)\n", utility_name);
        return -1;
    } else if (sort_option == 1)
        cmpfunction = cmpname;
    else
        cmpfunction = cmpsize;

    filesrec(argv[1]);

    int i, j;
    for (i = 0; i < file_list_length; i++) {
        for (j = 0; j < file_list_length - i - 1; j++) {
            if (cmpfunction(file_list[j], file_list[j+1])) {
                file_t file_tmp = file_list[j];
                file_list[j] = file_list[j+1];
                file_list[j+1] = file_tmp;
            }
        }
    }


    if ((mkdir(dist_dir, 0777) == -1) && (errno == EACCES)) {
        fprintf(stderr, "%s: %s: %s\n", utility_name, dist_dir, strerror(errno));
        return -1;
    };

    char dist_file[PATH_MAX + 1];
    for(i = 0; i < file_list_length; i++) {
        j = i;
        while ((++j < file_list_length)&&(!strcmp(file_list[i].name, file_list[j].name))) {
            char add_index[5];
            sprintf(add_index," (%d)", j-i);
            strcat(file_list[j].name, add_index);
        }
        strcpy(dist_file, dist_dir);
        strcat(dist_file, file_list[i].name);
        //printf("%s\n", dist_file);
        // if (symlink(file_list[i].path, dist_file) != -1) {
        //     fprintf(stderr, "%s: %s: %s\n", utility_name, dist_file, strerror(errno));
        // };
    }

    return 0;
}
