#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "discord_rpc.h"


int main(
  int    argc,
  char **argv)
{
	char *appid      = NULL;
	int   retry_load = 0;

	config_init("config.json");

reload:
	if (!config_load())
	{
		printf("attempting to save defaults.\n");
		config_save_defaults();

		if (retry_load)
		{
			printf("the file 'config.json' could not be loaded.\n");
			goto end;
		}

		retry_load = 1;
		goto reload;
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

	DiscordRichPresence presence
	  = { 0 };

	presence.state            = config_get_string("state");
	presence.startTimestamp   = config_get_number("timestamp-start");
	presence.endTimestamp     = config_get_number("timestamp-end");
	presence.largeImageKey    = config_get_string("image-large");
	presence.smallImageKey    = config_get_string("image-small");
	presence.partyId          = config_get_string("party-id");
	presence.partySize        = config_get_number("party-current");
	presence.partyMax         = config_get_number("party-maximum");
	presence.partyPrivacy     = DISCORD_PARTY_PUBLIC; // TODO: This
	presence.matchSecret      = config_get_string("match");
	presence.joinSecret       = config_get_string("join");
	presence.spectateSecret   = config_get_string("spectate");
	presence.instance         = 0;
	presence.buttons[0].label = config_get_button(0, "message");
	presence.buttons[0].url   = config_get_button(0, "uri");
	presence.buttons[1].label = config_get_button(1, "message");
	presence.buttons[1].url   = config_get_button(1, "uri");
	Discord_UpdatePresence(&presence);

	// free(presence.spectateSecret);
	// free(presence.joinSecret);
	// free(presence.matchSecret);
	// free(presence.partyId);
	// free(presence.smallImageKey);
	// free(presence.largeImageKey);
	// free(presence.state);

  while (1)
  {

  }
end:
  return 0;
}