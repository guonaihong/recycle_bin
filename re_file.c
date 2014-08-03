#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

void re_file(const char *fname) {
    DIR *dp = NULL;
    struct dirent *entry = NULL;
    struct stat sb;
    
    dp = opendir(fname);
    if (dp == NULL) {
        fprintf(stderr, "can not open directory <%s>\n", fname);
        return ;
    }

    chdir(fname);
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' 
                    || (entry->d_name[1] == '.' && entry->d_name[2] == '\0' )))
            continue;

        stat(entry->d_name, &sb);
        if (S_ISDIR(sb.st_mode)) {
            re_file(entry->d_name);
        } else {
            char path[PATH_MAX + 1];
            printf("%s/%s\n", getcwd(path, PATH_MAX), entry->d_name);
        }
    }
    chdir("..");
    closedir(dp);
    return ;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "<Usage> directory name\n");
        return 1;
    }

    re_file(argv[1]);
    return 0;
}
