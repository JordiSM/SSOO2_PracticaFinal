/*
AUTORES: 
    Diaz Jimenez, Iker
    Moreno Martínez, Jogil
    Sevilla Marí, Jordi
*/

#include "simulacion.h"

int acabados = 0;

// Función enterrador.
void reaper() {
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0) {
        acabados++;
    }
}

/*
    Función encargada de crear unos procesos de prueba que acceden concurrentemente al sistema de ficheros para su posterior verificación.
    Devuelve EXIT_SUCCESS (0) si todo ha ido bien, o EXIT_FAILURE (1) en caso de error.
*/
int main(int argc, char **argv) {
    char *nombreDisp, nombreSimul[MAXTAMRUTA], nombreProceso[MAXTAMRUTA], nombreFichero[MAXTAMRUTA], fechaStr[15];
    int proceso, operacion, error;
    time_t fecha;
    pid_t pid;
    struct REGISTRO registro;

    //En caso de error en la sintaxis
    if (argc != 2) {
        fprintf(stderr, "Sintaxis: ./simulacion <disco>\n");
        return EXIT_FAILURE;
    }

    //Guardamos el nombre del dispositivo
    nombreDisp = argv[1];

    //Montamos dispositivo
    if (bmount(nombreDisp) == -1) {
        fprintf(stderr, "Error en bmount\n");
        return EXIT_FAILURE;
    }

    //Creamos el directorio de simulación
    fecha = time(NULL);
    strftime(fechaStr, sizeof(fechaStr), "%Y%m%d%H%M%S", localtime(&fecha));
    sprintf(nombreSimul, "/simul_%s/", fechaStr);
    if ((error = mi_creat(nombreSimul, 7)) < 0) {
        fprintf(stderr, "Error al crear directorio %s:\n", nombreSimul);
        mostrar_error_buscar_entrada(error);
        bumount();
        return EXIT_FAILURE;
    }

    signal(SIGCHLD, reaper);
    printf("*** SIMULACIÓN DE %d PROCESOS REALIZANDO CADA UNO %d ESCRITURAS *** \n\n", NUMPROCESOS, NUMESCRITURAS);

    for (proceso = 1; proceso <= NUMPROCESOS; proceso++) {
        //Si es el hijo
        if (fork() == 0) { 
            pid = getpid();
            //Montamos el dispositivo
            bmount(nombreDisp);
            //Creamos el directorio
            sprintf(nombreProceso, "%sproceso_%d/", nombreSimul, pid);
            if ((error = mi_creat(nombreProceso, 7)) < 0) {
                fprintf(stderr, "Error creando %s:\n", nombreProceso);
                mostrar_error_buscar_entrada(error);
                bumount();
                exit(EXIT_FAILURE);
            }
            //Creamos el fichero
            sprintf(nombreFichero, "%sprueba.dat", nombreProceso);
            if ((error = mi_creat(nombreFichero, 7)) < 0) {
                fprintf(stderr, "Error creando %s:\n", nombreFichero);
                mostrar_error_buscar_entrada(error);
                bumount();
                exit(EXIT_FAILURE);
            }
            //Inicializar la semilla de números aleatorios
            srand(time(NULL) + getpid());
            for (operacion = 0; operacion < NUMESCRITURAS; operacion++) {
                //Inicializar registro
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = operacion + 1;
                registro.nRegistro =  rand() % REGMAX;
                //Escribimos el registro en el struct
                if ((error = mi_write(nombreFichero, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO))) < 0) {
                    fprintf(stderr, "Error en mi_write al escribir en %s:\n", nombreFichero);
                    mostrar_error_buscar_entrada(error);
                    bumount();
                    exit(EXIT_FAILURE);
                }
                //Esperamos para realizar la siguiente escritura
                usleep(50000);
            }
            //Desmontamos dispositivo
            bumount();
            printf("Proceso %d: Completadas %d escrituras en %s\n", proceso, NUMESCRITURAS, nombreFichero);
            //printf("\033[1mProceso %d: Completadas %d escrituras en %s\033[0m\n", proceso, NUMESCRITURAS, nombreFichero);
            exit(EXIT_SUCCESS);
        }
        //Esperamos para lanzar el siguiente proceso
        usleep(200000);
    }
    //Permitir que el padre espere por todos los hijos
    while (acabados < NUMPROCESOS) {
        pause();
    }

    //Desmontar dispositivo
    if (bumount() == -1) {
        fprintf(stderr, "Error en bumount\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}