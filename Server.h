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
	char* name;
	char* email;
	char* postCode;
	char* message;

} feedbackInfo;

typedef struct {
	char* employeeID;
	char* employeePIN;
} punchInfo_s;

typedef struct {
	const unsigned char* employeeID;
	int employeePresent;
} returnedEmployee_s;

typedef enum {
	PI_ID,
	PI_PIN
} punchInfoFields_e;

typedef enum {
	FB_NAME,
	FB_EMAIL,
	FB_POSTCODE,
	FB_MESSAGE
} feedbackFields;

/* Function Prototypes */
void UnpackMessage(punchInfo_s* pi, char* recvBuf);

/* Function Prototypes -- sqlite_api.cpp */
void WriteFeedbackToDB(sqlite3* db, feedbackInfo* fb);
sqlite3* OpenDatabase(const char* path);
void WritePunchToDB(sqlite3* db, punchInfo_s* pi);