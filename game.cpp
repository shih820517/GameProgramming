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


class MyCharacter : public FnCharacter {
public:
	MyCharacter(int myBlood = 0){
		blood = myBlood;
		fullBlood = myBlood;
		frame=0;
		state = 0;
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

		if(theta > 2 || theta < -2) {
			TurnRight(theta);
		}
	}
	int blood, fullBlood;
	int frame;
	int state;
	GEOMETRYid bloodBarID;
};


VIEWPORTid vID;                 // the major viewport
SCENEid sID;                    // the 3D scene
OBJECTid cID, tID, oID;         // the main camera and the terrain for terrain following
CHARACTERid actorID, npcaID, npcbID; // the major character
/* actor actions */
ACTIONid idleID, runID, dieID, combatIdleID, guardID, curPoseID; // actor move & die
ACTIONid normalAttack1ID, normalAttack2ID, normalAttack3ID, normalAttack4ID; // actor normal attack
ACTIONid heavyAttack1ID, heavyAttack2ID, heavyAttack3ID, ultimateAttackID; // actor heavy & ultimate attack
ACTIONid heavyDamagedID, leftDamagedID, rightDamagedID; // actor hurt
/* npca actions */
ACTIONid npcaIdleID, npcaRunID, npcaDieID, npcaDefenceID; // npca move & die
ACTIONid npcaAttackL1ID, npcaAttackL2ID, npcaAttackHID, npcaHeavyAttackID;// npca attack
ACTIONid npcaDamageLID, npcaDamageHID;	// npca hurt
/* npcb actions */
ACTIONid npcbCombatIdleID, npcbRunID, npcbDieID, npcbMoveRightID, npcbMoveLeftID; // npcb move & die
ACTIONid npcbNormalAttack1ID, npcbNormalAttack2ID, npcbHeavyAttack1ID;// npcb attack
ACTIONid npcbDamage1ID, npcbDamage2ID; // npcb hurt

ROOMid terrainRoomID = FAILED_ID;
TEXTid textID = FAILED_ID;

GEOMETRYid bloodBarID = FAILED_ID;//actorè¡€æ¢?
GEOMETRYid bloodBarNPC1ID = FAILED_ID;//NPC1è¡€æ¢?
GEOMETRYid bloodBarNPC2ID = FAILED_ID;//NPC2è¡€æ¢?

// some globals
int frame = 0;
int oldX, oldY, oldXM, oldYM, oldXMM, oldYMM;

int combatWait; // time to return idle

/*
0 UP
1 DOWN
2 LEFT
3 RIGHT
*/
bool moveKeyState[4] = {false, false, false, false};

int cameraRotateState = 0;

int cameraZoomState = 0;

float cameraDistance = 800.0f;

/*
0 normal attack
1 heavy attack
*/
bool attackKeyState[2] = {false, false};

bool attackKeyLocked = false;
bool movementKeyLocked = false;
bool normalCombo = false;
bool heavyCombo = false;

// hotkey callbacks
void QuitGame(BYTE, BOOL4);
void Movement(BYTE, BOOL4);
void Attack(BYTE, BOOL4);
void Reset(BYTE, BOOL4);
void cameraRotate(BYTE, BOOL4);
void cameraZoom(BYTE, BOOL4);


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

// our function
inline float myDist(float *apos, float *bpos); //find distance
inline float cameraHieght(float dist); // give the hieght we need
inline bool hitcheck(float *a, float *b, float fDir[3]);// check if attack hit
bool isHit(float *, float *, float *, float , float);

MyCharacter npca(200);
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

   // put the character on terrain
   float pos[3], fDir[3], uDir[3];

   // actor
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
   // end actor

   // npca
   npca.ID(npcaID);

   pos[0] = 3769.0f; pos[1] = -3208.0f; pos[2] = 1000.0f;
   fDir[0] = -1.0f; fDir[1] = 0.0f; fDir[2] = 0.0f;
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
   // end npca

   // npcb
   npcb.ID(npcbID);

   pos[0] = 3769.0f; pos[1] = -3308.0f; pos[2] = 1000.0f;
   fDir[0] = -1.0f; fDir[1] = 0.0f; fDir[2] = 0.0f;
   uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
   npcb.SetDirection(fDir, uDir);

   npcb.SetTerrainRoom(terrainRoomID, 10.0f);
   beOK = npcb.PutOnTerrain(pos);

   // Get npcb actions Robber02
   npcbCombatIdleID = npcb.GetBodyAction(NULL, "CombatIdle");
   npcbRunID = npcb.GetBodyAction(NULL, "Run");
   npcbMoveRightID = npcb.GetBodyAction(NULL, "MoveRight");
   npcbMoveLeftID = npcb.GetBodyAction(NULL, "MoveLeft");
   npcbNormalAttack1ID = npcb.GetBodyAction(NULL, "NormalAttack1");
   npcbNormalAttack2ID = npcb.GetBodyAction(NULL, "NormalAttack2");
   npcbHeavyAttack1ID = npcb.GetBodyAction(NULL, "HeavyAttack1");
   npcbDamage1ID = npcb.GetBodyAction(NULL, "Damage1");
   npcbDamage2ID = npcb.GetBodyAction(NULL, "Damage2");
   npcbDieID = npcb.GetBodyAction(NULL, "Die");

   // set the character to idle action
   curPoseID = npcbCombatIdleID;
   npcb.SetCurrentAction(NULL, 0, curPoseID);
   npcb.Play(START, 0.0f, FALSE, TRUE);
   // end npcb

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
   //npca 
   npca.setBB(scene);
   //npcb
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
   FyDefineHotKey(FY_X, Attack, FALSE);
   FyDefineHotKey(FY_F1, Reset, FALSE);

   FyDefineHotKey(FY_A, cameraRotate, FALSE);
   FyDefineHotKey(FY_S, cameraRotate, FALSE);

   FyDefineHotKey(FY_Q, cameraZoom, FALSE);
   FyDefineHotKey(FY_W, cameraZoom, FALSE);

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


inline float myDist(float *apos, float *bpos)
{
	return sqrt((apos[0]-bpos[0])*(apos[0]-bpos[0])+(apos[1]-bpos[1])*(apos[1]-bpos[1]));
}

inline float cameraHieght(float dist)
{
	return sqrt(641600.0f - dist * dist) + 60.0f;
}

inline bool hitcheck(float *a, float *b,float fDir[3])
{   
	bool hit = false;
	float r1 = sqrt((a[0]-b[0])*(a[0]-b[0])+(a[1]-b[1])*(a[1]-b[1]));
	float cos1 = ((a[0]-b[0])*fDir[0] + (a[1]-b[1])*fDir[1])/(r1*sqrt(fDir[0]*fDir[0]+fDir[1]*fDir[1]));
		if(cos1 > (1/sqrt(1.5)) && r1 < 180.0f){
				hit = true;
		}
	return hit;
}

bool isHit(float *apos, float *bpos, float *afDir, float attackDist, float attackAngle) 
{
	float dist = myDist(apos, bpos);
	float cosTheta = ((bpos[0] - apos[0]) * afDir[0] + (bpos[1] - apos[1]) * afDir[1]) / (dist * sqrt(afDir[0] * afDir[0] + afDir[1] * afDir[1]));
	if (dist < attackDist && cosTheta > cos(attackAngle * M_PI / 360.0f))
	{
		return true;
	}
	return false;
}

inline float turnDegree(float dist, float stepLength)
{
	return acos((dist * dist + dist * dist - stepLength * stepLength) / (2 * dist * dist)) * 180.0f / M_PI;
}

void cameraRotating(){
	//load the object
	FnObject object;
	object.ID(oID);

	float pos[3], opos[3];

	actor.GetPosition(pos);
	object.GetPosition(opos);

	//right
	if(cameraRotateState == 1){
		object.TurnRight(-turnDegree(myDist(pos, opos), 10.0f));
		object.MoveRight(10.0f);
	}
	//left
	else if(cameraRotateState == 2){
		object.TurnRight(turnDegree(myDist(pos, opos), 10.0f));
		object.MoveRight(-10.0f);
	}
	else if(cameraRotateState == 0){

	}
}

void cameraZooming(){
	//zoom in
	if(cameraZoomState == 1){
		if((cameraDistance-10.0f) >= 500.0f){
			cameraDistance -= 10.0f;
		}
		else cameraDistance = 500.0f;

		
	}
	//zoom out
	else if(cameraZoomState == 2){
		if((cameraDistance+10.0f) <= 800.0f){
			cameraDistance += 10;
		}
		else cameraDistance = 800.0f;
		
	}
	else if(cameraZoomState == 0){

	}
}

bool moving()
{
	FnObject object;

	object.ID(oID);

	float pos[3], fDir[3], uDir[3], opos[3], ofDir[3], ouDir[3];

	actor.GetPosition(pos);
	object.GetPosition(opos);

	actor.GetDirection(fDir, uDir);
	object.GetDirection(ofDir, ouDir);
	
	if (moveKeyState[0] && moveKeyState[2])
	{
		/* UP and RIGHT */
		fDir[0] = ofDir[0] + ofDir[1];
		fDir[1] = ofDir[1] - ofDir[0];
		actor.SetDirection(fDir, uDir);
		if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
		{
			object.MoveRight(10.0f/sqrt(2.0f));
			object.MoveForward(10.0f/sqrt(2.0f));
			return true;
		}
		return false;
	}
	else if (moveKeyState[0] && moveKeyState[3])
	{
		/* UP & LEFT */
		fDir[0] = ofDir[0] - ofDir[1];
		fDir[1] = ofDir[1] + ofDir[0];
		actor.SetDirection(fDir, uDir);
		if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
		{
			object.MoveRight(-10.0f/sqrt(2.0f));
			object.MoveForward(10.0f/sqrt(2.0f));
			return true;
		}
		return false;
	}
	else if (moveKeyState[1] && moveKeyState[2])
	{
		/* DOWN & RIGHT */
		fDir[0] = -ofDir[0] + ofDir[1];
		fDir[1] = -ofDir[1] - ofDir[0];
		actor.SetDirection(fDir, uDir);
		if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
		{
			object.MoveRight(10.0f/sqrt(2.0f));
			object.MoveForward(-10.0f/sqrt(2.0f));
			return true;
		}
		return false;
	}
	else if (moveKeyState[1] && moveKeyState[3])
	{
		/* DOWN & LEFT */
		fDir[0] = -ofDir[0] - ofDir[1];
		fDir[1] = -ofDir[1] + ofDir[0];
		actor.SetDirection(fDir, uDir);
		if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
		{
			object.MoveRight(-10.0f/sqrt(2.0f));
			object.MoveForward(-10.0f/sqrt(2.0f));
			return true;
		}
		return false;
	}
	else if (moveKeyState[0])
	{
		/* UP */
		fDir[0] = ofDir[0];
		fDir[1] = ofDir[1];
		actor.SetDirection(fDir, uDir);
		if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
		{
			object.MoveForward(10.0f);
			return true;
		}
		return false;
	}
	else if (moveKeyState[1])
	{
		/* DOWN */
		fDir[0] = -ofDir[0];
		fDir[1] = -ofDir[1];
		actor.SetDirection(fDir, uDir);
		if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
		{
			object.MoveForward(-10.0f);
			return true;
		}
		return false;
	}
	else if (moveKeyState[2])
	{
		/* RIGHT */
		fDir[0] = ofDir[1];
		fDir[1] = -ofDir[0];
		actor.SetDirection(fDir, uDir);
		actor.TurnRight(turnDegree(myDist(pos, opos), 10.0f) / 2.0f);
		if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
		{
			actor.TurnRight(turnDegree(myDist(pos, opos), 10.0f) / 2.0f);
			object.TurnRight(turnDegree(myDist(pos, opos), 10.0f));
			return false;
		}
		else
		{
			actor.TurnRight(turnDegree(myDist(pos, opos), 10.0f) / 2.0f);
			object.TurnRight(turnDegree(myDist(pos, opos), 10.0f) / 2.0f);
			object.MoveRight(-10.f);
			object.TurnRight(turnDegree(myDist(pos, opos), 10.0f) / 2.0f);
			return true;
		}
	}
	else if (moveKeyState[3])
	{
		/* LEFT */
		fDir[0] = -ofDir[1];
		fDir[1] = ofDir[0];
		actor.SetDirection(fDir, uDir);	
		actor.TurnRight(-turnDegree(myDist(pos, opos), 10.0f) / 2.0f);
		if(actor.MoveForward(10.0f, TRUE, FALSE, FALSE, TRUE) == WALK)
		{
			actor.TurnRight(-turnDegree(myDist(pos, opos), 10.0f) / 2.0f);
			object.TurnRight(-turnDegree(myDist(pos, opos), 10.0f));
			return false;
		}
		else
		{
			actor.TurnRight(-turnDegree(myDist(pos, opos), 10.0f) / 2.0f);
			object.TurnRight(-turnDegree(myDist(pos, opos), 10.0f) / 2.0f);
			object.MoveRight(10.f);
			object.TurnRight(-turnDegree(myDist(pos, opos), 10.0f) / 2.0f);
			return true;
		}
	}
	else
	{
		if (combatWait)
		{
			actor.state = 2;
			actor.SetCurrentAction(NULL, 0, combatIdleID);
		}
		else
		{
			actor.state = 0;
			actor.SetCurrentAction(NULL, 0, idleID);
		}
		return false;
	}
}


/*-------------------------------------------------------------
  30fps timer callback in fixed frame rate for major game loop
  C.Wang 1103, 2007
 --------------------------------------------------------------*/
void GameAI(int skip)
{
	FnCamera camera;
	FnObject object, terrain;

	bool walk = false;
	 
	float pos[3], fDir[3], uDir[3], ohitdir[3];
	float opos[3], cfDir[3], cuDir[3];
	float apos[3], bpos[3];

    ohitdir[0] = 0.0f; ohitdir[1] = 0.0f; ohitdir[2] = -1.0f;

	object.ID(oID);
	camera.ID(cID);
	terrain.ID(tID);

	actor.GetPosition(pos);
	npca.GetPosition(apos);
	npcb.GetPosition(bpos);

	actor.GetDirection(fDir, uDir);

	npca.BB();
	npcb.BB();
	actor.BB();

	// check if running
	if (actor.state != 1 && movementKeyLocked == false && (moveKeyState[0] || moveKeyState[1] || moveKeyState[2] || moveKeyState[3]))
	{
		actor.state = 1;
		actor.SetCurrentAction(NULL, 0, runID);
	}

/*
	actor.state:
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
	switch(actor.state){
		case 0:
		// idle			
			actor.Play(LOOP, (float) skip, FALSE, TRUE);
			break;
		case 1:
		// run
			actor.Play(LOOP, (float) skip, FALSE, TRUE);
			if (combatWait > 0)
			{
				combatWait--;
			}
			walk = moving();
			break;
		case 2:
		// combat idle
			actor.Play(LOOP, (float) skip, FALSE, TRUE);
			if (combatWait > 0)
			{
				combatWait--;
			}
			if (combatWait == 0)
			{
				actor.state = 0;
				actor.SetCurrentAction(NULL, 0, idleID);
			}
			if (FyCheckHotKeyStatus(FY_UP) || FyCheckHotKeyStatus(FY_RIGHT) || FyCheckHotKeyStatus(FY_LEFT) || FyCheckHotKeyStatus(FY_DOWN))
			{
				actor.state = 1;
				actor.SetCurrentAction(NULL, 0, runID);
			}
			break;
		case 3:
		// normal attack 1 25 frames	
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			actor.frame++;

			if (actor.frame == 10){
				if(hitcheck(apos, pos,fDir)){
					//actorHP=actorHP-1;
					if (npca.state != 9)
					{
						npca.blood-=5;
						npca.state = 7;
						npca.SetCurrentAction(NULL, 0, npcaDamageLID);
						npca.frame = 0;
					}		
				}
				if(hitcheck(bpos, pos,fDir)){
					//actorHP=actorHP-1;
					if (npcb.state != 9)
					{
						npcb.blood-= 5;
						npcb.state = 7;
						npcb.SetCurrentAction(NULL, 0, npcbDamage1ID);
						npcb.frame = 0;
					}
				}	
			}
			if (actor.frame == 12)
			{
				attackKeyLocked = false;
			}
			if (actor.frame == 24)
			{
				if(normalCombo){
					actor.state = 4;
					actor.frame = 0;
					actor.SetCurrentAction(NULL, 0, normalAttack2ID);
					normalCombo = false;
				}
				else{
					actor.state = 2; 
					actor.frame = 0;
					combatWait = 150;
					actor.SetCurrentAction(NULL, 0, combatIdleID);
					movementKeyLocked = false;
				}	
			}
			break;
		case 4:
		// normal attack 2 48 frames
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			actor.frame++;

			if (actor.frame == 28){
				if(hitcheck(apos, pos,fDir)){
					//actorHP=actorHP-1;
					if (npca.state != 9)
					{
						npca.blood-= 10;
						npca.state = 7;
						npca.SetCurrentAction(NULL, 0, npcaDamageLID);
						npca.frame = 0;
					}	
				}
				if(hitcheck(bpos, pos,fDir)){
					//actorHP=actorHP-1;
					if (npcb.state != 9)
					{
						npcb.blood-= 10;
						npcb.state = 7;
						npcb.SetCurrentAction(NULL, 0, npcbDamage1ID);
						npcb.frame = 0;
					}	
				}
			}
			if (actor.frame == 32)
			{
				attackKeyLocked = false;
			}
			if (actor.frame == 47)
			{
				if(normalCombo){
					actor.state = 5;
					actor.frame = 0;
					actor.SetCurrentAction(NULL, 0, normalAttack3ID);
					normalCombo = false;
				}
				else{
					actor.state = 2;
					actor.frame = 0;
					combatWait = 150;
					actor.SetCurrentAction(NULL, 0, combatIdleID);
					movementKeyLocked = false;
				}	
			}
			break;
		case 5:
		// normal attack 3 46 frames
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			actor.frame++;

			if (actor.frame == 28){
				if(hitcheck(apos, pos,fDir)){
					//actorHP=actorHP-1;
					if (npca.state != 9)
					{
						npca.blood-=20;
						npca.state = 7;
						npca.SetCurrentAction(NULL, 0, npcaDamageLID);
						npca.frame = 0;
					}	
				}
				if(hitcheck(bpos, pos,fDir)){
					//actorHP=actorHP-1;
					if (npcb.state != 9)
					{
						npcb.blood-=20;
						npcb.state = 7;
						npcb.SetCurrentAction(NULL, 0, npcbDamage1ID);
						npcb.frame = 0;
					}	
				}
			}
			if (actor.frame == 30)
			{
				attackKeyLocked = false;
			}
			if (actor.frame == 45)
			{
				if(normalCombo){
					actor.state = 6;
					actor.frame = 0;
					actor.SetCurrentAction(NULL, 0, normalAttack4ID);
					normalCombo = false;
				}
				else{
					actor.state = 2;
					actor.frame = 0;
					combatWait = 150;
					actor.SetCurrentAction(NULL, 0, combatIdleID);
					movementKeyLocked = false;
				}	
			}
			break;
		case 6:
		// normal attack 4 49 frames
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			actor.frame++;

			if (actor.frame == 30){
				if(hitcheck(apos, pos,fDir)){
					//actorHP=actorHP-1;
					if (npca.state != 9)
					{
						npca.blood-=30;
						npca.state = 7;
						npca.SetCurrentAction(NULL, 0, npcaDamageHID);
						npca.frame = 0;
					}	
				}
				if(hitcheck(bpos, pos,fDir)){
					//actorHP=actorHP-1;
					if (npcb.state != 9)
					{
						npcb.blood-=30;
						npcb.state = 7;
						npcb.SetCurrentAction(NULL, 0, npcbDamage2ID);
						npcb.frame = 0;
					}
				}
			}
			if (actor.frame == 44)
			{
				attackKeyLocked = false;
			}
			if (actor.frame == 48)
			{
				if(normalCombo){
					actor.state = 3;
					actor.frame = 0;
					actor.SetCurrentAction(NULL, 0, normalAttack1ID);
					normalCombo = false;
				}
				else{
					actor.state = 2;
					actor.frame = 0;
					combatWait = 150;
					actor.SetCurrentAction(NULL, 0, combatIdleID);
					movementKeyLocked = false;
				}	
			}
			break;
		case 7:
		// heavy attack 1 73 frames
			break;
		case 8:
		// heavy attack 2 59 frames
			break;
		case 9:
		// heavy attack 3 57 frames
			break;
		case 10:
		// ultimate attack 121 frames
			break;
		case 11:
		// guard 34 frames
			break;
		case 12:
		// heavy damage 24 frames
			break;
		case 13:
		// right damage 24 frames
			break;
		case 14:
		// left damage 20 frames
			break;
		case 15:
		// die
			actor.Play(ONCE, (float) skip, FALSE, TRUE);
			break;
		default:
			break;
	}
	
	// check npc die 
	if (npca.blood <= 0 && npca.state != 9)
	{
		npca.state = 9;
		npca.SetCurrentAction(NULL, 0, npcaDieID);
	}
	if (npcb.blood <= 0 && npcb.state != 9)
	{
		npcb.state = 9;
		npcb.SetCurrentAction(NULL, 0, npcbDieID);
	}

/*
	npca.state:
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
	switch(npca.state){
		case 0:
		// idle
			npca.Play(LOOP, (float) skip, FALSE, TRUE);
			break;
		case 1:
		// run
			npca.Play(LOOP, (float) skip, FALSE, TRUE);
			break;
		case 2:
		// attackL 1 36 frames
			break;
		case 3:
		// attackL 2 41 frames
			break;
		case 4:
		// attack H 86 frames
			break;
		case 5:
		// heavy attack 66 frames
			break;
		case 6:
		// defence 41 frames
			break;
		case 7:
		// damage L 26
			npca.frame++;
			npca.Play(ONCE, (float) skip, FALSE, TRUE);
			if (npca.frame == 25)
			{
				npca.state = 0;
				npca.SetCurrentAction(NULL, 0, npcaIdleID);
				npca.frame = 0;
			}
			break;
		case 8:
		// damageH 36
			npca.frame++;
			npca.Play(ONCE, (float) skip, FALSE, TRUE);
			if (npca.frame == 35)
			{
				npca.state = 0;
				npca.SetCurrentAction(NULL, 0, npcaIdleID);
				npca.frame = 0;
			}
			break;
		case 9:
		// die
			npca.Play(ONCE, (float) skip, FALSE, TRUE);
			break;
		default:
			break;
	}
	
/*
	npcb.state:
		0 combatIdle
		1 run
		2 moveRight
		3 moveLeft
		4 normalAttack1
		5 normalAttack2
		6 heavyAttack1
		7 damage1
		8 damage2
		9 die
*/
	switch(npcb.state){
		case 0:
		// combat idle
			npcb.Play(LOOP, (float) skip, FALSE, TRUE);
			break;
		case 1:
		// run
			npcb.Play(LOOP, (float) skip, FALSE, TRUE);
			break;
		case 2:
		// move right 
			npcb.Play(LOOP, (float) skip, FALSE, TRUE);
			break;
		case 3:
		// move left 
			npcb.Play(LOOP, (float) skip, FALSE, TRUE);
			break;
		case 4:
		// normal attack 1 36 frames
			break;
		case 5:
		// normal attack 2 26 frames
			break;
		case 6:
		// heavy attack 1 31 frames
			break;
		case 7:
		// damage1 16 frames
			npcb.frame++;
			npcb.Play(ONCE, (float) skip, FALSE, TRUE);
			if (npcb.frame == 15)
			{
				npcb.state = 0;
				npcb.SetCurrentAction(NULL, 0, npcbCombatIdleID);
				npcb.frame = 0;
			}
			break;
		case 8:
		// damage2 26 frames
			npcb.frame++;
			npcb.Play(ONCE, (float) skip, FALSE, TRUE);
			if (npcb.frame == 25)
			{
				npcb.state = 0;
				npcb.SetCurrentAction(NULL, 0, npcbCombatIdleID);
				npcb.frame = 0;
			}
			break;
		case 9:
		// die
			npcb.Play(ONCE, (float) skip, FALSE, TRUE);
			break;
		default:
			break;
	}

	

	//camera rotating
	cameraRotating();
	cameraZooming();



// camera hit test	
	object.GetPosition(opos);
	actor.GetPosition(pos);
	camera.GetDirection(cfDir, cuDir);

	float d_oa = myDist(opos, pos);

	if(terrain.HitTest(opos, ohitdir) > 0)
	{
		if(walk){
			if(d_oa < 800.0f)
			{
				object.MoveForward(-10.0f);
				object.GetPosition(opos);
				d_oa = myDist(opos, pos);
				if(d_oa > 800.0f){
					opos[2] = cameraHieght(800.0f);
					opos[0] = pos[0] - sqrt(640000.0f / (1 + (cfDir[1] * cfDir[1]) / (cfDir[0] * cfDir[0]))) * cfDir[0] / fabs(cfDir[0]);
					opos[1] = pos[1] - sqrt(640000.0f / (1 + (cfDir[0] * cfDir[0]) / (cfDir[1] * cfDir[1]))) * cfDir[1] / fabs(cfDir[1]);
					object.SetPosition(opos);
					cfDir[0] = pos[0] - opos[0];
					cfDir[1] = pos[1] - opos[1];
					cfDir[2] = pos[2] + 60.0f - opos[2];
					cuDir[2] =  (-cfDir[0] * cuDir[0] - cfDir[1] * cuDir[1]) / cfDir[2];
 					camera.SetDirection(cfDir, cuDir);
				}
				else{
					opos[2] = cameraHieght(d_oa);
					object.SetPosition(opos);
					cfDir[0] = pos[0] - opos[0];
					cfDir[1] = pos[1] - opos[1];
					cfDir[2] = pos[2] + 60.0f - opos[2];
					cuDir[2] =  (-cfDir[0] * cuDir[0] - cfDir[1] * cuDir[1]) / cfDir[2];
 					camera.SetDirection(cfDir, cuDir);
 				}
			}
		}
	}
	else 
	{
       	object.MoveForward(10.0f); 
       	object.GetPosition(opos);
		d_oa = myDist(opos, pos);
		opos[2] = cameraHieght(d_oa);
		object.SetPosition(opos);
		cfDir[0] = pos[0] - opos[0];
		cfDir[1] = pos[1] - opos[1];
		cfDir[2] = pos[2] + 60.0f - opos[2];
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

	float cpos[3], cfDir[3], cuDir[3], pos[3], apos[3], bpos[3];

	camera.GetPosition(cpos);
	camera.GetDirection(cfDir, cuDir);

 	actor.GetPosition(pos);
 	npca.GetPosition(apos);
 	npcb.GetPosition(bpos);

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

   char posS[256], aposS[256], bposS[256], distS[256];
   char cposS[256], cfDirS[256], cuDirS[256];
   char actorBloodS[256], npcaBloodS[256], npcbBloodS[256];
   char cameraRotateS[256], cameraDistanceS[256]; 

   sprintf(cposS, "cpos: %8.3f %8.3f %8.3f", cpos[0], cpos[1], cpos[2]);
   sprintf(cfDirS, "cfacing: %8.3f %8.3f %8.3f", cfDir[0], cfDir[1], cfDir[2]);
   sprintf(cuDirS, "cup: %8.3f %8.3f %8.3f", cuDir[0], cuDir[1], cuDir[2]);
   sprintf(posS, "pos: %8.3f %8.3f %8.3f", pos[0], pos[1], pos[2]);
   sprintf(aposS, "apos: %8.3f %8.3f %8.3f", apos[0], apos[1], apos[2]);
   sprintf(bposS, "bpos: %8.3f %8.3f %8.3f", bpos[0], bpos[1], bpos[2]);
   sprintf(distS, "dist: %8.3f", myDist(cpos, pos));
   sprintf(actorBloodS, "Actor HP: %d / %d", actor.blood, actor.fullBlood);
   sprintf(npcaBloodS, "Npc A HP: %d / %d", npca.blood, npca.fullBlood);
   sprintf(npcbBloodS, "Npc B HP: %d / %d", npcb.blood, npcb.fullBlood);
   sprintf(cameraRotateS, "cameraRotate: %d", cameraRotateState);
   sprintf(cameraDistanceS, "cameraDistance: %8.3f", cameraDistance);

   text.Write(cposS, 20, 35, 255, 255, 0);
   text.Write(cfDirS, 20, 50, 255, 255, 0);
   text.Write(cuDirS, 20, 65, 255, 255, 0);
   text.Write(distS, 20, 95, 255, 255, 0);
   text.Write(posS, 20, 110, 255, 255, 0);
   text.Write(aposS, 20, 125, 255, 255, 0);
   text.Write(bposS, 20, 140, 255, 255, 0);
   text.Write(actorBloodS, 20, 140, 255, 255, 0);
   text.Write(npcaBloodS, 20, 155, 255, 255, 0);
   text.Write(npcbBloodS, 20, 170, 255, 255, 0);
   text.Write(cameraRotateS, 20, 185, 255, 255, 0);
   text.Write(cameraDistanceS, 20, 200, 255, 255, 0);


   text.End();

   // swap buffer
   FySwapBuffers();
}


void cameraZoom(BYTE code, BOOL4 value){

	if(value){
		if(code == FY_Q){
			cameraZoomState = 1;
		}
		else if(code == FY_W){
			cameraZoomState = 2;
		}
		else{
			cameraZoomState = 0;
		}
	}
	else{
		cameraZoomState = 0;
	}
	
	
}

void cameraRotate(BYTE code, BOOL4 value){

	if(value){
		if(code == FY_A){
			cameraRotateState = 1;
		}
		else if(code == FY_S){
			cameraRotateState = 2;
		}
		else{
			cameraRotateState = 0;
		}
	}
	else{
		cameraRotateState = 0;
	}
	
	
}

/*------------------
  movement control
  C.Wang 1103, 2006
 -------------------*/
void Movement(BYTE code, BOOL4 value)
{
	if (value)
	{
		if (code == FY_UP)
		{
			moveKeyState[0] = true;
		}
		else if (code == FY_DOWN)
		{
			moveKeyState[1] = true;
		}
		else if (code == FY_RIGHT)
		{
			moveKeyState[2] = true;
		}
		else
		{
			moveKeyState[3] = true;
		}
	}
	else
	{
		if (code == FY_UP)
		{
			moveKeyState[0] = false;
		}
		else if (code == FY_DOWN)
		{
			moveKeyState[1] = false;
		}
		else if (code == FY_RIGHT)
		{
			moveKeyState[2] = false;
		}
		else
		{
			moveKeyState[3] = false;
		}
	}
}

void Attack(BYTE code, BOOL4 value)
{
	if(!attackKeyLocked){
		if(value){
			switch(actor.state){
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
					actor.state = 3;
					actor.frame = 0;
					actor.SetCurrentAction(NULL, 0, normalAttack1ID);
					attackKeyLocked = true;
					movementKeyLocked = true;
					break;
			}
		}
	}
}

void Attack1(BYTE code, BOOL4 value)
{
	if (value)
	{
		if (code == FY_X)
		{
			attackKeyState[0] = true;
		}
		else
		{
			attackKeyState[1] = true;
		}
	}
	else
	{
		if (code == FY_X)
		{
			attackKeyState[0] = false;
		}
		else
		{
			attackKeyState[1] = false;
		}
	}
}

void Reset(BYTE code, BOOL4 value)
{
	if (value)
	{
		npca.blood = npca.fullBlood;
		npcb.blood = npcb.fullBlood;
		npca.state = 0;
		npcb.state = 0;
		npca.SetCurrentAction(NULL, 0, npcaIdleID);
		npcb.SetCurrentAction(NULL, 0, npcbCombatIdleID);
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
