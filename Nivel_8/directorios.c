#include "directorios.h"

/**
 * Introducida una cadena de caracteres camino por parámetro, separa su contenido en inicial y final
 *  y guarda el tipo, sea directorio o fichero.
 * Devuelve 0 si no ha habido ningún problema o -1 en caso de error.
 */

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    const char *posBarra;

    if (camino[0] != '/'){
        return -1;
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

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){
    
    struct superbloque SB;

    if (bread(0, &SB) == -1) return -1;

    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial [sizeof(entrada.nombre)];
    char final [strlen(camino_parcial)+1];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    if(strcmp(camino_parcial, "/") == 0){
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return 0;
    }

    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial)+1);
    if(extraer_camino(camino_parcial,inicial, final, &tipo) == -1) {
        return ERROR_CAMINO_INCORRECTO;
    }

    fprintf(stderr,"[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
    
    leer_inodo(*p_inodo_dir, &inodo_dir);
  
    
    if ((inodo_dir.permisos & 4) != 4) { 
        fprintf(stderr, "[buscar_entrada()→ El inodo %d no tiene permisos de lectura]\n", *p_inodo_dir);
        return ERROR_PERMISO_LECTURA;
    }

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof (struct entrada); //CALCULAR NUMERO DE ENTRADAS
    
    num_entrada_inodo = 0;
    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    
    if(cant_entradas_inodo>0){
        
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == EXIT_FAILURE){
            fprintf(stderr, "Error en mi_read_f en buscar entrada \n");
            return -8;
        } 

                    

        while((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entrada.nombre) != 0)){
            num_entrada_inodo++;
            //printf("%d \n", num_entrada_inodo);
            //leemos entrada
            memset(entrada.nombre, 0, sizeof(entrada.nombre));
            if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1){
                fprintf(stderr, "Error en mi_read_f en buscar entrada \n");
                return -8;
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
                            fprintf(stderr,"[buscar_entrada()→ reservado inodo %d tipo d con permisos %d para %s]\n", entrada.ninodo, permisos, entrada.nombre);
                        } else {
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    } else {
                        entrada.ninodo = reservar_inodo('f', permisos);
                        fprintf(stderr,"[buscar_entrada()→ reservado inodo %d tipo f con permisos %d para %s]\n", entrada.ninodo, permisos, entrada.nombre);
                    }

                    if(mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == EXIT_FAILURE){
                        if(entrada.ninodo != -1)
                            liberar_inodo(entrada.ninodo);
                        
                        return -8;
                        
                    } else {
                        fprintf(stderr,"[buscar_entrada()→ Creada entrada: %s, %d]\n", entrada.nombre, entrada.ninodo);
                    }
                
                }
         }
    }

    if( (strcmp(final, "/")== 0) || (strcmp(final,"")== 0) ) {
        if((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1)){
            return ERROR_ENTRADA_YA_EXISTENTE;
        }

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
    case -8: fprintf(stderr, "Error: Error exterior (EXIT_FAILURE)\n");break;
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

int mi_chmod(const char *camino, unsigned char permisos) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error;

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos)) < 0){
        return error;
    }

    return mi_chmod_f(p_inodo, permisos);
}

int mi_stat(const char *camino, struct STAT *p_stat) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error;

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0){
        return error;
    }

    return mi_stat_f(p_inodo, p_stat);
}

/**
 * Pone el contenido de un directorio en un buffer de memoria.
 * Devuelve el número de entradas o un valor menor que 0 en caso de error.
 */
int mi_dir(const char *camino, char *buffer) {

    struct inodo inodo;
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0, num_entradas;
    int error;
    char tamBytes[16], tipo[2];

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0) return error;

    if (leer_inodo(p_inodo, &inodo) == -1) return -1;

    if (inodo.tipo != 'd') return -1;

    if ((inodo.permisos & 4) != 4) return -1; // Comprobamos el permiso de lectura que tiene el inodo.

    //Guardamos la cantidad de entradas
    num_entradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    //Utilizamos un buffer para que no haya que acceder al dispositivo cada vez que haya que leer una entrada
    struct entrada buffer_entradas[num_entradas];

    //Lectura de la entrada
    if (mi_read_f(p_inodo, &buffer_entradas, 0, sizeof(struct entrada) * num_entradas) == -1) return -1;

    strcpy(buffer, "");

    for (int i = 0; i < num_entradas; i++) {

        //Lectura inodo
        if (leer_inodo(buffer_entradas[i].ninodo, &inodo) == -1) return -1;
        
        if (inodo.tipo == 'd') {
            strcat(buffer, "\x1b[32m");
        }else{
            strcat(buffer, "\x1b[33m");
        }

        //Para cada entrada concatenamos su nombre al buffer e incorporamos la información del inodo
        sprintf(tipo, "%c", inodo.tipo);
        strcat(buffer, tipo);
        strcat(buffer, "\t\t");

        if ((inodo.permisos & 4) == 4) {
            strcat(buffer, "r");
        } else {
            strcat(buffer, "-");
        }

        if ((inodo.permisos & 2) == 2) {
            strcat(buffer, "w");
        } else {
            strcat(buffer, "-");
        }

        if ((inodo.permisos & 1) == 1) {
            strcat(buffer, "x");
        } else {
            strcat(buffer, "-");
        }

        strcat(buffer, "\t");

        struct STAT p_stat;
        struct tm *ts;
        char atime[80];
        char mtime[80];
        char ctime[80];
        ts = localtime(&p_stat.atime);
	    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
	    ts = localtime(&p_stat.mtime);
	    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
        strcat(buffer, mtime);
	    ts = localtime(&p_stat.ctime);
	    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
        strcat(buffer, "\t\t");
        //retorna el número de carácteres escritos al array
        sprintf(tamBytes, "%d", inodo.tamEnBytesLog);
        strcat(buffer, tamBytes);
        strcat(buffer, "\t\t\t");
        strcat(buffer, buffer_entradas[i].nombre);
        strcat(buffer, "\033[0m");
        strcat(buffer, "\n");
    }

    //Devolvemos el número de entradas
    return num_entradas;
}
