#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

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

    return 0;
}
