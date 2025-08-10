#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "discord_rpc.h"


int main(
  int    argc,
  char **argv)
{
  /* Temporary code until i get a proper configuration working... */
  FILE *file;
  long  length;
  char  appid[32];

  file   = fopen("secrets.txt", "r+");
  length = 0;
	memset(appid, 0, 32);
  
  if (file == NULL)
  {
		file = fopen("secrets.txt", "w+");
		if (file == NULL)
		{
			printf("Could not create the file. (0x%x)\n", errno);
			goto end;
		}

		printf("The secrets.txt file has been created\nput your appid in there and run this program again.\n", errno);
		fclose(file);
    goto end;
  }

  fseek(file, 0, SEEK_END);
  length = ftell(file);
  fseek(file, 0, SEEK_SET);
	
  fread(appid, sizeof(char), length, file);
	printf("%i\n", length);
	printf("%s\n", appid);
	fclose(file);

	DiscordEventHandlers events
		= { 0 };
	events.ready        = NULL;
	events.disconnected = NULL;
	events.errored      = NULL;
	events.joinGame     = NULL;
	events.spectateGame = NULL;
	events.joinRequest  = NULL;
	Discord_Initialize(appid, &events, 1, NULL);

	DiscordButton       buttons[2]
	  = { 0 };
	DiscordRichPresence presence
	  = { 0 };

  /* Temporary code until i get a proper configuration working... */
  buttons[0].label = "Website";
	buttons[0].url   = "https://fenrirbots.neocities.org/";
	buttons[1].label = "Github";
  buttons[1].url   = "https://github.com/FenrirBots";

	presence.state          = "Questioning life...";
	presence.startTimestamp = 0;
	presence.endTimestamp   = 0;
	presence.largeImageKey  = "mahiro";
	presence.smallImageKey  = "";
	presence.partyId        = "cafe";
	presence.partySize      = 1;
	presence.partyMax       = 4;
	presence.partyPrivacy   = DISCORD_PARTY_PUBLIC;
	presence.matchSecret    = "000003e8-73df-21f0-b900-325096b39f47";
	presence.joinSecret     = "000003e8-73df-21f0-8000-325096b39f47";
	presence.spectateSecret = "000003e8-73df-21f0-9200-325096b39f47";
	presence.instance       = 0;
	presence.buttons[0]     = buttons[0];
	presence.buttons[1]     = buttons[1];
	Discord_UpdatePresence(&presence);


  while (1)
  {

  }
end:
  return 0;
}