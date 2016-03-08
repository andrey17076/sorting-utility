#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define BUFSIZE 512

int copyfile(const char *sourse_file, const char *dist_file) {
    int sourse_f, dist_f;
    ssize_t bytes_number;
    char buffer[BUFSIZE];

    if ((sourse_f = open(sourse_file, O_RDONLY)) == -1)
        return (-1);
    if ((dist_f = open(dist_file, O_WRONLY|O_CREAT|O_TRUNC, 0777) == -1)) {
        close(sourse_f);
        return (-2);
    }

    while ((bytes_number = read(sourse_f, buffer, BUFSIZE)) > 0) {
        if (write(dist_f, buffer, bytes_number) < bytes_number) {
            close(sourse_f);
            close(dist_f);
            return (-3);
        }
    }

    close(sourse_f);
    close(dist_f);

    if (bytes_number == -1)
        return (-4);
    else
        return (0);
}


int main(int argc, char const *argv[]) {

    const char *utility_name = (char *) basename(argv[0]);
    if (argc != 4) {
        fprintf(stderr, "%s: Wrong number of arguments\n", utility_name);
        return -1;
    }

    DIR *directory;
    if ((directory = opendir(argv[1])) == NULL) {
        printf("%s: %s: %s\n", utility_name, argv[1], strerror(errno));
        return -1;
    }

    char sort_option = atoi(argv[2]);
    if (sort_option != 1 && sort_option != 2) {
        printf("%s: Wrong sorting option (Must be: 1 or 2)\n", utility_name);
        return -1;
    }

    // struct dirent *dir_item;
    // while (dir_item = readdir(directory)) {
    //     if (dir_item->d_ino != 0)
    //         printf("%s\n",dir_item->d_name);
    // }
    //
    // closedir(directory);

    return 0;
}
