/**
 * Bireysel Çaşışma
 * Berhan Berk Akgün / 21120205310
 * 
 * ChatGPT'den yardım aldığım noktalar oldu, 
 * local makinadan zaman bilgisini çekerken onun verdiği kodlardan yararlandım, 
 * bazı fonksyonların ve systemcall'larının nasıl 
 * çalıştığını ve alternatiflerinin neler olduğunu, nasıl kullanıldıklarını sordum.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#define INBUF_SIZE 256

/* these should be the same as multishell.c */
#define MY_FILE_SIZE 1024
#define MY_SHARED_FILE_NAME "/sharedlogfile"

char *addr = NULL;
int fd = -1;

int initmem()
{
    fd = shm_open(MY_SHARED_FILE_NAME, O_RDWR, 0);
    if (fd < 0){
        perror("singleshell.c:fd:line31");
        exit(1);
    }
    addr = mmap(NULL, MY_FILE_SIZE,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == NULL){
        perror("singleshell.c:mmap:");
        close(fd);
        exit(1);
    }
    return 0;
}
/**
 * todo: you can start with myshellv3.c from lecture notes
 */
int main(int argc, char *argv[])
{
    initmem();
    
    /* Zaman bilgisinin çekilmesi */
    time_t current_time;
    struct tm * time_info;
    time(&current_time);
    time_info = localtime(&current_time);
    
    char buf[INBUF_SIZE] = {'\0'}; 		/* Terminale girilecek veriyi almak için */
    int nbyte; 				/* Terminalden alınan verinin boyutunu tutması için */
    char start_msg[MY_FILE_SIZE]; 		/* Başlangıç zamanını ve process id leri tutması için */
    char end_msg[MY_FILE_SIZE]; 		/* Program sonlandırıldığında ki zaman bilgisini tutmak için*/
    char log_msg[MY_FILE_SIZE]; 		/* Terminale girilen verileri tutması için */ 
    char child_id_msg[MY_FILE_SIZE]; 		/* child process id'sini tutmak için */
    memset(child_id_msg, 0, MY_FILE_SIZE);
    
    sprintf(start_msg, "Process ID: %d, Parent Process ID: %d, Start Time: %s", getpid(), getppid(), asctime(time_info));
    strncpy(addr, start_msg, MY_FILE_SIZE);
   
    write(1, "$", 1);
     
    while (1) {
   
    	if ((nbyte = read(0, buf, 255)) <= 0) {
      		perror("input <=0 byte");
    	} else {
    		buf[nbyte - 1] = '\0';
    	}
    	
    	if (strncmp(buf, "exit", 4) == 0) {
    
    		time(&current_time);
		time_info = localtime(&current_time);
    		sprintf(end_msg, "End Time: %s", asctime(time_info));
    		
    		if (child_id_msg != NULL) {
    			strncpy(&addr[strlen(addr)], child_id_msg, MY_FILE_SIZE);
		}
    		
    		strncpy(&addr[strlen(addr)], end_msg, MY_FILE_SIZE);
      		exit(0);
    	}
    	
    	pid_t pid = fork();
	sprintf(child_id_msg, "Child Process ID: %d, ", getpid());
    	
    	if (pid == -1) {
        	perror("fork");
        	exit(1);
    	} else if (pid == 0) {
        	// Child process
        	int r = execl(buf, buf, NULL);
        	if (r == -1) {
        	
        		if(strchr(buf, ' ') != NULL) {
            			char command[255] = {'/', 'b', 'i', 'n','/', '\0'};
            			
            			/* Burada token kullanarak terminale girilen veriyi boşluk karakteri olan yerinden ayırıyorum */
            			char *token = strtok(buf, " ");
            			if (token != NULL) {
            				strncat(command, token, 250);
            			}
            			
            			/* Burada boşluk karakterinden sonraki veriyi alıp bir diziye atıyorum */
				char *ar[1] = {NULL};
				ar[0] = token;
            			char *filename = NULL;
				while (token != NULL) {
    					filename = token;
    					token = strtok(NULL, " ");
				}
			
            			r = execl(command, ar[0], filename, NULL);
            			if (r == -1) {
                			perror("execl");
                			exit(1);
            			}		
    			} else {
    				char command[255] = {'/', 'b', 'i', 'n','/', '\0'};
            			strncat(command, buf, 250);
            			
            			r = execl(command, buf, NULL);
            			if (r == -1) {
                			perror("execl");
                			exit(1);
            			}
    			}
        	}
    	} else {
        	wait(NULL);
        	sprintf(log_msg, "%s\n", buf);
		strncpy(&addr[strlen(addr)], child_id_msg, MY_FILE_SIZE);
    		strncpy(&addr[strlen(addr)], log_msg, MY_FILE_SIZE);
     		write(1, "$", 1);
      	}
    }

    // Unmap the shared memory
    munmap(addr, 1024);

    // Close the shared memory file
    close(fd);
}
