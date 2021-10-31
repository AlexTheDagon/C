//gcc -Wall a1.c -o a1


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>



typedef struct sections {
    char name[10];
    int offset;
    int type;
    int size;
} Section;



void list_dir(char *path, char *name, int recursive, int has_perm_execute){

    DIR *dir = opendir(path);
    
    struct dirent *dir_entry;
    while((dir_entry = readdir(dir)) != NULL) {
        if(name == NULL || (strstr(dir_entry->d_name, name) != NULL && strcmp(dir_entry->d_name, strstr(dir_entry->d_name, name)) ==0)){
            if(strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0) {
                if(name == NULL){
                    if(has_perm_execute == 0)printf("%s/%s\n", path, dir_entry->d_name);
                    else {
                        char *filepath = (char*)calloc(100, sizeof(char));
                        strcpy(filepath, path);
                        strcat(filepath, "/");
                        strcat(filepath, dir_entry->d_name);
                        if(access(filepath, X_OK) == 0) printf("%s/%s\n", path, dir_entry->d_name);
                        free(filepath);
                    }
                }
                else printf("%s/%s\n", path, dir_entry->d_name);
            }
        }
    }

    if(dir!= NULL) closedir(dir);
    //free(dir);
    //free(dir_entry);
}

void listing(char *path, char *name, int recursive, int has_perm_execute) {

    DIR *dir = opendir(path);

    list_dir(path, name, recursive, has_perm_execute);

    if(!recursive) {
        if(dir != NULL) closedir(dir);
        //free(dir);
        return;
    }

    struct dirent *dir_entry;
    while((dir_entry = readdir(dir)) != NULL) {
        if(dir_entry->d_type == DT_DIR && (strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0)){
            char* nextPath = (char*)calloc(100, sizeof(char));
            strcpy(nextPath, path);
            strcat(nextPath, "/");
            strcat(nextPath, dir_entry->d_name);
            listing(nextPath, name, recursive, has_perm_execute);
            free(nextPath);
        }
    }
    if(dir != NULL) closedir(dir);
    //free(dir);
    //free(dir_entry);
}

void listingOption(int argc, char **argv){
    char* name = NULL;
    char* path = NULL;
    int has_perm_execute = 0;
    int recursive = 0;
    // recursive
    // name_starts_with=string | has_perm_execute
    
    for(int i = 1; i < argc; ++i){
        if(strcmp(argv[i], "recursive") == 0) recursive = 1;
        if(strcmp(argv[i], "has_perm_execute") == 0) has_perm_execute = 1;
        
        if(strstr(argv[i], "path=") != NULL && strcmp(argv[i], strstr(argv[i], "path=")) == 0) {
            path = (char*)calloc(100, sizeof(char));
            strcpy(path, argv[i] + 5);
        }
        if(strstr(argv[i], "name_starts_with=") != NULL && strcmp(argv[i], strstr(argv[i], "name_starts_with=")) == 0) {
            name = (char*)calloc(100, sizeof(char));
            strcpy(name, argv[i] + 17);
        }
    }
    DIR *dir = opendir(path);
    if(dir == NULL) {
        printf("ERROR\ninvalid directory path\n");
    }
    else {
        printf("SUCCESS\n");
        listing(path, name, recursive, has_perm_execute);
    }
    if(dir != NULL) closedir(dir);

    free(name);
    free(path);
}



void parse(int argc, char **argv) {
    Section *s = NULL;
    char* path = NULL;
    for(int i = 1; i < argc; ++i) {
        if(strstr(argv[i], "path=") != NULL && strcmp(argv[i], strstr(argv[i], "path=")) == 0) {
            path = (char*)calloc(100, sizeof(char));
            strcpy(path, argv[i] + 5);
        }
    }
    int fd = open(path, O_RDONLY);

    char magic[4];
    char header_size[2];
    int version = 0;
    int no_of_sections = 0;

    if(read(fd, magic, 4) != 4) {
        printf("ERROR\nwrong magic\n");
        goto freeparse;
    }
    if(read(fd, header_size, 2) != 2) {
        printf("ERROR\nwrong header_size\n");
        goto freeparse;
    }
    if(read(fd, &version, 2) != 2) {
        printf("ERROR\nwrong version\n");
        goto freeparse;
    }
    if(read(fd, &no_of_sections, 1) != 1) {
        printf("ERROR\nwrong sect_nr\n");
        goto freeparse;
    }

    if(strcmp("48hl", magic) != 0) {
        printf("ERROR\nwrong magic\n");
        goto freeparse;
    }

    if(version < 83 || 176 < version) {
        printf("ERROR\nwrong version\n");
        goto freeparse;
    }
    //printf("%d\n", no_of_sections);
    if(no_of_sections < 2 || 16 < no_of_sections) {
        printf("ERROR\nwrong sect_nr\n");
        goto freeparse;
    }

    int n = no_of_sections;
    s = (Section*) calloc(n, sizeof(Section));
    if(s == NULL) goto freeparse;
    for(int ind = 0; ind < n; ++ ind){
        
        s[ind].type = 0;
        s[ind].offset = 0;
        s[ind].name[9] = 0;
        s[ind].size = 0;

        read(fd, s[ind].name, 9);
        
        if(read(fd, &s[ind].type, 2) != 2) {
            printf("ERROR\nwrong sect_types\n");
            goto freeparse;
        }
        

        if(s[ind].type != 87 && s[ind].type != 32){
            printf("ERROR\nwrong sect_types\n");
            goto freeparse;
        }
        

        
        read(fd, &s[ind].offset, 4);
        read(fd, &s[ind].size, 4);

    }
    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d\n", n);
    for(int i = 0; i < n; ++i){
        printf("section%d: ", i + 1);
        printf("%s ", s[i].name);
        printf("%d ", s[i].type);
        printf("%d\n", s[i].size);
    }

freeparse:
    free(path);
    free(s);
}

int get_line(int fd, int length, int line_no, int count_lines) {
    int currentLine = 1;
    char buff;
    for(int i = 0; i < length; ++i){
        read(fd, &buff, 1);
        if(buff == '\n'){
            if(i != length - 1)++currentLine;
        }
        else {
            if(count_lines == 0 && currentLine == line_no) {
                printf("%c", buff);
            }
        }
    }
    return currentLine;
}

int count_line(int fd, int length) {
    int currentLine = 1;
    char buff[length];
    read(fd, &buff, length);
    for(int i = 0; i < length; ++i){
        
        if(buff[i] == '\n'){
            if(i != length - 1)++currentLine;
        }
    }
    return currentLine;
}

void extract(int argc, char **argv) {
    Section *s = NULL;
    char* path = NULL;
    char* line_s = NULL;
    char* sections_s = NULL;
    for(int i = 1; i < argc; ++i) {
        if(strstr(argv[i], "path=") != NULL && strcmp(argv[i], strstr(argv[i], "path=")) == 0) {
            path = (char*)calloc(100, sizeof(char));
            strcpy(path, argv[i] + 5);
        }
        if(strstr(argv[i], "section=") != NULL && strcmp(argv[i], strstr(argv[i], "section=")) == 0) {
            sections_s = (char*)calloc(100, sizeof(char));
            strcpy(sections_s, argv[i] + 8);
        }
        if(strstr(argv[i], "line=") != NULL && strcmp(argv[i], strstr(argv[i], "line=")) == 0) {
            line_s = (char*)calloc(100, sizeof(char));
            strcpy(line_s, argv[i] + 5);
        }
    }

    int line = 0;
    int section = 0;
    for(int i = 0; i < strlen(line_s); ++i){
        line = line * 10 + line_s[i] - '0';
    }
    for(int i = 0; i < strlen(sections_s); ++i){
        section = section * 10 + sections_s[i] - '0';
    }

    //printf("%d %d\n\n", line, section);

    int fd = open(path, O_RDONLY);

    char magic[4];
    char header_size[2];
    int version = 0;
    int no_of_sections = 0;

    if(read(fd, magic, 4) != 4) {printf("ERROR\nwrong magic\n");goto freeextract;}
    if(read(fd, header_size, 2) != 2) {printf("ERROR\nwrong header_size\n");goto freeextract;}
    if(read(fd, &version, 2) != 2) {printf("ERROR\nwrong version\n");goto freeextract;}
    if(read(fd, &no_of_sections, 1) != 1) {printf("ERROR\nwrong sect_nr\n");goto freeextract;}
    if(strcmp("48hl", magic) != 0) {printf("ERROR\nwrong magic\n");goto freeextract;}
    if(version < 83 || 176 < version) {printf("ERROR\nwrong version\n");goto freeextract;}
    if(no_of_sections < 2 || 16 < no_of_sections) {printf("ERROR\nwrong sect_nr\n");goto freeextract;}

    int n = no_of_sections;
    s = (Section*) calloc(n, sizeof(Section));
    if(s == NULL) goto freeextract;
    for(int ind = 0; ind < n; ++ ind){
        s[ind].type = 0;
        s[ind].offset = 0;
        s[ind].name[9] = 0;
        s[ind].size = 0;

        read(fd, s[ind].name, 9);
        
        if(read(fd, &s[ind].type, 2) != 2) {printf("ERROR\nwrong sect_types\n");goto freeextract;}
        if(s[ind].type != 87 && s[ind].type != 32){printf("ERROR\nwrong sect_types\n");goto freeextract;}
        
        read(fd, &s[ind].offset, 4);
        read(fd, &s[ind].size, 4);

    }
    
    free(line_s);
    line_s = (char*)calloc(100, sizeof(char));
    section --;
    lseek(fd, s[section].offset, SEEK_SET);
    printf("SUCCESS\n");
    get_line(fd, s[section].size, line, 0);

    

freeextract:
    free(path);
    free(line_s);
    free(sections_s);
    free(s);
}

int check_file(char* path){
    Section *s = NULL;
    

    //printf("%d %d\n\n", line, section);

    int fd = open(path, O_RDONLY);
    int cnt = 0;
    char magic[4];
    char header_size[2];
    int version = 0;
    int no_of_sections = 0;

    if(read(fd, magic, 4) != 4) {goto freecheck;}
    if(read(fd, header_size, 2) != 2) {goto freecheck;}
    if(read(fd, &version, 2) != 2) {goto freecheck;}
    if(read(fd, &no_of_sections, 1) != 1) {goto freecheck;}
    if(strcmp("48hl", magic) != 0) {goto freecheck;}
    if(version < 83 || 176 < version) {goto freecheck;}
    if(no_of_sections < 2 || 16 < no_of_sections) {goto freecheck;}

    int n = no_of_sections;
    s = (Section*) calloc(n, sizeof(Section));
    if(s == NULL) goto freecheck;
    for(int ind = 0; ind < n; ++ ind){
        s[ind].type = 0;
        s[ind].offset = 0;
        s[ind].name[9] = 0;
        s[ind].size = 0;

        read(fd, s[ind].name, 9);
        
        if(read(fd, &s[ind].type, 2) != 2) {goto freecheck;}
        if(s[ind].type != 87 && s[ind].type != 32){goto freecheck;}
        
        read(fd, &s[ind].offset, 4);
        read(fd, &s[ind].size, 4);

        
    }
    
    for(int i = 0; i < n; ++i){
        lseek(fd, s[i].offset, SEEK_SET);
        if(count_line(fd, s[i].size) == 16) ++ cnt;
    }
    
    
    

    

freecheck:
    free(s);

    if(cnt > 3) return 1;
    return 0;
}

void last_listing(char *path) {

    DIR *dir = opendir(path);


    struct dirent *dir_entry;
    while((dir_entry = readdir(dir)) != NULL) {
        if(dir_entry->d_type == DT_DIR && (strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0)){
            char* nextPath = (char*)calloc(100, sizeof(char));
            strcpy(nextPath, path);
            strcat(nextPath, "/");
            strcat(nextPath, dir_entry->d_name);
            last_listing(nextPath);
            free(nextPath);
        }
        else {
            if(dir_entry->d_type == DT_REG){
                char* nextPath = (char*)calloc(100, sizeof(char));
                strcpy(nextPath, path);
                strcat(nextPath, "/");
                strcat(nextPath, dir_entry->d_name);
                if(check_file(nextPath) == 1){
                    printf("%s\n", nextPath);
                }
                free(nextPath);
            }
        }
    }
    if(dir != NULL) closedir(dir);
    //free(dir);
    //free(dir_entry);
}

void findall(int argc, char **argv){
    char* path = NULL;
    for(int i = 1; i < argc; ++i) {
        if(strstr(argv[i], "path=") != NULL && strcmp(argv[i], strstr(argv[i], "path=")) == 0) {
            path = (char*)calloc(100, sizeof(char));
            strcpy(path, argv[i] + 5);
        }
    }
    DIR *dir = opendir(path);
    if(dir == NULL) {
        printf("ERROR\ninvalid directory path\n");
    }
    printf("SUCCESS\n");
    last_listing(path);
    if(dir != NULL) closedir(dir);
}

int main(int argc, char **argv){
    if(argc >= 2){
        if(strcmp(argv[1], "variant") == 0){
            printf("56050\n");
        }
        if(strcmp(argv[1], "list") == 0){
            listingOption(argc, argv);
        }
        if(strcmp(argv[1], "parse") == 0){
            parse(argc, argv);
        }
        if(strcmp(argv[1], "extract") == 0){
            extract(argc, argv);
        }
        if(strcmp(argv[1], "findall") == 0){
            findall(argc, argv);
        }
    }
    return 0;
}