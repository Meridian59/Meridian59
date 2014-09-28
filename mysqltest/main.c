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
		/************************************************************************/
		/*							TEST 1										*/
		/************************************************************************/
		printf("-------------------------------------------\n");
		printf("    Test 1: Write one of each record...    \n");
		printf("-------------------------------------------\n");

		if (!MySQLRecordTotalMoney(1000))
			printf("T1:1\t - TotalMoney\n");
		else 
			printf("T1:1\t + TotalMoney\n");

		if (!MySQLRecordMoneyCreated(100))
			printf("T1:1\t - MoneyCreated\n");
		else
			printf("T1:1\t + MoneyCreated\n");

		if (!MySQLRecordPlayerLogin("test1acc", "test1char", "127.0.0.1"))
			printf("T1:1\t - PlayerLogin\n");
		else
			printf("T1:1\t + PlayerLogin\n");

		if (!MySQLRecordPlayerAssessDamage("test1who", "test1attacker", 1, 2, 3, 4, "test1weapon"))
			printf("T1:1\t - PlayerAssessDamage\n");
		else
			printf("T1:1\t + PlayerAssessDamage\n");
		
		Sleep(SLEEPWRITESHORT);
		
		/************************************************************************/
		/*								TEST 2                                  */
		/************************************************************************/
		printf("-------------------------------------------\n");
		printf("     Test 2: Hit queue limit...            \n");
		printf("-------------------------------------------\n");
		
		for(i = 0; i < 1000; i++)
		{
			if (!MySQLRecordTotalMoney(2000))
				printf("T2:%i\t - TotalMoney\n", i);
			else 
				printf("T2:%i\t + TotalMoney\n", i);

			if (!MySQLRecordMoneyCreated(200))
				printf("T2:%i\t - MoneyCreated\n", i);
			else
				printf("T2:%i\t + MoneyCreated\n", i);

			if (!MySQLRecordPlayerLogin("test2acc", "test2char", "172.24.5.1"))
				printf("T2:%i\t - PlayerLogin\n", i);
			else
				printf("T2:%i\t + PlayerLogin\n", i);

			if (!MySQLRecordPlayerAssessDamage("test2who", "test2attacker", 1, 2, 3, 4, "test2weapon"))
				printf("T2:%i\t - PlayerAssessDamage\n", i);
			else
				printf("T2:%i\t + PlayerAssessDamage\n", i);
		}
		
		Sleep(SLEEPWRITELONG);
		
		/************************************************************************/
		/*								TEST 3                                  */
		/************************************************************************/
		printf("-------------------------------------------\n");
		printf("    Test 3: Writes with 100ms waits...     \n");
		printf("-------------------------------------------\n");
		
		for(i = 0; i < 1000; i++)
		{
			if(!MySQLRecordTotalMoney(3000))
				printf("T3:%i\t - TotalMoney\n", i);
			else 
				printf("T3:%i\t + TotalMoney\n", i);

			if (!MySQLRecordMoneyCreated(300))
				printf("T3:%i\t - MoneyCreated\n", i);
			else
				printf("T3:%i\t + MoneyCreated\n", i);

			if (!MySQLRecordPlayerLogin("test3acc", "test3char", "122.1.5.1"))
				printf("T3:%i\t - PlayerLogin\n", i);
			else
				printf("T3:%i\t + PlayerLogin\n", i);

			if (!MySQLRecordPlayerAssessDamage("test3who", "test3attacker", 1, 2, 3, 4, "test3weapon"))
				printf("T3:%i\t - PlayerAssessDamage\n", i);
			else
				printf("T3:%i\t + PlayerAssessDamage\n", i);

			Sleep(100);
		}
		
		Sleep(SLEEPWRITELONG);
	}

	MySQLEnd();

	return 0;
}

