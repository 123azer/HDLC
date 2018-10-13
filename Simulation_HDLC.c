                                             // BINOME: ELKANTOURI Badr-Eddine et EL AFOUI Moaad  
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
int p[2];
int pAcquit[2];
char buf[200];
char erreur[]={'0','\0'};


/*on va utiliser les semaphores pour simulé le controle de flux dans les deux direction, lorsque l'emmetteur envoie un message il va attendre que le recepteur termine le traitement pour envoyer un autre message */
/***********************************************************************************************************************************************************/
void P(int id,int sem) // // Pour faire l'opération P  
{
struct sembuf operation; // Une seule opération
operation.sem_num = sem; 
operation.sem_op = -1;
operation.sem_flg = 0;
semop (id, &operation, 1);
}

void V(int id,int sem) // Pour faire l'opération V
{
struct sembuf operation; 
operation.sem_num = sem;
operation.sem_op = 1;
operation.sem_flg = 0;
semop (id, &operation, 1);
}
/******************************************************************************************************************************************************************/
 


/*la fonction suivante ajoute un 0 a la table tab(parametre de la fonction) dans la position position(parametre de la fonction)
cette fonction decale les elements dont l'indice est superieur ou egale a position et il ajoute le bit 0 dans position,il sera utilisé pour ajouter des bite de transparance */
/******************************************************************************************************************************************************************/

void ajouterbitseparation(char tab[],int position) 
{
int j=0;
while(tab[j]!='\0')
     j++;
     while(j>=position)
        {
          tab[j+1]=tab[j];
          j--;
        }
tab[position]='0';
}

/******************************************************************************************************************************************************************/




/*la fonction suivante ajoute les fanions a une trame ici tab(parametre de la fonction) pour l envoyer*/
/******************************************************************************************************************************************************************/
void ajouterFanion(char tab[])  
{
int j,i;
j=0;
while(tab[j]!='\0')
      j++;
tab[j]='0';
for(i=j+1;i<j+7;i++)
      tab[i]='1';
tab[i]='0';
tab[i+1]='\0';
i++;
while(i>=0)
{
      tab[i+8]=tab[i];
      i--;
}
for(j=1;j<7;j++)
      tab[j]='1';
tab[0]='0';
tab[7]='0';
}

/******************************************************************************************************************************************************************/


/*la fonction sivante enleve les fanions a la reception de la table tab*/
/******************************************************************************************************************************************************************/
void enleverFanion(char tab[]) // 
{
int j=0;
while(tab[j+8]!='\0')
     {
     tab[j]=tab[j+8];
     j++;
     }
tab[j-8]='\0';
}

/******************************************************************************************************************************************************************/



/*la fonction  suivante enleve le bit dans la position position(parametre de la fonction) de la table tab(parametre) avec un decalage des elements superieur a position ce qui va ecrasé la valeur dans la position position(parametre) cette fonction sera utilisé pour enlevé le bit de transparance a la reception */
/******************************************************************************************************************************************************************/
void enleverbitseparation(char tab[],int position) //  
{
while(tab[position]!='\0')
{
     tab[position]=tab[position+1];
     position++;
}
}
/******************************************************************************************************************************************************************/

/*la fonction main*/
int main()
{


/* creation et intialisation de semaphore */
/******************************************************************************************************************************************************************/
int semid; // creation de semaphore
semid = semget(0, 2, IPC_CREAT|IPC_EXCL|0666);
semctl (semid, 1, SETVAL, 1);// initialiser le sémaphore numero 1 à 1 pour que l emeteur commence le premier 
semctl (semid, 0, SETVAL, -1);// initialiser le sémaphore numero 0 à -1 pour bloqué le recepteur avant la transmission de la trame
/******************************************************************************************************************************************************************/

/*les variables i,j et r seront utilisé pour parcourir les tableaux pour effectué les different traitements*/
int i=0;
int j,r;
/************************************************************************************************************/



/*les tableaux suivant  Corespondent a la meme trame on garde l'info dans la variable DATA au cas ou il faut faire une retransmission et on fait le traitement sur FCS et DATA_traite*/
/************************************************************************************************************/
char DATA[200];  // l info qu'on veut transmettre (compris l addresse)
char FCS[200];  // registre utilisé pour calculé le FCS
char DATA_traite[200];// la trame a envoyé
/************************************************************************************************************/ 

int count=0; // compteur utilisé pour compter le nombre des 1 afin d'ajouter ou supprimé les bits de transparance  
char car;// variable utiliser pour avancer dans le fichier qui contient les trames
int num_trame=0;// variable utilisé pour numeroté les trames
FILE* fichier;// fichier utilisé pour stocké les trames pour faire la simulation independament de l'utilisateur qui va les entres une seul fois
pipe(p);// pipe utilisé pour transmettre les datas de l'emmeteur vers le recepteur
pipe(pAcquit);// pipe utilisé pour acquitté les trames (de recepteur vers l'emmetteur)
fichier=fopen("file.txt","w");// on va ecrire dans le fichier
printf("entrer le nombre des trames a envoye\n");// demande a l'utilisateur de saisir le nombre des trames
scanf("%d",&i);// i=nombre des trames
fputc(' ',fichier);// un espace utilisé entre chaque deux trames pour facilité la lecture 
printf("commencer a entrer les trames:\n");

/*on va lire et stocké dans dans le fichier*/
/************************************************************************************************************/
while(i!=1)
{
    printf("=> ");
    scanf("%s",DATA);
    fprintf(fichier,"%s ",DATA);// on stock les trames dans le fichier
    i--;
}
printf("=> ");
scanf("%s",DATA);
fprintf(fichier,"%s",DATA);
fclose(fichier);

/**********************************************************************************************************/

/*le code de l'emmetteur*/
/**********************************************************************************************************/
if(fork())
{
   close(p[0]);
   fichier=fopen("file.txt","r");// on va lire les trames dans le fichier
   car=fgetc(fichier);// on va enlever l'espace crée entre chaque deux trames


/*tant que il a encore das trames a envoyé ou il a un acquittement d'erreur on va faire la transmission (le code dans le while)*/
/*********************************************************************************************************/
   while(car!= EOF || erreur[0]=='1')  
        {
        P(semid,1); // controle de flux (un seul message peut etre envoyé a la fois 'send and wait')
        if(num_trame!=0)// pour la premiere trame on a pas d'acquittement donc on va pas lire le Buffer  
            read(pAcquit[0],erreur,2); // recevoir l'aquittement

   /*si il n'y a pas un erreur dans la derniere transsmission on va lire la trame suivante*/
        if(erreur[0]!='1')
            {
            fscanf(fichier,"%s",DATA);
            car=fgetc(fichier);
            }
  /*sinon on va retransmettre la meme trame*/
        else
          num_trame--; // sinon on reste dans la meme trame


 /* on va garder le contenu de la trame DATA et faire le reste des operations sur FCS et DATA_traite*/
/************************************************************************************************************/
        i=0;
        while(DATA[i]!='\0') // on va copier le contenu de DATA dans FCS et DATA_traite
            {
             FCS[i]=DATA[i];  
             DATA_traite[i]=DATA[i];  // on garde notre donne DATA au cas ou il a un erreur de transmission
             i++;
             }
/***********************************************************************************************************/


/* on va faire place pour ajouter le reste dans DATA_traite[] et pour faire la division polynomial dans FCS[]*/
/***********************************************************************************************************/
        for(j=0;j<16;j++) 
             {
             FCS[i+j]='0';
             DATA_traite[i+j]='0';
             }
        DATA_traite[i+j]='\0';
        FCS[i+j]='\0';
        printf("la trame %d apres decalage = %s\n",num_trame,DATA_traite);
/***********************************************************************************************************/


/* on va effectuer la division sur le contenu de FCS , le principe de l'operation consiste a parcourir FCS jusqu'a trouvé un bit=1 et verifier si le nombre de bit qui reste et plus que 16 bit sinon le contenu de FCS est le reste de la division si c'est le cas on va faire une operation XOR donc pour le bit Corespond 1 dans le polynome generateur on va le remplacé par son inverce (si 1 alors 0 si 0 alors 1) pour les restes on va pas les toucher puisque ils Corespondent a 0 dans le polynome generateur, et a chaque fois on va positionné le compteur i a 0 et faire la meme operation jusqu'a trouvé le reste (qui verifier l'inverce de la condition while)    */
/*************************************************************************************************************/
        i=0;
        while(FCS[i+16]!='\0')
             {
              if(FCS[i]=='0') 
                 {i++; continue;}
              else
                 {
                  FCS[i]='0';        
                  i++;                                            
                  i++;
                  i++;
                  i++;
                  if(FCS[i]=='1')
                     FCS[i]='0';
                  else                                                             //10001000000100001 le polynome generateur
                     FCS[i]='1';
                  i++;
                  i++;
                  i++;
                  i++;
                  i++;
                  i++;
                  i++;
                  if(FCS[i]=='1')
                     FCS[i]='0';
                  else
                     FCS[i]='1';
                  i++;
                  i++;
                  i++;
                  i++;
                  if(FCS[i]=='1')
                     FCS[i]='0';
                  else
                     FCS[i]='1';
                  i=0;
             }  
         }
         printf("le FCS de la trame %d = %s\n",num_trame,FCS);

/***************************************************************************************************************/


/*on ajoute le reste de la division a la trame DATA_traite*/
/**************************************************************************************************************/
         j=0;
         i=0;
         while(FCS[j]!='\0')
            j++;
         while(DATA_traite[i]!='\0')
            i++;
         for(r=0;r<=16;r++)
            {
            DATA_traite[i-r]=FCS[j-r];    
            }
/*************************************************************************************************************/ 


/*ici on ajoute les bits de transparance*/
/*************************************************************************************************************/        
         i=0;
         while(DATA_traite[i]!='\0')
            {
            if(DATA_traite[i]=='1')
               count++;
            else
               count=0;
            if(count==5) // si 5 bits de 1 on ajoute un bit de transparance
               ajouterbitseparation(DATA_traite,i+1);
            i++;
            }
         printf("la trame %d apres ajout de fcs et les bits de separation = %s\n",num_trame,DATA_traite);
/************************************************************************************************************/

         // on ajoute les fanions
         ajouterFanion(DATA_traite); 
         printf("la trame %d apres ajout des fanions = %s",num_trame,DATA_traite);

/*...........................................................*/
         printf("\n");
         printf("le pere envoi l'info...\n");
         sleep(2);//pour simulé l'envoi
         printf("\n");
/*..........................................................*/
         write(p[1],DATA_traite,200);// la trame est envoyé
         num_trame++;// on passe a la trame suivante
         close(pAcquit[1]);
         V(semid,0);// on donne la main au recepteur
         }
    fclose(fichier);
/*********************************************************************************************************/

}
/*********************************************************************************************************/


/*le code de recepteur*/
/*********************************************************************************************************/
else
{
  while(1)
      {
      P(semid,0);// controle de flux
      close(pAcquit[0]);
      read(p[0],buf,200); // lire la trame recu  
      printf("\n");
      printf("la trame %d est recu = %s\n",num_trame,buf);// affichage de la trame reçu
      enleverFanion(buf); // on va enlevé les fanions
      printf("le recepteur enleve les fanion de de la trame %d = %s\n",num_trame,buf);


/*ici on va enlevé les bits de transparance*/
/********************************************************************************************************/
      i=0;
      count=0;
      while(buf[i]!='\0')
                 {
                 if(buf[i]=='1')
                 count++;
                 else
                    count=0;
                 if(count==5)
                    {
                     enleverbitseparation(buf,i+1);// on va enlevé les bits de separation
                     count=0;
                    }
                 i++;
                 }
      printf("le recepteur enleve les bits de separation de la trame %d = %s\n",num_trame,buf);
/*******************************************************************************************************/

//buf[1]='0';/*creation d'un erreur si l'erreur est crée le message sera transmis et retransmis infiniment puisque l'erreur sera toujours present*/

// on va effectue la division

/***************************************************************************************************************************************************/
      i=0;
      while(buf[i+16]!='\0')
                  {
                  if(buf[i]=='0')  
                        {i++; continue;}
                  else
                        {
                         buf[i]='0';        
                         i++;                                            //16,12,5,0 sont les puissance de la polynome generateur
                         i++;
                         i++;
                         i++;
                         if(buf[i]=='1')
                             buf[i]='0';
                         else
                             buf[i]='1';
                         i++;
                         i++;
                         i++;
                         i++;
                         i++;
                         i++; 
                         i++;
                         if(buf[i]=='1')
                           buf[i]='0';
                         else
                           buf[i]='1';
                         i++;
                         i++;
                         i++;
                         i++;
                         if(buf[i]=='1')
                           buf[i]='0';
                         else
                           buf[i]='1';
                         i=0;
                         }  
                   }
         printf("resultat de la division sur la trame %d = %s\n",num_trame,buf);
         i=0;
/****************************************************************************************************************************************************/


// tester si le resultat est 0 si c'est le cas alors on va acquitté avec 1 (demande de retransmission) sinon on va acquitté avec 0 et l'emmetteur va passé a la trame suivante si elle existe 
/****************************************************************************************************************************************************/
         while(buf[i]!='\0')
              {
               if(buf[i]!='0')
                 erreur[0]='1';
               i++;
              }
         if(erreur[0]=='1')
              {
               printf("erreur dans la trame numero %d\n",num_trame);
               write(pAcquit[1],erreur,2); //si erreur donc demande de retransmission
               num_trame--;
               printf("demande de retransmission...\n");
               sleep(4);
              }
        else
             {
             printf("la trame numero %d est correct\n",num_trame);
             erreur[0]='0';
             write(pAcquit[1],erreur,2); // on acquitte  la trame recu
             }

/***************************************************************************************************************************************************/

/*.......................................*/
        printf("\n");
        close(p[1]);
        num_trame++;
        V(semid,1);
/*......................................*/
        }
return 0;
}
}
