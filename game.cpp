/*==============================================================
  character movement testing using Fly2

  - Load a scene
  - Generate a terrain object
  - Load a character
  - Control a character to move
  - Change poses

  (C)2012 Chuan-Chang Wang, All Rights Reserved
  Created : 0802, 2012

  Last Updated : 1010, 2014, Kevin C. Wang
 ===============================================================*/
#include "FlyWin32.h"
#define _USE_MATH_DEFINES 
#include <cmath>

#define MAX_HP 100

class MyCharacter : public FnCharacter {
public:
	MyCharacter(int blood1=0){
		blood=blood1;
		fullBlood=blood1;
		frame=0;
		bloodBarID = FAILED_ID;
	}
	void setBB(FnScene scene){
		OBJECTid bbID = scene.CreateObject(OBJECT);
		FnObject bb(bbID);
		bb.SetParent(GetBaseObject());
		bb.Translate(0.0f, 0.0f, 95.0f, REPLACE);
		float size[2], color[4];

	    size[0] = 25.0f;
		size[1] = 1.2f;
		color[0] = color[3] = 1.0f; color[1] = color[2] = 0.0f;
		bloodBarID = bb.Billboard(NULL, size, NULL, 0, color);
	}
	void setBBA(FnScene scene){
		OBJECTid bbID = scene.CreateObject(OBJECT);
		FnObject bb(bbID);
		bb.SetParent(GetBaseObject());
		bb.Translate(0.0f, 0.0f, 95.0f, REPLACE);
		float size[2], color[4];

	    size[0] = 25.0f;
		size[1] = 1.2f;
		color[0] = color[3] = 0.0f; color[1] =1.0f;
		color[2] = 0.0f;
		bloodBarID = bb.Billboard(NULL, size, NULL, 0, color);
	}

	void BB(){
		FnBillboard bb(bloodBarID);
		float newSize[2];
		newSize[0]=25.0f*blood/fullBlood;
		newSize[1]=1.2f;
		bb.SetPositionSize(NULL, newSize);
	}
	void turnTo(MyCharacter actor) {
		
		float actPos[3], myPos[3], actfDir[3], actuDir[3], myfDir[3], myuDir[3];

		actor.GetPosition(actPos);
		actor.GetDirection(actfDir, actuDir);
		GetPosition(myPos);
		GetDirection(myfDir, myuDir);
		
		double dot = (myfDir[0]*(actPos[0]-myPos[0])+myfDir[1]*(actPos[1]-myPos[1]));
		double cosTheta = dot/((sqrt(pow(myfDir[0], 2)+pow(myfDir[1], 2)))*(sqrt(pow(actPos[0]-myPos[0], 2)+pow(actPos[1]-myPos[1], 2))));
		double theta1 = acos(cosTheta)/3.14*180;

		double cross = myfDir[0]*(actPos[1]-myPos[1])-myfDir[1]*(actPos[0]-myPos[0]);
		double sinTheta = cross/((sqrt(pow(myfDir[0], 2)+pow(myfDir[1], 2)))*(sqrt(pow(actPos[0]-myPos[0], 2)+pow(actPos[1]-myPos[1], 2))));
		double theta2 = asin(sinTheta)/3.14*180;
		double theta;
		if(theta1*theta2>0) 
			theta=-theta1;
		else
			theta=theta1;

		if(theta > 2 || theta<-2) {
			TurnRight(theta);
		}
	}
	int blood, fullBlood;
	int frame;
	GEOMETRYid bloodBarID;
};


VIEWPORTid vID;                 // the major viewport
SCENEid sID;                    // the 3D scene
OBJECTid cID, tID, oID;         // the main camera and the terrain for terrain following
CHARACTERid actorID, npcaID, npcbID; // the major character

ACTIONid idleID, runID, dieID, combatIdleID, guardID, curPoseID; // actor actions
ACTIONid normalAttack1ID, normalAttack2ID, normalAttack3ID, normalAttack4ID; // actor normal attacks
ACTIONid heavyAttack1ID, heavyAttack2ID, heavyAttack3ID, ultimateAttackID; // actor heavy & ultimate attacks
ACTIONid heavyDamagedID, leftDamagedID, rightDamagedID;

ACTIONid npcaIdleID, npcaRunID, npcaDieID, npcaDefenceID; // npca actions
ACTIONid npcaAttackL1ID, npcaAttackL2ID, npcaAttackHID, npcaHeavyAttackID;
ACTIONid npcaDamageLID, npcaDamageHID;

ACTIONid npcbCombatIdleID, npcbRunID, npcbDieID; // npcb actions

ROOMid terrainRoomID = FAILED_ID;
TEXTid textID = FAILED_ID;

GEOMETRYid bloodBarID = FAILED_ID;//actorè¡€æ¢?
GEOMETRYid bloodBarNPC1ID = FAILED_ID;//NPC1è¡€æ¢?
GEOMETRYid bloodBarNPC2ID = FAILED_ID;//NPC2è¡€æ¢?

// some globals
int frame = 0;
int oldX, oldY, oldXM, oldYM, oldXMM, oldYMM;
int actorHP = MAX_HP, npcaHP = MAX_HP, npcbHP = MAX_HP;
bool attackKeyLocked = false;
bool movementKeyLocked = false;
bool normalCombo = false;
int actorState = 0;
/*
0 idle
1 run
2 combatIdle
3 normalAttack1
4 normalAttack2
5 normalAttack3
6 normalAttack4
7 heavyAttack1
8 heavyAttack2
9 heavyAttack3
10 ultimateAttack
11 guard
12 heavyDamaged
11 rightDamaged
14 leftDamaged
15 die
*/
int actionFrame = 0;
int npcaState = 0;
/*
0 idle
1 run
2 attackL1
3 attackL2
4 attackH
5 heavyAttack
6 defence
7 damageL
8 damageH
9 die
*/
int npcaFrame = 0;
int npcbState = 0;
/*
0 combatIdle
1 run
*/
int npcbFrame = 0;
int combatWait;


// hotkey callbacks
void QuitGame(BYTE, BOOL4);
void Movement(BYTE, BOOL4);
void Attack(BYTE, BOOL4);

// timer callbacks
void GameAI(int);
void RenderIt(int);

// mouse callbacks
void InitPivot(int, int);
void PivotCam(int, int);
void InitMove(int, int);
void MoveCam(int, int);
void InitZoom(int, int);
void ZoomCam(int, int);

inline float oadistance(float *a, float *b);


MyCharacter npca(300);
MyCharacter npcb(80);
MyCharacter actor(300);

/*------------------
  the main program
  C.Wang 1010, 2014
 -------------------*/
void FyMain(int argc, char **argv)
{
   // create a new world
   BOOL4 beOK = FyStartFlyWin32("NTU@2014 Homework #03", 0, 0, 1024, 768, FALSE);

   // setup the data searching paths
   FySetShaderPath("Data\\NTU5\\Shaders");
   FySetModelPath("Data\\NTU5\\Scenes");
   FySetTexturePath("Data\\NTU5\\Scenes\\Textures");
   FySetScenePath("Data\\NTU5\\Scenes");

   // create a viewport
   vID = FyCreateViewport(0, 0, 1024, 768);
   FnViewport vp;
   vp.ID(vID);

   // create a 3D scene
   sID = FyCreateScene(10);
   FnScene scene;
   scene.ID(sID);

   // load the scene
   scene.Load("gameScene01");
   scene.SetAmbientLights(1.0f, 1.0f, 1.0f, 0.6f, 0.6f, 0.6f);

   // load the terrain
   tID = scene.CreateObject(OBJECT);
   FnObject terrain;
   terrain.ID(tID);
   BOOL beOK1 = terrain.Load("terrain");
   terrain.Show(FALSE);

   // set terrain environment
   terrainRoomID = scene.CreateRoom(SIMPLE_ROOM, 10);
   FnRoom room;
   room.ID(terrainRoomID);
   room.AddObject(tID);

   // load the characters
   FySetModelPath("Data\\NTU5\\Characters");
   FySetTexturePath("Data\\NTU5\\Characters");
   FySetCharacterPath("Data\\NTU5\\Characters");
   actorID = scene.LoadCharacter("Lyubu2");
   npcaID = scene.LoadCharacter("Donzo2");
   npcbID = scene.LoadCharacter("Robber02");

   // put characters in the room 
   //room.AddObject(actorID);
   //room.AddObject(npcaID);
   //room.AddObject(npcbID);

   // put the character on terrain
   float pos[3], fDir[3], uDir[3];
   //FnCharacter actor, npca, npcb;
   actor.ID(actorID);
   pos[0] = 3569.0f; pos[1] = -3208.0f; pos[2] = 1000.0f;
   fDir[0] = 1.0f; fDir[1] = 0.0f; fDir[2] = 0.0f;
   uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
   actor.SetDirection(fDir, uDir);

   actor.SetTerrainRoom(terrainRoomID, 10.0f);
   beOK = actor.PutOnTerrain(pos);

   // Get actor actions at Lyubu2
   idleID = actor.GetBodyAction(NULL, "Idle");
   runID = actor.GetBodyAction(NULL, "Run");
   combatIdleID = actor.GetBodyAction(NULL, "CombatIdle");
   normalAttack1ID = actor.GetBodyAction(NULL, "NormalAttack1");
   normalAttack2ID = actor.GetBodyAction(NULL, "NormalAttack2");
   normalAttack3ID = actor.GetBodyAction(NULL, "NormalAttack3");
   normalAttack4ID = actor.GetBodyAction(NULL, "NormalAttack4");
   heavyAttack1ID = actor.GetBodyAction(NULL, "HeavyAttack1");
   heavyAttack2ID = actor.GetBodyAction(NULL, "HeavyAttack2");
   heavyAttack3ID = actor.GetBodyAction(NULL, "HeavyAttack3");
   ultimateAttackID = actor.GetBodyAction(NULL, "UltimateAttack");
   guardID = actor.GetBodyAction(NULL, "Guard");
   heavyDamagedID = actor.GetBodyAction(NULL, "HeavyDamaged");
   rightDamagedID = actor.GetBodyAction(NULL, "RightDamaged");
   leftDamagedID = actor.GetBodyAction(NULL, "LeftDamaged");
   dieID = actor.GetBodyAction(NULL, "Die");

   // set the actor to idle action
   curPoseID = idleID;
   actor.SetCurrentAction(NULL, 0, curPoseID);
   actor.Play(START, 0.0f, FALSE, TRUE);

   // npca
   npca.ID(npcaID);
   pos[0] = 3769.0f; pos[1] = -3208.0f; pos[2] = 1000.0f;
   fDir[0] = 1.0f; fDir[1] = 0.0f; fDir[2] = 0.0f;
   uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
   npca.SetDirection(fDir, uDir);

   npca.SetTerrainRoom(terrainRoomID, 10.0f);
   beOK = npca.PutOnTerrain(pos);

   // Get npca actions Donzo2
   npcaIdleID = npca.GetBodyAction(NULL, "Idle");
   npcaRunID = npca.GetBodyAction(NULL, "Run");
   npcaAttackL1ID = npca.GetBodyAction(NULL, "AttackL1");
   npcaAttackL2ID = npca.GetBodyAction(NULL, "AttackL2");
   npcaAttackHID = npca.GetBodyAction(NULL, "AttackH");
   npcaHeavyAttackID = npca.GetBodyAction(NULL, "HeavyAttack");
   npcaDefenceID = npca.GetBodyAction(NULL, "Defence");
   npcaDamageLID = npca.GetBodyAction(NULL, "DamageL");
   npcaDamageHID = npca.GetBodyAction(NULL, "DamageH");
   npcaDieID = npca.GetBodyAction(NULL, "Die");

   // set the character to idle action
   curPoseID = npcaIdleID;
   npca.SetCurrentAction(NULL, 0, curPoseID);
   npca.Play(START, 0.0f, FALSE, TRUE);

   // npcb
   npcb.ID(npcbID);
   pos[0] = 3669.0f; pos[1] = -3208.0f; pos[2] = 1000.0f;
   fDir[0] = 1.0f; fDir[1] = 0.0f; fDir[2] = 0.0f;
   uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
   npcb.SetDirection(fDir, uDir);

   npcb.SetTerrainRoom(terrainRoomID, 10.0f);
   beOK = npcb.PutOnTerrain(pos);

   // Get npcb actions Robber02
   npcbCombatIdleID = npcb.GetBodyAction(NULL, "CombatIdle");
   npcbRunID = npcb.GetBodyAction(NULL, "Run");

   // set the character to idle action
   curPoseID = npcbCombatIdleID;
   npcb.SetCurrentAction(NULL, 0, curPoseID);
   npcb.Play(START, 0.0f, FALSE, TRUE);

   // create object
   oID = scene.CreateObject(OBJECT);
   FnObject object;
   object.ID(oID);

   // set object initial position and orientation
   pos[0] = 3069.0f; pos[1] = -3208.0f; pos[2] = 100.0f;
   fDir[0] = 1.0f; fDir[1] = 0.0f; fDir[2] = 0.0f;
   uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;

   object.SetPosition(pos);
   object.SetDirection(fDir, uDir);
   
   // translate the camera
   cID = scene.CreateObject(CAMERA);
   FnCamera camera;
   camera.ID(cID);
   camera.SetNearPlane(5.0f);
   camera.SetFarPlane(100000.0f); 
   camera.SetParent(oID);

   // set camera initial orientation
   fDir[0] = 1.0f; fDir[1] = 0.0f; fDir[2] = -0.08f;
   uDir[0] = 0.08f; uDir[1] = 0.0f; uDir[2] = 1.0f;

   camera.SetDirection(fDir, uDir);

   // setup a point light
   FnLight lgt;
   lgt.ID(scene.CreateObject(LIGHT));
   lgt.Translate(70.0f, -70.0f, 70.0f, REPLACE);
   lgt.SetColor(1.0f, 1.0f, 1.0f);
   lgt.SetIntensity(1.0f);

   // create a billboard and set parent to base object of the character to demo the simple bloodbar
   actor.setBBA(scene);
   //NPC1
   
   npca.setBB(scene);
   //NPC2
   npcb.setBB(scene);


   // create a text object for displaying messages on screen
   textID = FyCreateText("Trebuchet MS", 18, FALSE, FALSE);

   // set Hotkeys
   FyDefineHotKey(FY_ESCAPE, QuitGame, FALSE);  // escape for quiting the game
   FyDefineHotKey(FY_UP, Movement, FALSE);      // Up for moving forward
   FyDefineHotKey(FY_RIGHT, Movement, FALSE);   // Right for turning right
   FyDefineHotKey(FY_LEFT, Movement, FALSE);    // Left for turning left
   FyDefineHotKey(FY_DOWN, Movement, FALSE);    // Down for moving back
   FyDefineHotKey(FY_Z, Attack, FALSE);

   // define some mouse functions
   FyBindMouseFunction(LEFT_MOUSE, InitPivot, PivotCam, NULL, NULL);
   FyBindMouseFunction(MIDDLE_MOUSE, InitZoom, ZoomCam, NULL, NULL);
   FyBindMouseFunction(RIGHT_MOUSE, InitMove, MoveCam, NULL, NULL);

   // bind timers, frame rate = 30 fps
   FyBindTimer(0, 30.0f, GameAI, TRUE);
   FyBindTimer(1, 30.0f, RenderIt, TRUE);

   // invoke the system
   FyInvokeFly(TRUE);
}

inline float oadistance(float *a, float *b)
{
	return sqrt((a[0]-b[0])*(a[0]-b[0])+(a[1]-b[1])*(a[1]-b[1]));
}

inline float cameraHieght(float d_oa)
{
	return sqrt(251600.0f - d_oa * d_oa) + 60.0f;
}

inline float hitcheck(float *a, float *b,float afDir[3])
{   
	int hit=0;
	float r1=sqrt((a[0]-b[0])*(a[0]-b[0])+(a[1]-b[1])*(a[1]-b[1]));
	float cos1 = ((a[0]-b[0])*afDir[0] + (a[1]-b[1])*afDir[1])/(r1*sqrt(afDir[0]*afDir[0]+afDir[1]*afDir[1]));
		if((cos1 > (1/sqrt(2.0)))&&(40.f<r1< 100.f)){
				hit = 1;
		}
		else if(r1< 40.f){
				hit = 2;
		}
	return hit;
}

/*-------------------------------------------------------------
  30fps timer callback in fixed frame rate for major game loop
  C.Wang 1103, 2007
 --------------------------------------------------------------*/
void GameAI(int skip)
{
	//FnCharacter actor, npca, npcb;
	FnCamera camera;
	FnObject object, terrain;

	bool walk = false;
	 
	float afDir[3], auDir[3], cfDir[3], cuDir[3], ofDir[3], ouDir[3], obpos[3], acpos[3], napos[3], nbpos[3], ohitdir[3];

    ohitdir[0] = 0.0f; ohitdir[1] = 0.0f; ohitdir[2] = -1.0f;

	object.ID(oID);
	camera.ID(cID);
	terrain.ID(tID);

	actor.ID(actorID);

	// play character pose
	npca.ID(npcaID);
	
	
	npcb.ID(npcbID);
	npcb.Play(LOOP, (float) skip, FALSE, TRUE);

	object.GetPosition(obpos);
	actor.GetPosition(acpos);
	npca.GetPosition(napos);
	npcb.GetPosition(nbpos);

	float d_oa = oadistance(obpos, acpos);

	float TURN_A = acos((d_oa*d_oa + d_oa*d_oa - 100.0f) / (d_oa*d_oa + d_oa*d_oa)) * 180.0f / M_PI;

	actor.GetDirection(afDir, auDir);
	object.GetDirection(ofDir, ouDir);

	npca.BB();
	npcb.BB();
	actor.BB();

/////keys for testing 
	if (FyCheckHotKeyStatus(FY_L))
		{
			afDir[0] = ofDir[1];
			afDir[1] = -ofDir[0];
			actor.SetDirection(afDir, auDir);
			actor.TurnRight(TURN_A/2.0f);

			
			actor.TurnRight(TURN_A/2.0f);
			object.TurnRight(TURN_A/2.0f);
			object.MoveRight(-10.f);
			object.TurnRight(TURN_A/2.0f);
			walk = true;
		}

		if (FyCheckHotKeyStatus(FY_K))
		{
			afDir[0] = ofDir[1];
			afDir[1] = -ofDir[0];
			actor.SetDirection(afDir, auDir);
			actor.TurnRight(-TURN_A/2.0f);

			
			actor.TurnRight(-TURN_A/2.0f);
			object.TurnRight(-TURN_A/2.0f);
			object.MoveRight(10.f);
			object.TurnRight(-TURN_A/2.0f);
			walk = true;
		}


///actor state
	switch(actorState){
		case 0:			
			actor.Play(LOOP, (float) skip, FALSE, TRUE);
			break;
		case 1:
			actor.Play(LOOP, (float) skip, FALSE, TRUE);
			if (combatWait > 0)
			{
				combatWait--;
			}

			if (FyCheckHotKeyStatus(FY_UP))
			{
				if (FyCheckHotKeyStatus(FY_RIGHT))
				{
					/* UP and RIGHT */
					afDir[0] = ofDir[0] + ofDir[1];
					afDir[1] = ofDir[1] - ofDir[0];
					actor.SetDirection(afDir, auDir);
					if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
					{
						object.MoveRight(10.0f/sqrt(2.0f));
						object.MoveForward(10.0f/sqrt(2.0f));
						walk = true;
					}
				}
				else if (FyCheckHotKeyStatus(FY_LEFT))
				{
					/* UP and LEFT */
					afDir[0] = ofDir[0] - ofDir[1];
					afDir[1] = ofDir[1] + ofDir[0];
					actor.SetDirection(afDir, auDir);
					if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
					{
						object.MoveRight(-10.0f/sqrt(2.0f));
						object.MoveForward(10.0f/sqrt(2.0f));
						walk = true;
					}
				}
				else 
				{
					/* UP only */
					afDir[0] = ofDir[0];
					afDir[1] = ofDir[1];
					actor.SetDirection(afDir, auDir);
					if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
					{
						object.MoveForward(10.0f);
						walk = true;
					}
				}
			}
			else if (FyCheckHotKeyStatus(FY_DOWN))
			{
				if (FyCheckHotKeyStatus(FY_RIGHT))
				{
					/* DOWN and RIGHT */
					afDir[0] = -ofDir[0] + ofDir[1];
					afDir[1] = -ofDir[1] - ofDir[0];
					actor.SetDirection(afDir, auDir);
					if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
					{
						object.MoveRight(10.0f/sqrt(2.0f));
						object.MoveForward(-10.0f/sqrt(2.0f));
						walk = true;
					}
				}
				else if (FyCheckHotKeyStatus(FY_LEFT))
				{
					/* DOWN and LEFT */
					afDir[0] = -ofDir[0] - ofDir[1];
					afDir[1] = -ofDir[1] + ofDir[0];
					actor.SetDirection(afDir, auDir);
					if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
					{
						object.MoveRight(-10.0f/sqrt(2.0f));
						object.MoveForward(-10.0f/sqrt(2.0f));
						walk = true;
					}
				}
				else 
				{
					/* DOWN only */
					afDir[0] = -ofDir[0];
					afDir[1] = -ofDir[1];
					actor.SetDirection(afDir, auDir);
					if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
					{
						object.MoveForward(-10.0f);
						walk = true;
					}
				}
			}
			else if (FyCheckHotKeyStatus(FY_RIGHT))
			{
				if (FyCheckHotKeyStatus(FY_UP))
				{
					/* RIGHT and UP */
					afDir[0] = ofDir[1] + ofDir[0];
					afDir[1] = -ofDir[0] + ofDir[1];
					actor.SetDirection(afDir, auDir);
					if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
					{
						object.MoveRight(10.0f/sqrt(2.0f));
						object.MoveForward(10.0f/sqrt(2.0f));
						walk = true;
					}
				}
				else if (FyCheckHotKeyStatus(FY_DOWN))
				{
					/* RIGHT and DOWN */
					afDir[0] = ofDir[1] - ofDir[0];
					afDir[1] = -ofDir[0] - ofDir[1];
					actor.SetDirection(afDir, auDir);
					if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
					{
						object.MoveRight(10.0f/sqrt(2.0f));
						object.MoveForward(-10.0f/sqrt(2.0f));
						walk = true;
					}
				}
				else 
				{
					/* RIGHT only */
					afDir[0] = ofDir[1];
					afDir[1] = -ofDir[0];
					actor.SetDirection(afDir, auDir);
					actor.TurnRight(TURN_A/2.0f);

					if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
					{
						actor.TurnRight(TURN_A/2.0f);
						object.TurnRight(TURN_A);
					}
					else
					{
						actor.TurnRight(TURN_A/2.0f);
						object.TurnRight(TURN_A/2.0f);
						object.MoveRight(-10.f);
						object.TurnRight(TURN_A/2.0f);
						walk = true;
					}
				}
			}
			else if (FyCheckHotKeyStatus(FY_LEFT))
			{
				if (FyCheckHotKeyStatus(FY_UP))
				{
					/* LEFT and UP */
					afDir[0] = -ofDir[1] + ofDir[0];
					afDir[1] = ofDir[0] + ofDir[1];
					actor.SetDirection(afDir, auDir);
					if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
					{
						object.MoveRight(-10.0f/sqrt(2.0f));
						object.MoveForward(10.0f/sqrt(2.0f));
						walk = true;
					}
				}
				else if (FyCheckHotKeyStatus(FY_DOWN))
				{
					/* LEFT and DOWN */
					afDir[0] = -ofDir[1] - ofDir[0];
					afDir[1] = ofDir[0] - ofDir[1];
					actor.SetDirection(afDir, auDir);
					if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
					{
						object.MoveRight(-10.0f/sqrt(2.0f));
						object.MoveForward(-10.0f/sqrt(2.0f));
						walk = true;
					}
				}
				else 
				{
					/* LEFT only */
					afDir[0] = -ofDir[1];
					afDir[1] = ofDir[0];
					actor.SetDirection(afDir, auDir);	
					actor.TurnRight(-TURN_A/2.0f);
					if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
					{
						actor.TurnRight(-TURN_A/2.0f);
						object.TurnRight(-TURN_A);
					}
					else
					{
						actor.TurnRight(-TURN_A/2.0f);
						object.TurnRight(-TURN_A/2.0f);
						object.MoveRight(10.f);
						object.TurnRight(-TURN_A/2.0f);
						walk = true;
					}
				}
			}

			break;
		case 2:
			actor.Play(LOOP, (float) skip, FALSE, TRUE);
			if (combatWait > 0)
			{
				combatWait--;
			}
			if (combatWait == 0)
			{
				actorState = 0;
				actor.SetCurrentAction(NULL, 0, idleID);
			}
			if (FyCheckHotKeyStatus(FY_UP) || FyCheckHotKeyStatus(FY_RIGHT) || FyCheckHotKeyStatus(FY_LEFT) || FyCheckHotKeyStatus(FY_DOWN))
			{
				actorState = 1;
				actor.SetCurrentAction(NULL, 0, runID);
			}
			break;
		case 3:			
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			actionFrame++;

			if (actionFrame == 10){
				if(hitcheck(napos, acpos,afDir)){
					//actorHP=actorHP-1;
					if (npcaState != 9)
					{
						npca.blood-=5;
						npcaState = 7;
						npca.SetCurrentAction(NULL, 0, npcaDamageLID);
						npcaFrame = 0;
					}		
				}
				if(hitcheck(nbpos, acpos,afDir)){
					//actorHP=actorHP-1;
					npcb.blood-=5;
				}
			}
			if (actionFrame == 12)
			{
				attackKeyLocked = false;
			}
			if (actionFrame == 24)
			{
				if(normalCombo){
					actorState = 4;
					actionFrame = 0;
					actor.SetCurrentAction(NULL, 0, normalAttack2ID);
					normalCombo = false;
				}
				else{
					actorState = 2;
					actionFrame = 0;
					combatWait = 150;
					actor.SetCurrentAction(NULL, 0, combatIdleID);
					movementKeyLocked = false;
				}	
			}
			break;
		case 4:
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			actionFrame++;

			if (actionFrame == 30){
				if(hitcheck(napos, acpos,afDir)){
					//actorHP=actorHP-1;
					if (npcaState != 9)
					{
						npca.blood-= 10;
						npcaState = 7;
						npca.SetCurrentAction(NULL, 0, npcaDamageLID);
						npcaFrame = 0;
					}	
				}
				if(hitcheck(nbpos, acpos,afDir)){
					//actorHP=actorHP-1;
					npcb.blood-=5;
				}
			}
			if (actionFrame == 32)
			{
				attackKeyLocked = false;
			}
			if (actionFrame == 48)
			{
				if(normalCombo){
					actorState = 5;
					actionFrame = 0;
					actor.SetCurrentAction(NULL, 0, normalAttack3ID);
					normalCombo = false;
				}
				else{
					actorState = 2;
					actionFrame = 0;
					combatWait = 150;
					actor.SetCurrentAction(NULL, 0, combatIdleID);
					movementKeyLocked = false;
				}	
			}
			break;
		case 5:
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			actionFrame++;

			if (actionFrame == 28){
				if(hitcheck(napos, acpos,afDir)){
					//actorHP=actorHP-1;
					if (npcaState != 9)
					{
						npca.blood-=20;
						npcaState = 7;
						npca.SetCurrentAction(NULL, 0, npcaDamageLID);
						npcaFrame = 0;
					}	
				}
				if(hitcheck(nbpos, acpos,afDir)){
					//actorHP=actorHP-1;
					npcb.blood-=10;
				}
			}
			if (actionFrame == 30)
			{
				attackKeyLocked = false;
			}
			if (actionFrame == 46)
			{
				if(normalCombo){
					actorState = 6;
					actionFrame = 0;
					actor.SetCurrentAction(NULL, 0, normalAttack4ID);
					normalCombo = false;
				}
				else{
					actorState = 2;
					actionFrame = 0;
					combatWait = 150;
					actor.SetCurrentAction(NULL, 0, combatIdleID);
					movementKeyLocked = false;
				}	
			}
			break;
		case 6:
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			actionFrame++;

			if (actionFrame == 30){
				if(hitcheck(napos, acpos,afDir)){
					//actorHP=actorHP-1;
					if (npcaState != 9)
					{
						npca.blood-=30;
						npcaState = 7;
						npca.SetCurrentAction(NULL, 0, npcaDamageLID);
						npcaFrame = 0;
					}	
				}
				if(hitcheck(nbpos, acpos,afDir)){
					//actorHP=actorHP-1;
					npcb.blood-=15;
				}
			}
			if (actionFrame == 44)
			{
				attackKeyLocked = false;
			}
			if (actionFrame == 49)
			{
				if(normalCombo){
					actorState = 3;
					actionFrame = 0;
					actor.SetCurrentAction(NULL, 0, normalAttack1ID);
					normalCombo = false;
				}
				else{
					actorState = 2;
					actionFrame = 0;
					combatWait = 150;
					actor.SetCurrentAction(NULL, 0, combatIdleID);
					movementKeyLocked = false;
				}	
			}
			break;
		default:
			break;
	}
	
	// check npc die 

	if (npca.blood <= 0 && npcaState != 9)
	{
		npcaState = 9;
		npca.SetCurrentAction(NULL, 0, npcaDieID);
	}

	//npca state
	switch(npcaState){
		case 0:
			npca.Play(LOOP, (float) skip, FALSE, TRUE);
			break;
		case 1:
			npca.Play(LOOP, (float) skip, FALSE, TRUE);
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			npcaFrame++;
			npca.Play(ONCE, (float) skip, FALSE, TRUE);
			if (npcaFrame == 26)
			{
				npcaState = 0;
				npca.SetCurrentAction(NULL, 0, npcaIdleID);
				npcaFrame = 0;
			}
			break;
		case 8:
			npcaFrame++;
			npca.Play(ONCE, (float) skip, FALSE, TRUE);
			if (npcaFrame == 36)
			{
				npcaState = 0;
				npca.SetCurrentAction(NULL, 0, npcaIdleID);
				npcaFrame = 0;
			}
			break;
		case 9:
			npca.Play(ONCE, (float) skip, FALSE, TRUE);
			break;
		default:
			break;
	}
	
	object.GetPosition(obpos);
	actor.GetPosition(acpos);
	camera.GetDirection(cfDir, cuDir);


// camera hit test
	if(terrain.HitTest(obpos, ohitdir) > 0)
	{
		if(walk){
			if(d_oa < 500.0f)
			{
				object.MoveForward(-10.0f);
				object.GetPosition(obpos);
				d_oa = oadistance(obpos, acpos);
				if(d_oa > 500.0f){
					obpos[2] = cameraHieght(500.0f);
					obpos[0] = acpos[0] - sqrt(250000.0f / (1 + (cfDir[1] * cfDir[1]) / (cfDir[0] * cfDir[0]))) * cfDir[0] / fabs(cfDir[0]);
					obpos[1] = acpos[1] - sqrt(250000.0f / (1 + (cfDir[0] * cfDir[0]) / (cfDir[1] * cfDir[1]))) * cfDir[1] / fabs(cfDir[1]);
					object.SetPosition(obpos);
					cfDir[0] = acpos[0] - obpos[0];
					cfDir[1] = acpos[1] - obpos[1];
					cfDir[2] = acpos[2] + 60.0f - obpos[2];
					cuDir[2] =  (-cfDir[0] * cuDir[0] - cfDir[1] * cuDir[1]) / cfDir[2];
 					camera.SetDirection(cfDir, cuDir);
				}
				else{
					obpos[2] = cameraHieght(d_oa);
					object.SetPosition(obpos);
					cfDir[0] = acpos[0] - obpos[0];
					cfDir[1] = acpos[1] - obpos[1];
					cfDir[2] = acpos[2] + 60.0f - obpos[2];
					cuDir[2] =  (-cfDir[0] * cuDir[0] - cfDir[1] * cuDir[1]) / cfDir[2];
 					camera.SetDirection(cfDir, cuDir);
 				}
			}
		}
	}
	else 
	{
       	object.MoveForward(10.0f); 
       	object.GetPosition(obpos);
		d_oa = oadistance(obpos, acpos);
		obpos[2] = cameraHieght(d_oa);
		object.SetPosition(obpos);
		cfDir[0] = acpos[0] - obpos[0];
		cfDir[1] = acpos[1] - obpos[1];
		cfDir[2] = acpos[2] + 60.0f - obpos[2];
		cuDir[2] =  (-cfDir[0] * cuDir[0] - cfDir[1] * cuDir[1]) / cfDir[2];
		camera.SetDirection(cfDir, cuDir);
	}
}

/*----------------------
  perform the rendering
  C.Wang 0720, 2006
 -----------------------*/
void RenderIt(int skip)
{
   FnViewport vp;

   // render the whole scene
   vp.ID(vID);
   vp.Render3D(cID, TRUE, TRUE);

   // get camera's data
   FnCamera camera;
   camera.ID(cID);

   FnObject object;
   object.ID(oID);

   //FnCharacter actor;
   actor.ID(actorID);

	float pos[3], fDir[3], uDir[3], opos[3],apos[3];
	camera.GetPosition(pos);
	camera.GetDirection(fDir, uDir);
	object.GetPosition(opos);
 	actor.GetPosition(apos);

   // show frame rate
   static char string[128];
   if (frame == 0) {
      FyTimerReset(0);
   }

   if (frame/10*10 == frame) {
      float curTime;

      curTime = FyTimerCheckTime(0);
      sprintf(string, "Fps: %6.2f", frame/curTime);
   }

   frame += skip;
   if (frame >= 1000) {
      frame = 0;
   }

   FnText text;
   text.ID(textID);

   text.Begin(vID);
   text.Write(string, 20, 20, 255, 0, 0);

   char posS[256], fDirS[256], uDirS[256], dis[256], OposS[256], AposS[256], actorHPS[256], npcaHPS[256], npcbHPS[256];
   sprintf(posS, "pos: %8.3f %8.3f %8.3f", pos[0], pos[1], pos[2]);
   sprintf(fDirS, "facing: %8.3f %8.3f %8.3f", fDir[0], fDir[1], fDir[2]);
   sprintf(uDirS, "up: %8.3f %8.3f %8.3f", uDir[0], uDir[1], uDir[2]);
   sprintf(OposS, "opos: %8.3f %8.3f %8.3f", opos[0], opos[1], opos[2]);
   sprintf(AposS, "apos: %8.3f %8.3f %8.3f", apos[0], apos[1], apos[2]);
   sprintf(dis, "oadis: %8.3f", oadistance(opos,apos));
   sprintf(actorHPS, "actorHP: %d/100", actorHP);
   sprintf(npcaHPS, "npcaHP: %d/100", npcaHP);
   sprintf(npcbHPS, "npcbHP: %d/100", npcbHP);

   text.Write(posS, 20, 35, 255, 255, 0);
   text.Write(fDirS, 20, 50, 255, 255, 0);
   text.Write(uDirS, 20, 65, 255, 255, 0);
   text.Write(dis, 20, 95, 255, 255, 0);
   text.Write(OposS, 20, 110, 255, 255, 0);
   text.Write(AposS, 20, 125, 255, 255, 0);
   text.Write(actorHPS, 20, 140, 255, 255, 0);
   text.Write(npcaHPS, 20, 155, 255, 255, 0);
   text.Write(npcbHPS, 20, 170, 255, 255, 0);

   text.End();

   // swap buffer
   FySwapBuffers();
}


/*------------------
  movement control
  C.Wang 1103, 2006
 -------------------*/
void Movement(BYTE code, BOOL4 value)
{
	//FnCharacter actor;
	actor.ID(actorID);
	if(!movementKeyLocked){
		if(value) {
			if (actorState == 0 || actorState == 2)
			{
				actorState = 1;
				actor.SetCurrentAction(NULL, 0, runID);
			}
		}
		else if(!FyCheckHotKeyStatus(FY_UP) && !FyCheckHotKeyStatus(FY_RIGHT) && !FyCheckHotKeyStatus(FY_LEFT) && !FyCheckHotKeyStatus(FY_DOWN)) {
			if (actorState == 1)
			{
				actorState = 0;
				actor.SetCurrentAction(NULL, 0, idleID);
			}
		}
	}
}

void Attack(BYTE code, BOOL4 value)
{
	//FnCharacter actor;
	actor.ID(actorID);
	if(!attackKeyLocked){
		if(value){
			switch(actorState){
				case 3:
					normalCombo = true;
					attackKeyLocked = true;
					break;
				case 4:
					normalCombo = true;
					attackKeyLocked = true;
					break;
				case 5:
					normalCombo = true;
					attackKeyLocked = true;
					break;
				default:
					actorState = 3;
					actionFrame = 0;
					actor.SetCurrentAction(NULL, 0, normalAttack1ID);
					attackKeyLocked = true;
					movementKeyLocked = true;
					break;
			}
		}
	}
}


/*------------------
  quit the demo
  C.Wang 0327, 2005
 -------------------*/
void QuitGame(BYTE code, BOOL4 value)
{
   if (code == FY_ESCAPE) {
      if (value) {
         FyQuitFlyWin32();
      }
   }
}



/*-----------------------------------
  initialize the pivot of the camera
  C.Wang 0329, 2005
 ------------------------------------*/
void InitPivot(int x, int y)
{
   oldX = x;
   oldY = y;
   frame = 0;
}


/*------------------
  pivot the camera
  C.Wang 0329, 2005
 -------------------*/
void PivotCam(int x, int y)
{
   FnObject model;

   if (x != oldX) {
      model.ID(cID);
      model.Rotate(Z_AXIS, (float) (x - oldX), GLOBAL);
      oldX = x;
   }

   if (y != oldY) {
      model.ID(cID);
      model.Rotate(X_AXIS, (float) (y - oldY), GLOBAL);
      oldY = y;
   }
}


/*----------------------------------
  initialize the move of the camera
  C.Wang 0329, 2005
 -----------------------------------*/
void InitMove(int x, int y)
{
   oldXM = x;
   oldYM = y;
   frame = 0;
}


/*------------------
  move the camera
  C.Wang 0329, 2005
 -------------------*/
void MoveCam(int x, int y)
{
   if (x != oldXM) {
      FnObject model;

      model.ID(cID);
      model.Translate((float)(x - oldXM)*2.0f, 0.0f, 0.0f, LOCAL);
      oldXM = x;
   }
   if (y != oldYM) {
      FnObject model;

      model.ID(cID);
      model.Translate(0.0f, (float)(oldYM - y)*2.0f, 0.0f, LOCAL);
      oldYM = y;
   }
}


/*----------------------------------
  initialize the zoom of the camera
  C.Wang 0329, 2005
 -----------------------------------*/
void InitZoom(int x, int y)
{
   oldXMM = x;
   oldYMM = y;
   frame = 0;
}


/*------------------
  zoom the camera
  C.Wang 0329, 2005
 -------------------*/
void ZoomCam(int x, int y)
{
   if (x != oldXMM || y != oldYMM) {
      FnObject model;

      model.ID(cID);
      model.Translate(0.0f, 0.0f, (float)(x - oldXMM)*10.0f, LOCAL);
      oldXMM = x;
      oldYMM = y;
   }
}
