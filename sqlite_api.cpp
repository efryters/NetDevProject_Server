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
#include <time.h>

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

#define PIN_BUF_SIZE 16

int WritePunchToDB(sqlite3* db, punchInfo_s* pi)
{
	time_t now = time(NULL);
	// Shorten bind indx parameter function
	typedef int (*bind_param_f)(sqlite3_stmt*, const char*);
	bind_param_f bp_idx = &sqlite3_bind_parameter_index;

	int status = 0;
	int rows = 0;
	const char* sql_select = "SELECT pin, present FROM employees WHERE id = :id";
	char* buf = NULL;

	returnedEmployee_s re = {0};

	// First query the db to see if ID matches their pin or exists
	sqlite3_stmt* stmt = NULL;
	sqlite3_prepare_v2(db, sql_select, -1, &stmt, NULL);
	sqlite3_bind_text(stmt, bp_idx(stmt, ":id"), pi->employeeID, -1, SQLITE_STATIC);

	status = sqlite3_step(stmt);
	while (status == SQLITE_ROW)
	{
		// Got a row. Process row. We should only see a single row in this case as ID unique.
		buf = (char*)malloc(sizeof(char)* PIN_BUF_SIZE);
		strcpy_s(buf, PIN_BUF_SIZE, (const char*) sqlite3_column_text(stmt, 0));
		re.employeePIN = buf;			// pin
		re.employeePresent = sqlite3_column_int(stmt, 1);		// present status

		// Step to next row
		rows++;
		status = sqlite3_step(stmt);
	}

	sqlite3_finalize(stmt);

	if (status != SQLITE_DONE)
	{
		puts("[SQL] Error returning rows.");
		return SQL_INTERNAL_ERROR;
	}

	if (rows == 0)
	{
		return SQL_INVALID_EMPLOYEE;
	}

	if (0 != strcmp((const char*)re.employeePIN, pi->employeePIN))
	{
		free(buf);
		return SQL_INVALID_PIN;
	}
	// Determine wether they are punching in or out based on the status of the present flag on employee

	/*
		sqlite3_prepare_v2(db, sql_select, -1, &stmt, NULL);
	sqlite3_bind_text(stmt, bp_idx(stmt, ":id"), pi->employeeID, -1, SQLITE_STATIC);

	status = sqlite3_step(stmt);
	
	*/
	if (re.employeePresent)
	{
		// employee present, punch out... UPDATE statement 
		// First SELECT id from punchData WHERE timeOut is NULL AND employee = 5000;
		// Then UPDATE punchData SET timeOut=12345 WHERE id = 1;
		
		int id = 0;

		// Get the ID of the last punch data without punchout time
		const char* sql_get_id = "SELECT id from punchData WHERE timeOut is NULL AND employee = :employeeID";
		sqlite3_prepare_v2(db, sql_get_id, -1, &stmt, NULL);
		sqlite3_bind_text(stmt, bp_idx(stmt, ":employeeID"), pi->employeeID, -1, SQLITE_STATIC);
		status = sqlite3_step(stmt);
		while (status == SQLITE_ROW)
		{
			id = sqlite3_column_int(stmt, 0);
			status = sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);

		// Use the ID to update that row
		const char* sql_update = "UPDATE punchData SET timeOut= :timeNow WHERE id = :id";

		sqlite3_prepare_v2(db, sql_update, -1, &stmt, NULL);
		sqlite3_bind_int(stmt, bp_idx(stmt, ":id"), id);
		sqlite3_bind_int64(stmt, bp_idx(stmt, ":timeNow"), now);
		status = sqlite3_step(stmt);
		sqlite3_finalize(stmt);

	}
	else
	{
		// not present, punch in... INSERT statement
		const char* sql_insert = "INSERT INTO punchData (\"employee\", \"timeIn\") VALUES(:employeeID, :timeNow)";
		sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
		sqlite3_bind_text(stmt, bp_idx(stmt, ":employeeID"), pi->employeeID, -1, SQLITE_STATIC);
		sqlite3_bind_int64(stmt, bp_idx(stmt, ":timeNow"), now);
		status = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}
		// Toggle employee present status
	const char* sql_toggle_present = "UPDATE employees SET present= :present WHERE id = :employeeID";
	sqlite3_prepare_v2(db, sql_toggle_present, -1, &stmt, NULL);
	sqlite3_bind_int(stmt, bp_idx(stmt, ":present"), !(bool)re.employeePresent);
	sqlite3_bind_text(stmt, bp_idx(stmt, ":employeeID"), pi->employeeID, -1, SQLITE_STATIC);
	status = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	// Then update the present flag on the employee 

	free(buf);
	return SQL_OK;

}

sqlite3* OpenDatabase(const char* path)
{

	return nullptr;
}
