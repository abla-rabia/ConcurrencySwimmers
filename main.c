#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define  nc 2
#define np 5
#define nbNag 10 //nb de nageurs 
// les semaphores
#define mutex 0
#define span 1
#define scab 2
//les variables partagée
#define npd 0//nombre de paniers demandes
#define npo 1//nombre de paniers occupes

int sem ;// semaphore
int memp; // memoire partagee

int main(){
    // Creation des semaphores 
    key_t cle;
    int p ;
    /* Création d'une clé  */
    cle = ftok("main.c" ,1);
    if(cle ==-1) {
	printf("\n erreur : la clé n’a pas été créée");
	exit(1);
	} 
    /* Création d'un tableau de 3 sémaphores */
    sem = semget(cle, 3, IPC_CREAT|0666);
     /*initialisation des semaphores*/
    semctl(sem,mutex,SETVAL,1);//mutex
    semctl(sem,span,SETVAL,0);//span
    semctl(sem,scab,SETVAL,nc);//scab
    //--------------------------------------------------
    // Creation de la memoire partagee 
    /* Créer un segment de mémoire partagée. */
	int taille =  2* sizeof(int);
       memp = shmget (cle, taille, IPC_CREAT | 0666);
     if(memp==-1){
	 printf(" \n erreur  dans la creation du segment "); 
	 exit(2);
	}
    /* Attacher le segment de mémoire partagée.*/
	int *tnp; //le tableau des varibles patagées
     
	tnp =  shmat (memp, 0, 0);
	if(tnp == NULL) 
	{ printf("\nErreur dans l'attachement du segment ");
 	exit(3);
 	} 
    //initialisation des variables partagées
    tnp[npo]=0;
    tnp[npd]=0;
    /* Détacher le segment de mémoire partagée. */
    shmdt (tnp);
    //---------------------------------------------
    //---------- Creation des processus 
    int i;
    char ic[20];
    for ( i = 0; i<nbNag; i++)
        {
                p = fork();
              if(p==0){
    /*transformer entier chaine*/
            sprintf(ic,"%d",i);
         execlp("./pgme_nageur","pgme_nageur",ic,NULL);
          
        }
        else {
        if (p <0 ) printf("Erreur");
       } }
    
    
	while ( wait(NULL)!= -1){
	}
    
    /* Supprimer les sémaphores */
	semctl(sem, IPC_RMID, 0);
  
    /* Libèrer le segment de mémoire partagée. */
	shmctl (memp, IPC_RMID, 0);
	
 
   printf("\n fin du programme principal \n");
   return 0;
}