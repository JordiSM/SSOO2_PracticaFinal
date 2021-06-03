/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

#include "bloques.h"
#include "semaforo_mutex_posix.h"

//  VARIABLES
static int descriptor = 0;
static sem_t *mutex;
static unsigned int inside_sc = 0;

/*
    Esta función monta el dispositivo virtual. Recibe como parámetro el PATH. Devuelve un entero
    con el valor del descriptor si ha ido bien, o -1 en caso contrario
*/
int bmount(const char *camino){
    umask(000);
    descriptor = open(camino, O_RDWR|O_CREAT, 0666 );

    if(descriptor < 0){ //error en la apertura del descriptor
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }
    
    if (!mutex) { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
       mutex = initSem(); 
       if (mutex == SEM_FAILED) {
           return -1;
       }
   }


    return descriptor;
}

/*
    Función encargada de desmontar el dispositivo virtual. Devuelve el valor -1 en caso de error,
    por el contrario devuelve 0.
*/
int bumount(){
    int estado = close(descriptor);
    
    deleteSem(); 

    if (estado < 0){        //Error en el cierre del fichero
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    return estado; // EXIT_SUCCESS
}


/*
    Función encargada de escribir un bloque en el dispositivo virtual. el parámetro nbloque, es el lugar a escribir,
    y buf es un puntero dirigido a un buffer de memorio que se volcará en el dispositivo virtual. Si la función
    ha sido exitosa devolverá el número de bytes escritos, en caso contrario devolverá -1.
*/
int bwrite(unsigned int nbloque, const void *buf){

    off_t desplazamiento = nbloque * BLOCKSIZE;

    if(lseek(descriptor, desplazamiento, SEEK_SET) < 0){   //Error en el desplazamiento
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }
    
    if((write(descriptor, buf, BLOCKSIZE)) != BLOCKSIZE){ //Error escribiendo el bloque 
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }
        
    return BLOCKSIZE;
}


/*
    Función encargada de leer en el bloque del dispositivo virtual. Recibe por parámetro el bloque
    que va a leer y el buffer de memoria en el que se volcará. Devuelve un entero, -1 en caso de error
    o BLOCKSIZE si ha ido bien
*/
int bread(unsigned int nbloque, void *buf){

    off_t desplazamiento = nbloque * BLOCKSIZE;

    if(lseek(descriptor, desplazamiento, SEEK_SET) < 0){   //Error en el desplazamiento
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }

    if((read(descriptor, buf, BLOCKSIZE)) != BLOCKSIZE){    //Error leyendo el bloque
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }
    
    return BLOCKSIZE;
}


void mi_waitSem() {
   if (!inside_sc) { // inside_sc==0
       waitSem(mutex);
   }
   
   inside_sc++;
}



void mi_signalSem() {
   inside_sc--;
   if (!inside_sc) {
       signalSem(mutex);
   }
}


