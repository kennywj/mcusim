# MCU simulator
The project ported the FreeRTOS simulator, integrated lwip, fatfs, mbedtls, etc, to simulate the MCU environment. It can use the serial port/PPP protocol to connect to a remote PPP server, and then access the Internet. For testing purposes, some applications are also integrated in the project.

git clone --recursive https://github.com/kennywj/mcusim.git

## usage
"make" will generate executable program "mcusim".  
  $make <br>
To execute "mcusim" will have a command line console on screen.  
Input the "help" command will shows available commands in system.  

## Use GDB to debug the program<br>
it should disable SIGUSR1 signal<br>
(gdb) handle SIGUSR1 noprint nostop
