#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

typedef struct Filozof{
	float dusunme;  //zaman
    float yemeSuresi;  //zaman
    int pirinc;
    int id;
    int masaId;
    int durum; //0: bekleme, 1: yemek, 2: dusunme, 3: bitti
    pthread_t diningThread;
    pthread_mutex_t lock;
} Filozof;

typedef struct Masa{
	Filozof **diningPhil;
    pthread_mutex_t lock_full;
    int tuketilenPirinc;  
    int tazeleme;  //masa taezeleme
    int sandalye;  //oturma duzeni icin
    float fatura;
    int id;
    int dolu;  //dolu masa
    int bos;  // bos sandalyeler
    int dolanYerler;
    int yeniMasa;  //yeni masa acma
    int pirinc;   // pirinc tutarı
    
} Masa;



Filozof *filozoflar;
Filozof *filozof;
Masa *masalar;
int pirincToplam = 0;
float acmaUcreti = 99.90;
float tazelemeUcreti = 19.90;
float pirinc = 20;
int toplam = 0;



Filozof* getFilozof(){
    for(int i =1 ; i <= 80; i++){
        if(pthread_equal(pthread_self(),filozoflar[i].diningThread)) {
            return &filozoflar[i];
        }
    }
    return 0;
}

Masa* getMasa(int masa){
    for(int i = 0; i < 8; i++){
        if(masalar[i].id == masa){ 
        return &masalar[i];
        }
    }
    return 0;
}


void setFilozof(int filozof,int masa){
    filozoflar = (Filozof*) calloc(filozof, sizeof(Filozof));
    Filozof f;
    for(int i = 0; i < filozof; i++){
        f.pirinc = 0;
        f.masaId= 0;
        f.dusunme = ((float)rand()/(float)(RAND_MAX/5)) / 10;
        f.yemeSuresi = ((float)rand()/(float)(RAND_MAX/5)) / 10;
        pthread_t ptid;
        Filozof f;
        f.id = i;
        f.diningThread = ptid;
        filozoflar[i] = f;
        
    }
}

void setMasa(int masa,int filozof,int sandalye){
    masalar = (Masa*) calloc(masa, sizeof(Masa));
    for(int i = 0; i < masa; i++){
        Masa m;
        m.pirinc = 0;
        m.sandalye = 0;
        m.diningPhil = (Filozof**)malloc(filozof * sizeof(Filozof*));
        pthread_mutex_init(&m.lock_full, NULL);
        m.dolu = sandalye;
        m.bos = sandalye;
        m.id = i;
        m.yeniMasa = 0;
        masalar[i] = m;
      
    }
}


void filozofLoop(){
    Filozof *filozof = getFilozof();
    Masa *masa = getMasa(filozof->id);
    while(1){
        Masa *masa = getMasa(filozof->id);
        if(masa->pirinc >= 2000){  //gr
            pthread_mutex_lock(&masa->lock_full);
            bool b = true;
            for(int i = 0; i < 8; i++){
                if(masa->diningPhil[i]->pirinc <= 0) b = false;
            }
           if(b){
                
                for(int i = 0; i < 8; i++){
                    printf("tuketilen pirinc miktari -> %d>\n", masa->diningPhil[i]->pirinc);
                    pirincToplam += masa->diningPhil[i]->pirinc;
                     masa->diningPhil[i]->durum = 3;
                }
             
                masa->yeniMasa = 0;
                masa->fatura = 0;
                masa->pirinc = 0;
                masa->tazeleme = 0;
                masa->bos = 8;
                pthread_mutex_unlock(&masa->lock_full);
                 toplam += masa->fatura;
                printf("masasinin odemesi gereken fiyat -> %f\n", masa->fatura);
            }else{
                masa->tazeleme += 1; 
                masa->pirinc = 2000;  //gr
                masa->fatura += tazelemeUcreti;
                pthread_mutex_unlock(&masa->lock_full);
            }
        }else{
            if(filozof->durum == 0){
                sleep(0.2);
            }else if(filozof->durum == 1){          
                filozof->pirinc += 100;
                masa->pirinc -= 100;
                masa->pirinc += 100;
                filozof->durum = 2;
            }else if(filozof->durum == 2){
                filozof->durum = 3;
                sleep(filozof->dusunme);
            }else if(filozof->durum == 3){
                pthread_exit(NULL);
            }
        }
    }
}

void* filozofThreadsOlustur(void* arg){
    for(int i = 0; i < 8; i++){
        if(masalar[i].bos > 0 && masalar[i].bos < 10){
        	for(int i =1 ; i <= 80; i++){
        		if(pthread_equal(pthread_self(),filozoflar[i].diningThread)) {
          	  		filozof= &filozoflar[i];
       		 	}
            filozof-> id = i;
            filozof->durum = 0;
            masalar[i].diningPhil[masalar[i].sandalye] = filozof;
            masalar[i].sandalye++;
            masalar[i].bos--;
            printf("%d . filozof %d . masaya yerleşti \n",filozof->id , masalar[i].id);
            pthread_mutex_lock(&masalar[i].lock_full);
            
          if(masalar[i].yeniMasa == 0 && masalar[i].bos == 0){  //yeni masa acma
                masalar[i].yeniMasa = 1;
                masalar[i].pirinc = 2000;
                masalar[i].fatura = acmaUcreti;
                for(int j = 0; j < 8; j++){
                    masalar[i].diningPhil[j]->durum = 1; // yemege basladılar
                }
                printf("%d masa acıldı \n",masalar[i].id);
            } 
            pthread_mutex_unlock(&masalar[i].lock_full);
            filozofLoop();
            break;
        }
    }
    }
    pthread_exit(NULL);
}


void masaKapat(Masa *masa){
    pthread_mutex_lock(&masa->lock_full);
    if(masa->yeniMasa == 0 || masa->bos == 0){
        masa->yeniMasa = 1;
        masa->pirinc = 2000;  //gr
        
    }
    pthread_mutex_unlock(&masa->lock_full);
}

void olustur(int masaSayisi,int masadakiFilozoflar,int sandalye,int FilozofSayisi){
	for(int i = 0; i < FilozofSayisi; i++){
        pthread_create(&filozoflar[i].diningThread, NULL, &filozofThreadsOlustur, NULL);
    }
    for(int i = 0; i < FilozofSayisi; i++){
        printf("%d filozof threadi olusuturuldu\n",filozoflar[i].id);
        pthread_join(filozoflar[i].diningThread, NULL);
    }
 	setMasa(masaSayisi,masadakiFilozoflar,sandalye);
    setFilozof(FilozofSayisi,masaSayisi);
}



int main(void){

	int toplamFiyat;
    olustur(8, 8, 8, 80);
    printf("toplam odenecek fiyat -> %d\n",toplamFiyat);
    
    return 0;
}
