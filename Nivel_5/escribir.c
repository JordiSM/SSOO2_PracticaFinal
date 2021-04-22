#include "ficheros_basico.h"

int main(int argc, char **argv){

    if(argc < 2){
        fprintf(stderr, "Error, número de argumentos no válidos\n");
        fprintf(stderr, "Formato de uso :\n\t./escribir \"nombre_de_memoria\" \"$(cat fichero)\" \"diferentes_inodos\"");
        exit(EXIT_FAILURE);
    }

    if(bmount(argv[1]) == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }

    int nbytes = strlen(argv[2]);

    int ninodo1, ninodo2, ninodo3, ninodo4, ninodo5;
    int bytes_escritos1, bytes_escritos2, bytes_escritos3, bytes_escritos4, bytes_escritos5;

    int diferentes_inodos = atoi(argv[3]);

    struct STAT stat;
    
    if(diferentes_inodos == 1){ //Diferentes inodos

        ninodo1 = reservar_inodo('f', 6);

        if(ninodo1 == -1){
            bumount();
            exit(EXIT_FAILURE);
        }
        printf("\n\n");
        printf("Nº inodo reservado: %d\n", ninodo1);
        
        traducir_bloque_inodo(ninodo1, 8, 1);
        printf("offset: 9000\n");
        bytes_escritos1 = mi_write_f(ninodo1,argv[2], 9000, nbytes * sizeof(char));
        printf("Bytes escritos: %d\n", bytes_escritos1);

        mi_stat_f(ninodo1, &stat);
        printf("stat.tamEnBytesLog= %d\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados= %d\n", stat.numBloquesOcupados);

        
        //---------------------------------------------------------------
        ninodo2 = reservar_inodo('f', 6);

        if(ninodo2 == -1){
            bumount();
            exit(EXIT_FAILURE);
        }
        printf("\n\n");
        printf("Nº inodo reservado: %d\n", ninodo2);
        
        traducir_bloque_inodo(ninodo2, 204, 1);
        printf("offset: 209000\n");
        bytes_escritos2 = mi_write_f(ninodo2,argv[2], 209000, nbytes * sizeof(char));
        printf("Bytes escritos: %d\n", bytes_escritos2);

        mi_stat_f(ninodo2, &stat);
        printf("stat.tamEnBytesLog= %d\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados= %d\n", stat.numBloquesOcupados);

         //---------------------------------------------------------------
         ninodo3 = reservar_inodo('f', 6);

        if(ninodo3 == -1){
            bumount();
            exit(EXIT_FAILURE);
        }
        printf("\n\n");
        printf("Nº inodo reservado: %d\n", ninodo3);
        
        traducir_bloque_inodo(ninodo3, 30004, 1);
        printf("offset: 30725000\n");
         bytes_escritos3 = mi_write_f(ninodo3,argv[2], 30725000, nbytes * sizeof(char));
        printf("Bytes escritos: %d\n", bytes_escritos3);

        mi_stat_f(ninodo3, &stat);
        printf("stat.tamEnBytesLog= %d\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados= %d\n", stat.numBloquesOcupados);

         //---------------------------------------------------------------
         ninodo4 = reservar_inodo('f', 6);

        if(ninodo4 == -1){
            bumount();
            exit(EXIT_FAILURE);
        }
        printf("\n\n");
        printf("Nº inodo reservado: %d\n", ninodo4);
        
        traducir_bloque_inodo(ninodo4, 400004, 1);
        printf("offset: 409605000\n");
         bytes_escritos4 = mi_write_f(ninodo4,argv[2], 409605000, nbytes * sizeof(char));
        printf("Bytes escritos: %d\n", bytes_escritos4);
        
        mi_stat_f(ninodo4, &stat);        
        printf("stat.tamEnBytesLog= %d\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados= %d\n", stat.numBloquesOcupados);

        //---------------------------------------------------------------
         ninodo5 = reservar_inodo('f', 6);


        if(ninodo5 == -1){
            bumount();
            exit(EXIT_FAILURE);
        }
        printf("\n\n");
        printf("Nº inodo reservado: %d\n", ninodo5);
        
        traducir_bloque_inodo(ninodo5, 468750, 1);
        printf("offset: 480000000\n");
         bytes_escritos5 = mi_write_f(ninodo5,argv[2], 480000000, nbytes * sizeof(char));
        printf("Bytes escritos: %d\n", bytes_escritos5);
        
        mi_stat_f(ninodo5, &stat);
        printf("stat.tamEnBytesLog= %d\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados= %d\n", stat.numBloquesOcupados);
        
        bumount();
    } else {

        ninodo1 = reservar_inodo('f', 6);

        if(ninodo1 == -1){
            bumount();
            exit(EXIT_FAILURE);
        }
        printf("Nº inodo reservado: %d\n", ninodo1);
        
        printf("\n\n");
        traducir_bloque_inodo(ninodo1, 8, 1);
        printf("offset: 9000\n");
        bytes_escritos1 = mi_write_f(ninodo1,argv[2], 9000, nbytes * sizeof(char));
        printf("Bytes escritos: %d\n", bytes_escritos1);

        mi_stat_f(ninodo1, &stat);
        printf("stat.tamEnBytesLog= %d\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados= %d\n", stat.numBloquesOcupados);

        printf("\n\n");
        traducir_bloque_inodo(ninodo1, 204, 1);
        printf("offset: 209000\n");
        bytes_escritos2 = mi_write_f(ninodo1,argv[2], 209000, nbytes * sizeof(char));
        printf("Bytes escritos: %d\n", bytes_escritos2);

        mi_stat_f(ninodo1, &stat);
        printf("stat.tamEnBytesLog= %d\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados= %d\n", stat.numBloquesOcupados);

        printf("\n\n");
        traducir_bloque_inodo(ninodo1, 30004, 1);
        printf("offset: 30725000\n");
         bytes_escritos3 = mi_write_f(ninodo1,argv[2], 30725000, nbytes * sizeof(char));
        printf("Bytes escritos: %d\n", bytes_escritos3);

        mi_stat_f(ninodo1, &stat);
        printf("stat.tamEnBytesLog= %d\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados= %d\n", stat.numBloquesOcupados);

        printf("\n\n");
        traducir_bloque_inodo(ninodo1, 400004, 1);
        printf("offset: 409605000\n");
         bytes_escritos4 = mi_write_f(ninodo1,argv[2], 409605000, nbytes * sizeof(char));
        printf("Bytes escritos: %d\n", bytes_escritos4);

        mi_stat_f(ninodo1, &stat);
        printf("stat.tamEnBytesLog= %d\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados=%d\n", stat.numBloquesOcupados);

        printf("\n\n");
        traducir_bloque_inodo(ninodo1, 468750, 1);
        printf("offset: 480000000\n");
         bytes_escritos5 = mi_write_f(ninodo1,argv[2], 480000000, nbytes * sizeof(char));
        printf("Bytes escritos: %d\n", bytes_escritos5);

        mi_stat_f(ninodo1, &stat);
        printf("stat.tamEnBytesLog= %d\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados= %d\n", stat.numBloquesOcupados);

        bumount();
    }

}