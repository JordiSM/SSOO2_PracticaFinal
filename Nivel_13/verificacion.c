#include "verificacion.h"
#define TAM_BUFFER 1024
#include <inttypes.h>

int main(int argc, char *argv[]){
    char *disco, *dir_simulacion;
    int error;
    // Comprobar la sintaxis  //Uso: verificacion <nombre_dispositivo> <directorio_simulación>
    if (argc != 3) {
        fprintf(stderr, "Formato incorrecto: ./verificacion <nombre_dispositivo> <directorio_simulación>\n");
        return EXIT_FAILURE;
    }

    // Montar el dispositivo
    disco = argv[1];
    if (bmount(disco) == EXIT_FAILURE){
        fprintf(stderr, "Error al montar el dispositivo\n");
        return EXIT_FAILURE;
    }

    // Calcular el nº de entradas del directorio de simulación a partir del stat de su inodo
    struct STAT stat;
    dir_simulacion = argv[2];
    if (mi_stat(dir_simulacion, &stat) < 0) {
        fprintf(stderr, "Error mi_stat.\n");
        bumount();
        exit(EXIT_FAILURE);
    }

    int n_entradas = stat.tamEnBytesLog / sizeof(struct entrada);
     
    printf("Total de entradas: %d\n", n_entradas);

    // Si numentradas != NUMPROCESOS  entonces ERROR fsi
    if(n_entradas!= NUMPROCESOS){
        fprintf(stderr, "error, numero de entradas no coincide con numero de procesos\n");
        bumount();
        return EXIT_FAILURE;
    }

    struct entrada entradas[n_entradas];

    //Leemos entradas del directorio de simulacion
	if ((error = mi_read(dir_simulacion, entradas, 0, sizeof(entradas))) < 0) {
		printf("Error: Fallo al leyendo entradas del directorio de simulacion\n");
		mostrar_error_buscar_entrada(error);
        exit(EXIT_FAILURE);
	}

   // Crear el fichero "informe.txt" dentro del directorio de simulación
    char fichero_informe[128];
    strcpy(fichero_informe, dir_simulacion);
    strcat(fichero_informe, "informe.txt");
    if((error = mi_creat(fichero_informe, 7)) < 0){
        fprintf(stderr, "Error creando %s", fichero_informe);
        mostrar_error_buscar_entrada(error);
        bumount();
        return EXIT_FAILURE;
    }

    // Para cada entrada directorio de un proceso hacer
    int bytesInforme = 0;

    for (int i = 0; i < n_entradas; i++) {
        //Extraer el PID a partir del nombre de la entrada y guardarlo en el registro info 
        pid_t pid = atoi(strchr(entradas[i].nombre, '_') + 1);

        //Recorrer secuencialmente el fichero prueba.dat utilizando buffer de N registros de escrituras:  
        char fichero[128];

        strcpy(fichero, dir_simulacion);
        strcat(fichero, entradas[i].nombre);
        strcat(fichero, "/");
        strcat(fichero, "prueba.dat");

        int n_registros_buffer_registros = 256;
        struct REGISTRO buffer_registros[n_registros_buffer_registros];

        struct INFORMACION info;

        //Inicializamos el struct INFORMACION
        info.pid = pid;
        info.nEscrituras = 0;

        info.PrimeraEscritura.fecha = 0;
        info.PrimeraEscritura.pid = 0;
        info.PrimeraEscritura.nEscritura = 0;
        info.PrimeraEscritura.nRegistro = 0;

        info.UltimaEscritura.fecha = 0;
        info.UltimaEscritura.pid = 0;
        info.UltimaEscritura.nEscritura = 0;
        info.UltimaEscritura.nRegistro = 0;

        info.MenorPosicion.fecha = 0;
        info.MenorPosicion.pid = 0;
        info.MenorPosicion.nEscritura = 0;
        info.MenorPosicion.nRegistro = 0;

        info.MayorPosicion.fecha = 0;
        info.MayorPosicion.pid = 0;
        info.MayorPosicion.nEscritura = 0;
        info.MayorPosicion.nRegistro = 0;

        int offset = 0;

        while (mi_read(fichero, buffer_registros, offset, sizeof(struct REGISTRO) * n_registros_buffer_registros) > 0) {
            for (int j = 0; j < n_registros_buffer_registros; j++) {
                //Si la escritura es válida entonces
                if (buffer_registros[j].pid == pid) {
                    //Si es la primera escritura validada entonces
                    if (info.nEscrituras == 0) {
                        //Inicializar los registros significativos con los datos de esa escritura
                        //ya será la menor posición
                        info.PrimeraEscritura = buffer_registros[j];
                        info.UltimaEscritura = buffer_registros[j];
                        info.MenorPosicion = buffer_registros[j];
                        info.MayorPosicion = buffer_registros[j];
                    } else {
                        //Comparar nº de escritura (para obtener primera y última) y actualizarla si es preciso
                        if (info.PrimeraEscritura.nEscritura > buffer_registros[j].nEscritura) {
                            info.PrimeraEscritura = buffer_registros[j];
                        } else if (info.UltimaEscritura.nEscritura < buffer_registros[j].nEscritura) {
                            info.UltimaEscritura = buffer_registros[j];
                        }
                        if (info.PrimeraEscritura.nRegistro > buffer_registros[j].nRegistro) {
                            info.MenorPosicion = buffer_registros[j];
                        } else if (info.UltimaEscritura.nRegistro < buffer_registros[j].nRegistro) {
                            info.MayorPosicion = buffer_registros[j];
                        }
                    }
                    //Incrementar contador escrituras validadas
                    info.nEscrituras++; 

                }
            }
            //Limpiamos el buffer y incrementamos el offset
            memset(buffer_registros, 0, sizeof(struct REGISTRO) * n_registros_buffer_registros);
            offset += sizeof(struct REGISTRO) * n_registros_buffer_registros;

        }
        //Obtener la escritura de la última posición
        printf("%i) %i escrituras validadas en %s\n", i+1, info.nEscrituras, fichero);

        //Añadir la información del struct info al fichero informe.txt por el final
        char buffer[1024];
        memset(buffer, 0, 1024);

        sprintf(buffer, "%s %i\n", "PID: ", info.pid);

        sprintf(buffer + strlen(buffer), "%s %i\n",
            "Numero escrituras: ",
            info.nEscrituras);

        sprintf(buffer + strlen(buffer), "%s %i %i \t%s",
            "Primera escritura: ",
            info.PrimeraEscritura.nEscritura,
            info.PrimeraEscritura.nRegistro,
            asctime(localtime(&info.PrimeraEscritura.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i \t%s",
            "Ultima escritura: ",
            info.UltimaEscritura.nEscritura,
            info.UltimaEscritura.nRegistro,
            asctime(localtime(&info.UltimaEscritura.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i \t%s",
            "Menor posicion: ",
            info.MenorPosicion.nEscritura,
            info.MenorPosicion.nRegistro,
            asctime(localtime(&info.MenorPosicion.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i %s \t%s",
            "Mayor posicion: ",
            info.MayorPosicion.nEscritura,
            info.MayorPosicion.nRegistro,
            asctime(localtime(&info.MayorPosicion.fecha)), "\n");

        //Actualizamos el informe por el final
        if (mi_write(fichero_informe, buffer, bytesInforme, strlen(buffer)) < 0) {
            printf("Error: Escritura en el informe incorrecta.\n");
            bumount();
            exit(EXIT_FAILURE);
        }

        //Aumentamos el offset
        bytesInforme += strlen(buffer);
    }

    bumount();
    
    printf("\n*** Verificacion finalizada ***\n");
    exit(EXIT_SUCCESS);
}
