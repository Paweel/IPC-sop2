#pragma once
struct pdata
{
	char nick[16];
	char pass[16];
	char name[16];
	char lastname[16];
	int pesel;
};
struct patient
{
	struct pdata personal;
	int login;
	int registered;
	struct list * head;
	int uncorfimedVisit;
	int deletedVisit;
};
struct doctor
{
	struct pdata personal;
	int login;
	int registered;
	int id;
	struct list * head;
	struct list * leavesList;
};
struct small_struct
{
	long mtype;
	int subtype;
	long int date;
	long int date2;
	int pid;
	struct pdata personal;
} small_struct;
int small_size = sizeof(small_struct)-sizeof(long);
struct visit
{
	struct pdata personal;
	int status;
	// 0 - czeka 1 - obyta 2 - nieodbyta 3 - wymaga potwierdzenia 4 - niepotwierdzona
	char doctor[16];
	int doctorID;
	int patientID;
	long int date;
};
struct visit_send
{
	long mtype;
	struct visit data;
	int more;
};
struct
{
	long mtype;
	int subtype;
	long int data[20];
} info_struct;
int info_size = sizeof(info_struct)-sizeof(long);
struct 
{
    long mtype;
	int subtype;
	char nick[16][16];
	int more;
} doctors_names;
