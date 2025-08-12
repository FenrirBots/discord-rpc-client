#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "discord_rpc.h"


int main(
  int    argc,
  char **argv)
{
	char               *appid = NULL;
	DiscordRichPresence presence
	  = { 0 };

	config_init("config.json");

	if (!config_load())
	{
		goto end;
	}

	if (!config_validate())
	{
	  printf("Validation failed.\n");
	}

	DiscordEventHandlers events
		= { 0 };
	events.ready        = NULL;
	events.disconnected = NULL;
	events.errored      = NULL;
	events.joinGame     = NULL;
	events.spectateGame = NULL;
	events.joinRequest  = NULL;

	appid = config_get_secret("application-id");
	Discord_Initialize(appid, &events, 1, NULL);
	free(appid);

	char *privacy = config_get_string("privacy");	
	presence.partyPrivacy     = DISCORD_PARTY_PUBLIC;

	if (strnicmp(privacy, "private", 7))
		presence.partyPrivacy   = DISCORD_PARTY_PRIVATE;

	presence.state            = config_get_string("state");
	presence.startTimestamp   = config_get_number("timestamp-start");
	presence.endTimestamp     = config_get_number("timestamp-end");
	presence.largeImageKey    = config_get_string("image-large");
	presence.smallImageKey    = config_get_string("image-small");
	presence.partyId          = config_get_string("party-id");
	presence.partySize        = config_get_number("party-current");
	presence.partyMax         = config_get_number("party-maximum");
	presence.matchSecret      = config_get_string("match");
	presence.joinSecret       = config_get_string("join");
	presence.spectateSecret   = config_get_string("spectate");
	presence.instance         = 0;
	presence.buttons[0].label = config_get_button(0, "message");
	presence.buttons[0].url   = config_get_button(0, "uri");
	presence.buttons[1].label = config_get_button(1, "message");
	presence.buttons[1].url   = config_get_button(1, "uri");
	Discord_UpdatePresence(&presence);

  while (1)
  {
		Sleep(250);
  }
end:
  return 0;
}