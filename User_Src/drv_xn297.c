#include "spi.h"
#include "xn297.h"

#define spi_cson SPI_CE_L
#define spi_csoff SPI_CE_H
void xn_writereg( int reg , int val)
{
	reg = reg&0x0000003F;
	reg = reg|0x00000020;
	spi_cson();
	spi_sendbyte( reg);
	spi_sendbyte( val);
	spi_csoff();
}

int xn_readreg( int reg)
{
	int val;
	reg = reg&0x0000001F;
	spi_cson();
	spi_sendrecvbyte( reg);
	val =spi_sendrecvbyte( 255);
	spi_csoff();
	return val;
}

int xn_command( int command)
{
	int status;
	spi_cson();
	status = spi_sendrecvbyte(command);
	spi_csoff();
	return status;
}
//
void _spi_write_address( int reg, int val);

void _spi_write_address( int reg, int val)
{
	spi_cson();
	spi_sendbyte( reg);
	spi_sendbyte( val);
	spi_csoff();
}


void xn_readpayload( int *data , int size )
{
	int index = 0;
	spi_cson();
	spi_sendrecvbyte( 97 ); // read rx payload
	while(index<size)
	{
	data[index]=	spi_sendrecvbyte( 255 );
	index++;
	}
	spi_csoff();
}



void xn_writerxaddress(  int *addr )	
{
 int index = 0;
spi_cson();
spi_sendbyte(0x2a);
	while(index<5)
	{
	spi_sendbyte( addr[index] );
	index++;
	}
spi_csoff();
}















