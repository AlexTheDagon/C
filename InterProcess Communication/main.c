#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

#define PIPE_RESP "RESP_PIPE_56050"
#define PIPE_REQ "REQ_PIPE_56050"
#define SHM_NAME "/2NLV4f"


char msg_success[100] = "SUCCESS";
char msg_error[100] = "ERROR";
char msg_ping[100] = "PING";
char msg_pong[100] = "PONG";
char msg_create_shm[100] = "CREATE_SHM";
char msg_write_to_shm[100] = "WRITE_TO_SHM";
char msg_map_file[100] = "MAP_FILE";
char msg_read_from_file_offset[100] = "READ_FROM_FILE_OFFSET";
char msg_read_from_file_section[100] = "READ_FROM_FILE_SECTION";
char msg_read_from_logical_space_offset[100] = "READ_FROM_LOGICAL_SPACE_OFFSET";





int resp;
int req;


int shm;
unsigned int shm_size;
unsigned int shm_offset;
char shm_value_char[4];
char *shm_data;


int shm_file;
char *shm_file_data;
char shm_file_name[100];
unsigned int shm_file_size;
unsigned int shm_file_offset;
unsigned int shm_file_no_of_bytes;

unsigned int cur_byte;
unsigned int no_of_sections;
unsigned int sect_offset;
unsigned int sect_size;

unsigned int section_no;
unsigned int section_offset;
unsigned int section_no_of_bytes;

unsigned int logical_offset;
unsigned int logical_no_of_bytes;
unsigned int alignment = 4096;
unsigned int no_of_alignments = 0;
unsigned int current_alignment = 0;
unsigned int next_alignment = 0;

int no_exit;
char dim;
char buff[100];
char init_buff[100];

char aux;

unsigned int convert_string_to_unsigned_int(char *nr, int nr_size) {
    unsigned int res = 0;
    unsigned char aux = 0;
    for(int i = nr_size - 1; i >= 0; --i) {
        aux = (unsigned char)(*(nr + i));
        res = (res << 8) + aux;
    }
    return res;
}

void write_message_on_pipe(char my_message[]) {
    dim = strlen(my_message);
    write(resp, &dim, 1);
    write(resp, my_message, dim);
}

void ping() {
    write_message_on_pipe(msg_ping);
    write_message_on_pipe(msg_pong);
    unsigned int x = 56050;
    write(resp, &x, 4);
}

void create_shm() {
    read(req, &shm_size, 4);

    shm = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0664);
    
    if(shm < 0) {
        write_message_on_pipe(msg_create_shm);
        write_message_on_pipe(msg_error);
        return;
    }

    ftruncate(shm, shm_size);
    shm_data = (char*)mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);

    if(shm_data == MAP_FAILED) {
        write_message_on_pipe(msg_create_shm)
        ;
        write_message_on_pipe(msg_error);
        return;
    }
    write_message_on_pipe(msg_create_shm);
    write_message_on_pipe(msg_success);
}

void write_shm() {
    
    
    read(req, &shm_offset, 4);
    read(req, shm_value_char, 4);
    if(shm_offset + 3 > shm_size) {
        write_message_on_pipe(msg_write_to_shm);
        write_message_on_pipe(msg_error);
    }

    strncpy(shm_data + shm_offset, shm_value_char, 4);

    write_message_on_pipe(msg_write_to_shm);
    write_message_on_pipe(msg_success);
}

void map_file() {
    read(req, &dim, 1);
    read(req, shm_file_name, dim);
    shm_file_name[(int)dim] = '\0';
    
    shm_file = open(shm_file_name, O_RDONLY);
    

    if(shm_file < 0) {
        write_message_on_pipe(msg_map_file);
        write_message_on_pipe(msg_error);
        return;
    }

    shm_file_size = lseek(shm_file, 0, SEEK_END);
    ftruncate(shm_file, shm_file_size);
    shm_file_data = (char*) mmap(0, shm_file_size, PROT_READ , MAP_SHARED, shm_file, 0);
    
    if(shm_file_data == MAP_FAILED) {
        write_message_on_pipe(msg_map_file);
        write_message_on_pipe(msg_error);
        return;
    }
    write_message_on_pipe(msg_map_file);
    write_message_on_pipe(msg_success);
}

void read_from_file_offset() {
    read(req, &shm_file_offset, 4);
    read(req, &shm_file_no_of_bytes, 4);
    if(shm_file < 0 || shm_file_data == MAP_FAILED || shm_file_offset + shm_file_no_of_bytes - 1 >= shm_file_size) {
        write_message_on_pipe(msg_read_from_file_offset);
        write_message_on_pipe(msg_error);
        return;
    }
    //printf("%u %u\n", shm_file_offset, shm_file_no_of_bytes);
    strncpy(shm_data, shm_file_data + shm_file_offset, shm_file_no_of_bytes);
    
    write_message_on_pipe(msg_read_from_file_offset);
    write_message_on_pipe(msg_success);
    
}

void read_from_file_section() {
    read(req, &section_no, 4);
    read(req, &section_offset, 4);
    read(req, &section_no_of_bytes, 4);
    no_of_sections = (unsigned char)shm_file_data[8];
    //printf("NR OF SECTIONS: %d\n", no_of_sections);
    if(no_of_sections < section_no) {
        write_message_on_pipe(msg_read_from_file_section);
        write_message_on_pipe(msg_error);
        return;
    }
    //printf("MY SIZE: %d %d %d\n", (int)shm_file_data[4], (int)shm_file_data[5], (int)shm_file_data[4] * 256 + (int)shm_file_data[5]);
    cur_byte = 9 + 19 * (section_no - 1) + 11; 
    sect_offset = convert_string_to_unsigned_int(shm_file_data + cur_byte, 4);
    cur_byte += 4;

    sect_size = convert_string_to_unsigned_int(shm_file_data + cur_byte, 4); 
    cur_byte += 4;

    if(section_offset + section_no_of_bytes >= sect_size) {
        write_message_on_pipe(msg_read_from_file_section);
        write_message_on_pipe(msg_error);
        return;
    }
    strncpy(shm_data, shm_file_data + sect_offset + section_offset, section_no_of_bytes);
    write_message_on_pipe(msg_read_from_file_section);
    write_message_on_pipe(msg_success);
}

void read_from_logical_space_offset() {
    read(req, &logical_offset, 4);
    read(req, &logical_no_of_bytes, 4);
    no_of_sections = (unsigned char)shm_file_data[8];
    current_alignment = 0;
    section_no = 1;
    for(unsigned char i = 1; i <= no_of_sections; ++ i) {
        cur_byte = 9 + 19 * (i - 1) + 11;
        sect_offset = convert_string_to_unsigned_int(shm_file_data + cur_byte, 4);
        cur_byte += 4;

        sect_size = convert_string_to_unsigned_int(shm_file_data + cur_byte, 4); 
        cur_byte += 4;

        no_of_alignments = (sect_size - 1) / alignment + 1;
        next_alignment = current_alignment + no_of_alignments * alignment;

        if(current_alignment <= logical_offset && logical_offset < next_alignment) {
            if(logical_offset + logical_no_of_bytes - 1 >= next_alignment) {
                write_message_on_pipe(msg_read_from_logical_space_offset);
                write_message_on_pipe(msg_error);
                return;
            }
            if(logical_offset - current_alignment + sect_offset + logical_no_of_bytes - 1 >= sect_offset + sect_size) {
                write_message_on_pipe(msg_read_from_logical_space_offset);
                write_message_on_pipe(msg_error);
                return;
            }
            strncpy(shm_data, shm_file_data + logical_offset - current_alignment + sect_offset, logical_no_of_bytes);
            write_message_on_pipe(msg_read_from_logical_space_offset);
            write_message_on_pipe(msg_success);
            return;
        }
        current_alignment = next_alignment;
    }
   
    write_message_on_pipe(msg_read_from_logical_space_offset);
    write_message_on_pipe(msg_error);
    return;
}

int main(int argc, char *argv[]) {
    // First we open the pipes
    mkfifo(PIPE_RESP, 0600);
    req = open(PIPE_REQ, O_RDONLY);
    resp = open(PIPE_RESP, O_WRONLY);
    
    if(resp < 0) {
        printf("ERROR\ncannot create the response pipe\n");
        goto mr_proper;
    }

    if(req < 0) {
        printf("ERROR\n cannot open the request pipe\n");
        goto mr_proper;
    }

    printf("SUCCESS\n");
    dim = 7;
    write(resp, &dim, 1);
    write(resp, "CONNECT", 7);
    

    no_exit = 1;
    while(no_exit) {
        
        read(req, &dim, 1);
        read(req, buff, dim);
        buff[(int)dim] = '\0';

        if(strcmp(buff, "PING") == 0) ping();
        if(strcmp(buff, "CREATE_SHM") == 0) create_shm();
        if(strcmp(buff, "WRITE_TO_SHM") == 0) write_shm();       
        if(strcmp(buff, "MAP_FILE") == 0) map_file(); 
        if(strcmp(buff, "READ_FROM_FILE_OFFSET") == 0) read_from_file_offset();   
        if(strcmp(buff, "READ_FROM_FILE_SECTION") == 0) read_from_file_section();  
        if(strcmp(buff, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0) read_from_logical_space_offset();  

        if(strcmp(buff, "EXIT") == 0) no_exit = 0;
    }



mr_proper: // Mai curat si mai usor, cu asa un ajutor
    close(resp);
    close(req);
    unlink(PIPE_RESP);
    shm_unlink(SHM_NAME);
    if(strlen(shm_file_name) != 0)shm_unlink(shm_file_name);
    if(shm_data != NULL && shm_data != MAP_FAILED) munmap(shm_data, shm_size);
    if(shm_file_data != NULL && shm_file_data != MAP_FAILED) munmap(shm_file_data, shm_size);

    return 0;
}