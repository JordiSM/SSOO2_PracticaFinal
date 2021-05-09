#include "directorios.h"

/**
 * Introducida una cadena de caracteres camino por parámetro, separa su contenido en inicial y final
 *  y guarda el tipo, sea directorio o fichero.
 * Devuelve 0 si no ha habido ningún problema o -1 en caso de error.
 */

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    const char *posBarra;

    if (camino[0] != '/'){
        return EXIT_FAILURE;
    }

    camino++;
    posBarra = strchr(camino, '/');

    if (posBarra != NULL) {
        strcpy(final, posBarra);
        strncpy(inicial, camino, posBarra - camino);
        inicial[posBarra - camino] = 0;
        *tipo = 'd';
        
    } else {
        strcpy(inicial, camino);
        strcpy(final, "");
        *tipo = 'f';
    }

    return EXIT_SUCCESS;
}

/*
Buscamos la entradita a tu culito bb
*/

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){
    
    struct superbloque SB;

    if (bread(0, &SB) == 1) return 1;

    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial [sizeof(entrada.nombre)];
    char final [strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    if(strcmp(camino_parcial, "/") == 0){
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return 0;
    }

    if(extraer_camino(camino_parcial,inicial, final, &tipo) == 1) {
        return ERROR_CAMINO_INCORRECTO;
    }

    printf("[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
    
    if(leer_inodo(*p_inodo_dir, &inodo_dir) == 1) {
        return ERROR_PERMISO_LECTURA;
    }

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof (struct entrada); //CALCULAR NUMERO DE ENTRADAS
    
    num_entrada_inodo = 0;
    if(cant_entradas_inodo>0){
        //leemos entrada
        memset(&entrada, 0, sizeof(entrada));
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == EXIT_FAILURE){
            fprintf(stderr, "Error en mi_read_f en buscar entrada \n");
            return EXIT_FAILURE;
        } 

                    printf("llega antes del while\n");

        while((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entrada.nombre) != 0)){
            num_entrada_inodo++;
            printf("%d \n", num_entrada_inodo);
            //leemos entrada
            memset(&entrada, 0, sizeof(entrada));
            if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == EXIT_FAILURE){
                fprintf(stderr, "Error en mi_read_f en buscar entrada \n");
                return EXIT_FAILURE;
            } 
        }
    }
    
    if(strcmp(inicial, entrada.nombre) != 0) {
        switch (reservar) {
            case 0: return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            case 1:
                if(inodo_dir.tipo == 'f'){ return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO; }
                if((inodo_dir.permisos & 2) != 2){    //AND de X1X & 010  == 010
                    return ERROR_PERMISO_ESCRITURA;
                } else {
                    strcpy(entrada.nombre, inicial);
                    if(tipo == 'd'){
                        if(strcmp(final , "/") == 0){
                            entrada.ninodo = reservar_inodo('d', permisos);
                            //printf("[buscar_entrada()→ reservado inodo %d tipo d con permisos %d]\n", entrada.ninodo, permisos);
                        } else {
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    } else {
                        entrada.ninodo = reservar_inodo('f', permisos);
                        //printf("[buscar_entrada()→ reservado inodo %d tipo f con permisos %d]\n", entrada.ninodo, permisos);
                    }

                    if(mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == 1){
                        if(entrada.ninodo != -1){
                            liberar_inodo(entrada.ninodo);
                        }
                        return EXIT_FAILURE;
                    }
                }
        }
    }

    if( (strcmp(final, "/")== 0) || (strcmp(final,"")== 0) ) {
        if((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1)){
            return ERROR_ENTRADA_YA_EXISTENTE;
        }

        printf("[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d]\n", num_entrada_inodo, inodo_dir.tipo, permisos);
        printf("[buscar_entrada()→ Creada entrada: %s, %d]\n", entrada.nombre, num_entrada_inodo);

        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return 0;
    } else {
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    
    return 0;
}

void mostrar_error_buscar_entrada(int error) {
    switch (error) {
    case -1: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
    case -2: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
    case -3: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
    case -4: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
    case -5: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
    case -6: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
    case -7: fprintf(stderr, "Error: No es un directorio.\n"); break;
    }
}

/*

*/
int mi_creat(const char *camino, unsigned char permisos) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    return buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
}

