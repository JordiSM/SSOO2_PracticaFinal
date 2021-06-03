/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

#include "directorios.h"

static struct UltimaEntrada UltimaEntradaEscritura;
static struct UltimaEntrada UltimaEntradaLectura;

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

    //fprintf(stderr,"[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
    
    leer_inodo(*p_inodo_dir, &inodo_dir);
  
    
    if ((inodo_dir.permisos & 4) != 4) { 
        //fprintf(stderr, "[buscar_entrada()→ El inodo %d no tiene permisos de lectura]\n", *p_inodo_dir);
        return ERROR_PERMISO_LECTURA;
    }

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof (struct entrada); //CALCULAR NUMERO DE ENTRADAS
    
    num_entrada_inodo = 0;
    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    
    if(cant_entradas_inodo>0){
        
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1){
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
                            //fprintf(stderr,"[buscar_entrada()→ reservado inodo %d tipo d con permisos %d para %s]\n", entrada.ninodo, permisos, entrada.nombre);
                        } else {
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    } else {
                        entrada.ninodo = reservar_inodo('f', permisos);
                        //fprintf(stderr,"[buscar_entrada()→ reservado inodo %d tipo f con permisos %d para %s]\n", entrada.ninodo, permisos, entrada.nombre);
                    }

                    if(mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1){
                        if(entrada.ninodo != -1)
                            liberar_inodo(entrada.ninodo);
                        
                        return -8;
                        
                    } else {
                        //fprintf(stderr,"[buscar_entrada()→ Creada entrada: %s, %d]\n", entrada.nombre, entrada.ninodo);
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
    int res_buscar;
    
	mi_waitSem();
    res_buscar = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
	mi_signalSem();
    
    return res_buscar;
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
    //fprintf(stderr, "nº de inodo: %d\n",p_inodo);
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

        //struct STAT p_stat;
        struct tm *ts;
        char mtime[80];
	    ts = localtime(&inodo.mtime);
	    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
        //sprintf(mtime, "%d %d:%d:%d %d:%d:%d", ts->tm_wday, ts->tm_year, ts->tm_mon, ts->tm_mday, ts->tm_hour, ts->tm_min, ts->tm_sec);
        strcat(buffer, mtime);

        //retorna el número de carácteres escritos al array
        sprintf(tamBytes, "\t\t%d", inodo.tamEnBytesLog);
        strcat(buffer, tamBytes);
        strcat(buffer, "\t\t");
        strcat(buffer, buffer_entradas[i].nombre);
        strcat(buffer, "\033[0m");
        strcat(buffer, "\n");
    }

    //Devolvemos el número de entradas
    return num_entradas;
}


//MOVER ESTO AL NIVEL 9
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes) {
    int bytesLeidos, error;
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    
    if(strcmp (camino, UltimaEntradaLectura.camino) == 0){
        p_inodo = UltimaEntradaLectura.p_inodo;
        //printf("\n\x1b[31m[mi_read() → Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\x1b[0m\n");

    } else{
        if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) <0 ){
            return error;
        }
        
        UltimaEntradaLectura.p_inodo = p_inodo;
        strcpy(UltimaEntradaLectura.camino, camino);
        //printf("\n\x1b[31m[mi_read() → Actualizamos la caché de lectura]\x1b[0m\n");
    }

    if ((bytesLeidos = mi_read_f(p_inodo, buf, offset, nbytes)) == -1) return -1;
    return bytesLeidos;
}

/**
 * Escribe el contenido del buffer en un fichero optimizado utilizando una caché de directorios
 * Devuelve los bytes escritos si todo ha ido correctamente o un valor menor que 0 si ha habido un error.
 */
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
    unsigned int p_inodo_dir= 0, p_inodo = 0, p_entrada = 0;
    int bytesEscritos, error;

    //Comprobamos si el camino con la ultima escritura coincide
    if(strcmp (camino, UltimaEntradaEscritura.camino) == 0){
        p_inodo = UltimaEntradaEscritura.p_inodo;
        //printf("\n\x1b[31m[mi_write() → Utilizamos la caché de escritura en vez de llamar a buscar_entrada()]\x1b[0m\n");
    }else{

        //Obtenemos la entrada camino
        if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6)) < 0) {
            return error;
        }

        //Actualizamos el p_inodo de la ultima entrada de escritura
        UltimaEntradaEscritura.p_inodo = p_inodo;
        strcpy(UltimaEntradaEscritura.camino, camino);
        //printf("\n\x1b[31m[mi_write() → Actualizamos la caché de escritura]\x1b[0m\n");
    }


    if ((bytesEscritos = mi_write_f(p_inodo, buf, offset, nbytes)) == -1){ 
        return -8;
    }

    return bytesEscritos;
  }

/*
    Crea el enlace de una entrada de directorio camino2 al 
    inodo especificado por otra entrada de directorio camino1 
*/
int mi_link(const char *camino1, const char *camino2) {
	mi_waitSem();
    	unsigned int p_inodo_dir1= 0, p_inodo_dir2 = 0, p_inodo1 = 0, p_inodo2 = 0, p_entrada1 = 0, p_entrada2 = 0;
    	int error;

    	struct inodo inodo;

        //Comprobación de que el camino1 existe obteniendo inodo
    	if ((error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 4)) < 0) {
		    fprintf(stderr, "Error en buscar_entrada para camino1\n");
		    mi_signalSem();
		    return error;
	    }

        //Leemos inodo
	    if (leer_inodo(p_inodo1, &inodo) < 0){
		    fprintf(stderr, "Error en leer_inodo\n");
		    mi_signalSem();
		    return -8;
	    }

        //Comprobamos que sea tipo fichero
	    if(inodo.tipo != 'f'){
		    fprintf(stderr, "Error, camino1 no es un fichero\n");
		    mi_signalSem();
		    return -8;
	    }

        //Comprueba que tenga permisos de lectura
	    if((inodo.permisos & 4) == 0){
		    fprintf(stderr, "Error, inodo asociado al fichero1 no tiene permisos de lectura\n");
		    mi_signalSem();
		    return -8;
	    }

        //Comprobación de que el camino2 existe obteniendo inodo
	    if ((error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6)) < 0) {
		    fprintf(stderr, "Error en buscar_entrada para camino2\n");
		    mi_signalSem();
		    return error;
	    }

	    struct entrada entrada2;

	    if (mi_read_f(p_inodo_dir2, &entrada2, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) == EXIT_FAILURE){
		    fprintf(stderr, "Error en mi_read_f\n");
		    mi_signalSem();
		    return -8;
	    }

	    entrada2.ninodo = p_inodo1;
	    
        //Escribimos la entrada modificada en p_inodo_dir2
	    if (mi_write_f(p_inodo_dir2, &entrada2, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) < 0){
		    fprintf(stderr, "error en mi_write_f\n");
		    mi_signalSem();
		    return -8;
	    } 

        //Liberamos inodo asociado a la entrada
	    if (liberar_inodo(p_inodo2) == EXIT_FAILURE){
		    fprintf(stderr, "Error en liberar inodo\n");
		    mi_signalSem();
		    return -8;
	    } 
	    
        //Incrementamos cantidad de enlaces
	    inodo.nlinks++;
        //Actualizamos ctime
	    inodo.ctime = time(NULL);
	    
        //Salvamos inodo
	    int resultado;
	    resultado = escribir_inodo(p_inodo1, inodo);
	    
	    mi_signalSem();
	    
	    return resultado;
}



int mi_unlink(const char *camino) {
    mi_waitSem();
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    struct inodo inodo;
    struct entrada ultima_entrada;
    int error, num_entradas;

    //Comprobamos que la entrada camino existe y obtenemos su número de entrada
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0){
        fprintf(stderr, "error en buscar entrada\n");
        mi_signalSem();
        return error;
    }
    

    if (leer_inodo(p_inodo, &inodo) == -1){
        fprintf(stderr, "error en leer_inodo\n");
        mi_signalSem();
        return -8;
    }
    
    //Si se trata de un directorio y no está vacío salimos de la función, porque no se puede borrar
    if (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0){
        fprintf(stderr, "Error: El directorio %s no está vacio. \n", camino);
        mi_signalSem();
        return -8;
    }
    
    //Lectura del inodo asociado al directorio 
    if (leer_inodo(p_inodo_dir, &inodo) == -1){
        fprintf(stderr, "Error en leer_inodo\n");
        mi_signalSem();
        return -8;
    }
    
    num_entradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    
    //Si no es la última entrada
    if (p_entrada != num_entradas - 1) {
        //Lectura de la última entrada
        if (mi_read_f(p_inodo_dir, &ultima_entrada, (num_entradas - 1) * sizeof(struct entrada), sizeof(struct entrada)) == -1){
            fprintf(stderr, "Error en mi_read\n");
            mi_signalSem();
            return -8;
        }
        //Escribimos en la posición de entrada que queremos eliminar
        if (mi_write_f(p_inodo_dir, &ultima_entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == -1){
            fprintf(stderr, "Error en mi_write\n");
            mi_signalSem();
            return -8;
        }
    }
    
    //Truncamos el inodo
    if (mi_truncar_f(p_inodo_dir, inodo.tamEnBytesLog - sizeof(struct entrada)) == -1){
        fprintf(stderr, "Error en mi_truncar\n");
        mi_signalSem();
        return -8;
    }

    //Leemos el inodo asociado a la entrada eliminada
    if (leer_inodo(p_inodo, &inodo) == -1){
        fprintf(stderr, "Error en leer_inodo\n");
        mi_signalSem();
        return -8;
    }

    //Decrementamos el número de enlaces
    inodo.nlinks--;
    
    int aux;

    //Si no quedan enlaces liberamos el inodo
    if (inodo.nlinks == 0) {
       if((aux = liberar_inodo(p_inodo)) == -1){
           mi_signalSem();
           return -8;
       } else {
           mi_signalSem();
           return aux;
       }
    }
    
    //Actualizamos ctime
    inodo.ctime = time(NULL);
    
    //Salvamos inodo
    if((aux = escribir_inodo(p_inodo, inodo)) == -1){
        mi_signalSem();
        return -8;
    } else {
        mi_signalSem();
        return aux;
    }
}
