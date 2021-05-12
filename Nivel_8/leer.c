/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

#include "ficheros_basico.h"
#define tambuffer 1500  //Tamaño buffer

int main(int argc, char **argv){
    
    //Declaración buffer texto
    unsigned char buffer_texto[tambuffer];
    //Ponemos a 0 el buffer
    memset(buffer_texto, 0, tambuffer);
    //Obtenemos el inodo a leer
    int ninodo = atoi(argv[2]); 
    int offset = 0;

    //Control de error entrada incorrecta
    if(argc < 3){
        fprintf(stderr,"Uso: escribir \"nombre_dispositivo\" \"$(cat fichero)\" \"diferentes_inodos\".\n");
        exit(-1);
    }

    //Montamos el disco
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en bmount\n");
        exit(-1);
    }

    //Calculamos los bytes leidos
    int bytesleidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);

    //Inicializamos los bytes totales
    int bytestotal = 0;

    //Si no ha habido ningún error al calcular los bytes leidos
    if(bytesleidos != -1){

        //Mientras haya bytes por leer
        while( bytesleidos > 0){

            offset += tambuffer;

            if( write(1, buffer_texto, bytesleidos) == -1 ){
                fprintf(stderr,"Error. No se ha podido realizar la escritura correctamente.\n");
                return -1;
            }

            //Aumentamos bytes totales con los leidos actualmente
            bytestotal +=  bytesleidos;

            //Volvemos a poner a 0 el array
            memset(buffer_texto, 0, tambuffer);

            //Calculamos de nuevo los bytes leidos
            bytesleidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
        }
    }

    //Definición estructura stat para leer los datos
    struct STAT stat;
    mi_stat_f(ninodo,&stat); 

    fprintf(stderr,"\nTotal_leidos= %d",bytestotal);

    fprintf(stderr,"\nTamEnBytesLog= %d \n",stat.tamEnBytesLog);

    //Desmontamos el disco
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        return -1;
    }

    return 0;
}