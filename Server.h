#pragma once
/*
	Eric Fryters
	PROG2065 Ast 6
	March 31 - 2020
	TCP Server implementation - Header file
*/

#define MAX_FIELD 256

/* Structs and Enums */

typedef struct {
	char* employeeID;
	char* employeePIN;
} punchInfo_s;

typedef struct {
	const char* employeePIN;
	int employeePresent;
} returnedEmployee_s;

typedef enum {
	PI_ID,
	PI_PIN
} punchInfoFields_e;

typedef enum {
	SQL_OK,
	SQL_INVALID_EMPLOYEE,
	SQL_INVALID_PIN,
	SQL_INTERNAL_ERROR
} SqlReturnStatus_e;

/* Function Prototypes */
void UnpackMessage(punchInfo_s* pi, char* recvBuf);

/* Function Prototypes -- sqlite_api.cpp */
sqlite3* OpenDatabase(const char* path);
int WritePunchToDB(sqlite3* db, punchInfo_s* pi);