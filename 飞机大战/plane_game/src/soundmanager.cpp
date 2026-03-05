#include "soundmanager.h"
#include <stdlib.h>
#include <stdio.h>
#include <easyx.h>
#include <time.h>
#include <stdint.h>
#include <Windows.h>

void soundPlay(struct soundManager *e)
{
	int currentTime = GetTickCount();
	int* pAlias = (int*)malloc(sizeof(int));
	if (pAlias == NULL)
		return;
	*pAlias = currentTime;
	e->vecSoundAlias.append(&e->vecSoundAlias, pAlias);

	char command[100];
	sprintf(command, "open %s alias %d", e->soundPath, currentTime);
	mciSendString(command, NULL, 0, NULL);
	sprintf(command, "play %d", currentTime);
	mciSendString(command, NULL, 0, NULL);
}

void soundClose(struct soundManager *e, int interval)
{
	int currentTime = GetTickCount();
	char command[100];
	for (int i = 0; i < e->vecSoundAlias.size; i++)
	{
		int* pAlias = (int*)e->vecSoundAlias.get(&e->vecSoundAlias, i);
		if (currentTime - *pAlias < interval)
			continue;
		sprintf(command, "close %d", *pAlias);
		mciSendString(command, NULL, 0, NULL);
		free(pAlias);
		e->vecSoundAlias.remove(&e->vecSoundAlias, i);
		i--;
	}
}

void soundManagerInit(struct soundManager*e, const char *soundPath)
{
	e->play = soundPlay;
	e->close = soundClose;

	vectorInit(&e->vecSoundAlias);
	strcpy(e->soundPath, soundPath);
}

void soundManagerDestroy(struct soundManager* e)
{
	char command[100];
	for (int i = 0; i < e->vecSoundAlias.size; i++)
	{
		int*pAlias = (int*)e->vecSoundAlias.get(&e->vecSoundAlias, i);
		sprintf(command, "close %d", *pAlias);
		mciSendString(command, NULL, 0, NULL);
		free(pAlias);
	}
	vectorDestroy(&e->vecSoundAlias);
}