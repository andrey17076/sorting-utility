#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define BUFSIZE 512

typedef struct {
    char path[PATH_MAX];
    char name[FILENAME_MAX];
    int  size;
} file_t;

const char *utility_name;

int (*cmpfunction)(file_t file1, file_t file2);

int cmpname(file_t file1, file_t file2) {
    return strcmp(file1.name, file2.name);
}

int cmpsize(file_t file1, file_t file2) {
    return file1.size - file2.size;
}

int copyfile(const char *sourse_file, const char *dist_file) {
    int sourse_f, dist_f;
    ssize_t bytes_number;
    char buffer[BUFSIZE];

    if ((sourse_f = open(sourse_file, O_RDONLY)) == -1)
        return (-1);

    if ((dist_f = open(dist_file, O_WRONLY|O_CREAT|O_TRUNC, 0777) == -1)) {
        close(sourse_f);
        return (-1);
    }

    while ((bytes_number = read(sourse_f, buffer, BUFSIZE)) > 0) {
        if (write(dist_f, buffer, bytes_number) < bytes_number) {
            close(sourse_f);
            close(dist_f);
            return (-1);
        }
    }

    close(sourse_f);
    close(dist_f);

    if (bytes_number == -1)
        return (-1);
    else
        return (0);
}

int files_rec(const char *dir_name) {
    DIR *directory;
    if ((directory = opendir(dir_name)) == NULL) {
        fprintf(stderr, "%s: %s: %s\n", utility_name, dir_name, strerror(errno));
        return -1;
    }

    struct dirent *dir_item;
    struct stat statbuf;
    chdir(dir_name);
    while((dir_item = readdir(directory)) != NULL) {
        stat(dir_item->d_name, &statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            if(strcmp(".", dir_item->d_name) == 0 ||
                strcmp("..", dir_item->d_name) == 0)
                continue;
            files_rec(dir_item->d_name);
        } else {
            file_t file_tmp;
            strcpy(file_tmp.name, dir_item->d_name);
            char *full_path;
            char buf[PATH_MAX + 1];
            realpath(dir_item->d_name, buf);
            strcpy(file_tmp.path, buf);
            file_tmp.size = (int) statbuf.st_size;
            printf("%s %s %d\n", file_tmp.path, file_tmp.name, file_tmp.size);
        }
    }
    chdir("..");
    closedir(directory);
}

int main(int argc, char const *argv[]) {

    utility_name = (char *) basename(argv[0]);
    if (argc != 4) {
        fprintf(stderr, "%s: Wrong number of arguments\n", utility_name);
        return -1;
    }

    char sort_option = atoi(argv[2]);
    if (sort_option != 1 && sort_option != 2) {
        printf("%s: Wrong sorting option (Must be: 1 or 2)\n", utility_name);
        return -1;
    }

    if (sort_option == 1)
        cmpfunction = cmpname;
    else if(sort_option == 2)
        cmpfunction = cmpsize;

    file_t *files;
    files_rec(argv[1]);

    return 0;
}
