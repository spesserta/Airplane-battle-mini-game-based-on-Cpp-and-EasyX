#include "mainscene.h"
#include "enemy.h"
#include "enemy0.h"
#include "enemy1.h"
#include "enemy2.h"
#include "bullet.h"
#include <stdio.h>

void mainSceneDraw(struct mainScene* s)
{
    //  清空绘制vector
    s->vecElements.clear(&s->vecElements);

    //  向绘图vector里添加元素
    //  背景
    s->vecElements.append(&s->vecElements, s->bk);
    //  敌机
    for (int i = 0; i < s->vecEnemy.size; i++)
    {
        struct enemy* pEmemy = (struct enemy *)s->vecEnemy.get(&s->vecEnemy, i);
        s->vecElements.append(&s->vecElements, pEmemy);
    }
    //  hero
    s->vecElements.append(&s->vecElements, s->hero);
    //  hero子弹
    for (int i = 0; i < s->vecBullets.size; i++)
    {
        struct bullet* pBullet = (struct bullet*)s->vecBullets.get(&s->vecBullets, i);
        s->vecElements.append(&s->vecElements, pBullet);
    }

    //  遍历所有图元开始绘制
    for (int i = 0; i < s->vecElements.size; i++)
    {
        struct sprite* pSprite = (struct sprite*)(s->vecElements.pData[i]);
        pSprite->draw(pSprite);
    }

    //  计分板
    char buff[10];
    sprintf(buff, "得分:%d", s->mark);
    outtextxy(0, 0, buff);
}

void generateNewEnemy(struct mainScene* s)
{
    //  出现各样式敌机的概率一共60份
    int r = rand() % 60;
    struct enemy* pEnemy = NULL;
    
    //  40/60出现enemy1
    if (0 <= r && r < 40)
    {
        struct enemy0* pEnemy0 = (struct enemy0*)malloc(sizeof(struct enemy0));
        if (pEnemy0 == NULL)
            return;
        enemy0Init(pEnemy0);
        pEnemy = (struct enemy*)pEnemy0;
    }
    //  15/60出现enemy2
    else if (40 <= r && r < 55)
    {
        struct enemy1* pEnemy1 = (struct enemy1*)malloc(sizeof(struct enemy1));
        if (pEnemy1 == NULL)
            return;
        enemy1Init(pEnemy1);
        pEnemy = (struct enemy*)pEnemy1;
    }
    //  5/60出现enemy3
    else if (55 <= r && r < 60)
    {
        struct enemy2* pEnemy2 = (struct enemy2*)malloc(sizeof(struct enemy2));
        if (pEnemy2 == NULL)
            return;
        enemy2Init(pEnemy2);
        pEnemy = (struct enemy*)pEnemy2;
    }

    //  随机新敌机位置
    int m, n;
    m = 20;
    n = 422 - 20;
    //  x坐标在区间【20，402】之内随机
    pEnemy->super.x = rand() % (n - m + 1) + m;

    //  y坐标
    if (pEnemy->enemyType == enemyType0)
        pEnemy->super.y = -26;
    else if (pEnemy->enemyType == enemyType1)
        pEnemy->super.y = -59;
    else if (pEnemy->enemyType == enemyType2)
        pEnemy->super.y = -162;

    s->vecEnemy.append(&s->vecEnemy, pEnemy);
}

void destroyInvalidEnemy(struct mainScene* s)
{
    //  检查并删除飞出窗体或状态为destroy的敌机
    for (int i = 0; i < s->vecEnemy.size; i++)
    {
        struct enemy* pEnemy = (struct enemy*)s->vecEnemy.get(&s->vecEnemy, i);

        if (pEnemy->super.y > 750 || pEnemy->status == enemy_destroy)
        {
            //  将击毁的敌机按照种类计分
            if (pEnemy->status == enemy_destroy)
            {
                switch (pEnemy->enemyType)
                {
                    case enemyType0: s->mark += 10; break;
                    case enemyType1: s->mark += 30; break;
                    case enemyType2: s->mark += 50; break;
                }
            }

            s->vecEnemy.remove(&s->vecEnemy, i);
            pEnemy->destroy(pEnemy);
            free(pEnemy);
            i--;
        }
    }
}

void generateBullet(struct mainScene* s)
{
    struct bullet* pBullet = (struct bullet*)malloc(sizeof(struct bullet));
    if (pBullet == NULL)
        return;

    bulletInit(pBullet);
    pBullet->super.x = s->hero->super.x + 32;
    pBullet->super.y = s->hero->super.y - 6;
    s->vecBullets.append(&s->vecBullets, pBullet);
}

void destroyInvalidBullet(struct mainScene* s)
{
    //  场景内是否有子弹
    if (s->vecBullets.size <= 0)
        return;

    //  检查第一个子弹是否飞出窗体
    struct bullet* pBullet = (struct bullet*)s->vecBullets.get(&s->vecBullets, 0);
    if (pBullet->super.y < -14)
    {
        s->vecBullets.remove(&s->vecBullets, 0);
        bulletDestroy(pBullet);
        free(pBullet);
    }
}

void bulletHitEnemyCheck(struct mainScene* s)
{
    //  碰撞检测
    for (int i = 0; i < s->vecBullets.size; i++)
    {
        //  将子弹抽象为头部的一个点
        struct bullet* pBullet = (struct bullet*)s->vecBullets.get(&s->vecBullets, i);
        POINT bulletPoint;
        bulletPoint.x = pBullet->super.x + 6 / 2;
        bulletPoint.y = pBullet->super.y;

        //  检查每一颗子弹是否碰撞到任意敌机
        for (int j = 0; j < s->vecEnemy.size; j++)
        {
            struct enemy* pEnemy = (struct enemy*)s->vecEnemy.get(&s->vecEnemy, j);
            
            //  敌机的宽度与高度
            int width, height;
            width = pEnemy->super.width;
            height = pEnemy->super.height;

            //  敌机矩形区域
            int left, top, right, bottom;
            left = pEnemy->super.x;
            top = pEnemy->super.y;
            right = left + width;
            bottom = top + height;

            //  检查子弹是否在矩形区域内
            if (bulletPoint.x > left && bulletPoint.x < right &&
                    bulletPoint.y > top && bulletPoint.y < bottom
            )
            {
                if (pEnemy->life != 0)
                {
                    //  子弹撞击到敌机后，销毁子弹
                    bulletDestroy(pBullet);
                    free(pBullet);
                    s->vecBullets.remove(&s->vecBullets, i);
                    i--;
                    //  敌机击中
                    pEnemy->hited(pEnemy);
                    if (pEnemy->life == 0)
                        s->enemyDownSoundMgr.play(&s->enemyDownSoundMgr);
                    break;
                }
            }
        }
    }
}

bool heroHitEnemyCheck(struct mainScene* s)
{
    //  hero矩形区域
    RECT rectHero;
    rectHero.left = s->hero->super.x + 16;
    rectHero.top = s->hero->super.y + 10;
    rectHero.right = s->hero->super.x + 16 * 3;
    rectHero.bottom = s->hero->super.y + 62;

    for (int i = 0; i < s->vecEnemy.size; i++)
    {
        struct enemy* pEnemy = (struct enemy*)s->vecEnemy.get(&s->vecEnemy, i);
        int enemyWidth = 0, enemyHeight = 0;
        if (pEnemy->status != enemy_normal)
            continue;

        //  敌机矩形区域
        RECT rectEnemy;
        rectEnemy.left = pEnemy->super.x;
        rectEnemy.top = pEnemy->super.y;
        rectEnemy.right = pEnemy->super.x + pEnemy->super.width;
        rectEnemy.bottom = pEnemy->super.y + pEnemy->super.height;

        //  两区域是否重叠
        if (rectHero.left <= rectEnemy.right && rectHero.right >= rectEnemy.left &&
            rectHero.top <= rectEnemy.bottom && rectHero.bottom >= rectEnemy.top)
        {
            if (s->hero->status == hero_normal0 || s->hero->status == hero_normal1)
                return true;
        }
    }
    return false;
}

void mainSceneUpdate(struct mainScene* s)
{
    //  update sprites
    for (int i = 0; i < s->vecElements.size; i++)
    {
        struct sprite* pSprite = (struct sprite*)s->vecElements.pData[i];
        pSprite->update(pSprite);
    }

    //  20分之一概率出现新敌机
    int n = rand() % 20;
    if (n == 0)
    {
        generateNewEnemy(s);
    }

    //  销毁无效敌机
    destroyInvalidEnemy(s);

    //  产生hero子弹  
    //  每15帧发射一次
    s->bulletGenCnt++;
    if (s->bulletGenCnt >= 15)
    {
        generateBullet(s);
        s->bulletGenCnt = 0;
    }

    //  销毁无效hero子弹
    destroyInvalidBullet(s);

    //  敌机击中碰撞检测
    bulletHitEnemyCheck(s);

    //  hero碰撞敌机检测
    bool isHited = heroHitEnemyCheck(s);
    if (isHited == true)
    {
        s->hero->life--;
        s->hero->status = hero_down1;
        //  播放英雄爆炸音乐
        mciSendString("open sounds/hero_down.wma", NULL, 0, NULL);
        mciSendString("play sounds/hero_down.wma", NULL, 0, NULL);
    }

    s->soundCloseCnt++;
    if (s->soundCloseCnt >= 60)
    {
        s->enemyDownSoundMgr.close(&s->enemyDownSoundMgr, 1000);
        s->soundCloseCnt = 0;
    }
}

void mainSceneControl(struct mainScene*s, ExMessage *msg)
{
    if (msg->message == WM_MOUSEMOVE)
    {
        s->hero->super.x = msg->x - 35;
        s->hero->super.y = msg->y - 35;
    }
}

bool mainSceneIsQuit(struct mainScene* s)
{
    //  游戏结束检查
    if (s->hero->status == hero_destroy)
        return true;
    return false;
}

void mainSceneInit(struct mainScene* s)
{
    s->super.draw = (void (*)(struct scene*))mainSceneDraw;
    s->super.update = (void (*)(struct scene*))mainSceneUpdate;
    s->super.control = (void (*)(struct scene*, ExMessage*))mainSceneControl;
    s->super.isQuit = (bool (*)(struct scene*))mainSceneIsQuit;

    s->hero = (struct hero *)malloc(sizeof(struct hero));
    heroInit(s->hero);

    s->bulletGenCnt = 0;
    s->soundCloseCnt = 0;
    
    s->bk = (struct background *)malloc(sizeof(struct background));
    backgroundInit(s->bk);

    vectorInit(&s->vecElements);
    vectorInit(&s->vecEnemy);
    vectorInit(&s->vecBullets);

    //  计分板
    s->mark = 0;
    settextcolor(WHITE);
    settextstyle(30, 0, "微软雅黑");
    setbkmode(TRANSPARENT);

    soundManagerInit(&s->enemyDownSoundMgr, "sounds/enemy_down.wma");

    //  播放背景音乐
    mciSendString("open sounds/background.wma", NULL, 0, NULL);
    mciSendString("play sounds/background.wma repeat", NULL, 0, NULL);
}

void mainSceneDestroy(struct mainScene* s)
{	
    //  停止背景音乐
    mciSendString("close sounds/background.wma", NULL, 0, NULL);
    //  停止英雄爆炸音乐
    mciSendString("close sounds/hero_down.wma", NULL, 0, NULL);

    soundManagerDestroy(&s->enemyDownSoundMgr);

    heroDestroy(s->hero);
    free(s->hero);

    backgroundDestroy(s->bk);
    free(s->bk);
    
    vectorDestroy(&s->vecElements);

    for (int i = 0; i < s->vecEnemy.size; i++)
    {
        struct enemy* pEnemy = (struct enemy *)s->vecEnemy.get(&s->vecEnemy, i);
        pEnemy->destroy(pEnemy);
        free(pEnemy);
    }
    vectorDestroy(&s->vecEnemy);
    
    
    for (int i = 0; i < s->vecBullets.size; i++)
    {
        struct bullet* pBullet = (struct bullet*)s->vecBullets.get(&s->vecBullets, i);
        bulletDestroy(pBullet);
        free(pBullet);
    }
    vectorDestroy(&s->vecBullets);
    
}