#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


//#define SEM_MUTEX_KEY 0x0000DEAD /* Note: you should change this value        */ 
                                 /* to other unique values  such as your      */
                                 /* group# or etc. to not share the semaphore */
                                 /* with other groups. For example if your    */ 
                                 /* group number is 12, use 0x00000012        */
                                 /* Also after running your program by using   */
                                 /* ipcrm command remove the semaphores        */    



union semun1 {          //changed name from semun to semun1 (because gives redefinition error)
        int val;                  /* value for SETVAL */
        struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
        unsigned short *array;    /* array for GETALL, SETALL */
                                  /* Linux specific part: */
        struct seminfo *__buf;    /* buffer for IPC_INFO */
};

static int sem_mutex; // Semaphore id's
void setSemValue(int semaphore, int count)
{
    union semun1 sem_union;

    sem_union.val = count;

    // Initalize semaphore with new count
    if ( semctl(semaphore, 0, SETVAL, sem_union) == -1)
    {
        fprintf(stderr, "(#%d) ", errno);
        perror("semctl() failed");
        exit(EXIT_FAILURE);
    }
    
}

int createSem()
{
    //sem_mutex = semget((key_t)SEM_MUTEX_KEY, 1, 0666 | IPC_CREAT);
    int sem_mutex = semget(IPC_PRIVATE,1,0666|IPC_CREAT);
     
    //printf("[**] Semaphore MUTEX successfully attached with id: %d\n", sem_mutex);

 
    // Initalize MUTEX semaphore
    setSemValue(sem_mutex, 1);
    //printf("[*] Semaphore MUTEX count initalized to 1.\n");
        
    return sem_mutex;
}
       

void down(int semaphore)
{
    struct sembuf semBuff;

    // Decrease semaphore count by 1
    semBuff.sem_num = 0;
    semBuff.sem_op = -1;
    semBuff.sem_flg = SEM_UNDO;

    if (semop(semaphore, &semBuff, 1) == -1)
    {
        fprintf(stderr, "(#%d) ", errno);
        perror("semop() failed");
        exit(EXIT_FAILURE);
    }
}

void up(int semaphore)
{
    struct sembuf semBuff;
    
    // Increase semaphore count by 1
    semBuff.sem_num = 0;
    semBuff.sem_op = +1;
    semBuff.sem_flg = SEM_UNDO;

    if (semop(semaphore, &semBuff, 1) == -1)
    {
        fprintf(stderr, "(#%d) ", errno);
        perror("semop() failed");
        exit(EXIT_FAILURE);
    }
}




