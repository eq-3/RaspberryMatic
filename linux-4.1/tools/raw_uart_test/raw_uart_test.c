/*
 * raw_uart_test.c
 *
 *  Created on: Nov 18, 2015
 *      Author: willenborg
 */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <errno.h>


#define DEFAULT_BYTES_TO_SEND 32
#define DEFAULT_REPETITION 1

#define IOCTL_MAGIC 'u'
/* Set and get the priority for the current channel */
#define IOCTL_IOCSPRIORITY _IOW(IOCTL_MAGIC,  1, unsigned long)
#define IOCTL_IOCGPRIORITY _IOR(IOCTL_MAGIC,  2, unsigned long)

int main( int argc, char *argv[] )
{
  int option;
  char mode[2] = "w";
  char *device = NULL;
  int fp = 0;
  int bytes_to_send = DEFAULT_BYTES_TO_SEND;
  int repetition = DEFAULT_REPETITION;
  char *txbuf = NULL;
  bool verbose = false;
  bool prio_high = false;


  fprintf( stdout, "Welcome to the wonderful raw uart test software!\n");

  /* evaluate parameters */
  while( (option = getopt( argc, argv, "vn:r:?d:m:h")) != -1 )
  {
    switch( option )
    {
    case 'd': /* Device */
      device = malloc( strlen(optarg) + 1 );
      if( device != NULL )
      {
        strcpy( device, optarg );
        fprintf( stdout, "Device: %s\n", device );
      }
      else
      {
        fprintf( stderr, "Error allocating memory for device name.\n" );
        return -1;
      }
      break;

    case 'm': /* Mode: read (r) /write (w) */
      if( optarg[0] == 'r' )
      {
        mode[0] ='r';
      }
      fprintf( stdout, "Mode: %s\n", mode);
      break;

    case 'n': /*Bytes to send*/
      bytes_to_send = atoi( optarg );
      if( bytes_to_send == 0 )
      {
        fprintf( stderr, "Set bytes to send to default.\n" );
        bytes_to_send = DEFAULT_BYTES_TO_SEND;
      }
      break;

    case 'r': /*repetitions*/
      repetition = atoi( optarg );
      if( repetition == 0 )
      {
        fprintf( stderr, "Set repetitions to default.\n" );
        repetition = DEFAULT_REPETITION;

      }
      break;

    case 'h': /*higher priority*/
      prio_high = true;
      break;

    case 'v': /*verbose*/
      verbose = true;
      break;

    case '?': /* Help message */
      fprintf( stdout, "raw_uart_test -d <uart device> -m <read(r)|write(w)> -v (verbose) -n <bytes to send> -r <repetitions> -h (higher priority)\n");
      break;
    }
  }
  fp = open( device, mode[0]=='r' ? O_RDONLY : O_WRONLY );
  if( fp == -1 )
  {
    fprintf( stderr, "Cannot open %s\n", device );
    goto error;
  }
  fprintf( stdout, "Device %s opened.\n", device );

  /* -- Read -- */
  if( mode[0] == 'r' )
  {
    while( 1 )
    {
      char c[1024];
      size_t r;
      r = read( fp, c, 1024 );
      if( r > 0 )
      {
        for( int i=0; i<r; i++ )
        {
          fputc( c[i], stdout );
        }
      }

      usleep(5000);
    }

  }
  /* -- Write -- */
  else
  {
    /*Set priority*/
    int prio = 0; /*lowest priority*/
    if( prio_high == true )
    {
      prio = 1;
    }
    if( ioctl(fp, IOCTL_IOCSPRIORITY, &prio) != 0 )
    {
      fprintf( stderr, "Error while setting priority.\n" );
      goto error;
    }


    txbuf = malloc( bytes_to_send + 2 + 5 );
    if( txbuf != NULL )
    {
      for( int i=0; i<bytes_to_send; i++ )
      {
        if( prio_high == true )
        {
          txbuf[i+5] = '0' + (i % 10);  /*Write numbers*/
        }
        else
        {
          txbuf[i+5] = 'A' + (i % 26);  /*Write numbers*/
        }
      }
      txbuf[bytes_to_send+5] = '\n';
      txbuf[bytes_to_send+1+5] = 0;
    }
    else
    {
      fprintf( stderr, "Error allocating memory for tx buffer.\n" );
      goto error;
    }

    for( int i=0; i<repetition; i++ )
    {
      sprintf( txbuf, "%04i", i );
      if( prio_high == true )
      {
        txbuf[4] = '=';
      }
      else
      {
        txbuf[4] = '-';
      }

      if( write( fp, txbuf, bytes_to_send+2+5 ) == (bytes_to_send + 2 + 5) )
      {
        if( verbose == true )
        {
          fprintf( stdout, txbuf );
        }
      }
      else
      {
        fprintf( stderr, "Error writing data to device: %i\n", errno );
      }
    }

    free( txbuf );
  }

error:
  if( fp )
  {
    close( fp );
  }
  free( device );
	return 0;
}

