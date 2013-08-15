/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: uart_util.c

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-Aug 2011       Created                                         Chang, Jianjun

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#include "bs_cfg.h"
#include "flog.h"
#include "uart_util.h"

int uart_fd = -1;

static speed_t sys_speed_arr[] = {B38400, B19200, B9600, B4800, B2400, B1200, B300,
          B38400, B19200, B9600, B4800, B2400, B1200, B300};

static int name_arr[] = {38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,  
          19200,  9600, 4800, 2400, 1200,  300};

int opendev (char *dev)
{
    int fd = open( dev, O_RDWR );         //| O_NOCTTY | O_NDELAY

    if (fd == -1)
    {
        FLOG_ERROR("Can't Open Serial Port\n");
        return -1;
    }
    else
    {
        return fd;
    }

    return -1;
}

/*
set serial port speed 
*/

int set_speed(int fd, int speed)
{
    int   i; 
    int   status; 
    struct termios   opt;

    if(tcgetattr(fd, &opt) !=0)
    {
        FLOG_ERROR("setupspeed error\n");     
        return 1;
    }

    for ( i= 0;  i < (int)(sizeof(sys_speed_arr) / sizeof(int));  i++)
    {
        if  (speed == name_arr[i])
        {     
            tcflush(fd, TCIOFLUSH);     
            cfsetispeed(&opt, sys_speed_arr[i]);
            cfsetospeed(&opt, sys_speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &opt);

            if  (status != 0)
            {
                FLOG_ERROR("tcsetattr error\n");
                return 1;
            }

            tcflush(fd, TCIOFLUSH);
        }
    }

    return 0;
}
 

int set_parity(int fd, int databits, int stopbits, int parity)
{ 
    struct termios options;

    if  ( tcgetattr( fd,&options)  !=  0)
    {
        FLOG_ERROR("setupserial error \n");
        return 1;
    }

    options.c_cflag &= ~CSIZE;
    switch (databits) //set  bits in one byte
    {   
        case 7:		
            options.c_cflag |= CS7; 
            break;
        case 8:     
            options.c_cflag |= CS8;
            break;   
        default:    
            FLOG_ERROR("Unsupported data size\n");
            return 1;
    }
    switch (parity)  //set  parity
    {   
        case 'n':
        case 'N':    
            options.c_cflag &= ~PARENB;   // clear parity enable 
            options.c_iflag &= ~INPCK;     // enable parity checking  
            break;  
        case 'o':   
        case 'O':     
            options.c_cflag |= (PARODD | PARENB); // enable odd parity
            options.c_iflag |= INPCK;             // Disnable parity checking
            break;  
        case 'e':  
        case 'E':   
            options.c_cflag |= PARENB;     // enable parity     
            options.c_cflag &= ~PARODD;   // enable even parity 
            options.c_iflag |= INPCK;       // Disnable parity checking
            break;
        case 'S': 
        case 's':  //as no parity   
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;  
        default:   
            FLOG_ERROR("Unsupported parity\n");
            return 1;
    }
 
    switch (stopbits)
    {   
        case 1:    
            options.c_cflag &= ~CSTOPB;  
            break;  
        case 2:    
            options.c_cflag |= CSTOPB;  
            break;
        default:    
            FLOG_ERROR("Unsupported stop bits\n");  
            return 1;
    }

    /* Set input parity option */ 
    if (parity != 'n')   
        options.c_iflag |= INPCK;

    tcflush(fd,TCIFLUSH);
    options.c_cc[VTIME] = 150; /* timeout 15 seconds*/   
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */

    if (tcsetattr(fd,TCSANOW,&options) != 0)   
    { 
        FLOG_ERROR("SetupSerial error\n");
        return 1;
    }

    return 0;
}

int init_uart(void)
{
    int ret = 0;
    char tmp_string[128];

    ret = get_global_param ("UART_DEV_NAME", tmp_string);

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters UART_DEV_NAME error\n");
    }

    uart_fd = opendev(tmp_string);

    set_speed(uart_fd, 19200);

    if (set_parity(uart_fd, 8, 1, 'N') == -1)
    {
        FLOG_ERROR("Set UART Parity Error %d\n");
        return 1;
    }

    return 0;
}

int release_uart(void)
{
    if (uart_fd != 0)
    {
        close(uart_fd);
        uart_fd = -1;
    }

    return 0;
}

int write_uart(char * buf, int len)
{
#ifdef _UART_ENABLE_
    int ret = 0;

    ret = write(uart_fd, buf , len);

    if (ret != len)
    {
        FLOG_WARNING("Write UART device error\n");
        return 1;
    }
#else
    (void) buf;
    (void) len;
#endif

    return 0;
}

