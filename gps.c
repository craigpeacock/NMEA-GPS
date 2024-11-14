#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <time.h>
#include <stdlib.h>

int hex2int(char *c);
int checksum_valid(char *string);
int parse_comma_delimited_str(char *string, char **fields, int max_fields);
int debug_print_fields(int numfields, char **fields);
int OpenGPSPort(const char *devname);
int SetTime(char *date, char *time);

int main(int argc, char **argv)
{
	int fd;
	char buffer[255];
	int nbytes;
	int i;
	char *field[20];

	if ((fd = OpenGPSPort("/dev/ttyUSB0")) < 0)
	{
		printf("Cannot open GPS port\r\n.");
		return 0;
	}

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
				//printf("[%s]\r\n",buffer);
				if (checksum_valid(buffer)) {
					if ((strncmp(buffer, "$GP", 3) == 0) |
						(strncmp(buffer, "$GN", 3) == 0)) {

						if (strncmp(&buffer[3], "GGA", 3) == 0) {
							i = parse_comma_delimited_str(buffer, field, 20);
							//debug_print_fields(i,field);
							printf("UTC Time  :%s\r\n",field[1]);
							printf("Latitude  :%s\r\n",field[2]);
							printf("Longitude :%s\r\n",field[4]);
							printf("Altitude  :%s\r\n",field[9]);
							printf("Satellites:%s\r\n",field[7]);
						}
						if (strncmp(&buffer[3], "RMC", 3) == 0) {
							i = parse_comma_delimited_str(buffer, field, 20);
							//debug_print_fields(i,field);
							printf("Speed     :%s\r\n",field[7]);
							//printf("UTC Time  :%s\r\n",field[1]);
							//printf("Date      :%s\r\n",field[9]);

							SetTime(field[9],field[1]);
						}
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

int SetTime(char *date, char *time)
{
	struct timespec ts;
	struct tm gpstime;
	time_t secs;
	char tempbuf[3];
	int ret;

	printf("GPS    UTC_Date %s, UTC_Time %s\r\n",date, time);
	// GPS date has format of ddmmyy
	// GPS time has format of hhmmss.ss

	if ((strlen(date) != 6) | (strlen(time) != 9)) {
		printf("No date or time fix. Exiting\r\n");
		return 1;
	}

	// Parse day:
	strncpy(tempbuf, (char *)date, 2);
	tempbuf[2] = '\0';
	gpstime.tm_mday = atoi(tempbuf);

	// Parse month:
	strncpy(tempbuf, (char *)date+2, 2);
	tempbuf[2] = '\0';
	gpstime.tm_mon = atoi(tempbuf) - 1;

	// Parse year:
	strncpy(tempbuf, (char *)date+4, 2);
	tempbuf[2] = '\0';
	gpstime.tm_year = atoi(tempbuf) + 100;

	// Parse hour:
	strncpy(tempbuf, (char *)time, 2);
	tempbuf[2] = '\0';
	gpstime.tm_hour = atoi(tempbuf);

	// Parse minutes:
	strncpy(tempbuf, (char *)time+2, 2);
	tempbuf[2] = '\0';
	gpstime.tm_min = atoi(tempbuf);

	// Parse seconds:
	strncpy(tempbuf, (char *)time+4, 2);
	tempbuf[2] = '\0';
	gpstime.tm_sec = atoi(tempbuf);

	printf("Converted UTC_Date %02d%02d%02d, UTC_Time %02d%02d%02d.00\r\n",gpstime.tm_mday,(gpstime.tm_mon)+1,(gpstime.tm_year)%100, gpstime.tm_hour, gpstime.tm_min, gpstime.tm_sec);

	ts.tv_sec = mktime(&gpstime);
	// Apply GMT offset to correct for timezone
	ts.tv_sec += gpstime.tm_gmtoff;

	printf("Number of seconds since Epoch %ld\r\n",ts.tv_sec);

	ts.tv_nsec = 0;
	ret = clock_settime(CLOCK_REALTIME, &ts);
	if (ret)
		perror("Set Clock");

	//clock_gettime(CLOCK_REALTIME, &ts);
	//printf("Number of seconds since Epoch %ld\r\n",ts.tv_sec);
	//gpstime = gmtime(&ts.tv_sec);
	//printf("System UTC_Date %02d%02d%02d, ",gpstime->tm_mday,(gpstime->tm_mon)+1,(gpstime->tm_year)%100);
	//printf("UTC_Time %02d%02d%02d.00\r\n", gpstime->tm_hour, gpstime->tm_min, gpstime->tm_sec);
	printf("\r\n");
}

int OpenGPSPort(const char *devname)
{
	int fd;
	struct termios options;

	if ((fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY)) < 0) {
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

	// Set input modes
	options.c_iflag |= ICRNL;

	// Set 8 bits, no parity, 1 stop bit
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	options.c_lflag &= ~ECHO;
	options.c_lflag |= ICANON;

	// Set port attributes
	tcsetattr(fd, TCSAFLUSH, &options);

	return(fd);
}
