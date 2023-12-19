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



/*creation des controleurs des semaphores*/
struct sembuf Pmutex={mutex,-1,0};
struct sembuf Vmutex={mutex,1,0};
struct sembuf Pspan={span,-1,0};
struct sembuf Vspan={span,1,0};
struct sembuf Pscab={scab,-1,0};
struct sembuf Vscab={scab,1,0};

//les procedures 
void Demander_panier(int *Nump){
    int *tnp; //le tableau des varibles patagées
    
    /* Attacher le segment de mémoire partagée.*/
	tnp =  shmat (memp, 0, 0);
    if(tnp == NULL) 
	{ printf("\nErreur dans l'attachement du segment ");
 	exit(3);
 	}
   //-------------------------------------
   semop(sem,&Pmutex,1);  // P(mutex)
   tnp[npd]++;
   if (tnp[npo]==np){
    semop(sem,&Vmutex,1);  // V(mutex)
    semop(sem,&Pspan,1);  // P(span)
   }
   tnp[npd]--;
   tnp[npo]++;
   *Nump=tnp[npo];
   semop(sem,&Vmutex,1);  // V(mutex)
   

}
void Liberer_panier(int i,int *Nump){
    int *tnp; //le tableau des varibles patagées
    /* Attacher le segment de mémoire partagée.*/  
	tnp =  shmat (memp, 0, 0);
    if(tnp == NULL) 
	{ printf("\nErreur dans l'attachement du segment ");
 	exit(3);
 	} 
   //-------------------------------------
   semop(sem,&Pmutex,1);  // P(mutex)
   tnp[npo]--;
   *Nump=tnp[npo];
   if(tnp[npd]>0){
    printf("Nageur %d va liberer un panier et il ya %d demandes en attente\n",i,tnp[npd]);
    semop(sem,&Vspan,1);  // V(span)
   }
   else{
    semop(sem,&Vmutex,1);  // V(mutex)
   }
   shmdt(tnp);
}
void Demander_cabine (){
    semop(sem,&Pscab,1);  // P(scab)
}
void Liberer_cabine (){
    semop(sem,&Vscab,1);  // V(scab)
}
int main(int argc ,char *argv[]){
    //------------------------------- 
    /*-------------- Programme nageur---------*/
    int Nump;
    //récupération de l'argument
    if (argc<2){
        printf("Pas d'arguments .\n");
        return 20;
    }
    char numNag[20];
    strcpy(numNag,argv[1]);
    /*transformer chaine entier*/
    int i=atoi(numNag);
    // ----------- Variables-------------
    // recuperation des semaphores 
    key_t cle;
    /* Création d'une clé  */
    cle = ftok("main.c" ,1);
    if(cle ==-1) {
	printf("\n erreur : la clé n’a pas été recuperee");
	exit(1);
	} 
    /* recuperation d'un tableau de 3 sémaphores */
    sem = semget(cle, 3, 0666);
    //--------------------------------------------------
    // recupere de la memoire partagee 
    /* recuperer un segment de mémoire partagée. */
	int taille =  2* sizeof(int);
       memp = shmget (cle, taille, IPC_CREAT | 0666);
     if(memp==-1){
	 printf(" \n erreur  dans la recuperation du segment "); 
	 exit(2);
	}
    
	
    Demander_panier(&Nump);
    Demander_cabine();
    sleep(4);
    Liberer_cabine();
    sleep(7);
    printf("Je suis le nageur numero : %d j'occupe le panier : %d \n",i,Nump);
    Demander_cabine();
    sleep(4);
    Liberer_cabine();
    Liberer_panier(i,&Nump);
    printf("Je suis le nageur %d , j'ai libere un panier, il reste %d paniers libres \n",i,np-Nump);
    printf("\n Fin du programme pgme_nageur%d\n",i);
}