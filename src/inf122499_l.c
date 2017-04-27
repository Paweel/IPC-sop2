#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "inf122499_queue.h"
#include "inf122499_struct.h"
////////////////
int login;
struct pdata log_data;
////////////////
void menu()
	{
	if (!login)	printf("1 -rejestracja konta\n2 - logowanie \n9 - wyjdź\n");
	else printf("nick:\t\t%s\n"
			"name:\t\t%s\n"
			"lastname:\t%s\n"
			"pesel:\t\t%d\n"
			"1 - Urlop\n"
			"2 - Przyjęcie pacjenta\n"
			"3 - pokaz wizyty\n"
			"9 - wyloguj\n", log_data.nick, log_data.name, log_data.lastname, log_data.pesel);
	}
void clear(struct pdata * data);
void showLoginError();
void showVisitTime();
void printVisit(struct visit_send visit);
int main(int argc, char** argv)
{

	int mtype = getpid();
	int id;
	char buf[16];
	int choice;
	struct list * queueHead = NULL;
	int small_size = sizeof(small_struct)-sizeof(long);
	id = msgget(63535, 0644);
	if (id == -1) 
		{
		printf("Nie ma kolejki komunikatów\n");
		return 1;
		}
	while (1)
	{
		while (!login)
		{
			menu();
			char temp[6];
			choice = -1;
			while (choice < 0)
			{
				scanf("%5s", temp);
				choice = strtol(temp,NULL,10) - 1;
			}
			system("clear");
			choice++;
			//printf("test\n");
			switch(choice)
			{
				case 1://rejestracja
					//		while(small_struct.date)
					{
						small_struct.mtype = 1;
						small_struct.subtype = 2;
						small_struct.pid = mtype;
						do
						{
							printf("podaj haslo\n");
							scanf("%15s", (small_struct.personal.pass));
							printf("powtórz haslo\n");
							scanf("%15s", buf);
						}while(strcmp(small_struct.personal.pass, buf));
						printf("podaj imie\n");
						scanf("%15s", small_struct.personal.name);
						printf("podaj nazwisko\n");
						scanf("%15s", small_struct.personal.lastname);
						printf("podaj nick\n");
						scanf("%15s", small_struct.personal.nick);
						printf("podaj pesel\n");
						scanf("%d", &small_struct.personal.pesel);
						log_data = small_struct.personal;
						msgsnd(id, &small_struct, small_size, 0);
						msgrcv(id, &small_struct, small_size, mtype, 0);
						if(small_struct.date == 1)
							printf("rejestracja nie powiodła się za mało miejsca\n");
						if(small_struct.date == 2)
							printf("rejestracja nie powiodła się nick pesel lub pid powtórzyły się\n");
						if(small_struct.date == 0)
							printf("rejestracja powiodła się\n");
					}
					break;
				case 2://logowanie
					small_struct.mtype = 1;
					small_struct.subtype = 4;
					small_struct.pid = mtype;
					printf("podaj nick\n");
					scanf("%15s", small_struct.personal.nick);
					printf("podaj haslo\n");
					scanf("%15s", small_struct.personal.pass);
					msgsnd(id, &small_struct, small_size, 0);
					msgrcv(id, &small_struct, small_size, mtype, 0);
					memcpy(&log_data, &small_struct.personal, sizeof(small_struct.personal));
					if(small_struct.date == 1)
						printf("logowanie nie powiodło się złe hasło\n");
					if(small_struct.date == 2)
						printf("logowanie nie powiodło się nick nie znaleziony\n");
					if(small_struct.date == 3)
						printf("logowanie nie powiodło się pacjent już zalogowany\n");
					if(small_struct.date == 0)
					{
						msgrcv(id, &info_struct, small_size, mtype, 0);
						showLoginError();
						printf("logowanie powiodło się\n");
						login = 1;
					}
					break;
				case 9: return 0;
			}
		}
		while (login)
		{
			menu();
			char temp[6];
			choice = -1;
			while (choice < 0)
			{
				scanf("%5s", temp);
				choice = strtol(temp,NULL,10) - 1;
			}
			system("clear");
			choice++;
			switch(choice)
			{
				case 1:
					{
						struct tm * date;
						time_t time1 = 0;
						time_t time2 = 0;
						time(&time1);
						int year,mon,mday;
						while(time1 >= time2)
						{
							printf("podaj datę w formacie rrrr mm dd\n");
							date = localtime( &time1 );
							scanf("%d %d %d",
									&year,
									&mon,
									&mday);
							date->tm_year = year - 1900;
							date->tm_mon = mon - 1;
							date->tm_mday = mday;
							date->tm_hour = 0;
							date->tm_min = 0;
							date->tm_sec = 0;
							time1 = mktime(date);
							small_struct.date = time1;
							printf("podaj datę w formacie rrrr mm dd\n");
							date = localtime( &time2 );
							scanf("%d %d %d",
									&year,
									&mon,
									&mday);
							date->tm_year = year - 1900;
							date->tm_mon = mon - 1;
							date->tm_mday = mday;
							date->tm_hour = 0;
							date->tm_min = 0;
							date->tm_sec = 0;
							time2 = mktime(date);
							small_struct.date2 = time2;
						}
						small_struct.mtype = 1;
						small_struct.subtype = 51;
						if (date->tm_isdst == 1)
						{
							small_struct.date -= 3600;
							small_struct.date2 -= 3600;
						}
						msgsnd(id, &small_struct, small_size, 0);
						msgrcv(id, &small_struct, small_size, mtype, 0);
						if(small_struct.date == 1)
							printf("nie powiodło się stara data\n");
						if(small_struct.date == 2)
							printf("nie powiodło się nick nie znaleziony\n");
						if(small_struct.date == 3)
							printf("nie powiodło się brak terminów\n");
						if(small_struct.date == 0)
						{
							printf("Urlop przyznany\n");
						}
						break;
					}
				case 3:// pokaz wizyty
					{
						struct visit_send visit;
						small_struct.mtype = 1;
						small_struct.subtype = 52 ;
						msgsnd(id, &small_struct, small_size, 0);
						delList(&queueHead);
						int i = 1;
						do
						{
							msgrcv(id, &visit, sizeof(visit), mtype, 0);
							if (visit.more == -1)
							{
								printf("brak wizyt\n");
								break;
							}
							addEnd(&queueHead,&visit.data,sizeof(visit.data));
							printf("%d: ",i++);
							printVisit(visit);
						}while(visit.more == 1);
						break;
					}
				case 2:
					{
						char temp[5];
						int nr = -1;
						while (nr < 0)
						{
							printf("przyjmij pacjenta nr: \n");
							scanf("%5s",temp);
							nr = strtol(temp,NULL,10) - 1;
						}
						small_struct.mtype = 1;
						small_struct.subtype = 53;
						if (get(queueHead,nr) != NULL)
						{
							small_struct.date = ((struct visit*)(get(queueHead,nr)->data))->date;
							msgsnd(id, &small_struct, small_size, 0);
							msgrcv(id, &small_struct, small_size, mtype, 0);
							if (small_struct.date == 0 )
								printf("pacjent przyjęty\n");
							else if (small_struct.date == 3 )
								printf("zła data, brak pacjentów lub nieaktualna lista pacjentów, proszę zaktualizować listę pacjentów\n");
							else 
								printf("błąd\n");
						}
					}
					break;
				case 9:
					{
						small_struct.mtype = 1;
						small_struct.subtype = 50;
						msgsnd(id, &small_struct, small_size, 0);
						msgrcv(id, &small_struct, small_size, mtype, 0);
						if(small_struct.date == 0)
							printf("wylogowanie powiodło się\n");
						else
							printf("Nieznany błąd\n");
						clear(&log_data);
						login = 0;
						break;
					}
			}
		}
	}
	return 0;
}
void clear(struct pdata * data)
{
	for(int i=0; i < 16 ; i++)
	{
		data->nick[i] = '\0';
		data->name[i] = '\0';
		data->pass[i] = '\0';
		data->lastname[i] = '\0';
	}
	data->pesel = 0;
}
void showVisitTime()
{
	printf("Data wizyty to: %s", ctime(&small_struct.date2));
}
void showLoginError()
{
	int i = 0;
	while (info_struct.data[i] != 0)
	{
		printf("Nieudane logowanie o: %s", ctime(&info_struct.data[i]));
		if (++i == 19) break;
	}
}

void printVisit(struct visit_send visit)
{
	printf("%s\t%s\t%s\t%d\n", ctime(&visit.data.date), visit.data.personal.name,visit.data.personal.lastname, visit.data.personal.pesel );

}
