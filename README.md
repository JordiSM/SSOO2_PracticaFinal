# SSOO2_PracticaFinal
## Practica Final SSOO2 Curso 20/21


La práctica implementará un sistema de ficheros (basado en el ext2 de Unix) en un disco virtual (fichero), el módulo de gestión de ficheros del sistema operativo y la interacción de procesos de usuarios.


### Estructura general
La práctica estará compuesta por los siguientes elementos:
 - Un conjunto de bibliotecas, divididas en niveles, que darán la funcionalidad a las primitivas para acceder al sistema de ficheros y gestionarlo.
 - Un programa mi_mkfs para crear, con la ayuda de dichas bibliotecas, el sistema de ficheros en el disco.
 - El sistema de ficheros (SF) estará realmente contenido en un fichero (simulando un dispositivo de memoria secundaria), al que llamaremos disco y tiene la siguiente estructura:
 - Un conjunto de programas para operar por consola, entre ellos uno que muestre el contenido de un fichero por pantalla (mi_cat), uno que cambie sus permisos de acceso (mi_chmod), uno uno que cree enlaces físicos (mi_ln), uno que liste directorios (mi_ls), uno que borre ficheros y directorios (mi_rm), y uno que muestre la metainformación de un fichero (mi_stat).
     - Su funcionamiento es similar al funcionamiento básico de los correspondientes programas cat, chmod, ln, ls, rm/rmdir, stat de GNU/Linux.
 - Un programa simulador encargado de crear unos procesos de prueba que accedan de forma concurrente al sistema de ficheros (utilizando un semáforo binario) de modo que se pueda comprobar el correcto funcionamiento de las citadas bibliotecas.

