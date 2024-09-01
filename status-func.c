#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <strings.h>
#include <math.h>

char* displaytime(void);
char* display_battery_status(int,int*);
int   display_battery_int(void);
char* get_battery_life(int);

char*
displaytime(void)
{
	char* t = malloc(sizeof(char) * 25);
	time_t current_time;
	struct tm* time_info;
	char time_string[25] = {0};

	time(&current_time);

	time_info = localtime(&current_time);
	strftime(time_string,sizeof(char) * 25,"%d/%m/%Y %T", time_info);
	strcpy(t,time_string);


	return t;
}

int 
display_battery_int(void)
{
	char info[5];
	FILE* fp = fopen(
			"/sys/class/power_supply/BAT1/capacity","r");

	if (fgets(info, 4, fp) == NULL) {
			perror("Failed to read battery info");
			fclose(fp);
			return -1;
	}
	fclose(fp);

	return atoi(info);
}


char*
display_battery_status(int charge,int *mode)
{

	char status[12] = {0};
	char* info = malloc(sizeof(char) * 5);

	FILE* fp = fopen(
			"/sys/class/power_supply/BAT1/status","r");
	
	if (fgets(status,12,fp) == NULL) {
		perror("Failed to read battery status");
		fclose(fp);
		return NULL;
	}
	char symbol = 0;
	if(!strncmp(status,"Charging",8)) {
		symbol = '+';
		*mode = 1;
	}
	else if (!strncmp(status,"Discharging",11)) {
		symbol= '-';
		*mode = 0;
	}
	else {
		symbol = 'o';
		*mode = 0;
	}

	char* icon = NULL;
	switch(symbol) {
		case('-'):
        icon = 
            (charge >= 95)? "󰁹":
            (charge >= 85)? "󰂂":
            (charge >= 75)? "󰂁":
            (charge >= 65)? "󰂀":
            (charge >= 55)? "󰁿":
            (charge >= 45)? "󰁾":
            (charge >= 35)? "󰁽":
            (charge >= 25)? "󰁼":
            (charge >= 15)? "󰁻":
            (charge >= 10)? "󰁺":
            (charge < 10)?  "󱃍":
            "󰂑";
				break;
		case('+'):
        icon = 
            (charge >= 95)? "󰂅":
            (charge >= 85)? "󰂋":
            (charge >= 75)? "󰂊":
            (charge >= 65)? "󰢞":
            (charge >= 55)? "󰂉":
            (charge >= 45)? "󰢝":
            (charge >= 35)? "󰂈":
            (charge >= 25)? "󰂇":
            (charge >= 15)? "󰂆":
            (charge >= 10)? "󰢜":
            (charge < 10)?  "󰢟":
            "󰂑";
				break;
		case('o'):
        icon = 
            (charge >= 95)? "󰁹":
            (charge >= 85)? "󰂂":
            (charge >= 75)? "󰂁":
            (charge >= 65)? "󰂀":
            (charge >= 55)? "󰁿":
            (charge >= 45)? "󰁾":
            (charge >= 35)? "󰁽":
            (charge >= 25)? "󰁼":
            (charge >= 15)? "󰁻":
            (charge >= 10)? "󰁺":
            (charge < 10)?  "󱃍":
            "󰂑";
				break;
		default:
				icon = "?";
				break;
	}


	strncpy(info,icon,sizeof(char)*5);
	fclose(fp);
	

	return info;
}

char*
get_battery_life(int mode)
{
	char* info = malloc(sizeof(char) * 64);
	char buffer[256];
	long charge_now = 0, charge_full = 0, current_now = 0;

	   FILE* fp = fopen("/sys/class/power_supply/BAT1/charge_now", "r");
    if (fp == NULL) {
        perror("Error opening charge_now fp");
        return NULL;
    }
    fgets(buffer, sizeof(buffer), fp);
    charge_now = atoi(buffer);
    fclose(fp);

    // Read charge_full
    fp = fopen("/sys/class/power_supply/BAT1/charge_full", "r");
    if (fp == NULL) {
        perror("Error opening charge_full fp");
        return NULL;
    }
    fgets(buffer, sizeof(buffer), fp);
    charge_full = atoi(buffer);
    fclose(fp);

    // Read current_now
    fp = fopen("/sys/class/power_supply/BAT1/current_now", "r");
    if (fp == NULL) {
        perror("Error opening current_now fp");
        return NULL;
    }
    fgets(buffer, sizeof(buffer), fp);
    current_now = atoi(buffer);
    fclose(fp);

    if (mode == 0) {
        // Calculate estimated remaining battery life
        if (current_now != 0) {
            float seconds= (float)(charge_now * 3600) / current_now;
						int hours = floor(seconds/3600);
						int minutes = floor((float)((int)seconds % 3600)/60);
						if(hours >= 1)
							snprintf(info,64,"Remaining Battery Life: %d hours, %d minutes",
									hours,minutes);
						else
							snprintf(info,64,"Remaining Battery Life: %d minutes",
									(int)minutes);
        } else {
            snprintf(info,64,"Battery is not discharging\n");
        }
    } else if (mode == 1) {
        // Calculate charge time to full
        float remaining_charge = charge_full - charge_now;
        if (current_now != 0 && remaining_charge > 0) {
            float seconds = (remaining_charge * 3600) / current_now;
						int hours = floor(seconds/3600);
						int minutes = floor((float)((int)seconds % 3600)/60);
						if(hours >= 1)
							snprintf(info,64,"Time until Full Charge: %d hours, %d minutes",
									hours,minutes);
						else
							snprintf(info,64,"Time until Full Charge: %d minutes",minutes);
        } else {
            snprintf(info,64,"Battery is already fully charged\n");
        }
    } else {
        snprintf(info,64,"Invalid mode\n");
    }

	return info;
}

int
main(int argc, char** argv)
{
  if (argc < 2) {
		fprintf(stdout,"N/A");
		return 1;
	}

	if (!strcmp(argv[1],"--time")) {
		char *info =  displaytime();
    if(argc > 2)
      if(!strcmp(argv[2],"-i"))
      {
        fprintf(stdout," ");
        return 0;
      }
		fprintf(stdout,"%s",info);
		free(info);
	}

	else if(!strcmp(argv[1],"--battery")) {
		int charge = display_battery_int();
		int mode = 0;
    if(argc > 2)
    {
      if (!strcmp(argv[2],"-i"))
      {
        char* info = display_battery_status(charge,&mode);
        fprintf(stdout,"%s ",info);
        free(info);
        return 0;
      }
      else if(!strcmp(argv[2],"-c"))
      {
        display_battery_status(charge,&mode);
        char* info = get_battery_life(mode);
        fprintf(stdout,"%s",info);
        free(info);
        return 0;
      }
    }
		fprintf(stdout,"%d%% ",charge);
	}
	else {
		fprintf(stdout,"N/A");
		return 1;
	}

	return 0;
}
