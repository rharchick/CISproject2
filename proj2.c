//CIS 340 - Project #2
//Authors - Samuel Turnage(ID:2579621)
//
//Decription - Provides a shell to look at the FAT tables and memory structure of a floppy disk

//include files
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

/*
This function converts a specified number of bytes (in little endian order) from a character array into a decimal equivalent value and returns that value as an unsigned int
fileInfo is the character array to read from, highestIndex is the location of the most significant byte you are targetting, numBytes is how many bytes long the value is, and the result of the calculation is stored in newValue
*/
unsigned int convertHexToDec(unsigned char fileInfo[], short highestIndex, short numBytes, unsigned int * newValue)
{
    short power = numBytes*2 - 1; //one character takes up 2 hex digits, with the least significant hex digit representing 16^0
    *newValue = 0; //dereference and clear newValue
    unsigned int upperHex, lowerHex, decValue = 0; //variables used for performing calculations
    short i,z; //loop counters
    for(z = highestIndex; z > (highestIndex - numBytes); --z)
    {
        decValue = (unsigned int)fileInfo[z]; //cast the character value to an unsigned int
        if (decValue == 0) //case of hex '00'
        {
            power = power - 2;  //two hex values per character
            continue;
        }
        upperHex = (decValue/16); //get upper 4 bits
        if (decValue > 15) //get lower 4 bits
            lowerHex = (decValue%16);
        else
            lowerHex = decValue;
        for (i = power; i > 0; --i) //serves as an exponent calculator using 16, executing 'power' number of times for the upper value and 'power-1' for the lower value
        {
            upperHex = upperHex*16;
            if ((i - 1) > 0)
                lowerHex = lowerHex*16;
        }
        *newValue = *newValue + upperHex + lowerHex; //accumulate newValue using the upper and lower hex values (now decimal totals)
        power = power - 2;  //two hex values per character
    }
    return *newValue; //return dereferenced newValue which should be an unsigned int value
}

/*
Mounts the floppy file or image to be used
*/
void fmount(char arg[],char *floppyName[])
{
    if(open(arg, O_RDONLY) != -1)//file opened
    {
        strcpy(*floppyName,arg);//stores file name in flops to allow for easy unmounting
        printf("\n%s was mounted\n",*floppyName);
    }
    else//error, file not opened
    {
        printf("\nFile %s could not be found\n", arg);
        return;
    }
}

/*
Unmounts the floppy
*/
void fumount(char * floppyName[])
{
    close(*floppyName);
    printf("\n%s was unmounted\n",*floppyName);
}

void structure()
{
    unsigned char bootSector[62] = {'\0'};
    unsigned int newDecValue = 0;
    
    lseek(3, 0, SEEK_SET); //seek to beginning of floppy
    read(3, bootSector, 62); //read in necessary characters from the boot sector
    printf("\n# of FAT: 			%d", bootSector[16]);
    newDecValue = convertHexToDec(bootSector, 23, 2, &newDecValue); //convert bytes 22 to 23 from chars to an integer total
    printf("\n# of sectors used by FAT: 	%d", newDecValue);
    printf("\n# of sectors per cluster: 	%d", bootSector[13]);
    newDecValue = convertHexToDec(bootSector, 18, 2, &newDecValue); //convert bytes 17 to 18 from chars to an integer total
    printf("\n# of ROOT Entries: 		%d", newDecValue);
    newDecValue = convertHexToDec(bootSector, 12, 2, &newDecValue); //convert bytes 11 to 12 from chars to an integer total
    printf("\n# of bytes per sector: 		%d\n\n", newDecValue);
    printf("Sector #   		Sector Types\n");
    printf("----------		----------\n");
    printf("0			BOOT\n");
    printf("01 -- 09		FAT1\n");
    printf("10 -- 18		FAT2\n");
    printf("19 -- 32		ROOT DIRECTORY\n");
}

/*
This function is used by traverse when the user specifies the '-l' flag for additional file info
function takes two args, one is a character array containing the root entry of the file and the other is also an array with the same info in unsigned int format
*/
void PrintMoreSectorInfo(unsigned int fileInfoDec[], char fileInfo[])
{
    //File type is displayed first
    char attributes[] = "-----";
    unsigned int attributeNum = fileInfoDec[11]; //get the attributes byte
    short i,z; //loop variables
    if (attributeNum > 15) //if upper 4 bits have a non-zero value, check for archive/subdir status
    {
        if (attributeNum < 32)//Subdirectory
            attributes[0] = 'D'; 
        else if (attributeNum < 48)//Archive
            attributes[1] = 'A'; 
    }
    if ((attributeNum % 16) == 0)
    {
        attributeNum = (attributeNum % 16);
        if ((attributeNum % 2) != 0) //if lowest bit is set,file is read-only
            attributes[2] = 'R';
        if (attributeNum == 2 || attributeNum == 3 || attributeNum == 6 || attributeNum == 7 || attributeNum == 10 || attributeNum == 11 || attributeNum >= 14) //if second-lowest bit is set
            attributes[3] = 'H'; //file is hidden
        if (attributeNum > 3 && attributeNum < 8 || attributeNum > 11) //if second-highest bit is set
            attributes[4] = 'S'; //system file
    }
    printf("%s  ", attributes); //display file attributes
    
    //DATE
    unsigned int year = 1980,monthNum = 0,dayNum = 0,monthDay,yearOffset = 0;
    //YEAR
    yearOffset = (fileInfoDec[17]/2); //will automatically cast off lowest bit, which is part of month anyway
    yearOffset = ((yearOffset/16)*10 + yearOffset%16); //convert decimal yearOffset to hex numeric equivalent for adding
    year = year + yearOffset;
    //MONTH
    monthDay = fileInfoDec[16]; //lower byte is used for month and day calculation
    monthDay = (monthDay - (monthDay%32)); //cast off the lower 5 bits
    if (monthDay == 32 || monthDay == 96 || monthDay == 160 || monthDay == 224)monthNum++; //lowest bit set
    if (monthDay == 64 || monthDay == 96 || monthDay == 192 || monthDay == 224)monthNum = monthNum + 2; //second lowest bit sit
    if (monthDay > 128) monthNum = monthNum + 4; //highest bit set
    if(fileInfoDec[17] % 2 != 0) monthNum = monthNum + 8;//if there is a remainder of 1, then the lowest bit is set and indicates that the highest bit is set in the month portion
    //DAY
    monthDay = fileInfoDec[16]; //restore monthDay
    dayNum = (((monthDay/16)*10) + (monthDay%16)); //convert dayNum to hex
    if (dayNum > 32) //day must be 31 or less, if it is not already then the remainder is the correct day
        dayNum = dayNum % 32;
    printf("%d/%d/%d \t", monthNum, dayNum, year); //print date
    //TIME
    unsigned int hourMin = fileInfoDec[15]; //upper byte used for hour/minute calculation
    unsigned int minSec = fileInfoDec[14]; //lower byte used for minute/second calculation
    //HOUR
    hourMin = (hourMin/8); //cast off lower 3 bits to obtain hours
    printf("%02d:", hourMin);
    //MINUTE
    hourMin = fileInfoDec[15]; //restore hourMin
    minSec = (hourMin%8)*8 + (minSec/32); //raise lower 3 bits of hourMin to be upper 3 bits, while upper 3 bits of minSec become lower 3 bits
    printf("%02d:", minSec);
    //SECONDS
    minSec = fileInfoDec[14]; //restore minSec
    minSec = (minSec%32); //cast off upper 3 bits to obtain seconds
    printf("%02d \t", minSec);
    
    //File Size
    unsigned int newDecValue = 0; 
    newDecValue = convertHexToDec(fileInfo, 31, 4, &newDecValue); //convert bytes 28 to 31 from chars to an integer total
    printf("%d\t", newDecValue);
    //First Sector
    newDecValue = convertHexToDec(fileInfo, 27, 2, &newDecValue); //convert bytes 26 to 27 from chars to an integer total
    printf("%d\t", newDecValue);
}

void traverse(short flag)
{
    short sector,file,a; //variables used for loop counters
    unsigned char fileInfo[32] = {'\0'}; //char array to store a root entry at a time
    unsigned int fileInfoDec[32] = {'\0'}; //int array will store decimal equivalents of fileInfo characters
    
    if (flag == 1) {
        printf("*****************************\n");
        printf("** FILE ATTRIBUTE NOTATION **\n");
        printf("**                         **\n");
        printf("** R ------ READ ONLY FILE **\n");
        printf("** S ------ SYSTEM FILE    **\n");
        printf("** H ------ HIDDEN FILE    **\n");
        printf("** A ------ ARCHIVE FILE   **\n");
        printf("*****************************\n");
        printf("ATTRIB\tDATE\t\tTIME\t\tSIZE\tSECTOR\tNAME");
    }
    
    for (sector = 19; sector < 33; ++sector) //reads through sectors 19-32 (root sectors)
    {
        lseek(3, 512*sector, SEEK_SET); //changes start position to beginning of sector 'a'
        for (file = 0; file < 16; ++file) //16 file entries per sector
        {
            lseek(3, (512*sector + file*32), SEEK_SET); //start position is set to beginning of current sector plus y*32 bytes
            read(3, fileInfo, 32); //reads 32 bytes into file info (FAT12 file systems allocate exactly 32 bytes per root entry)
            if (fileInfo[0] == '\0') //null file entry condition for now
                continue;
            for (a = 0; a < 32; ++a) //this for loop casts characters to integers so that certain bits in the file entry can be evaluated properly
                fileInfoDec[a] = (unsigned int)fileInfo[a];
            if (fileInfoDec[11] == 15) //IF BIT 11 OF FILE ENTRY IN ROOT IS x0F hex(15 decimal), THEN IT CAN BE IGNORED FOR THIS ASSIGNMENT
                continue;
            printf("\n/");
            if (flag == 1) //extended output flag will result in longTraverse function call
                PrintMoreSectorInfo(fileInfoDec, fileInfo);
            //DIRECTORY
            if (fileInfo[11] == 16) //IF ENTRY IS A DIRECTORY
            {
                for (a = 0; a < 11; ++a)
                    printf("%c", fileInfo[a]);
                printf("\t<DIR>");
            }
            //REGULAR FILE
            else
            {
                for (a = 0; a < 11; ++a)
                {
                    if(fileInfo[a]!=' ')printf("%c", fileInfo[a]);
                    if (a == 7) //last 3 characters will be the extension, 'appending' a dot after a filename for stylization
                        printf(".");
                }
            }
        }
    }
    printf("\n");
}

void showfat(char b[])
{
    short a = atoi(b); //cast argument to an integer
    short i; //variable to be used as loop counter
    short startSector, endSector = 0; //will indicate which fat sectors to read between
    char buffer[512] = {'\0'}; //buffer stores a sector at a time
    
    if(a == 1) //1st fat table, sectors 1-9
    {
        startSector = 1;
        endSector = 9;
        printf("\n\nFAT Table 1:\n");
    }
    else if (a == 2) //2nd fat table, sectors 10-18
    {
        startSector = 10;
        endSector = 18;
        printf("\n\nFAT Table 2:\n");
    }
    else //if not specified, both are printed, 1-18
    {
        startSector = 1;
        endSector = 18;
        printf("\n\nFAT table 1 and 2: \n");
    }
    
    for(a = startSector; a <= endSector; ++a)
    {
    	printf("Sector : %d",a);
    	printf("         0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f");
        lseek(3, 512*a, SEEK_SET); //changes to the start position of sector 'a' (512 bytes per sector)
        read(3, buffer, 512); //reads 512 bytes (the entire sector) into the buffer
        printf("\n ");
        for(i = 0; i < 512; ++i) //prints each character read in as hex values, rows of 16
        {
            if(i % 16 == 0)
                printf("\n  %03X  ",i);
            printf("%03X ", (unsigned char)buffer[i]); //casts to unsigned to avoid leading F's in certain characters
        }
    }
    printf("\n");
}

void showsector(char arg[])
{
    int sectorNum = atoi(arg); //cast arg to an int
    if(sectorNum >= 0 && sectorNum < 2880) //if the argument is a valid floppy sector
    {
        printf("\nSector : %s\n", arg);
        char buffer[512] = {'\0'};
        sectorNum = atoi(arg); //convert the user argument to an int
        lseek(3, 512*sectorNum, SEEK_SET); //reposition file reader to correct sector
        read(3, buffer, 512); //read 512 bytes (the full sector) into the character buffer
        printf("\n       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
        int i;
        for(i = 0; i < 512; ++i) //prints each character read in as hex values, rows of 16
        {
            if(i % 16 == 0) //print row number
            {
                printf("\n  %03X  ", i);
            }
            printf("%02X ", (unsigned char)buffer[i]); //casts to unsigned to avoid leading F's in certain characters
        }
    }
    else
    {
        printf("\nSector must be between 0 and 2879\n");
    }
    printf("\n");
}
int main()
{
	const char prompt[] = {"\n:flop: "};
	char *flopname[50];
	short i, j, isQuitting = 0; //loop and sentinel variables
	short mounted = 0; //indicates if a floppy has been mounted
	
	//While loop will continue to prompt user to enter commands; loop is broken upon receiving the quit command
	while (isQuitting == 0)
	{
		char words[10][20];
		char *arg1 = malloc(30); //used for storing arguments that users pass to commands
		short flagValue = 0;
		char input[50] = {'\0'}; //used for storing user input commands
		char command[30] = {'\0'};
		char filename[30] = {'\0'};
		char flagcommand[30] = {'\0'};
		
		i = 0;
		printf(prompt);
		fgets(input, 50, stdin);
		for(j = 0; input[j] != '\0' && j<50; j++)//Cast input to lowercase. Avoids complication with lower-case and upper-case arguments
			input[j] = tolower(input[j]);
		
		char* token;
		const char s[2] = " \n";
		   
		/* get the first token */
		token = strtok(input, s);
		strcpy(words[i],token);
		/* walk through other tokens */
		while( token != NULL ) 
		{
			i++;
			token = strtok(NULL, s);
			if(token != NULL)
				strcpy(words[i],token);
		}
		strcpy(words[i], "");

		strcpy(command, words[0]);//Copies the first input into command, this is the most important one
		i = 1; //starts at second word
		while(strcmp(words[i],"") != 0)//as long as THEY ARE NOT EQUAL
		{
			if(strcmp(words[i],"-l") == 0)//word is a flag....
				flagValue = 1;
			else //means its a word...more case checks!!!
			{
				if(flagValue == 1)// this word comes after the flag...
					strcpy(flagcommand,words[i]);
				strcpy(arg1,words[i]);
			}
			i++;
		}
		//Basically serves as a 'switch' statement to check the command entered against the valid list of commands
		if (strcmp(command,"help") == 0) 
		{
			printf("Commands: \n\n ");
			printf("help 			- Displays useable commands \n ");
			printf("fmount (Argument) 	- Mount a floppy disk (argument) \n ");
			printf("fumount 		- Unmount the mounted floppy disk \n ");
			printf("structure 		- Display the structure of a floppy disk \n ");
			printf("traverse (-l) 		- Display the root directory contents (with the option to show more info) \n ");
			printf("showfat 		- Display content of the FAT tables \n ");
			printf("showsector (sector #) 	- Show content of the sector \n ");
			printf("quit 			- Quit out of the floppy shell \n");
		}
		else if (strcmp(command,"fmount") == 0)
		{
			mounted = 1;
			fmount(arg1,flopname);
		}
		else if (strcmp(command,"fumount") == 0)
		{
			if(mounted == 1)
			{
				fumount(flopname);
				mounted = 0;//no disc mounted
			}
			else
				printf("\nNo Floppy mounted\n");
		}
		else if (strcmp(command,"quit") == 0)
		{
			if(mounted==1)
				fumount(flopname);
			isQuitting = 1;
		}
		else if (mounted == 0) //all commands below this point require a mounted floppy to execute; this prevents reading from an empty fd
			printf("\nThe command '%s' is invalid or you have no floppy disk mounted. Type 'help' for a list of commands.", command);
		else if (strcmp(command,"structure") == 0)
			structure();
		else if (strcmp(command,"traverse") == 0)
			traverse(flagValue);
		else if (strcmp(command,"showfat") == 0)
			showfat(arg1);
		else if (strcmp(command,"showsector") == 0)
			showsector(arg1);
		else
			printf("\nThe command '%s' is invalid. Type 'help' for a list of commands.", command);

	}
	return 0;
}

