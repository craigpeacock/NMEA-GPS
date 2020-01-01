#include <stdio.h> 
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

int main(int argc, char **argv)
{
	int fd; 	
	char buffer[255];  
	int  nbytes;       
	struct termios options;

	if ((fd = open("/dev/ttyUSB1", O_RDWR | O_NOCTTY | O_NDELAY)) < 0) {
		perror("Open");
		return 1;
	} 
	
	// Set to blocking
	fcntl(fd, F_SETFL, 0); 

	// Get port attributes
	tcgetattr(fd, &options);

	// Set input and output baud rates
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);

	// Set 8 bits, no parity, 1 stop bit
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	// Set port attributes
	tcsetattr(fd, TCSAFLUSH, &options);

	do {
		nbytes = read(fd, &buffer, sizeof(buffer));
		if (nbytes == 0) perror("Read");
		else {
			buffer[nbytes] = 0;
			if (strncmp(buffer, "$GNGGA", 6) == 0)
				printf("%s",buffer);	
			if (strncmp(buffer, "$GNRMC", 6) == 0)
				printf("%s",buffer);	
		}
	} while(1);

	if (close(fd) < 0){
		perror("Close");
		return 1;
	}

	return (0);
}






