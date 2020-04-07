/*
* Abstract some of the sqlite functionality & keeping main clean. 
* Eric Fryters
* March 30 2020
*/

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "../NetDev_Server/sqlite3.h"
#include "../NetDev_Server/Server.h"


/*
The life-cycle of a prepared statement object usually goes like this:

1. Create the prepared statement object using sqlite3_prepare_v2().
2. Bind values to parameters using the sqlite3_bind_*() interfaces.
3. Run the SQL by calling sqlite3_step() one or more times.
4. Reset the prepared statement using sqlite3_reset() then go back to step 2. Do this zero or more times.
5. Destroy the object using sqlite3_finalize().


*/

void WriteFeedbackToDB(sqlite3* db, feedbackInfo* fb)
{
	const char* sql_str = "INSERT INTO feedback (userName, userEmail, postCode, feedbackMsg) VALUES (:userName, :userEmail, :postCode, :feedbackMsg)";

	// Create the statement object
	sqlite3_stmt *stmt = NULL;
	sqlite3_prepare_v2(db, sql_str, 512, &stmt, NULL);

	// Bind values to the statement object
	sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, ":userName"), fb->name, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, ":userEmail"), fb->email, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, ":postCode"), fb->postCode, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, ":feedbackMsg"), fb->message, -1, SQLITE_TRANSIENT);

	// Run SQL
	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
		//Something went wrong
		puts("[SQL] Did not complete transaction.");
	}

	// Destoy the object
	sqlite3_finalize(stmt);


}

void WritePunchToDB(sqlite3* db, punchInfo_s* pi)
{
	// Shorten bind parameter function
	typedef int (*bind_param_f)(sqlite3_stmt*, const char*);
	bind_param_f bp_idx = &sqlite3_bind_parameter_index;

	int status = 0;
	const char* sql_select = "SELECT pin, present FROM employees WHERE id = :id";

	returnedEmployee_s re = {0};

	// First query the db to see if ID matches their pin or exists
	sqlite3_stmt* stmt = NULL;
	sqlite3_prepare_v2(db, sql_select, -1, &stmt, NULL);
	sqlite3_bind_text(stmt, bp_idx(stmt, ":id"), pi->employeeID, -1, SQLITE_TRANSIENT);

	status = sqlite3_step(stmt);
	while (status == SQLITE_ROW)
	{
		// Got a row. Process row. We should only see a single row in this case as ID unique.
		re.employeeID = sqlite3_column_text(stmt, 0);			// pin
		re.employeePresent = sqlite3_column_int(stmt, 1);		// present status

		// Step to next row
		status = sqlite3_step(stmt);
	}
	if (status != SQLITE_DONE)
	{
		puts("[SQL] Error returning rows.");
	}

	sqlite3_finalize(stmt);

	// Determine wether they are punching in or out based on the status of the present flag on employee

	// If yes, then build the query to add to the DB, taking system time as clock

	// Then update the present flag on the employee 

}

sqlite3* OpenDatabase(const char* path)
{

	return nullptr;
}
