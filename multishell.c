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

/* these should be the same as multishell.c */
#define MY_FILE_SIZE 1024
#define MY_SHARED_FILE_NAME "/sharedlogfile"

#define MAX_SHELL 10
#define DEFAULT_NSHELL 2
char *addr = NULL; /*mmap addres*/
int fd = -1;       /*fd for shared file object*/

int initmem()
{
    fd = shm_open(MY_SHARED_FILE_NAME, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd < 0) {
        perror("multishell.c:open file:");
        exit(1);
    }
    if (ftruncate(fd, 1024) == -1) {
        perror("ftruncate");
        exit(1);
    }

    addr = mmap(NULL, MY_FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == NULL) {
        perror("mmap:");
        exit(1);
    }
    return 0;
}

int main(int argc, char **argv) {

    int n_shell = DEFAULT_NSHELL; /* Açılacak shell sayısı */
    
    if (argc > 1) {
        n_shell = atoi(argv[1]);
    }

    char buf[128]; /* txt dosyasının adını tutmak için */
    time_t t = time(NULL);
    strftime(buf, 128, "shelllog-%Y%m%d-%H%M%S.txt", localtime(&t));

    /* Create shared memory */
    initmem();

    pid_t pid;
    int status;

    for (int i = 0; i < n_shell; ++i) {
        
        pid = fork();
        
        if (pid == -1) {
            perror("Failed to fork");
            exit(1);
        } else if (pid == 0) { 
            /* Child process */
            execl("/usr/bin/xterm", "xterm", "-e", "./singleshell", NULL);
            perror("execl failed");
            exit(1);
        }
        
    }

    /* Child processleri beklemek için */
    while (wait(&status) != -1) {
        /* Boş */
    }

    /* shelllog dosyasının oluşturulması */
    int fd = open(buf, O_CREAT | O_WRONLY | O_TRUNC);
    if (fd == -1) {
        perror("Failed to create log file");
        exit(1);
    }

    /* dosyaya addr nin yazılması */
    if (write(fd, addr, strlen(addr)) == -1) {
        perror("Failed to write to file");
        exit(1);
    }

    /*unlink mmap*/
    munmap(addr, 1024);
    
    /* close the shared memory file*/
    close(fd);

    return 0;
}
