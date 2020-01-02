#include <stdio.h> 
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

int hex2int(char *c);
int checksum_valid(char *string);
int parse_comma_delimited_str(char *string, char **fields, int max_fields);
int debug_print_fields(int numfields, char **fields);

int main(int argc, char **argv)
{
	int fd; 	
	char buffer[255];  
	int nbytes; 
	int i; 
	struct termios options;
	char *field[20];
		
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
	
	options.c_lflag &=ECHO;

	// Set port attributes
	tcsetattr(fd, TCSAFLUSH, &options);

	do {
		if ((nbytes = read(fd, &buffer, sizeof(buffer))) < 0) {
			perror("Read");
			return 1;
		} else {
			if (nbytes == 0) {
				printf("No communication from GPS module\r\n");
				sleep(1);
			} else {
				buffer[nbytes - 1] = '\0';
				printf("%s\r\n",buffer);
				if (checksum_valid(buffer)) {
					if (strncmp(buffer, "$GNGGA", 6) == 0) {
						i = parse_comma_delimited_str(buffer, field, 20);
						//debug_print_fields(i,field);
						printf("UTC Time  :%s\r\n",field[1]);
						printf("Latitude  :%s\r\n",field[2]);
						printf("Longitude :%s\r\n",field[4]);
						printf("Altitude  :%s\r\n",field[9]);
						printf("Satellites:%s\r\n",field[7]);
					}
					if (strncmp(buffer, "$GNRMC", 6) == 0) {
						i = parse_comma_delimited_str(buffer, field, 20);
						//debug_print_fields(i,field);
						printf("Speed     :%s\r\n",field[7]);
					}
				}
			}
		}
	} while(1);

	if (close(fd) < 0) {
		perror("Close");
		return 1;
	}

	return (0);
}

int debug_print_fields(int numfields, char **fields)
{
	printf("Parsed %d fields\r\n",numfields);
	
	for (int i = 0; i <= numfields; i++) {
		printf("Field %02d: [%s]\r\n",i,fields[i]);
	}
}

int hexchar2int(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

int hex2int(char *c)
{
	int value;	

	value = hexchar2int(c[0]);
	value = value << 4;
	value += hexchar2int(c[1]);

	return value;
}

int checksum_valid(char *string)
{
	char *checksum_str;					
	int checksum;
	unsigned char calculated_checksum = 0;
	
	// Checksum is postcede by *
	checksum_str = strchr(string, '*');
	if (checksum_str != NULL){
		// Remove checksum from string
		*checksum_str = '\0';
		// Calculate checksum, starting after $ (i = 1)  	
		for (int i = 1; i < strlen(string); i++) {
			calculated_checksum = calculated_checksum ^ string[i];
		}
		checksum = hex2int((char *)checksum_str+1);
		//printf("Checksum Str [%s], Checksum %02X, Calculated Checksum %02X\r\n",(char *)checksum_str+1, checksum, calculated_checksum);
		if (checksum == calculated_checksum) {
			//printf("Checksum OK");
			return 1;
		}
	} else {
		//printf("Error: Checksum missing or NULL NMEA message\r\n");
		return 0;
	}
	return 0;
}

int parse_comma_delimited_str(char *string, char **fields, int max_fields)
{
	int i = 0;
	fields[i++] = string;

	while ((i < max_fields) && NULL != (string = strchr(string, ','))) {
		*string = '\0';
		fields[i++] = ++string;
	}

	return --i;
}




