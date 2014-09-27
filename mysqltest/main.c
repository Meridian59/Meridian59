#include <database.h>
#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>

#define HOST	"hostname.domain.com"
#define USER	"username"
#define PASSWD	"password"
#define DB		"database"

#define SLEEPINIT		500
#define SLEEPWRITESHORT	5000
#define SLEEPWRITELONG	20000
#define SLEEPEND		1000

int _tmain(int argc, _TCHAR* argv[])
{	
	int i;
	
	MySQLInit(HOST, USER, PASSWD, DB);
	Sleep(SLEEPINIT);
		
	while (TRUE)
	{
		/************************** TEST 1 **************************************/
		/*						SINGLE WRITE									*/
		/************************************************************************/
		printf("Running Test 1: Write one of each record...\n");
		MySQLRecordTotalMoney(1000);
		MySQLRecordMoneyCreated(100);
		MySQLRecordPlayerLogin("test1acc", "test1char", "127.0.0.1");
		MySQLRecordPlayerAssessDamage("test1who", "test1attacker", 1, 2, 3, 4, "test1weapon");
		Sleep(SLEEPWRITESHORT);
		
		/************************** TEST 2 **************************************/
		/*						FILL UP QUEUE                                   */
		/************************************************************************/
		printf("Running Test 2: Fill up queue fast...\n");
		for(i = 0; i < 1000; i++)
		{
			MySQLRecordTotalMoney(2000);
			MySQLRecordMoneyCreated(200);
			MySQLRecordPlayerLogin("test2acc", "test2char", "172.24.5.1");
			MySQLRecordPlayerAssessDamage("test2who", "test2attacker", 1, 2, 3, 4, "test2weapon");
		}
		Sleep(SLEEPWRITELONG);
		
		/************************** TEST 3 **************************************/
		/*					MANY WRITES WITH WAITS								*/
		/************************************************************************/
		printf("Running Test 3: Lots of writes with waits...\n");
		for(i = 0; i < 1000; i++)
		{
			MySQLRecordTotalMoney(3000);
			MySQLRecordMoneyCreated(300);
			MySQLRecordPlayerLogin("test3acc", "test3char", "122.1.5.1");
			MySQLRecordPlayerAssessDamage("test3who", "test3attacker", 1, 2, 3, 4, "test3weapon");
			Sleep(50);
		}
		Sleep(SLEEPWRITELONG);
	}

	MySQLEnd();

	return 0;
}

