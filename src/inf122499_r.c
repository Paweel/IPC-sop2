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
//cos
//2cos
#define MAX_P 20
#define MAX_D 20
const int CONFIRM_TIME = 3600 * 24 * 14;
long int localTime = 3600;
int findPatient(struct patient *tab, char *nick);
int findDoctor(struct doctor *tab, char *nick);
void reply(int id, int type, int error, int pid);
void sendBackLoginErrorPatient(int id, int pid, struct patient * elem);
void sendBackLoginErrorDoctor(int id, int pid, struct doctor * elem);
void loginErrorPatient(struct patient * elem);
void loginErrorDoctor(struct doctor * elem);
int compSmallStruct(void* a, void* b);
int compVisitStruct(void* a, void* b);
struct visit findGoodDate(struct list * queueHead,struct visit visit, struct doctor *doctor, int doctors_amount);
void sendVisit(struct patient base, int id, struct list * queueHead, int pid);
void sendVisitDoctor(struct doctor doctor, int id, struct list * queueHead, int pid);
unsigned int getLocalTime();
void printList(struct list * head);
int chooseDoctor(struct doctor * doctors, int doctors_amount, long int date);
int checkLeave(struct doctor doctor, long int time);
int checkUncorfimedVisit(struct patient patient,struct list * queueHead);
int setStatus(struct doctor doctor, struct list * queueHead,long int date, int status);
void undoVisit(struct small_struct small_struct, struct list ** queueHead, struct patient * patient);
void checkVisits(struct list ** queueHead, struct patient * patients);

int main(int argc, char** argv)
{
	localTime = time(NULL);
	struct list * queueHead = NULL;
	struct patient patients[MAX_P];
	struct doctor doctors[MAX_D];
	int patients_amount = 0;
	int doctors_amount = 0;
	int id = msgget(63535, 0644 | IPC_CREAT);
	{
		int i;
		for(i = 0 ; i < MAX_P ; i++) 
		{
			patients[i].registered = 0;
			patients[i].login = 0;
			patients[i].head = NULL;
		}
		for(i = 0 ; i < MAX_D ; i++)
		{
			doctors[i].registered = 0;
			doctors[i].login = 0;
			doctors[i].head = NULL;
			doctors[i].leavesList = NULL;
		}
	}
	if (fork() == 0)
	{
		int choice;
		while(1)
		{
			char readDate[4][16];
			printf("1 - zmien date\t 9-koniec\n");
			char temp[6];
			choice = -1;
			while (choice < 0)
			{
				scanf("%5s", temp);
				choice = strtol(temp,NULL,10) - 1;
			}
			choice++;
			switch (choice)
			{
				case 1:
					{
						long int time1 = time(NULL);
						struct tm date;
						date = *localtime(&time1);
						int bad = 4;
						time1 = 0;
						while (time1 <= localTime)
						{
							bad = 4;
							while (bad)
							{
								bad = 4;
								printf("podaj datę w formacie rrrr mm dd hh\n");
								scanf("%15s %15s %15s %15s",readDate[0] , readDate[1], readDate[2], readDate[3]);
								for(int i=0; i<4 ; i++) 
								{
									if (strtol(readDate[i],NULL,10) >=0) bad--;
								}
							}
							date.tm_year = strtol(readDate[0], NULL, 10) - 1900;
							date.tm_mon = strtol(readDate[1], NULL, 10)  - 1;
							date.tm_mday = strtol(readDate[2], NULL, 10) ;
							date.tm_hour = strtol(readDate[3], NULL, 10) ;
							date.tm_min = 0;
							date.tm_sec = 0;
							time1 = mktime(&date);
						}
						localTime = time1;
						small_struct.date = localTime;
						small_struct.subtype = 1000;
						small_struct.mtype = 1;
						msgsnd(id, &small_struct, small_size, 0);
					}
					break;
				case 9:
						small_struct.subtype = 1001;
						small_struct.mtype = 1;
						msgsnd(id, &small_struct, small_size, 0);
						return 0;
					break;
			}
		}
	}
	else while(1)
	{
		///
		printList(queueHead);
		///
		msgrcv(id, &small_struct, small_size, 1, 0);
		switch (small_struct.subtype)
		{
			case 1:
				if (patients_amount < MAX_P)
				{
					int i = 0;
					for(i = 0 ; i < patients_amount ; i++) 
					{
						if ((strcmp(patients[i].personal.nick, small_struct.personal.nick) == 0) || (patients[i].personal.pesel == small_struct.personal.pesel)) break;
					}
					if (i == patients_amount)
					{
						memcpy(&patients[patients_amount].personal, &small_struct.personal, sizeof(small_struct.personal));
						reply(id, 1, 0, small_struct.pid);
						patients[patients_amount].uncorfimedVisit = 0;
						patients[patients_amount].deletedVisit = 0;
						patients[patients_amount++].registered = 1;
					}
					else reply(id, 1, 2, small_struct.pid);//nick pesel lub pid powtórzyły się
				}
				else reply(id, 1, 1, small_struct.pid);//za mało miejsca
				break;
			case 2:
				if (doctors_amount < MAX_P)
				{
					int i = 0;
					for(i = 0 ; i < doctors_amount ; i++) 
					{
						if ((doctors[i].personal.nick == small_struct.personal.nick) || (doctors[i].personal.pesel == small_struct.personal.pesel) /*|| (doctors[i].login == small_struct.date2)*/) break;
					}
					if (i == doctors_amount)
					{
						memcpy(&doctors[doctors_amount].personal, &small_struct.personal, sizeof(small_struct.personal));
						reply(id, 1, 0, small_struct.pid);
						doctors[doctors_amount].id = doctors_amount;
						doctors[doctors_amount++].registered = 1;
					}
					else reply(id, 1, 2, small_struct.pid);//nick pesel lub pid powtórzyły się
				}
				else reply(id, 1, 1, small_struct.pid);//za mało miejsca
				break;
			case 3: // logowanie pacjenta
				{
					int error = 1;
					int i = findPatient(patients, small_struct.personal.nick);
					if (i == -1) reply(id, 3, 2, small_struct.pid); //zły nick
					else
					{
						//if (patients[i].login != 0) reply(id, 3, 3, small_struct.pid); //juz zalogowany
						//else
						{
							if (strcmp(patients[i].personal.pass, small_struct.personal.pass) == 0)
							{
								patients[i].login = small_struct.pid;
								memcpy(&small_struct.personal, &patients[i].personal, sizeof(small_struct.personal));
								small_struct.date2 = 0;
								if (patients[i].uncorfimedVisit) small_struct.date2 = 1;
								if (patients[i].deletedVisit == 1) small_struct.date2 += 2;
								reply(id, 3, 0, small_struct.pid); //ok
								error = 0;
								patients[i].deletedVisit = 0;
								patients[i].uncorfimedVisit = 0;
								sendBackLoginErrorPatient(id, small_struct.pid, &patients[i]);
							}
							else reply(id, 3, 1, small_struct.pid);//złe haslo
						}
						if (error) loginErrorPatient(&patients[i]);
					}
				}
				break;
			case 4:
				{
					int error = 1;
					int i = findDoctor(doctors, small_struct.personal.nick);
					if (i == -1) reply(id, 3, 2, small_struct.pid); //zły nick
					else
					{
						if (doctors[i].login != 0) reply(id, 3, 3, small_struct.pid); //juz zalogowany
						else
						{
							if (strcmp(doctors[i].personal.pass, small_struct.personal.pass) == 0)
							{
								doctors[i].login = small_struct.pid;
								memcpy(&small_struct.personal, &doctors[i].personal, sizeof(small_struct.personal));
								reply(id, 3, 0, small_struct.pid); //ok
								error = 0;
								sendBackLoginErrorDoctor(id, small_struct.pid, &doctors[i]);
							}
							else reply(id, 3, 1, small_struct.pid);//złe haslo
						}
						if (error) loginErrorDoctor(&doctors[i]);
					}
				}
				break;
			case 5: //umów wizytę
				{
					if (doctors_amount <= 0) reply(id, 5, 4, small_struct.pid); //brak lekarza
					else
					{
						int i = findPatient(patients, small_struct.personal.nick);
						if (i == -1) reply(id, 5, 2, small_struct.pid); //zły nick
						else
						{	
							long int time;
							time = small_struct.date;// + 3600 * (timeCorrection - localDST);
							if (time > localTime)
							{
								struct visit temp;
								temp.date = time;
								temp.patientID = i;
								temp = findGoodDate(queueHead, temp, doctors, doctors_amount);
								if (difftime(time, localTime) > (3600 * 24 * 30 * 2))
									temp.status = 4;
								else
									temp.status = 0;
								memcpy(&temp.personal, &patients[i].personal, sizeof(temp.personal));
								small_struct.date2 = temp.date;
								addAsc(&queueHead, &temp, sizeof(temp), compVisitStruct);
								reply(id, 5, 0, small_struct.pid); //ok
							}
							else 
								reply(id, 5, 1, small_struct.pid); //zla data
						}
					}
				}
				break;
			case 9: // lista wizyt dla pacjenta
				{
					printf("przesyłam listę wizyt\n");
					int i = findPatient(patients, small_struct.personal.nick);
					if (i == -1);// reply(id, 5, 2, small_struct.date2); //zły nick
					else
					{
						sendVisit(patients[i], id, queueHead, small_struct.pid);
					}

					break;

				}
			case 11:
				{
					struct visit visit;
					visit.date = small_struct.date;
					int i = findPatient(patients, small_struct.personal.nick);
					if (i == -1) reply(id, 11, 3, small_struct.pid); //zły nick
					else
					{
						if (search(queueHead, &visit, compVisitStruct) != -1)
						{
							struct visit * visit_ptr;
							visit_ptr = (struct visit*)get(queueHead, search(queueHead, &visit, compVisitStruct));
							if ((visit_ptr->status > 2) || (visit_ptr->status == 0))
							{
								del(&queueHead,search(queueHead, &visit, compVisitStruct));
								reply(id, 11, 0, small_struct.pid);
							}
							else
							{
								reply(id, 11, 1, small_struct.pid); // nie mozna usunac wizyty
							}
						}
						else
						{
							reply(id, 11, 2, small_struct.pid); // bledna data
						}
					}
				}
				break;
			case 12: //potwierdź wizytę
				{
					int i = findPatient(patients, small_struct.personal.nick);
					if (i == -1) reply(id, 12, 2, small_struct.pid); //zły nick
					else
					{
						int j = small_struct.date2;
						int answer;
						answer = setStatus(doctors[j], queueHead, small_struct.date, 0);
						reply(id, 12, answer, small_struct.pid); 
					}
				}
				break;
			case 20:
				{
					printf("pacjent wylogowowuje się\n");
					int i = findPatient(patients, small_struct.personal.nick);
					if (i == -1) 
						reply(id, 12, 2, small_struct.pid); //zły nick
					else 
					{
						patients[i].login = 0;
						reply(id, 12, 0, small_struct.pid); //ok
					}
				}
				break;
			case 50:
				{
					printf("lekarz wylogowowuje się\n");
					int i = findDoctor(doctors, small_struct.personal.nick);
					if (i == -1) 
						reply(id, 50, 2, small_struct.pid); //zły nick
					else 
					{
						doctors[i].login = 0;
						reply(id, 50, 0, small_struct.pid); //ok
					}
				}
				break;
			case 51: //urlop
				{
					printf("lekarz bierze urlop\n");
					int i = findDoctor(doctors, small_struct.personal.nick);
					if (i == -1) 
						reply(id, 51, 2, small_struct.pid); //zły nick
					else 
					{
						if (checkLeave(doctors[i],small_struct.date) ||
								checkLeave(doctors[i],small_struct.date2) ||
								small_struct.date <= getLocalTime())	
						{
							reply(id, 51, 1, small_struct.pid); //zła data
						}
						else
						{
							addAsc(&(doctors[i].leavesList),&small_struct,small_size,compSmallStruct);
							undoVisit(small_struct, &queueHead, patients);
							reply(id, 51, 0, small_struct.pid); //ok
						}
					}
				}
				break;
			case 52: //przesłanie wizyt do lekarza z aktualnego dnia
				{
					printf("przesyłam listę wizyt\n");
					int i = findDoctor(doctors, small_struct.personal.nick);
					if (i == -1);// reply(id, 5, 2, small_struct.date2); //zły nick
					else
					{
						sendVisitDoctor(doctors[i], id, queueHead, small_struct.pid);
					}
				}
				break;
			case 53: //przyjecia pacjenta
				{
					int i = findDoctor(doctors, small_struct.personal.nick);
					if (i == -1) 
						reply(id, 53, 2, small_struct.pid); //zły nick
					else 
					{
						if (small_struct.date > getLocalTime())
						{
							if (setStatus(doctors[i], queueHead, small_struct.date,1) == 0)
								reply(id, 53, 0, small_struct.pid); //ok
							else
								reply(id, 53, 1, small_struct.pid); //nie ok
						}
						else
							reply(id, 53, 3, small_struct.pid); //nie ok
					}
				}
				break;
			case 1000:
				{
					localTime = small_struct.date;
					long int time = getLocalTime();
					checkVisits(&queueHead, patients);
					printf("data zmieniona na: %s\n",ctime(&time));
				}
				break;
			case 1001:
				{
					msgctl(id, IPC_RMID, 0);
					return 0;
				}
				break;
		}
	}
	return 0;
}
int checkLeave(struct doctor doctor, long int time)
{
	struct list * temp;
	temp = doctor.leavesList;
	struct small_struct * data;
	long int a,b;
	while (temp != NULL)
	{
		data = (struct small_struct*)(temp->data);
		a = difftime(data->date,time);
		if (a <= 0) 
		{
			b = difftime(data->date2,time);
			if (b >= 0) return 1; //zachodzi na urlop
		}
		temp = temp->next;
	}
	return 0;// nie zachodzi na urlop
}
void loginErrorPatient(struct patient * elem)
{
	int t = getLocalTime();
	addEnd(&(elem->head),&t,sizeof(t));
}
void loginErrorDoctor(struct doctor * elem)
{
	int t = getLocalTime();
	addEnd(&(elem->head),&t,sizeof(t));
}
int chooseDoctor(struct doctor * doctors, int doctors_amount, long int date)
{
	static int i;
	int tab[MAX_D] = {0};
	int isThereAnyDoctor = doctors_amount;
	if (doctors[0].registered == 0) return -1; //nie ma żadnego lekarza
	do
	{
		i = (i + 1) % MAX_D;
		if (checkLeave(doctors[i], date)) 
		{
			--isThereAnyDoctor;
			tab[i] = 1;
		}
		if (isThereAnyDoctor == 0) return -1;
	}while((!doctors[i].registered) || (tab[i] == 1));
	return i;
}
void sendBackLoginErrorPatient(int id, int pid, struct patient * elem)
{
	//	info_struct.date = error;
	info_struct.mtype = pid;
	struct list * temp;
	for(int i = 0; i < 20; i++)
	{
		temp = showElem((elem->head),i);
		if (temp)
			info_struct.data[i] = *(int*)(temp->data);
		else
			info_struct.data[i] = 0;

	}
	delList(&(elem->head));
	msgsnd(id, &info_struct, small_size, 0);
}
void sendBackLoginErrorDoctor(int id, int pid, struct doctor * elem)
{
	//	info_struct.date = error;
	info_struct.mtype = pid;
	struct list * temp;
	for(int i = 0; i < 20; i++)
	{
		temp = showElem((elem->head),i);
		if (temp)
			info_struct.data[i] = *(int*)(temp->data);
		else
			info_struct.data[i] = 0;

	}
	delList(&(elem->head));
	msgsnd(id, &info_struct, small_size, 0);
}
void reply(int id, int type, int error, int pid)
{
	printf("wysyłam odpowiedź do: %d\n",pid);
	switch(type)
	{
		case 1:
		case 2:
		case 3:
		case 5:
		case 11:
		case 12:
		case 20:
		case 50:
		case 51:
		case 53:
			small_struct.date = error;
			small_struct.mtype = pid;
			msgsnd(id, &small_struct, small_size, 0);
			break;
	}
}
int findPatient(struct patient *tab, char *nick)
{
	for(int i = 0; i < MAX_P ; i++)
	{
		if (strcmp(tab[i].personal.nick, nick) == 0) return i;
	}
	return -1;
}
int findDoctor(struct doctor *tab, char *nick)
{
	for(int i = 0; i < MAX_P ; i++)
	{
		if (strcmp(tab[i].personal.nick, nick) == 0) return i;
	}
	return -1;
}
unsigned int getLocalTime()
{
	return localTime;
}
int compSmallStruct(void* a, void* b)
{
	if (((struct small_struct*)a)->date >= 3600 + ((struct small_struct*)b)->date ) return 1;
	if (((struct small_struct*)a)->date + 3600 <= ((struct small_struct*)b)->date ) return -1;
	return 0;
}
int compVisitStruct(void* a, void* b)
{
	if (((struct visit*)a)->date >= 3600 + ((struct visit*)b)->date ) return 1;
	if (((struct visit*)a)->date + 3600 <= ((struct visit*)b)->date ) return -1;
	if (((struct visit*)a)->doctorID > ((struct visit*)b)->doctorID)  return 1;
	if (((struct visit*)a)->doctorID < ((struct visit*)b)->doctorID)  return -1;
	return 0;
}
struct visit findGoodDate(struct list * queueHead,struct visit visit, struct doctor *doctors,int doctors_amount)
{
	struct tm * calendar;
	calendar = localtime(&visit.date);
	static int i;
	int j = i;
	i = (i + 1) % doctors_amount;
	visit.doctorID = i;
	while ( (((calendar->tm_hour > 20) || (calendar->tm_hour < 8))) ||
			(search(queueHead, &visit, compVisitStruct) != -1) ||
			(checkLeave(doctors[i], visit.date) == 1) )
	{
		i = (i + 1) % doctors_amount;
		if (i == j) visit.date += 3600; 
		calendar = localtime(&visit.date);
		visit.doctorID = i;
	}
	memcpy(visit.doctor, doctors[i].personal.lastname, sizeof(doctors[i].personal.lastname)); 
	return visit;
}
void printList(struct list * head)
{
	struct list * temp;
	int i = 0;
	struct visit *tempCos;
	while ((temp = showElem(head, i++)) != NULL) 
	{
		tempCos = (temp->data);
		printf("%d: %ld\t%d\n",i,tempCos->date,tempCos->personal.pesel);
	}

}
const struct visit NULL_VISIT_SEND;
void sendVisitDoctor(struct doctor doctor, int id, struct list * queueHead, int pid)
{
	struct list * temp;
	struct visit_send toSend;
	toSend.mtype = pid;
	struct visit visit;
	struct visit old;
	int first = 1;
	temp = queueHead;
	while (temp != NULL)
	{
		if ((strcmp((*(struct visit*)(temp->data)).doctor, doctor.personal.lastname) == 0) &&
				((*(struct visit*)(temp->data)).date > getLocalTime()) &&
				((*(struct visit*)(temp->data)).date / (3600 * 24)) == (getLocalTime() / (3600 * 24)))

		{
			visit = *(struct visit*)(temp->data);
			if (first)
			{
				old = visit;
				first = 0;
			}
			else
			{
				toSend.more = 1;
				toSend.data = old;
				msgsnd(id,&toSend,sizeof(toSend),0);
				old = visit;
			}
		}
		temp = temp->next;
	}
	if (first)
		toSend.more = -1;
	else
		toSend.more = 0;
	toSend.mtype = pid;
	toSend.data = visit;
	msgsnd(id,&toSend,sizeof(toSend),0);
}
void sendVisit(struct patient base, int id, struct list * queueHead, int pid)
{
	struct list * temp;
	struct visit_send toSend;
	toSend.mtype = pid;
	struct visit visit;
	struct visit old;
	int first = 1;
	temp = queueHead;
	while (temp != NULL)
	{
		if ((*(struct visit*)(temp->data)).personal.pesel == base.personal.pesel)
		{
			visit = *(struct visit*)(temp->data);
			if (first)
			{
				old = visit;
				first = 0;
			}
			else
			{
				toSend.more = 1;
				toSend.data = old;
				msgsnd(id,&toSend,sizeof(toSend),0);
				old = visit;
			}
		}
		temp = temp->next;
	}
	if (first)
		toSend.more = -1;
	else
		toSend.more = 0;
	toSend.mtype = pid;
	toSend.data = visit;
	msgsnd(id,&toSend,sizeof(toSend),0);

}
int checkUncorfimedVisit(struct patient patient,struct list * queueHead)
{
	struct list * temp;
	temp = queueHead;
	int a;
	struct visit * visit;
	while (temp != NULL)
	{
		if ((*(struct visit*)(temp->data)).personal.pesel == patient.personal.pesel)
		{
			visit = temp->data;
			a = difftime(getLocalTime(),visit->date);
			if ((visit->status == 3) &&
					(a > 0) &&
					(a < CONFIRM_TIME))
			{
				return 1;
			}
		}
		temp = temp->next;
	}
	return 0;
}
int setStatus(struct doctor doctor, struct list * queueHead,long int date, int status)
{
	printf("setStatus::Start\n");
	struct visit visit;
	visit.date = date;
	memcpy(visit.doctor, doctor.personal.lastname, 16);
	visit.doctorID = doctor.id;
	if (search(queueHead, &visit, compVisitStruct) != -1)
	{
		struct visit * visit_ptr = (struct visit*)(get(queueHead, search(queueHead, &visit, compVisitStruct))->data);
		if 
			((visit_ptr->status == 4) && (status == 3)) visit_ptr->status = status;
		else if
			((visit_ptr->status == 0) && (status == 1)) visit_ptr->status = status;
		else if
			((visit_ptr->status == 3) && (status == 0)) visit_ptr->status = status;
		else
			return 2;
	}
	else 
		return 1;
	printf("setStatus::End\n");
	return 0;
}
void undoVisit(struct small_struct small_struct, struct list ** queueHead,struct patient * patients)
{
	struct list * temp;
	temp = *queueHead;
	struct visit * visit;
	int i = 0;
	while (temp != NULL)
	{
		visit = (struct visit*)(temp->data);
		if ((visit->date >= small_struct.date) && (visit->date <= small_struct.date2))
		{
			patients[visit->patientID].deletedVisit = 1;
			temp = del(queueHead, i);
		}
		else 
		{
			i++;
			temp = temp->next;
		}
	}
}

void checkVisits(struct list ** queueHead, struct patient * patients)
{
	struct list * temp;
	temp = *queueHead;
	struct visit * visit;
	int i = 0;
	while (temp != NULL)
	{
		visit = (struct visit*)(temp->data);
		if ((visit->status == 4) && (difftime(visit->date, getLocalTime()) < 3600 * 24 * 30 * 2))
		{
			visit->status = 3;
			patients[(visit->patientID)].uncorfimedVisit = 1;
		}
		if ((visit->status == 3) && (difftime(visit->date, getLocalTime()) < 3600 * 24 * 7 * 2))
		{
			patients[visit->patientID].deletedVisit = 1;
			temp = del(queueHead, i);
		}
		else
		{
			++i;
			temp = temp->next;
		}
		if ((visit->status == 0) && (difftime(visit->date, getLocalTime()) < 0))
			visit->status = 2;
	}

}
