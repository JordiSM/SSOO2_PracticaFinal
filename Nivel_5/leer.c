#include "ficheros_basico.h"
#define tambuffer 1500
/*
int main(int argc, char **argv){

    if(argc < 1){
        printf("Error, número de argumentos no válidos\n");
        printf("Formato de uso 1 :\n\t./leer \"nombre_de_memoria\" \"ninodo\"");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    if(bmount(argv[1])==1){
        exit(EXIT_FAILURE);
    }

}*/

int main(int argc, char **argv){
    
    unsigned char buffer_texto[tambuffer];
    memset(buffer_texto, 0, tambuffer);
    int ninodo = atoi(argv[2]); 
    int offset = 0;
    if(argc != 3){
        fprintf(stderr,"Sintaxis: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>.\n");
        return -1;
    }

    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }

    int bytesleidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);

    int bytestotal = 0;

    if(bytesleidos != EXIT_FAILURE){
        while( bytesleidos > 0){

            offset += tambuffer;

            if( write(1, buffer_texto, bytesleidos) == -1 ){
                fprintf(stderr,"Error. No se ha podido realizar la escritura correctamente.\n");
                return -1;
            }

            bytestotal +=  bytesleidos;

            memset(buffer_texto, 0, tambuffer);

            bytesleidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
        }
    }

    struct STAT stat;
    mi_stat_f(ninodo,&stat); 

    fprintf(stderr,"\nTotal_leidos= %d",bytestotal);

    fprintf(stderr,"\nTamEnBytesLog= %d \n",stat.tamEnBytesLog);

    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        return -1;
    }

    return 0;
}