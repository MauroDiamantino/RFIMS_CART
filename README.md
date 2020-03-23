# RFIMS_CART

///////////////////////////////////////////////////////////////ENGLISH/////////////////////////////////////////////////////////////////////

This software is intended to run in the "RF Interference Measurement System (RFIMS)" which is going to be installed beside the China-Argentina Radio Telescope (CART) to analize the RF interference (RFI) which could reach the telescope, taking into account different azimuth angles and two antenna polarizations, horizontal and vertical.

It was designed to capture RF power measurements from a spectrum analyzer Aaronia Spectran HF-60105 V4 X, using an antenna which is mounted on a structure that allow the antenna to be rotated to point the horizon with different azimuth angles and whose polarization could be changed between horizontal and vertical. A sweep from 1 GHz (or maybe less) to 9.4 GHz is captured in each antenna position and then it is calibrated, processed to identify the RFI, saved into memory, plotted with the detected RFI and finally the measurements are sent to a remote server. The initial and stop frequencies are configurable, as many other parameters, through the files which are accesed by the software to load those parameters and which are inside the directories /home/pi/RFIMS-CART/.

To calibrate the measurements, at the beginning of each measurement cycle, which is the set of sweeps corresponding to a turning of 360° of the azimuth angle, a calibration of the RF front end is performed which consists in connecting a noise source (NS) at the input y an capturing two sweeps, one of them with the NS turned off and the other one with the NS turned on. With these sweeps, the curves of the front end parameters, total gain and total noise figure, versus frequency are estimated. Then, the sweeps captured with the antenna connected at the input are calibrated taking into account the estimated front end parameters, so that the distorsion produced by the front end beacuse of its no-flat frequency response, so that the power values represent the signals at the input and so that the internal noise of the front end which has been added to the measurements, to be removed.

The software has been designed to this particual purpose so this can only be run in a Raspberry Pi 3 board o later version with Raspbian Stretch or later, y the software will only work with the spectrum analyzer Aaronia Spectran HF-60105 V4 X and with the Aaronia GPS receiver with integrated sensors.

Before the installation of the software, it is necessary to install the following applications and libraries:

- Driver FTDI D2XX 2, versión 1.4.8 ARMv7 hard-float: https://www.ftdichip.com/Drivers/D2XX.htm
- Library libnmea: http://nmea.io/
- Library WiringPi 4, versión 2.46 o later: http://wiringpi.com/
- Software gnuplot 5 o later: http://www.gnuplot.info/
- The set of C++ libraries “boost” (https://www.boost.org/), versión 1.69.0, of which the following ones were used:
	- Library header-only Boost.Algorithm
	- Library header-only Boost.Date_Time
	- Library Boost.Filesystem
	- Library Boost.System
	- Library header-only Boost.Bimap
	- Library Boost.Timer

To compile and install the software, a terminal must be opened, the current directory must be changed to the base directory of the project and the following commands must be run:

	make all
	sudo make copy-all-files

The first instruction compiles the software and it generates the binay file ./bin/rfims-cart. The second instruction copies the previous binary file to the path /usr/local/bin/, which is inside the environment variable PATH, so that the software could be run without writing the path where the binary file is; the python scritp ./scripts/client.py, which is used by the software to upload the data, is copied to the path /usr/local/; the file ./data/99-aaronia-spectran.rules is copied to the path /etc/udev/rules.d/, which allows a non-root user to run the software; the directory tree ./data/RFIMS-CART/, which contains several files which are accesed by the software, are copied to the path /home/pi/. These directories and files are accesed by the software to load the configuration parameters and to save there the measurements. And finally, the file ./scripts/rfims.desktop is copied to /home/pi/.config/autostart. This file allows the software to be automatically started when the Raspberry Pi turns on.

If you want to recompile the software and copy only the binary to the corresponding directory, so then you must execute the following commands:

	make all
	sudo make copy-bin

To ensure the software can read and write into the files which are in /home/pi/RFIMS-CART/, it is necessary to run the following:

	sudo chmod -r a+rw /home/pi/RFIMS-CART

To run the software, it must be typed "rfims-cart" in a terminal. The software has several arguments which define its behavior. To know the arguments and their usage it must be typed "rfims-cart --help" or "rims-cart -h".

To avoid interferences produced by the Raspberry Pi itself, it is very important to disable the Wi-Fi and Bluetooth interfaces, which is done editing the file /boot/config.txt, the following lines must be added:

	dtoverlay=disable-wifi
	dtoverlay=disable-bt

To genearate/regenerate the software manual, you must run the following commands in a terminal:

	cd doc
	doxygen Doxyfile
	cd latex
	make pdf

After that, the linux scripts "Software_manual_pdf" and "Software_manual_html", which are in the folder doc/, will allow you to access the corresponding files.

//////////////////////////////////////////////////////////////////ESPAÑOL///////////////////////////////////////////////////////////////////////

Este software está pensado para ser ejecutado en el "Sistema de Monitoreo de Interferencias de RF (RFIMS)" que será instalado junto al RadioTelescopio Chino-Argentino (CART) para analizar las interferencias de RF (RFI) que podrían alcanzar al telescopio, teniendo en cuenta diferentes ángulos azimutales y dos polarizaciones de la antena, horizontal y vertical.

El mismo está diseñado para capturar las mediciones de potencia de RF de un analizador de espectro Aaronia Spectran HF-60105 V4 X, usando una antena montada sobre una estructura que le permite rotar para apuntar al horizonte con diferentes ángulos azimutales y que le permite cambiar su polarización entre vertical y horizontal. En cada posición de la antena se captura un barrido desde 1 GHz (o quizás una menor frecuencia) hasta 9.4 GHz y luego el barrido es calibrado, procesado para identificar la RFI, se almacena en memoria no volátil, es graficado en la pantalla con la RFI detectada y, finalmente, todos los datos recolectados son enviados a un servidor remoto. Las frecuencias inicial y final son configurables, al igual que muchos otros parámetros, mediante los archivos a los cuales el software accede para levantar estos datos y que se ubican dentro de los directorios /home/pi/RFIMS-CART/.

Para calibrar las mediciones, al inicio de cada ciclo de medición, el conjunto de barridos que corresponden a un recorrido de 360° azimutal de la antena, se realiza una calibración del front end de RF que consiste en conectar un generador de ruido a la entrada y capturar dos barridos, uno con el generador apagado y otro con el generador encendido. Con estos barridos, se estiman las curvas de la ganancia total y la figura de ruido total del front end, ambas en función de la frecuencia. Luego, los barridos capturados con la antena son calibrados teniendo en cuenta los parameteros estimados anteriores, de modo que se elimine la distorsión introducida por el front end por su respuesta frecuencial no plana, que las potencias queden referenciadas a la entrada del front end y de modo que se elimine el ruido interno del front end adicionado a las señales de entrada.

El software está diseñado para esta aplicación particular por lo que solo puede ejecutarse en una placa Raspberry Pi 3 o superior con Raspbian Stretch o superior, y solo funcionará con el analizador de espectro Aaronia Spectran HF-60105 V4 X y con el receptor GPS con sensores integrados de Aaronia.

Antes de instalar este software, es necesario instalar las siguientes aplicaciones y bibliotecas:

- Driver FTDI D2XX 2, la versión 1.4.8 ARMv7 hard-float: https://www.ftdichip.com/Drivers/D2XX.htm
- Biblioteca libnmea: http://nmea.io/
- Biblioteca WiringPi 4, versión 2.46 o superior: http://wiringpi.com/
- Software gnuplot 5 o superior: http://www.gnuplot.info/
- El paquete de bibliotecas de C++ “boost” (https://www.boost.org/), versión 1.69.0, de las que se utilizaron las siguientes:
	- Biblioteca header-only Boost.Algorithm
	- Biblioteca header-only Boost.Date_Time
	- Biblioteca Boost.Filesystem
	- Biblioteca Boost.System
	- Biblioteca header-only Boost.Bimap
	- Biblioteca Boost.Timer

Para compilar e instalar el software por primera vez se debe abrir una terminal, ubicarse sobre el directorio base del proyecto y ejecutar los siguientes comandos:

	make all
	sudo make copy-all-files

Con la primer instrucción se compila el programa y se genera el binario ./bin/rfims-cart. Con la segunda instrucción se copia el binario anterior a la ruta /usr/local/bin/, que está dentro de la variable de entorno PATH, para que se puede ejecutar el mismo sin escribir la ruta donde se encuentra; se copia el script de python ./scripts/client.py a la ruta /usr/local/, el cual es usado por el programa para enviar los datos al servidor remoto; se copia el archivo con udev rules ./data/99-aaronia-spectran.rules a la ruta /etc/udev/rules.d/, que permite que un usuario no root pueda ejecutar el software; se copia el árbol de directorios con archivos ./data/RFIMS-CART/ a /home/pi/. Estos directorios y archivos son utilizados por el programa para cargar los parámetros de configuración y para almacenar las mediciones y datos capturados. Y por ultimo, se copia el fichero ./scripts/rfims.desktop a /home/pi/.config/autostart, el cual permite que el software se inicie automaticamente cuando se prende la Raspberry Pi.

Si deseas recompilar el software y copiar solo el binario al directorio correspondiente, entonces debes ejecutar los siguientes comandos:

	make all
	sudo make copy-bin

Para asegurarse de que el software pueda escribir en los archivos correspondientes ubicados en /home/pi/RFIMS-CART es necesario modificar los permisos de este arbol de directorios, con el siguiente comando:

	sudo chmod -R a+rw /home/pi/RFIMS-CART

Para ejecutar el programa se debe tipear "rfims-cart" en la terminal. El programa tiene multiples argumentos que permiten modificar su comportamiento. Para conocer los argumentos y cómo deben usarse, se debe tipear "rfims-cart --help" o "rfims-cart -h".

Para evitar interferencias producidas por la misma placa Raspberry Pi, resulta trascendental desactivar las interfaces Wi-Fi y Bluetooth, lo cual se realiza modificando el archivo /boot/config.txt, se deben agregar las siguientes lineas:

	dtoverlay=disable-wifi
	dtoverlay=disable-bt

Para generar/regenerar el manual del software, ejecutar en una terminal los siguientes comandos:

	cd doc
	doxygen Doxyfile
	cd latex
	make pdf

Luego, los scripts de linux "Software_manual_pdf" y "Software_manual_html" de la carpeta doc/ permitirán abrir los archivos correspondientes.

