#pragma once

#include "scene.h"
#include "background.h"
#include "hero.h"
#include "vector.h"
#include "soundmanager.h"

struct mainScene {
    struct scene super;
    hero *hero;
    background *bk;
    vector vecElements;
    vector vecEnemy;
    vector vecBullets;
    int bulletGenCnt;
    int mark;
    soundManager enemyDownSoundMgr;
    int soundCloseCnt;
};

void mainSceneInit(struct mainScene* s);
void mainSceneDestroy(struct mainScene* s);
