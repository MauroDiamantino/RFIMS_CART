#/################RFIMS-CART client script###################/#

# Version 2

## \file client.py
# \brief The aim of this script is to upload the data collected by the software 'rfims-cart' (which calls this script).
#
# First, the script checks many times if there is Internet connection, through one hour, if so then it sends a compressed
# archive file with the rfims-cart data to the remote server, using scp, later it wakes up the server and, finally, it
# deletes the local archive file. If any error occurs the local archive file is not removed and it remains in a
# queue (in the software rfims-data) waiting to be uploaded. The path and name of the file to be sent is passed as an argument.
#
# \author Leandro Saldivar
# \author Mauro Diamantino

import os
import sys
import urllib.request, urllib.error, urllib.parse
import paramiko
import getpass
#import pysftp

from paramiko import SSHClient
from scp import SCPClient
from time import sleep

## The public IP of the MV Amazon remote server.
HOST = ""
## The username which must be used to log in the remote server.
USERNAME = ""
## The password which must be used to log in the remote server.
PASSWORD = ""
## The port through which the data file is sent, using scp.
PORT = 22
## The remote folder where the file is stored.
REMOTE_FOLDER = "/home/server/RFIMS-CART/downloads/"
## The remote path to the sript which must be executed to wake up the server.
SERVER_SCRIPT_PATH = "/home/server/RFIMS-CART/Server2.py"
## The number of times the Internet connection is checked, with an interval time between each try.
TRIES = 6
## The interval time which is waited between two tries of checking Internet connection, expressed in seconds (s).
WAIT_SECONDS = 600
## The interval time which is waited between two tries of checking Internet connection, expressed in minutes.
WAIT_MINS = WAIT_SECONDS / 60

## This function checks if there is Internet connection.
# \return A `true` value is returned if theres is Internet connection and a `false` value otherwise.
def Internet_on(): #Verifica si existe conexión a internet
    try:
        response = urllib.request.urlopen('https://www.google.co.in', timeout=20)
        return True
    except urllib.error.URLError as err:
        pass
    return False

## This function sends the given file to the remote server, using scp.
# \return A `true` value is returned if the operation finished successfully, and a `false` value otherwise.
def SendData(file_path): #Envía archivos comprimidos a traves de scp
    try:
        ssh = SSHClient()
        ssh.load_system_host_keys()
        ssh.connect(HOST, port=PORT, username=USERNAME, password=PASSWORD)
        scp = SCPClient(ssh.get_transport())
        scp.put(file_path, remote_path=REMOTE_FOLDER)
        ssh.close()

        """ OPCION DE ENVÍO A TRAVES DE SFTP (import pysftp)
        try:
            with pysftp.Connection(HOST, username=USERNAME, password=PASSWORD) as sftp: 
                with sftp.cd(REMOTE_FOLDER):
                    sftp.put(file_path)
        except:
            print("Conexión sftp interrumpida")
            return False
        """
    except Exception as e:
        #print(e)
        return False
    return True

## This function wakes up the remote server to that one processes the sent file.
# \return A `true` value is returned if the operation finished successfully, and a `false` value otherwise.
def ServerWakeUp(file_name):
    try:
        command = "python3 " + SERVER_SCRIPT_PATH + " " + file_name
        #command = "python3 " + SERVER_SCRIPT_PATH
        ssh_client = SSHClient()
        ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        ssh_client.connect(HOST, PORT, USERNAME, PASSWORD)
        entrada, salida, error = ssh_client.exec_command(command)
#        salida.channel.recv_exit_status()
#        lines = salida.readlines()
#        for line in lines:
#            print(line)
        ssh_client.close()
    except Exception as e:
        #print(e)
        return False
    return True

## This function extracts the filename from the script argument.
# \return The name of the file to be sent.
def NameExtractor(file_path):
    for i in range(len(file_path)-1,0,-1):
        #if file_path[i] == "\\" #PARA WINDOWS
        if file_path[i] == "/":  #PARA LINUXXX
            index = i + 1
            break
    return str(file_path[index:])

#--------------------------------------------------------------------------------------------------
if __name__ == "__main__": # Usage: client.py file_path
    ## The script argument which contains the path and name of the file to be sent.
    file_path = sys.argv[1]
    ## The name of the file to be sent.
    file_name = NameExtractor(file_path)
    for i in range(0,TRIES):
        if not Internet_on():
            #print("Warning: there is no internet connection, it will be waited " + WAIT_MINS + " minutes to check it again")
            sleep(WAIT_SECONDS) #espera 10 minutos
        else:
            break

    if i == (TRIES-1):
    #Fin del script debido a que no hubo conexion a internet en una hora
        #exit("No internet connection")
        exit(3)
    else:
        if SendData(file_path):
            if ServerWakeUp(file_name):
                os.remove(file_path)
                #Fin del script con exito
                #print("OK")
                exit(0)
            else:
                #Fin del script debido a que el servidor remoto no se desperto
                #exit("The server did not wake up")
                exit(4) 
        else:
            #Fin del script debido que no se transmitio el archivo al servidor remoto
            #exit("No transmission")
            exit(5)
