#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __WIN32__
#include <windows.h>
#include <winsock.h>
#define close(s) closesocket(s)
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#endif

#define BUFFER_SIZE 1 << 10
#define PORT	8080

int write_data(int s, char *buf, int n)
{
  int bcount;
  int br;
  bcount = 0;
  br = 0;
  while(bcount < n) {
    if ((br = send(s,buf,n-bcount,0)) > 0) {
      bcount += br;
      buf += br;
    }
    else if (br < 0)
      return br;
  }
  return (bcount);
}

int main(int argc, char *argv[])
{
	FILE *fd;
	char *buffer;
	int i;
	int size, size2;
	int sock;
	struct sockaddr_in addr;
#ifdef __WIN32__
	WSADATA wsa;
#endif

	if(argc != 3)
	{
		printf("usage: sendelf [wii ip] [file.elf]\n");
		return -1;
	}

#ifdef __WIN32__
	if (WSAStartup(MAKEWORD(1, 1), &wsa))
	{
		printf("WSAStartup() failed, %lu\n", (unsigned long)GetLastError());
		return EXIT_FAILURE;
	}
#endif

	fd = fopen(argv[2], "rb");
	if(fd == NULL)
	{
		perror("fopen failed");
		return -1;
	}

#ifdef __WIN32__
	addr.sin_addr.S_un.S_addr = inet_addr(argv[1]);
	if(addr.sin_addr.S_un.S_addr == INADDR_NONE)
#else
	if(inet_aton(argv[1], &addr.sin_addr) == 0)
#endif
	{
		perror("inet_aton failed");
		fclose(fd);
		return -1;
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("socket() failed");
		fclose(fd);
		return -1;
	}

	addr.sin_port = htons(PORT);
	addr.sin_family = AF_INET;

	printf("connecting to %s:%d\n", inet_ntoa(addr.sin_addr), PORT);
	if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		perror("connect() failed");
		fclose(fd);
		close(sock);
		return -1;
	}

	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	printf("size: %08X\n", size);
	buffer = malloc(BUFFER_SIZE);
	size2 = ((size & 0xFF000000) >> 24) |
	        ((size & 0x00FF0000) >>  8) |
	        ((size & 0x0000FF00) <<  8) |
  	        ((size & 0x000000FF) << 24);
	printf("size2: %08X\n", size2);
	memcpy(buffer, &size2, sizeof(size));
	if((i = write_data(sock, buffer, sizeof(size))) < 0)
	{
		perror("send() failed");
		fclose(fd);
		close(sock);
		return -1;
	}

	i = 0;
	while(i < size)
	{
		memset(buffer, 0, BUFFER_SIZE);
		printf("reading chunk #%d\n", i);
		if(fread(buffer, size > BUFFER_SIZE ? BUFFER_SIZE : size, 1, fd) < 1)
		{
			perror("fread returned < 1");
/*			fclose(fd);
			close(sock);
			return -1;*/
		}
		printf("%d/%d\n", i, size);
	  	if(write_data(sock, buffer, BUFFER_SIZE) < 0)
		{
			perror("send() failed");
			fclose(fd);
			close(sock);
			return -1;
		}
		i += BUFFER_SIZE;
	}

	printf("file sent successfully(?)\n");
	fclose(fd);
	close(sock);

	return 0;
}
