#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#include <wiringPi.h>

#define	BUF_LEN	1024

int get_pos()
{
	return ((digitalRead(23) << 1) + digitalRead(24));
}

int enc_table[4][4] = {
	{
		0,	// 00->00
		1,	// 00->01
		-1,	// 00->10
		0	// 00->11 => can't happen
	},
	{
		-1,	// 01->00
		0,	// 01->01
		0,	// 01->10 => can't happen
		1	// 01->11
	},
	{
		1,	// 10->00
		0,	// 10->01 => can't happen
		0,	// 10->10
		-1	// 10->11
	},
	{
		0,	// 11->00 => can't happen
		-1,	// 11->01
		1,	// 11->10
		0	// 11->11
	}
};

void post_msg(char *string)
{
	int len;
	int server_sockfd;
	int server_len;
	struct sockaddr_in server_address ;

	server_sockfd = socket(AF_INET,SOCK_STREAM,0);
	server_address.sin_family = AF_INET ;
	server_address.sin_addr.s_addr = INADDR_ANY ;
	server_address.sin_port = htons(4000) ;
	len = sizeof(server_address);

	connect(server_sockfd, (struct sockaddr *)&server_address, len);

	write(server_sockfd, string, strlen(string));
	close(server_sockfd);
}

static int	current_pos = 0;
static int	target_pos = 0;
#define	POS_OFF	0
#define	POS_LOW	4
#define	POS_MID	6
#define	POS_MAX	10

static int	current_thermo = 0;

void *thread_gpio(void *ptr)
{
	int	i;
	int	prev, current;
	int	vector;
	int	pos = 0;
	int	thermo = 0;
	char	msg[32];

	prev = get_pos();	// get initial position

	while (1) {
		// update position
		current = get_pos();
		if (prev != current) {
			printf("current %d%d\n", current / 2, current %2);
		}
		vector = enc_table[prev][current];
		if (vector != 0) {
			current_pos += vector;
			sprintf(msg, "Pos=%d", current_pos);
			post_msg(msg);
		}
		prev = current;
		
		// update thermostat
		thermo = digitalRead(25);
		if (current_thermo != thermo) {
			current_thermo = thermo;
			sprintf(msg, "Thermo=%d", current_thermo);
			post_msg(msg);
		}

		// control
		if (current_pos == target_pos) {
			digitalWrite(20, 0);
			digitalWrite(21, 0);
		}
		if (current_pos > target_pos) {
			digitalWrite(20, 0);
			digitalWrite(21, 1);
		}
		if (current_pos < target_pos) {
			digitalWrite(20, 1);
			digitalWrite(21, 0);
		}
		delay(10);
	}

	return;
}

typedef enum State_e {
	INIT = 0,
    BOOT,
	MIN,
	MID,
	OFF
} State;

State state = INIT;
void TRANS(State);
struct timeval tv_finish;

void StateFuncInit(char *ch)
{
	char *param = &ch[0];
	char *val = strchr(ch, '=');

	if (*param != 'C') {
		printf("StateFuncInit:%s\n", ch);
	}

	switch (*param) {
	case 'A':	// Adjust
		if (val != 0) {
			val++;
			current_pos += atoi(val);
		}
		break;
	case 'E':	// Entry/Exit
		if (strcmp(param, "Entry") == 0) {
		}
		if (strcmp(param, "Exit") == 0) {
		}
		break;
	case 'F':	// Fire
		if (val != 0) {
			val++;

			int cook_time = atoi(val);
			if (cook_time != 0) {
				gettimeofday(&tv_finish, 0);
				tv_finish.tv_sec += (cook_time * 60);
				TRANS(BOOT);
			}
		}
		break;
	case 'T':	// Thermo
		break;
	case 'P':	// Position
		break;
	case 'C':	// Click
		break;
	default:
		break;
	}
}

void StateFuncBoot(char *ch)
{
	char *param = &ch[0];
	char *val = strchr(ch, '=');

	struct timeval	tv;

	if (*param != 'C') {
		printf("StateFuncInit:%s\n", ch);
	}

	switch (*param) {
	case 'E':	// Entry/Exit
		if (strcmp(param, "Entry") == 0) {
			target_pos = POS_MAX;
		}
		if (strcmp(param, "Exit") == 0) {
		}
		break;
	case 'F':	// Fire
		if (val != 0) {
			val++;
			if (strcmp(val, "off") == 0) {
				TRANS(OFF);
			}
		}
		break;
	case 'P':	// Position
		break;
	case 'T':	// Thermo
		if (val != 0) {
			val++;
			if (atoi(val) == 0) {
				TRANS(MIN);
			}
		}
		break;
	case 'C':	// Click
		gettimeofday(&tv, 0);
		if (tv_finish.tv_sec < tv.tv_sec) {
			TRANS(OFF);
		}
		break;
	default:
		break;
	}
}

void StateFuncMin(char *ch)
{
	char *param = &ch[0];
	char *val = strchr(ch, '=');

	if (*param != 'C') {
		printf("StateFuncInit:%s\n", ch);
	}

	switch (*param) {
	case 'E':	// Entry/Exit
		if (strcmp(param, "Entry") == 0) {
			target_pos = POS_LOW;
		}
		if (strcmp(param, "Exit") == 0) {
		}
		break;
	case 'F':	// Fire
		if (val != 0) {
			if (strcmp(val, "off") == 0) {
				TRANS(OFF);
			}
		}
		break;
	case 'P':	// Position
		break;
	case 'T':	// Thermo
		if (val != 0) {
			val++;
			if (atoi(val) == 1) {
				TRANS(MID);
			}
		}
		break;
	default:
		break;
	}
}

void StateFuncMid(char *ch)
{
	char *param = &ch[0];
	char *val = strchr(ch, '=');

	if (*param != 'C') {
		printf("StateFuncInit:%s\n", ch);
	}

	switch (*param) {
	case 'E':	// Entry/Exit
		if (strcmp(param, "Entry") == 0) {
			target_pos = POS_MID;
		}
		if (strcmp(param, "Exit") == 0) {
		}
		break;
	case 'F':	// Fire
		if (val != 0) {
			val++;
			if (strcmp(val, "off") == 0) {
				TRANS(OFF);
			}
		}
		break;
	case 'P':	// Position
		break;
	case 'T':	// Thermo
		if (val != 0) {
			val++;
			if (atoi(val) == 0) {
				TRANS(MIN);
			}
		}
		break;
	default:
		break;
	}
}

void StateFuncOff(char *ch)
{
	char *param = &ch[0];
	char *val = strchr(ch, '=');

	if (*param != 'C') {
		printf("StateFuncInit:%s\n", ch);
	}

	switch (*param) {
	case 'E':	// Entry/Exit
		if (strcmp(param, "Entry") == 0) {
			target_pos = POS_OFF;
			if (current_pos == target_pos) {
				TRANS(INIT);
			}
			tv_finish.tv_sec = 0;
			tv_finish.tv_usec = 0;
		}
		if (strcmp(param, "Exit") == 0) {
		}
		break;
	case 'F':	// Fire
		if (val != 0) {
			if (strcmp(val, "on") == 0) {
			}
			if (strcmp(val, "off") == 0) {
			}
		}
		break;
	case 'P':	// Position
		if (val != 0) {
			val++;
			if (atoi(val) == POS_OFF) {
				TRANS(INIT);
			}
		}
		break;
	default:
		break;
	}
}

typedef void (*STATEFUNC)(char *);
STATEFUNC StateTable[] = {
	StateFuncInit,
	StateFuncBoot,
	StateFuncMin,
	StateFuncMid,
	StateFuncOff
};

void TRANS(State next) {
    StateTable[state]("Exit");
    state = next;
    StateTable[state]("Entry");
}

void *thread_server(void *ptr)
{
	int server_sockfd, client_sockfd ;
	int server_len, client_len ;
	struct sockaddr_in server_address ;
	struct sockaddr_in client_address ;

	struct timeval tv;

	server_sockfd = socket(AF_INET,SOCK_STREAM,0);
	server_address.sin_family = AF_INET ;
	server_address.sin_addr.s_addr = INADDR_ANY ;
	server_address.sin_port = htons(4000) ;

	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(server_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));

	server_len = sizeof(server_address);
	if (bind(server_sockfd , (struct sockaddr *)&server_address , server_len) < 0) {
		printf("bind failed\n");
		return 0;
	}

	if (listen(server_sockfd, 5) < 0) {
		printf("listen failed\n");
		return 0;
	}

	char ch[BUF_LEN];
	char resp[BUF_LEN];
	int	len;
	while(1) {
		client_sockfd = accept(server_sockfd ,
			(struct sockaddr *)&client_address , &client_len);
		if (client_sockfd != -1) {
			len = read(client_sockfd, ch, sizeof(ch));
			ch[len] = '\0';
			if (ch[0] != 'I') {		// Inquire
				StateTable[state](&ch[0]);
			} else {
				int	time_target = 0;
				if (tv_finish.tv_sec) {
					struct timeval	tv;
					gettimeofday(&tv, 0);
					time_target = tv_finish.tv_sec;
				}

				sprintf(resp,
					"{\"pos\":%d,\"thermo\":%d,\"time\":%d,\"state\":%d}",
					current_pos,
					current_thermo,
					time_target,
					state);
				write(client_sockfd, resp, strlen(resp));
			}
			close(client_sockfd);
		} else {
			StateTable[state]("Click");
		}
	}
	return 0;
}

int main(void)
{
	pthread_t thread1, thread2;

	// setup GPIO
	if (wiringPiSetupGpio() == -1) {
		printf("input setup failed\n");
		return 0;
	}
  
	// motor control
	pinMode(20, OUTPUT);
	pinMode(21, OUTPUT);

	// rotary encoder
	pinMode(23, INPUT);
	pullUpDnControl(23, PUD_UP);
	pinMode(24, INPUT);
	pullUpDnControl(24, PUD_UP);
  
	// thermostat
	pinMode(25, INPUT);
	pullUpDnControl(25, PUD_UP);

	// init state machine
   	StateTable[state]("Entry");

	// kick threads
	pthread_create( &thread1, NULL, thread_gpio, 0);
	pthread_create( &thread2, NULL, thread_server, 0);
 
	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL);
 
	return 0;
}

