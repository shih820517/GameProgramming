#include "FlyWin32.h"
#define _USE_MATH_DEFINES 
#include <cmath>

#define IDLE			0
#define RUN				1
#define COMBATIDLE		2
#define ATTACK1			3
#define ATTACK2			4
#define ATTACK3			5
#define ATTACK4			6
#define ULT				7
#define DAMAGE			8
#define DIE				9

typedef int STATE;

#define NONE			-1
#define ACTOR 			0
#define NPCA 			1
#define NPCB 			2
#define NPCC 			3
#define NPCD 			4
#define NPCE 			5
#define NPCF 			6
#define NPCG 			7

class MyCharacter : public FnCharacter {
public:
	MyCharacter(int myBlood = 0){
		blood = myBlood;
		fullBlood = myBlood;
		frame = 0;
		state = 0;
		combatWait = 0;
		bloodBarID = FAILED_ID;
	}
	void setBB(FnScene scene){
		OBJECTid bbID = scene.CreateObject(OBJECT);
		FnObject bb(bbID);
		bb.SetParent(GetBaseObject());
		bb.Translate(0.0f, 0.0f, 95.0f, REPLACE);
		float size[2], color[4];

	    size[0] = 25.0f; size[1] = 1.2f;
		color[0] = 0.0f; color[3] = 0.0f; color[1] =1.0f; color[2] = 0.0f;
		bloodBarID = bb.Billboard(NULL, size, NULL, 0, color);
	}
	void BB(){
		FnBillboard bb(bloodBarID);
		float newSize[2];
		newSize[0]=25.0f*blood/fullBlood;
		newSize[1]=1.2f;
		bb.SetPositionSize(NULL, newSize);
	}
	int blood, fullBlood;
	int frame;
	STATE state;
	int combatWait;
	GEOMETRYid bloodBarID;
};

class MyNpc : public FnCharacter {
public:
	MyNpc(int myBlood = 0){
		blood = myBlood;
		fullBlood = myBlood;
		frame = 0;
		state = 0;
		wait1 = 0;
      wait2 = 0;
		isFriend = false;
      buff = 1;
      cold = 0;
		target = NONE;
		bloodBarID = FAILED_ID;
	}
	void setBB(FnScene scene){
		OBJECTid bbID = scene.CreateObject(OBJECT);
		FnObject bb(bbID);
		bb.SetParent(GetBaseObject());
		bb.Translate(0.0f, 0.0f, 95.0f, REPLACE);
		float size[2], color[4];

	    size[0] = 25.0f; size[1] = 1.2f;
		color[0] = 1.0f; color[3] = 1.0f; color[1] = 0.0f; color[2] = 0.0f;
		bloodBarID = bb.Billboard(NULL, size, NULL, 0, color);
	}
	void BB(){
		FnBillboard bb(bloodBarID);
		float newSize[2];
		newSize[0]=25.0f*blood/fullBlood;
		newSize[1]=1.2f;
		bb.SetPositionSize(NULL, newSize);
	}
	int blood, fullBlood;
	int frame;
	STATE state;
	int wait1, wait2;
	bool isFriend;
	int target;
   int buff, cold;
	GEOMETRYid bloodBarID;
	ACTIONid idleID, runID, dieID;
	ACTIONid attack1ID, attack2ID; // npc attack
	ACTIONid damageID; // npc damage
};

VIEWPORTid vID;                 // the major viewport
SCENEid sID;                    // the 3D scene
SCENEid sID2;					// the 2D scene

OBJECTid cID, tID, oID;         // the main camera and the terrain for terrain following

OBJECTid spID0 = FAILED_ID;		// this is a sprite id for UI

CHARACTERid actorID; // the major character
CHARACTERid npcaID, npcbID, npccID, npcdID, npceID, npcfID, npcgID;
CHARACTERid bossID;
/* actor actions */
ACTIONid idleID, runID, dieID, combatIdleID, guardID, curPoseID; // actor move & die
ACTIONid normalAttack1ID, normalAttack2ID, normalAttack3ID, normalAttack4ID, ultimateAttackID; // actor normal attack
ACTIONid heavyDamagedID; // actor hurt

ROOMid terrainRoomID = FAILED_ID;
TEXTid textID = FAILED_ID;

GAMEFX_SYSTEMid gFXID = FAILED_ID;
GAMEFX_SYSTEMid dFXID = FAILED_ID;

AUDIOid mmID;//bkground
AUDIOid atID;//actor
AUDIOid npcID;//npcA
AUDIOid npc2ID;//npcB
AUDIOid pauseID;

// some globals
int frame = 0;
int oldX, oldY, oldXM, oldYM, oldXMM, oldYMM;

/*
0 UP
1 DOWN
2 LEFT
3 RIGHT
*/
bool moveKeyState[4] = {false, false, false, false};

bool pause = false;

/*
0 normal attack
1 heavy attack
*/
bool attackKeyState = false;

bool attackKeyLocked = false;
bool movementKeyLocked = false;
bool normalCombo = false;
bool isFollow[2] = {true, true};
int teammateID[2] = {NONE, NONE};

int cameraRotateState = 0;

int cameraZoomState = 0;

float cameraDistance = 700.0f;

// hotkey callbacks
void QuitGame(BYTE, BOOL4);
void Movement(BYTE, BOOL4);
void Attack(BYTE, BOOL4);
void NpcControl(BYTE, BOOL4);
void Reset(BYTE, BOOL4);
void cameraRotate(BYTE, BOOL4);
void cameraZoom(BYTE, BOOL4);
void PauseGame(BYTE, BOOL4);

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
inline float myDist(float *npcapos, float *npcbpos); //find distance
inline float cameraHieght(float dist); // give the hieght we need
bool isHit(float *npcapos, float *npcbpos, float *npcafDir, float attackDist, float attackAngle);
inline float turnDegree(float dist, float stepLength);
int nearestFriend(int myID, float *mypos);
int nearestEnemy(int myID, float *mypos);
bool moving();

MyNpc npca(200);
MyNpc npcb(200);
MyNpc npcc(200);
MyNpc npcd(200);
MyNpc npce(200);
MyNpc npcf(200);
MyNpc npcg(200);

MyCharacter actor(300);

/*------------------------------------------
 ||      ||    ||||      ||||||   ||||    ||
 ||||  ||||   ||  ||       ||     || ||   ||
 || |||| ||  ||    ||      ||     ||  ||  ||
 ||  ||  || ||||||||||     ||     ||   || ||
 ||      || ||      ||   ||||||   ||    ||||
 ------------------------------------------*/
void FyMain(int argc, char **argv)
{
	// create a new world
   	BOOL4 beOK = FyStartFlyWin32("NTU@2014 Homework #03", 0, 0, 1024, 768, FALSE);

   	// setup the data searching paths
   	FySetShaderPath("Data\\NTU5\\Shaders");
   	FySetModelPath("Data\\NTU5\\Scenes");
   	FySetTexturePath("Data\\NTU5\\Scenes\\Textures");
   	FySetScenePath("Data\\NTU5\\Scenes");
   	FySetGameFXPath("Data\\NTU5\\GameFXFiles\\FX2");

   	//setup the music 
	FySetAudioPath("Data\\NTU5\\sound");
	mmID = FyCreateAudio();
	FnAudio mP;
	mP.Object(mmID);
	mP.Load("bk2.wav");
	mP.Play(LOOP);


	

	// create a viewport
	vID = FyCreateViewport(0, 0, 1024, 768);
	FnViewport vp;
	vp.ID(vID);

	// create a 3D scene
	sID = FyCreateScene(10);
	FnScene scene;
	scene.ID(sID);

	//create a 2D scene for UI
	sID2 = FyCreateScene(1);
	FnScene spritescene;
	spritescene.Object(sID2);
	spritescene.SetSpriteWorldSize(1024, 768);

	//After create scene then create a sprite for user interface
	FnSprite sp;

	spID0 = spritescene.CreateObject(SPRITE);
	sp.Object(spID0);
	sp.SetSize(1024, 350);
	sp.SetImage("lbj", 0, NULL, FALSE, NULL, 2, TRUE, FILTER_LINEAR);
	sp.SetPosition(0, 256, 0);

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

	// put the character on terrain
	float pos[3], fDir[3], uDir[3];

	/*-----------------------------------------------------
	    ||||     ||||||||   ||||||||   ||||||||  |||||||||
	   ||  ||   ||      ||     ||     ||      || ||      ||
	  ||    ||  ||             ||     ||      || |||||||||
	 |||||||||| ||      ||     ||     ||      || ||     ||
	 ||      ||  ||||||||      ||      ||||||||  ||      ||
	 -----------------------------------------------------*/
	actorID = scene.LoadCharacter("Lyubu2");
	actor.ID(actorID);

	pos[0] = 3586.0f; pos[1] = -3523.0f; pos[2] = 1000.0f;
	fDir[0] = 0.0f; fDir[1] = 1.0f; fDir[2] = 0.0f;
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
	ultimateAttackID = actor.GetBodyAction(NULL, "UltimateAttack");
	heavyDamagedID = actor.GetBodyAction(NULL, "HeavyDamaged");
	dieID = actor.GetBodyAction(NULL, "Die");

	// set the actor to idle action
	curPoseID = idleID;
	actor.SetCurrentAction(NULL, 0, curPoseID);
	actor.Play(START, 0.0f, FALSE, TRUE);

	actor.setBB(scene);

	/*------------------------------------------
	 ||||    || |||||||||   ||||||||     ||||
	 || ||   || ||      || ||      ||   ||  ||
	 ||  ||  || |||||||||  ||          ||    ||
	 ||   || || ||         ||      || ||||||||||
	 ||    |||| ||          ||||||||  ||      ||
	 ------------------------------------------*/
	npcaID = scene.LoadCharacter("Donzo2");
	npca.ID(npcaID);

	pos[0] = 585.0f; pos[1] = 2981.0f; pos[2] = 1000.0f;
	fDir[0] = 0.0f; fDir[1] = -1.0f; fDir[2] = 0.0f;
	uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
	npca.SetDirection(fDir, uDir);

	npca.SetTerrainRoom(terrainRoomID, 10.0f);
	beOK = npca.PutOnTerrain(pos);

	// Get actions
	npca.idleID = npca.GetBodyAction(NULL, "Idle");
	npca.runID = npca.GetBodyAction(NULL, "Run");
	npca.attack1ID = npca.GetBodyAction(NULL, "AttackL1");
	npca.attack2ID = npca.GetBodyAction(NULL, "HeavyAttack");
	npca.damageID = npca.GetBodyAction(NULL, "DamageL");
	npca.dieID = npca.GetBodyAction(NULL, "Die");

	// set the character to idle action
	curPoseID = npca.idleID;
	npca.SetCurrentAction(NULL, 0, curPoseID);
	npca.Play(START, 0.0f, FALSE, TRUE);
	
	npca.setBB(scene);

	/*------------------------------------------
	 ||||    || |||||||||   ||||||||  |||||||||
	 || ||   || ||      || ||      || ||      ||
	 ||  ||  || |||||||||  ||         |||||||||
	 ||   || || ||         ||      || ||      ||
	 ||    |||| ||          ||||||||  |||||||||
	 ------------------------------------------*/
	npcbID = scene.LoadCharacter("Robber02");
	npcb.ID(npcbID);

	pos[0] = 685.0f; pos[1] = 2981.0f; pos[2] = 1000.0f;
	fDir[0] = 0.0f; fDir[1] = -1.0f; fDir[2] = 0.0f;
	uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
	npcb.SetDirection(fDir, uDir);

	npcb.SetTerrainRoom(terrainRoomID, 10.0f);
	beOK = npcb.PutOnTerrain(pos);

	// Get actions
	npcb.idleID = npcb.GetBodyAction(NULL, "CombatIdle");
	npcb.runID = npcb.GetBodyAction(NULL, "Run");
	npcb.attack1ID = npcb.GetBodyAction(NULL, "NormalAttack1");
	npcb.attack2ID = npcb.GetBodyAction(NULL, "HeavyAttack1");
	npcb.damageID = npcb.GetBodyAction(NULL, "Damage1");
	npcb.dieID = npcb.GetBodyAction(NULL, "Die");

	// set the character to idle action
	curPoseID = npcb.idleID;
	npcb.SetCurrentAction(NULL, 0, curPoseID);
	npcb.Play(START, 0.0f, FALSE, TRUE);
	
	npcb.setBB(scene);

	/*------------------------------------------
	 ||||    || |||||||||   ||||||||   |||||||| 
	 || ||   || ||      || ||      || ||      ||
	 ||  ||  || |||||||||  ||         ||
	 ||   || || ||         ||      || ||      ||
	 ||    |||| ||          ||||||||   ||||||||
	 ------------------------------------------*/
	npccID = scene.LoadCharacter("Robber02");
	npcc.ID(npccID);

	pos[0] = 485.0f; pos[1] = 2981.0f; pos[2] = 1000.0f;
	fDir[0] = 0.0f; fDir[1] = -1.0f; fDir[2] = 0.0f;
	uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
	npcc.SetDirection(fDir, uDir);

	npcc.SetTerrainRoom(terrainRoomID, 10.0f);
	beOK = npcc.PutOnTerrain(pos);

	// Get actions
	npcc.idleID = npcc.GetBodyAction(NULL, "CombatIdle");
	npcc.runID = npcc.GetBodyAction(NULL, "Run");
	npcc.attack1ID = npcc.GetBodyAction(NULL, "NormalAttack1");
	npcc.attack2ID = npcc.GetBodyAction(NULL, "HeavyAttack1");
	npcc.damageID = npcc.GetBodyAction(NULL, "Damage1");
	npcc.dieID = npcc.GetBodyAction(NULL, "Die");

	// set the character to idle action
	curPoseID = npcc.idleID;
	npcc.SetCurrentAction(NULL, 0, curPoseID);
	npcc.Play(START, 0.0f, FALSE, TRUE);
	
	npcc.setBB(scene);

	/*------------------------------------------
	 ||||    || |||||||||   ||||||||  ||||||||
	 || ||   || ||      || ||      || ||      ||
	 ||  ||  || |||||||||  ||         ||      ||
	 ||   || || ||         ||      || ||      ||
	 ||    |||| ||          ||||||||  ||||||||
	 ------------------------------------------*/
	npcdID = scene.LoadCharacter("AM001");
	npcd.ID(npcdID);

	pos[0] = 3586.0f; pos[1] = -2553.0f; pos[2] = 1000.0f;
	fDir[0] = 0.0f; fDir[1] = -1.0f; fDir[2] = 0.0f;
	uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
	npcd.SetDirection(fDir, uDir);

	npcd.SetTerrainRoom(terrainRoomID, 10.0f);
	beOK = npcd.PutOnTerrain(pos);

	// Get actions
	npcd.idleID = npcd.GetBodyAction(NULL, "Idle");
	npcd.runID = npcd.GetBodyAction(NULL, "Run");
	npcd.attack1ID = npcd.GetBodyAction(NULL, "Attack");
	npcd.attack2ID = npcd.GetBodyAction(NULL, "Skill2");
	npcd.damageID = FAILED_ID;// no damage
	npcd.dieID = npcd.GetBodyAction(NULL, "Die");

	// set the character to idle action
	curPoseID = npcd.idleID;
	npcd.SetCurrentAction(NULL, 0, curPoseID);
	npcd.Play(START, 0.0f, FALSE, TRUE);
	
	npcd.setBB(scene);

	/*------------------------------------------
	 ||||    || |||||||||   ||||||||  ||||||||||
	 || ||   || ||      || ||      || ||
	 ||  ||  || |||||||||  ||         ||||||||
	 ||   || || ||         ||      || ||
	 ||    |||| ||          ||||||||  ||||||||||
	 ------------------------------------------*/
	npceID = scene.LoadCharacter("AMA001");
	npce.ID(npceID);

	pos[0] = 1154.0f; pos[1] = -2086.0f; pos[2] = 1000.0f;
	fDir[0] = 0.0f; fDir[1] = -1.0f; fDir[2] = 0.0f;
	uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
	npce.SetDirection(fDir, uDir);

	npce.SetTerrainRoom(terrainRoomID, 10.0f);
	beOK = npce.PutOnTerrain(pos);

	// Get actions
	npce.idleID = npce.GetBodyAction(NULL, "Idle");
	npce.runID = npce.GetBodyAction(NULL, "Run");
	npce.attack1ID = npce.GetBodyAction(NULL, "Attack");
	npce.attack2ID = npce.GetBodyAction(NULL, "Attack");
	npce.damageID = FAILED_ID;
	npce.dieID = npce.GetBodyAction(NULL, "Die");

	// set the character to idle action
	curPoseID = npce.idleID;
	npce.SetCurrentAction(NULL, 0, curPoseID);
	npce.Play(START, 0.0f, FALSE, TRUE);
	
	npce.setBB(scene);

	/*------------------------------------------
	 ||||    || |||||||||   ||||||||  ||||||||||
	 || ||   || ||      || ||      || ||
	 ||  ||  || |||||||||  ||         ||||||||
	 ||   || || ||         ||      || ||
	 ||    |||| ||          ||||||||  ||
	 ------------------------------------------*/
	npcfID = scene.LoadCharacter("CA002");
	npcf.ID(npcfID);

	pos[0] = -1734.0f; pos[1] = -3555.0f; pos[2] = 1000.0f;
	fDir[0] = 0.0f; fDir[1] = 1.0f; fDir[2] = 0.0f;
	uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
	npcf.SetDirection(fDir, uDir);

	npcf.SetTerrainRoom(terrainRoomID, 10.0f);
	beOK = npcf.PutOnTerrain(pos);

	// Get actions
	npcf.idleID = npcf.GetBodyAction(NULL, "Idle");
	npcf.runID = npcf.GetBodyAction(NULL, "Run");
	npcf.attack1ID = npcf.GetBodyAction(NULL, "Attack");
	npcf.attack2ID = npcf.GetBodyAction(NULL, "Skill");
	npcf.damageID = FAILED_ID;
	npcf.dieID = npcf.GetBodyAction(NULL, "Die");

	// set the character to idle action
	curPoseID = npcf.idleID;
	npcf.SetCurrentAction(NULL, 0, curPoseID);
	npcf.Play(START, 0.0f, FALSE, TRUE);
	
	npcf.setBB(scene);

	/*------------------------------------------
	 ||||    || |||||||||   ||||||||   ||||||||
	 || ||   || ||      || ||      || ||      
	 ||  ||  || |||||||||  ||         ||    |||
	 ||   || || ||         ||      || ||      ||
	 ||    |||| ||          ||||||||   ||||||||
	 ------------------------------------------*/
	npcgID = scene.LoadCharacter("CA005");
	npcg.ID(npcgID);

	pos[0] = -1525.0f; pos[1] = -264.0f; pos[2] = 1000.0f;
	fDir[0] = -1.0f; fDir[1] = 0.0f; fDir[2] = 0.0f;
	uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
	npcg.SetDirection(fDir, uDir);

	npcg.SetTerrainRoom(terrainRoomID, 10.0f);
	beOK = npcg.PutOnTerrain(pos);

	// Get actions
	npcg.idleID = npcg.GetBodyAction(NULL, "Idle");
	npcg.runID = npcg.GetBodyAction(NULL, "Run");
	npcg.attack1ID = npcg.GetBodyAction(NULL, "Attack");
	npcg.attack2ID = npcg.GetBodyAction(NULL, "Skill");
	npcg.damageID = FAILED_ID;
	npcg.dieID = npcg.GetBodyAction(NULL, "Die");

	// set the character to idle action
	curPoseID = npcg.idleID;
	npcg.SetCurrentAction(NULL, 0, curPoseID);
	npcg.Play(START, 0.0f, FALSE, TRUE);
	
	npcg.setBB(scene);




	// create object
	oID = scene.CreateObject(OBJECT);
	FnObject object;
	object.ID(oID);

	// set object initial position and orientation
	pos[0] = 3586.0f; pos[1] = -4223.0f; pos[2] = 100.0f;
	fDir[0] = 0.0f; fDir[1] = 1.0f; fDir[2] = 0.0f;
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
	fDir[0] = 0.0f; fDir[1] = 1.0f; fDir[2] = -0.08f;
	uDir[0] = 0.0f; uDir[1] = 0.08f; uDir[2] = 1.0f;

	camera.SetDirection(fDir, uDir);

	// setup a point light
	FnLight lgt;
	lgt.ID(scene.CreateObject(LIGHT));
	lgt.Translate(70.0f, -70.0f, 70.0f, REPLACE);
	lgt.SetColor(1.0f, 1.0f, 1.0f);
	lgt.SetIntensity(1.0f);

	// create a text object for displaying messages on screen
	textID = FyCreateText("Trebuchet MS", 18, FALSE, FALSE);

	// set Hotkeys
	FyDefineHotKey(FY_TAB, PauseGame, FALSE);

	FyDefineHotKey(FY_ESCAPE, QuitGame, FALSE);  // escape for quiting the game
	FyDefineHotKey(FY_UP, Movement, FALSE);      // Up for moving forward
	FyDefineHotKey(FY_RIGHT, Movement, FALSE);   // Right for turning right
	FyDefineHotKey(FY_LEFT, Movement, FALSE);    // Left for turning left
	FyDefineHotKey(FY_DOWN, Movement, FALSE);    // Down for moving back
	FyDefineHotKey(FY_Z, Attack, FALSE);
	FyDefineHotKey(FY_A, NpcControl, FALSE);
	FyDefineHotKey(FY_S, NpcControl, FALSE);
	FyDefineHotKey(FY_D, NpcControl, FALSE);
	FyDefineHotKey(FY_F, NpcControl, FALSE);
	FyDefineHotKey(FY_F1, Reset, FALSE);
	FyDefineHotKey(FY_1, cameraRotate, FALSE);
	FyDefineHotKey(FY_2, cameraRotate, FALSE);

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

inline float myDist(float *npcapos, float *npcbpos)
{
	return sqrt((npcapos[0]-npcbpos[0])*(npcapos[0]-npcbpos[0])+(npcapos[1]-npcbpos[1])*(npcapos[1]-npcbpos[1]));
}

inline float cameraHieght(float dist)
{
	return sqrt(491600.0f - dist * dist) + 60.0f;
}

bool isHit(float *npcapos, float *npcbpos, float *npcafDir, float attackDist, float attackAngle) 
{
	float dist = myDist(npcapos, npcbpos);
	float cosTheta = ((npcbpos[0] - npcapos[0]) * npcafDir[0] + (npcbpos[1] - npcapos[1]) * npcafDir[1]) / (dist * sqrt(npcafDir[0] * npcafDir[0] + npcafDir[1] * npcafDir[1]));
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
		if((cameraDistance+10.0f) <= 700.0f){
			cameraDistance += 10;
		}
		else cameraDistance = 700.0f;
		
	}
	else if(cameraZoomState == 0){

	}
}

int nearestFriend(int myID, float *mypos)
{
	float pos[3], npcapos[3], npcbpos[3], npccpos[3], npcdpos[3], npcepos[3], npcfpos[3], npcgpos[3];
	float shortestDist = 0.0f;
	int result = NONE;
	npca.GetPosition(npcapos);
	npcb.GetPosition(npcbpos);
	npcc.GetPosition(npccpos);
	npcd.GetPosition(npcdpos);
	npce.GetPosition(npcepos);
	npcf.GetPosition(npcfpos);
	npcg.GetPosition(npcgpos);
	actor.GetPosition(pos);

   shortestDist = myDist(mypos, pos);
   result = ACTOR;

   if (myID != NPCD && npcd.isFriend && npcd.state != DIE)
   {
      if (shortestDist > myDist(mypos, npcdpos))
      {
         shortestDist = myDist(mypos, npcdpos);
         result = NPCD;
      }
   }
   if (myID != NPCE && npce.isFriend && npce.state != DIE)
   {
      if (shortestDist > myDist(mypos, npcepos))
      {
         shortestDist = myDist(mypos, npcepos);
         result = NPCE;
      }
   }
   if (myID != NPCF && npcf.isFriend && npcf.state != DIE)
   {
      if (shortestDist > myDist(mypos, npcfpos))
      {
         shortestDist = myDist(mypos, npcfpos);
         result = NPCF;
      }
   }
   if (myID != NPCG && npcg.isFriend && npcg.state != DIE)
   {
      if (shortestDist > myDist(mypos, npcgpos))
      {
         shortestDist = myDist(mypos, npcgpos);
         result = NPCG;
      }
   }
   if (shortestDist > 500.0f)
   {
      result = NONE;
   }

   return result;
}

int nearestEnemy(int myID, float *mypos)
{
	float npcapos[3], npcbpos[3], npccpos[3], npcdpos[3], npcepos[3], npcfpos[3], npcgpos[3];
	float shortestDist = 0.0f;
	int result = NONE;
	npca.GetPosition(npcapos);
	npcb.GetPosition(npcbpos);
	npcc.GetPosition(npccpos);
	npcd.GetPosition(npcdpos);
	npce.GetPosition(npcepos);
	npcf.GetPosition(npcfpos);
	npcg.GetPosition(npcgpos);

	shortestDist = myDist(mypos, npcapos);
	result = NPCA;

	if (npcb.state != DIE)
	{
		if (shortestDist > myDist(mypos, npcbpos))
		{
			shortestDist = myDist(mypos, npcbpos);
			result = NPCB;
		}
	}
	if (npcc.state != DIE)
	{
		if (shortestDist > myDist(mypos, npccpos))
		{	
			shortestDist = myDist(mypos, npccpos);
			result = NPCC;
		}
	}
	if (myID != NPCD && !npcd.isFriend && npcd.state != DIE)
	{
		if (shortestDist > myDist(mypos, npcdpos))
		{
			shortestDist = myDist(mypos, npcdpos);
			result = NPCD;
		}
	}
	if (myID != NPCE && !npce.isFriend && npce.state != DIE)
	{
		if (shortestDist > myDist(mypos, npcepos))
		{
			shortestDist = myDist(mypos, npcepos);
			result = NPCE;
		}
	}
	if (myID != NPCF && !npcf.isFriend && npcf.state != DIE)
	{
		if (shortestDist > myDist(mypos, npcfpos))
		{
			shortestDist = myDist(mypos, npcfpos);
			result = NPCF;
		}
	}
	if (myID != NPCG && !npcg.isFriend && npcg.state != DIE)
	{
		if (shortestDist > myDist(mypos, npcgpos))
		{
			shortestDist = myDist(mypos, npcgpos);
			result = NPCG;
		}
	}
	if (shortestDist > 600.0f)
	{
      result = NONE;
	}
	if (result == NPCA && npca.state == DIE)
	{
		result = NONE;
	}

	return result;
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
		if (actor.combatWait)
		{
			actor.state = COMBATIDLE;
			actor.SetCurrentAction(NULL, 0, combatIdleID);
			actor.frame = 0;
		}
		else
		{
			actor.state = IDLE;
			actor.SetCurrentAction(NULL, 0, idleID);
			actor.frame = 0;
		}
		return false;
	}
}

/*----------------------------------------------------------------
  ||||||||     ||||    ||      || ||||||||||    ||||      ||||||  
 ||           ||  ||   ||||  |||| ||           ||  ||       ||   
 ||    |||   ||    ||  || |||| || ||||||||    ||    ||      ||   
 ||      || |||||||||| ||  ||  || ||         ||||||||||     ||   
  ||||||||  ||      || ||      || |||||||||| ||      ||   ||||||   
 ----------------------------------------------------------------*/
void GameAI(int skip)
{
	if(!pause)
	{
		FnCamera camera;
		FnObject object, terrain;
		atID = FyCreateAudio();
		FnAudio aP;
		aP.Object(atID);

		npcID = FyCreateAudio();
		FnAudio hP;
		hP.Object(npcID);


		npc2ID = FyCreateAudio();
		FnAudio hP2;
		hP2.Object(npc2ID);

		bool walk = false;
		 
		float pos[3], fDir[3], uDir[3], ohitdir[3];
		float opos[3], cfDir[3], cuDir[3];
		float npcapos[3], npcafDir[3], npcauDir[3];
		float npcbpos[3], npcbfDir[3], npcbuDir[3];
		float npccpos[3], npccfDir[3], npccuDir[3];
		float npcdpos[3], npcdfDir[3], npcduDir[3];
		float npcepos[3], npcefDir[3], npceuDir[3];
		float npcfpos[3], npcffDir[3], npcfuDir[3];
		float npcgpos[3], npcgfDir[3], npcguDir[3];

	    ohitdir[0] = 0.0f; ohitdir[1] = 0.0f; ohitdir[2] = -1.0f;

		object.ID(oID);
		camera.ID(cID);
		terrain.ID(tID);	

		FnGameFXSystem gxS(gFXID);//actor
		FnGameFXSystem dxS(dFXID); //Don

		if(!attackKeyLocked){
			if(attackKeyState){
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
					case 6:
						break;
					default:
						actor.state = ATTACK1;
						actor.SetCurrentAction(NULL, 0, normalAttack1ID);
						actor.frame = 0;
						attackKeyLocked = true;
						movementKeyLocked = true;

						FnScene scene(sID);
						gFXID = scene.CreateGameFXSystem();
						OBJECTid baseID = actor.GetBaseObject();
						FnGameFXSystem gxS(gFXID);
						BOOL4 beOK = gxS.Load("Lyubu_skill01", TRUE);
						gxS.SetParentObjectForAll(baseID);
						

						break;
				}
			}
		}

		// play game FX
		if (gFXID != FAILED_ID) 
		{
			FnGameFXSystem gxS(gFXID);
			BOOL4 beOK = gxS.Play((float) skip, ONCE);
			if (!beOK) 
			{
				FnScene scene(sID);
				scene.DeleteGameFXSystem(gFXID);
				gFXID = FAILED_ID;
			}
		}

					// play game FX
		if (dFXID != FAILED_ID) 
		{
			FnGameFXSystem dxS(dFXID);
			BOOL4 beOK = dxS.Play((float) skip, ONCE);
			if (!beOK) 
			{
				FnScene scene(sID);
				scene.DeleteGameFXSystem(dFXID);
				dFXID = FAILED_ID;
			}
		}


		actor.GetPosition(pos);
		npca.GetPosition(npcapos);
		npcb.GetPosition(npcbpos);
		npcc.GetPosition(npccpos);
		npcd.GetPosition(npcdpos);
		npce.GetPosition(npcepos);
		npcf.GetPosition(npcfpos);
		npcg.GetPosition(npcgpos);

		actor.GetDirection(fDir, uDir);
		npca.GetDirection(npcafDir, npcauDir);
		npcb.GetDirection(npcbfDir, npcbuDir);
		npcc.GetDirection(npccfDir, npccuDir);
		npcd.GetDirection(npcdfDir, npcduDir);
		npce.GetDirection(npcefDir, npceuDir);
		npcf.GetDirection(npcffDir, npcfuDir);
		npcg.GetDirection(npcgfDir, npcguDir);

		/*-----------------------------------------------------
		    ||||     ||||||||   ||||||||   ||||||||  |||||||||
		   ||  ||   ||      ||     ||     ||      || ||      ||
		  ||    ||  ||             ||     ||      || |||||||||
		 |||||||||| ||      ||     ||     ||      || ||     ||
		 ||      ||  ||||||||      ||      ||||||||  ||      ||
		 -----------------------------------------------------*/
		// check if running
		if (actor.state != RUN && !movementKeyLocked && (moveKeyState[0] || moveKeyState[1] || moveKeyState[2] || moveKeyState[3]))
		{
			actor.state = RUN;
			actor.SetCurrentAction(NULL, 0, runID);
		}

		// check if die
		if (actor.blood <= 0 && actor.state != DIE)
		{
			actor.state = DIE;
			actor.SetCurrentAction(NULL, 0, dieID);
			attackKeyLocked = true;
			movementKeyLocked = true;
					
			//	gxS.Load("LyuDie", TRUE);
			//	gxS.Play(skip, ONCE);
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
				7 ultimateAttack
				8 heavyDamaged
				9 die
		*/
		switch(actor.state)
		{
			case IDLE:
			// idle			
				actor.Play(LOOP, (float) skip, FALSE, TRUE);
				break;
			case RUN:
			// run
				actor.Play(LOOP, (float) skip, FALSE, TRUE);
				if (actor.combatWait > 0)
				{
					actor.combatWait--;
				}
				walk = moving();
				break;
			case COMBATIDLE:
			// combat idle
				actor.Play(LOOP, (float) skip, FALSE, TRUE);
				if (actor.combatWait > 0)
				{
					actor.combatWait--;
				}
				if (actor.combatWait == 0)
				{
					actor.state = IDLE;
					actor.SetCurrentAction(NULL, 0, idleID);
					actor.frame = 0;
				}
				if ( moveKeyState[0] || moveKeyState[1] || moveKeyState[2] || moveKeyState[3] )
				{
					actor.state = RUN;
					actor.SetCurrentAction(NULL, 0, runID);
					actor.frame = 0;
				}
				break;
			case ATTACK1:
			// normal attack 1 25 frames	
				actor.Play(ONCE, (float) skip, FALSE, TRUE);
				actor.frame++;
				if (actor.frame == 5)
				{	
					aP.Load("att1.wav");
					aP.Play(ONCE);
					if (npca.state != DIE && !npca.isFriend)
					{
						if(isHit(pos, npcapos, fDir, 180.0f, 40.0f))
						{
							npca.blood -= 10;
							npca.state = DAMAGE;
							npca.SetCurrentAction(NULL, 0, npca.damageID);
							npca.frame = 0;
						}		
					}
					if (npcb.state != DIE && !npcb.isFriend)
					{
						if(isHit(pos, npcbpos, fDir, 180.0f, 40.0f))
						{
							npcb.blood -= 10;
							npcb.state = DAMAGE;
							npcb.SetCurrentAction(NULL, 0, npcb.damageID);
							npcb.frame = 0;
						}		
					}
					if (npcc.state != DIE && !npcc.isFriend)
					{
						if(isHit(pos, npccpos, fDir, 180.0f, 40.0f))
						{
							npcc.blood -= 10;
							npcc.state = DAMAGE;
							npcc.SetCurrentAction(NULL, 0, npcc.damageID);
							npcc.frame = 0;
						}		
					}
					if (npcd.state != DIE && !npcd.isFriend)
					{
						if(isHit(pos, npcdpos, fDir, 180.0f, 40.0f))
						{
							npcd.blood -= 10;
							npcd.state = DAMAGE;
							npcd.SetCurrentAction(NULL, 0, npcd.damageID);
							npcd.frame = 0;
						}		
					}
					if (npce.state != DIE && !npce.isFriend)
					{
						if(isHit(pos, npcepos, fDir, 180.0f, 40.0f))
						{
							npce.blood -= 10;
							npce.state = DAMAGE;
							npce.SetCurrentAction(NULL, 0, npce.damageID);
							npce.frame = 0;
						}		
					}
					if (npcf.state != DIE && !npcf.isFriend)
					{
						if(isHit(pos, npcfpos, fDir, 180.0f, 40.0f))
						{
							npcf.blood -= 10;
							npcf.state = DAMAGE;
							npcf.SetCurrentAction(NULL, 0, npcf.damageID);
							npcf.frame = 0;
						}		
					}
					if (npcg.state != DIE && !npcg.isFriend)
					{
						if(isHit(pos, npcgpos, fDir, 180.0f, 40.0f))
						{
							npcg.blood -= 10;
							npcg.state = DAMAGE;
							npcg.SetCurrentAction(NULL, 0, npcg.damageID);
							npcg.frame = 0;
						}		
					}
				}
				if (actor.frame == 12)
				{
					attackKeyLocked = false;
				}
				if (actor.frame == 24)
				{
					if(normalCombo)
					{
						actor.state = ATTACK2;
						actor.SetCurrentAction(NULL, 0, normalAttack2ID);
						actor.frame = 0;
						normalCombo = false;
					}
					else
					{
						actor.state = COMBATIDLE;
						actor.combatWait = 150;
						actor.SetCurrentAction(NULL, 0, combatIdleID);
						actor.frame = 0;
						movementKeyLocked = false;
					}	
				}
				break;
			case ATTACK2:
			// normal attack 2 48 frames
				actor.Play(ONCE, (float) skip, FALSE, TRUE);
				actor.frame++;

				if (actor.frame == 3){
						FnScene scene(sID);
						gFXID = scene.CreateGameFXSystem();
						OBJECTid baseID = actor.GetBaseObject();
						FnGameFXSystem gxS(gFXID);
						BOOL beOK = gxS.Load("Lyuatt3", TRUE);
						gxS.SetParentObjectForAll(baseID);
						aP.Load("att3.wav");
						aP.Play(ONCE);
				}
				if (actor.frame == 23)
				{
					if (npca.state != DIE && !npca.isFriend)
					{
						if(isHit(pos, npcapos, fDir, 180.0f, 360.0f))
						{
							npca.blood -= 20;
							npca.state = DAMAGE;
							npca.SetCurrentAction(NULL, 0, npca.damageID);
							npca.frame = 0;
						}		
					}
					if (npcb.state != DIE && !npcb.isFriend)
					{
						if(isHit(pos, npcbpos, fDir, 180.0f, 360.0f))
						{
							npcb.blood -= 20;
							npcb.state = DAMAGE;
							npcb.SetCurrentAction(NULL, 0, npcb.damageID);
							npcb.frame = 0;
						}		
					}
					if (npcc.state != DIE && !npcc.isFriend)
					{
						if(isHit(pos, npccpos, fDir, 180.0f, 360.0f))
						{
							npcc.blood -= 20;
							npcc.state = DAMAGE;
							npcc.SetCurrentAction(NULL, 0, npcc.damageID);
							npcc.frame = 0;
						}		
					}
					if (npcd.state != DIE && !npcd.isFriend)
					{
						if(isHit(pos, npcdpos, fDir, 180.0f, 360.0f))
						{
							npcd.blood -= 20;
							npcd.state = DAMAGE;
							npcd.SetCurrentAction(NULL, 0, npcd.damageID);
							npcd.frame = 0;
						}		
					}
					if (npce.state != DIE && !npce.isFriend)
					{
						if(isHit(pos, npcepos, fDir, 180.0f, 360.0f))
						{
							npce.blood -= 20;
							npce.state = DAMAGE;
							npce.SetCurrentAction(NULL, 0, npce.damageID);
							npce.frame = 0;
						}		
					}
					if (npcf.state != DIE && !npcf.isFriend)
					{
						if(isHit(pos, npcfpos, fDir, 180.0f, 360.0f))
						{
							npcf.blood -= 20;
							npcf.state = DAMAGE;
							npcf.SetCurrentAction(NULL, 0, npcf.damageID);
							npcf.frame = 0;
						}		
					}
					if (npcg.state != DIE && !npcg.isFriend)
					{
						if(isHit(pos, npcgpos, fDir, 180.0f, 360.0f))
						{
							npcg.blood -= 20;
							npcg.state = DAMAGE;
							npcg.SetCurrentAction(NULL, 0, npcg.damageID);
							npcg.frame = 0;
						}		
					}
				}
				if (actor.frame == 32)
				{
					attackKeyLocked = false;
				}
				if (actor.frame == 47)
				{
					if(normalCombo)
					{
						actor.state = ATTACK3;
						actor.SetCurrentAction(NULL, 0, normalAttack3ID);
						actor.frame = 0;
						normalCombo = false;
					}
					else
					{
						actor.state = COMBATIDLE;
						actor.combatWait = 150;
						actor.SetCurrentAction(NULL, 0, combatIdleID);
						actor.frame = 0;
						movementKeyLocked = false;
					}	
				}
				break;
			case ATTACK3:
			// normal attack 3 46 frames
				actor.Play(ONCE, (float) skip, FALSE, TRUE);
				actor.frame++;

				if (actor.frame == 15){
					FnScene scene(sID);
						gFXID = scene.CreateGameFXSystem();
						OBJECTid baseID = actor.GetBaseObject();
						FnGameFXSystem gxS(gFXID);
						BOOL beOK = gxS.Load("Lyuatt2", TRUE);
						gxS.SetParentObjectForAll(baseID);
					aP.Load("att4.wav");
					aP.Play(ONCE);
				}
				if (actor.frame == 25)
				{
					if (npca.state != DIE && !npca.isFriend)
					{
						if(isHit(pos, npcapos, fDir, 180.0f, 180.0f))
						{
							npca.blood -= 30;
							npca.state = DAMAGE;
							npca.SetCurrentAction(NULL, 0, npca.damageID);
							npca.frame = 0;
						}		
					}
					if (npcb.state != DIE && !npcb.isFriend)
					{
						if(isHit(pos, npcbpos, fDir, 180.0f, 180.0f))
						{
							npcb.blood -= 30;
							npcb.state = DAMAGE;
							npcb.SetCurrentAction(NULL, 0, npcb.damageID);
							npcb.frame = 0;
						}		
					}
					if (npcc.state != DIE && !npcc.isFriend)
					{
						if(isHit(pos, npccpos, fDir, 180.0f, 180.0f))
						{
							npcc.blood -= 30;
							npcc.state = DAMAGE;
							npcc.SetCurrentAction(NULL, 0, npcc.damageID);
							npcc.frame = 0;
						}		
					}
					if (npcd.state != DIE && !npcd.isFriend)
					{
						if(isHit(pos, npcdpos, fDir, 180.0f, 180.0f))
						{
							npcd.blood -= 30;
							npcd.state = DAMAGE;
							npcd.SetCurrentAction(NULL, 0, npcd.damageID);
							npcd.frame = 0;
						}		
					}
					if (npce.state != DIE && !npce.isFriend)
					{
						if(isHit(pos, npcepos, fDir, 180.0f, 180.0f))
						{
							npce.blood -= 30;
							npce.state = DAMAGE;
							npce.SetCurrentAction(NULL, 0, npce.damageID);
							npce.frame = 0;
						}		
					}
					if (npcf.state != DIE && !npcf.isFriend)
					{
						if(isHit(pos, npcfpos, fDir, 180.0f, 180.0f))
						{
							npcf.blood -= 30;
							npcf.state = DAMAGE;
							npcf.SetCurrentAction(NULL, 0, npcf.damageID);
							npcf.frame = 0;
						}		
					}
					if (npcg.state != DIE && !npcg.isFriend)
					{
						if(isHit(pos, npcgpos, fDir, 180.0f, 180.0f))
						{
							npcg.blood -= 30;
							npcg.state = DAMAGE;
							npcg.SetCurrentAction(NULL, 0, npcg.damageID);
							npcg.frame = 0;
						}		
					}
				}
				if (actor.frame == 30)
				{
					attackKeyLocked = false;
				}
				if (actor.frame == 45)
				{
					if(normalCombo)
					{
						actor.state = ATTACK4;
						actor.SetCurrentAction(NULL, 0, normalAttack4ID);
						actor.frame = 0;
						normalCombo = false;
					}
					else
					{
						actor.state = COMBATIDLE;
						actor.combatWait = 150;
						actor.SetCurrentAction(NULL, 0, combatIdleID);
						actor.frame = 0;
						movementKeyLocked = false;
					}	
				}
				break;
			case ATTACK4:
			// normal attack 4 49 frames
				actor.Play(ONCE, (float) skip, FALSE, TRUE);
				actor.frame++;

				if (actor.frame == 2){
					FnScene scene(sID);
					gFXID = scene.CreateGameFXSystem();
					OBJECTid baseID = actor.GetBaseObject();
					FnGameFXSystem gxS(gFXID);
					BOOL4 beOK = gxS.Load("Lyuatt3", TRUE);
					gxS.SetParentObjectForAll(baseID);
					aP.Load("att2.wav");
					aP.Play(ONCE);
				}
				if (actor.frame == 30)
				{
					if (npca.state != DIE && !npca.isFriend)
					{
						if(isHit(pos, npcapos, fDir, 180.0f, 40.0f))
						{
							npca.blood -= 40;
							npca.state = DAMAGE;
							npca.SetCurrentAction(NULL, 0, npca.damageID);
							npca.frame = 0;
						}		
					}
					if (npcb.state != DIE && !npcb.isFriend)
					{
						if(isHit(pos, npcbpos, fDir, 180.0f, 40.0f))
						{
							npcb.blood -= 40;
							npcb.state = DAMAGE;
							npcb.SetCurrentAction(NULL, 0, npcb.damageID);
							npcb.frame = 0;
						}		
					}
					if (npcc.state != DIE && !npcc.isFriend)
					{
						if(isHit(pos, npccpos, fDir, 180.0f, 40.0f))
						{
							npcc.blood -= 40;
							npcc.state = DAMAGE;
							npcc.SetCurrentAction(NULL, 0, npcc.damageID);
							npcc.frame = 0;
						}		
					}
					if (npcd.state != DIE && !npcd.isFriend)
					{
						if(isHit(pos, npcdpos, fDir, 180.0f, 40.0f))
						{
							npcd.blood -= 40;
							npcd.state = DAMAGE;
							npcd.SetCurrentAction(NULL, 0, npcd.damageID);
							npcd.frame = 0;
						}		
					}
					if (npce.state != DIE && !npce.isFriend)
					{
						if(isHit(pos, npcepos, fDir, 180.0f, 40.0f))
						{
							npce.blood -= 40;
							npce.state = DAMAGE;
							npce.SetCurrentAction(NULL, 0, npce.damageID);
							npce.frame = 0;
						}		
					}
					if (npcf.state != DIE && !npcf.isFriend)
					{
						if(isHit(pos, npcfpos, fDir, 180.0f, 40.0f))
						{
							npcf.blood -= 40;
							npcf.state = DAMAGE;
							npcf.SetCurrentAction(NULL, 0, npcf.damageID);
							npcf.frame = 0;
						}		
					}
					if (npcg.state != DIE && !npcg.isFriend)
					{
						if(isHit(pos, npcgpos, fDir, 180.0f, 40.0f))
						{
							npcg.blood -= 40;
							npcg.state = DAMAGE;
							npcg.SetCurrentAction(NULL, 0, npcg.damageID);
							npcg.frame = 0;
						}		
					}
				}
				if (actor.frame == 38)
				{
					attackKeyLocked = false;
				}
				if (actor.frame == 48)
				{
					actor.state = COMBATIDLE;
					actor.combatWait = 150;
					actor.SetCurrentAction(NULL, 0, combatIdleID);
					actor.frame = 0;
					movementKeyLocked = false;		
				}
				break;
			case ULT:
			// ultimate attack 121 frames
				actor.Play(ONCE, (float) skip, FALSE, TRUE);
				actor.frame++;
				if (actor.frame == 5)
				{	aP.Load("att5.wav");
					aP.Play(ONCE);
				}
				if (actor.frame == 120)
				{
					actor.state = COMBATIDLE;
					actor.combatWait = 150;
					actor.SetCurrentAction(NULL, 0, combatIdleID);
					actor.frame = 0;
					movementKeyLocked = false;
					attackKeyLocked = false;
				}
				break;
			case DAMAGE:
			// heavy damage 24 frames
				actor.Play(ONCE, (float) skip, FALSE, TRUE);
				actor.frame++;
				if (actor.frame == 2)
				{
					aP.Load("hurt2.wav");
					aP.Play(ONCE);
				}
				if (actor.frame == 20)
				{
					attackKeyLocked = false;
					movementKeyLocked = false;
				}
				if (actor.frame == 23)
				{
					actor.state = COMBATIDLE;
					actor.SetCurrentAction(NULL, 0, combatIdleID);
					actor.frame = 0;
				}
				break;
			case DIE:
			// die
				actor.Play(ONCE, (float) skip, FALSE, TRUE);
				actor.frame++;
				if (actor.frame == 20)
				{
					aP.Load("adie.wav");
					aP.Play(ONCE);
				}
				break;
			default:
				break;
		}

		actor.BB();


		// play game FX
			   if (gFXID != FAILED_ID) {
				  FnGameFXSystem gxS(gFXID);
				  BOOL4 beOK = gxS.Play((float) skip, ONCE);
				  if (!beOK) {
					 FnScene scene(sID);
					 scene.DeleteGameFXSystem(gFXID);
					 gFXID = FAILED_ID;
				  }
			   }

						// play game FX
			   if (dFXID != FAILED_ID) {
				  FnGameFXSystem dxS(dFXID);
				  BOOL4 beOK = dxS.Play((float) skip, ONCE);
				  if (!beOK) {
					 FnScene scene(sID);
					 scene.DeleteGameFXSystem(dFXID);
					 dFXID = FAILED_ID;
				  }
			   }

		actor.GetPosition(pos);
		npca.GetPosition(npcapos);
		npcb.GetPosition(npcbpos);
		npcc.GetPosition(npccpos);
		npcd.GetPosition(npcdpos);
		npce.GetPosition(npcepos);
		npcf.GetPosition(npcfpos);
		npcg.GetPosition(npcgpos);

		actor.GetDirection(fDir, uDir);
		npca.GetDirection(npcafDir, npcauDir);
		npcb.GetDirection(npcbfDir, npcbuDir);
		npcc.GetDirection(npccfDir, npccuDir);
		npcd.GetDirection(npcdfDir, npcduDir);
		npce.GetDirection(npcefDir, npceuDir);
		npcf.GetDirection(npcffDir, npcfuDir);
		npcg.GetDirection(npcgfDir, npcguDir);

		/*------------------------------------------
		 ||||    || |||||||||   ||||||||     ||||
		 || ||   || ||      || ||      ||   ||  ||
		 ||  ||  || |||||||||  ||          ||    ||
		 ||   || || ||         ||      || ||||||||||
		 ||    |||| ||          ||||||||  ||      ||
		 ------------------------------------------*/

		if (npca.blood <= 0 && npca.state != DIE)
		{
			npca.state = DIE;
			npca.SetCurrentAction(NULL, 0, npca.dieID);
			npca.frame = 0;
		}

	   if (npca.state != DIE)
	   {
	      if (npca.isFriend)
	      {
	         if ((teammateID[0] == NPCA && isFollow[0]) || (teammateID[1] == NPCA && isFollow[1]))
	         {
	            npca.target = ACTOR;
	         }
	         else
	         {
	            switch(npca.target)
	            {
	               case NONE:
	                  npca.target = nearestEnemy(NPCA, npcapos);
	                  if (npca.target == NONE)
	                  {
	                     npca.target = ACTOR;
	                  }
	                  break;
	               case ACTOR:
	                  if (actor.state == DIE)
	                  {
	                     npca.target = NONE;
	                  }
	                  if ((teammateID[0] == NPCA && !isFollow[0]) || (teammateID[1] == NPCA && !isFollow[1]))
	                  {
	                     npca.target = nearestEnemy(NPCA, npcapos);
	                     if (npca.target == NONE)
	                     {
	                        npca.target = ACTOR;
	                     }
	                  }
	                  break;
	               case NPCA:
	                  if (npca.state == DIE)
	                  {
	                     npca.target = NONE;
	                  }
	                  break;
	               case NPCB:
	                  if (npcb.state == DIE)
	                  {
	                     npca.target = NONE;
	                  }
	                  break;
	               case NPCC:
	                  if (npcc.state == DIE)
	                  {
	                     npca.target = NONE;
	                  }
	                  break;
	               case NPCD:
	                  if (npcd.state == DIE)
	                  {
	                     npca.target = NONE;
	                  }
	                  break;
	               case NPCE:
	                  if (npce.state == DIE)
	                  {
	                     npca.target = NONE;
	                  }
	                  break;
	               case NPCF:
	                  if (npcf.state == DIE)
	                  {
	                     npca.target = NONE;
	                  }
	                  break;
	               case NPCG:
	                  if (npcg.state == DIE)
	                  {
	                     npca.target = NONE;
	                  }
	                  break;
	               default:
	                  break;
	            }
	         }
	      }
	      else
	      {
	         switch(npca.target)
	         {
	            case NONE:
	               npca.target = nearestFriend(NPCA, npcapos);
	               break;
	            case ACTOR:
	               if (actor.state == DIE)
	               {
	                  npca.target = NONE;
	               }
	               break;
	            case NPCA:
	               if (npca.state == DIE)
	               {
	                  npca.target = NONE;
	               }
	               break;
	            case NPCB:
	               if (npcb.state == DIE)
	               {
	                  npca.target = NONE;
	               }
	               break;
	            case NPCC:
	               if (npcc.state == DIE)
	               {
	                  npca.target = NONE;
	               }
	               break;
	            case NPCD:
	               if (npcd.state == DIE)
	               {
	                  npca.target = NONE;
	               }
	               break;
	            case NPCE:
	               if (npce.state == DIE)
	               {
	                  npca.target = NONE;
	               }
	               break;
	            case NPCF:
	               if (npcf.state == DIE)
	               {
	                  npca.target = NONE;
	               }
	               break;
	            case NPCG:
	               if (npcg.state == DIE)
	               {
	                  npca.target = NONE;
	               }
	               break;
	            default:
	               break;
	         }
	      }
	   }

		/*
			npca.state:
				0 idle
				1 run
				2 attack1
				3 attack2
				4 damage
				5 die
		*/
		switch(npca.state)
		{
			case IDLE:
			// idle
				npca.Play(LOOP, (float) skip, FALSE, TRUE);
				switch (npca.target)
	         {
	            case NONE:
	               break;
	            case ACTOR:
	               if (npca.isFriend)
	               {
	                  if (myDist(pos, npcapos) >= 100.0f)
	                  {
	                     npca.state = RUN;
	                     npca.SetCurrentAction(NULL, 0, npca.runID);
	                     npca.frame = 0;
	                  }
	               }
	               else
	               {
	                  if (myDist(pos, npcapos) <= 500.0f && myDist(pos, npcapos) >= 100.0f)
	                  {  
	                     npca.state = RUN;
	                     npca.SetCurrentAction(NULL, 0, npca.runID);
	                     npca.frame = 0;
	                  }
	                  else if (myDist(pos, npcapos) < 100.0f  && actor.state != DIE)
	                  {
	                     npca.wait1++;
	                     npcafDir[0] = pos[0] - npcapos[0];
	                     npcafDir[1] = pos[1] - npcapos[1];
	                     npca.SetDirection(npcafDir, npcauDir);
	                     if (npca.wait1 % 50 == 0)
	                     {
	                        npca.wait2++;
	                        if (npca.wait2 % 2 == 0)
	                        {
	                           npca.state = ATTACK2;
	                           npca.SetCurrentAction(NULL, 0, npca.attack2ID);
	                           npca.frame = 0;
	                        }
	                        else
	                        {
	                           npca.state = ATTACK1;
	                           npca.SetCurrentAction(NULL, 0, npca.attack1ID);
	                           npca.frame = 0;
	                        }
	                     }
	                  }
	               }
	               break;
	            case NPCA:
	               if (myDist(npcapos, npcapos) <= 600.0f && myDist(npcapos, npcapos) >= 100.0f)
	               {
	                  npca.state = RUN;
	                  npca.SetCurrentAction(NULL, 0, npca.runID);
	                  npca.frame = 0;
	               }
	               else if (myDist(npcapos, npcapos) < 100.0f)
	               {
	                  npca.wait1++;
	                  npcafDir[0] = npcapos[0] - npcapos[0];
	                  npcafDir[1] = npcapos[1] - npcapos[1];
	                  npca.SetDirection(npcafDir, npcauDir);
	                  if (npca.wait1 % 50 == 0)
	                  {
	                     npca.wait1++;
	                     if (npca.wait2 % 2 == 0)
	                     {
	                        npca.state = ATTACK2;
	                        npca.SetCurrentAction(NULL, 0, npca.attack2ID);
	                        npca.frame = 0;
	                     }
	                     else
	                     {
	                        npca.state = ATTACK1;
	                        npca.SetCurrentAction(NULL, 0, npca.attack1ID);
	                        npca.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCB:
	               if (myDist(npcapos, npcbpos) <= 600.0f && myDist(npcapos, npcbpos) >= 100.0f)
	               {
	                  npca.state = RUN;
	                  npca.SetCurrentAction(NULL, 0, npca.runID);
	                  npca.frame = 0;
	               }
	               else if (myDist(npcapos, npcbpos) < 100.0f)
	               {
	                  npca.wait1++;
	                  npcafDir[0] = npcbpos[0] - npcapos[0];
	                  npcafDir[1] = npcbpos[1] - npcapos[1];
	                  npca.SetDirection(npcafDir, npcauDir);
	                  if (npca.wait1 % 50 == 0)
	                  {
	                     npca.wait1++;
	                     if (npca.wait2 % 2 == 0)
	                     {
	                        npca.state = ATTACK2;
	                        npca.SetCurrentAction(NULL, 0, npca.attack2ID);
	                        npca.frame = 0;
	                     }
	                     else
	                     {
	                        npca.state = ATTACK1;
	                        npca.SetCurrentAction(NULL, 0, npca.attack1ID);
	                        npca.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCC:
	               if (myDist(npcapos, npccpos) <= 600.0f && myDist(npcapos, npccpos) >= 100.0f)
	               {
	                  npca.state = RUN;
	                  npca.SetCurrentAction(NULL, 0, npca.runID);
	                  npca.frame = 0;
	               }
	               else if (myDist(npcapos, npccpos) < 100.0f)
	               {
	                  npca.wait1++;
	                  npcafDir[0] = npccpos[0] - npcapos[0];
	                  npcafDir[1] = npccpos[1] - npcapos[1];
	                  npca.SetDirection(npcafDir, npcauDir);
	                  if (npca.wait1 % 50 == 0)
	                  {
	                     npca.wait1++;
	                     if (npca.wait2 % 2 == 0)
	                     {
	                        npca.state = ATTACK2;
	                        npca.SetCurrentAction(NULL, 0, npca.attack2ID);
	                        npca.frame = 0;
	                     }
	                     else
	                     {
	                        npca.state = ATTACK1;
	                        npca.SetCurrentAction(NULL, 0, npca.attack1ID);
	                        npca.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCD:
	               if (myDist(npcapos, npcdpos) <= 600.0f && myDist(npcapos, npcdpos) >= 100.0f)
	               {
	                  npca.state = RUN;
	                  npca.SetCurrentAction(NULL, 0, npca.runID);
	                  npca.frame = 0;
	               }
	               else if (myDist(npcapos, npcdpos) < 100.0f)
	               {
	                  npca.wait1++;
	                  npcafDir[0] = npcdpos[0] - npcapos[0];
	                  npcafDir[1] = npcdpos[1] - npcapos[1];
	                  npca.SetDirection(npcafDir, npcauDir);
	                  if (npca.wait1 % 50 == 0)
	                  {
	                     npca.wait1++;
	                     if (npca.wait2 % 2 == 0)
	                     {
	                        npca.state = ATTACK2;
	                        npca.SetCurrentAction(NULL, 0, npca.attack2ID);
	                        npca.frame = 0;
	                     }
	                     else
	                     {
	                        npca.state = ATTACK1;
	                        npca.SetCurrentAction(NULL, 0, npca.attack1ID);
	                        npca.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCE:
	               if (myDist(npcapos, npcepos) <= 600.0f && myDist(npcapos, npcepos) >= 100.0f)
	               {
	                  npca.state = RUN;
	                  npca.SetCurrentAction(NULL, 0, npca.runID);
	                  npca.frame = 0;
	               }
	               else if (myDist(npcapos, npcepos) < 100.0f)
	               {
	                  npca.wait1++;
	                  npcafDir[0] = npcepos[0] - npcapos[0];
	                  npcafDir[1] = npcepos[1] - npcapos[1];
	                  npca.SetDirection(npcafDir, npcauDir);
	                  if (npca.wait1 % 50 == 0)
	                  {
	                     npca.wait1++;
	                     if (npca.wait2 % 2 == 0)
	                     {
	                        npca.state = ATTACK2;
	                        npca.SetCurrentAction(NULL, 0, npca.attack2ID);
	                        npca.frame = 0;
	                     }
	                     else
	                     {
	                        npca.state = ATTACK1;
	                        npca.SetCurrentAction(NULL, 0, npca.attack1ID);
	                        npca.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCF:
	               if (myDist(npcapos, npcfpos) <= 600.0f && myDist(npcapos, npcfpos) >= 100.0f)
	               {
	                  npca.state = RUN;
	                  npca.SetCurrentAction(NULL, 0, npca.runID);
	                  npca.frame = 0;
	               }
	               else if (myDist(npcapos, npcfpos) < 100.0f)
	               {
	                  npca.wait1++;
	                  npcafDir[0] = npcfpos[0] - npcapos[0];
	                  npcafDir[1] = npcfpos[1] - npcapos[1];
	                  npca.SetDirection(npcafDir, npcauDir);
	                  if (npca.wait1 % 50 == 0)
	                  {
	                     npca.wait1++;
	                     if (npca.wait2 % 2 == 0)
	                     {
	                        npca.state = ATTACK2;
	                        npca.SetCurrentAction(NULL, 0, npca.attack2ID);
	                        npca.frame = 0;
	                     }
	                     else
	                     {
	                        npca.state = ATTACK1;
	                        npca.SetCurrentAction(NULL, 0, npca.attack1ID);
	                        npca.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCG:
	               if (myDist(npcapos, npcgpos) <= 600.0f && myDist(npcapos, npcgpos) >= 100.0f)
	               {
	                  npca.state = RUN;
	                  npca.SetCurrentAction(NULL, 0, npca.runID);
	                  npca.frame = 0;
	               }
	               else if (myDist(npcapos, npcgpos) < 100.0f)
	               {
	                  npca.wait1++;
	                  npcafDir[0] = npcgpos[0] - npcapos[0];
	                  npcafDir[1] = npcgpos[1] - npcapos[1];
	                  npca.SetDirection(npcafDir, npcauDir);
	                  if (npca.wait1 % 50 == 0)
	                  {
	                     npca.wait1++;
	                     if (npca.wait2 % 2 == 0)
	                     {
	                        npca.state = ATTACK2;
	                        npca.SetCurrentAction(NULL, 0, npca.attack2ID);
	                        npca.frame = 0;
	                     }
	                     else
	                     {
	                        npca.state = ATTACK1;
	                        npca.SetCurrentAction(NULL, 0, npca.attack1ID);
	                        npca.frame = 0;
	                     }
	                  }
	               }
	               break;
	            default:
	               break;
	         }		
				break;
			case RUN:
			// run
				npca.Play(LOOP, (float) skip, FALSE, TRUE);
				
				switch(npca.target)
	         {
	            case NONE:
	               npca.state = IDLE;
	               npca.SetCurrentAction(NULL, 0, npca.idleID);
	               npca.frame = 0;
	               break;
	            case ACTOR:
	               npcafDir[0] = pos[0] - npcapos[0];
	               npcafDir[1] = pos[1] - npcapos[1];
	               npca.SetDirection(npcafDir, npcauDir);
	               npca.MoveForward(5.0f);
	               if (myDist(pos, npcapos) < 90.0f)
	               {
	                  npca.state = IDLE;
	                  npca.SetCurrentAction(NULL, 0, npca.idleID);
	                  npca.frame = 0;
	               }
	               break;
	            case NPCA:
	               npcafDir[0] = npcapos[0] - npcapos[0];
	               npcafDir[1] = npcapos[1] - npcapos[1];
	               npca.SetDirection(npcafDir, npcauDir);
	               npca.MoveForward(5.0f);
	               if (myDist(npcapos, npcapos) < 90.0f)
	               {
	                  npca.state = IDLE;
	                  npca.SetCurrentAction(NULL, 0, npca.idleID);
	                  npca.frame = 0;
	               }
	               break;
	            case NPCB:
	               npcafDir[0] = npcbpos[0] - npcapos[0];
	               npcafDir[1] = npcbpos[1] - npcapos[1];
	               npca.SetDirection(npcafDir, npcauDir);
	               npca.MoveForward(5.0f);
	               if (myDist(npcbpos, npcapos) < 90.0f)
	               {
	                  npca.state = IDLE;
	                  npca.SetCurrentAction(NULL, 0, npca.idleID);
	                  npca.frame = 0;
	               }
	               break;
	            case NPCC:
	               npcafDir[0] = npccpos[0] - npcapos[0];
	               npcafDir[1] = npccpos[1] - npcapos[1];
	               npca.SetDirection(npcafDir, npcauDir);
	               npca.MoveForward(5.0f);
	               if (myDist(npccpos, npcapos) < 90.0f)
	               {
	                  npca.state = IDLE;
	                  npca.SetCurrentAction(NULL, 0, npca.idleID);
	                  npca.frame = 0;
	               }
	               break;
	            case NPCD:
	               npcafDir[0] = npcdpos[0] - npcapos[0];
	               npcafDir[1] = npcdpos[1] - npcapos[1];
	               npca.SetDirection(npcafDir, npcauDir);
	               npca.MoveForward(5.0f);
	               if (myDist(npcdpos, npcapos) < 90.0f)
	               {
	                  npca.state = IDLE;
	                  npca.SetCurrentAction(NULL, 0, npca.idleID);
	                  npca.frame = 0;
	               }
	               break;
	            case NPCE:
	               npcafDir[0] = npcepos[0] - npcapos[0];
	               npcafDir[1] = npcepos[1] - npcapos[1];
	               npca.SetDirection(npcafDir, npcauDir);
	               npca.MoveForward(5.0f);
	               if (myDist(npcepos, npcapos) < 90.0f)
	               {
	                  npca.state = IDLE;
	                  npca.SetCurrentAction(NULL, 0, npca.idleID);
	                  npca.frame = 0;
	               }
	               break;
	            case NPCF:
	               npcafDir[0] = npcfpos[0] - npcapos[0];
	               npcafDir[1] = npcfpos[1] - npcapos[1];
	               npca.SetDirection(npcafDir, npcauDir);
	               npca.MoveForward(5.0f);
	               if (myDist(npcfpos, npcapos) < 90.0f)
	               {
	                  npca.state = IDLE;
	                  npca.SetCurrentAction(NULL, 0, npca.idleID);
	                  npca.frame = 0;
	               }
	               break;
	            case NPCG:
	               npcafDir[0] = npcgpos[0] - npcapos[0];
	               npcafDir[1] = npcgpos[1] - npcapos[1];
	               npca.SetDirection(npcafDir, npcauDir);
	               npca.MoveForward(5.0f);
	               if (myDist(npcgpos, npcapos) < 90.0f)
	               {
	                  npca.state = IDLE;
	                  npca.SetCurrentAction(NULL, 0, npca.idleID);
	                  npca.frame = 0;
	               }
	               break;
	            default:
	               break;
	         }
				break;
			case ATTACK1:
			// attackL 1 36 frames
				npca.Play(ONCE, (float) skip, FALSE, TRUE);
				npca.frame++;

				if (npca.frame == 12)
				{
					hP.Load("att3.wav");
					hP.Play(ONCE);

					if (npca.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npcapos, npcapos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npcapos, npcbpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npcapos, npccpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npcapos, npcdpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npcapos, npcepos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npcapos, npcfpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npcapos, npcgpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npcapos, pos, npcafDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npcapos, npcapos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npcapos, npcbpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npcapos, npccpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npcapos, npcdpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npcapos, npcepos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npcapos, npcfpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npcapos, npcgpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }	
				}
				if (npca.frame == 35)
				{
					npca.state = IDLE;
					npca.SetCurrentAction(NULL, 0, npca.idleID);	
					npca.frame = 0;
				}
				break;
			case ATTACK2:
			// attackL 2 41 frames
	         npca.Play(ONCE, (float) skip, FALSE, TRUE);
	         npca.frame++;

	         if (npca.frame == 12)
	         {
	            hP.Load("att3.wav");
	            hP.Play(ONCE);

	            if (npca.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npcapos, npcapos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npcapos, npcbpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npcapos, npccpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npcapos, npcdpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npcapos, npcepos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npcapos, npcfpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npcapos, npcgpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npcapos, pos, npcafDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npcapos, npcapos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npcapos, npcbpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npcapos, npccpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npcapos, npcdpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npcapos, npcepos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npcapos, npcfpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npcapos, npcgpos, npcafDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }  
	         }
	         if (npca.frame == 40)
	         {
	            npca.state = IDLE;
	            npca.SetCurrentAction(NULL, 0, npca.idleID); 
	            npca.frame = 0;
	         }
				break;
			case DAMAGE:
			// damage L 26
				npca.Play(ONCE, (float) skip, FALSE, TRUE);
				npca.frame++;

				if(npca.frame == 1){
					hP.Load("Dhurt1.wav");
					hP.Play(ONCE);
					FnScene scene(sID);
					dFXID = scene.CreateGameFXSystem();
					OBJECTid baseID = npca.GetBaseObject();
					FnGameFXSystem dxS(dFXID);
					BOOL4 beOK = dxS.Load("DonzuHurt", TRUE);
					dxS.SetParentObjectForAll(baseID);
					
				}
				if (npca.frame == 25)
				{
					npca.state = IDLE;
					npca.SetCurrentAction(NULL, 0, npca.idleID);
					npca.frame = 0;
				}
				break;
			case DIE:
			// die
				npca.Play(ONCE, (float) skip, FALSE, TRUE);
				npca.frame++;

				if (npca.frame == 95)
				{	
					hP.Load("Ddie.wav");
					hP.Play(ONCE);
				}	
				break;
			default:
				break;
		}

		npca.BB();

		/*------------------------------------------
		 ||||    || |||||||||   ||||||||  |||||||||
		 || ||   || ||      || ||      || ||      ||
		 ||  ||  || |||||||||  ||         |||||||||
		 ||   || || ||         ||      || ||      ||
		 ||    |||| ||          ||||||||  |||||||||
		 ------------------------------------------*/

		if (npcb.blood <= 0 && npcb.state != DIE)
		{
			npcb.state = DIE;
			npcb.SetCurrentAction(NULL, 0, npcb.dieID);
		}

	   if (npcb.state != DIE)
	   {
	      if (npcb.isFriend)
	      {
	         if ((teammateID[0] == NPCB && isFollow[0]) || (teammateID[1] == NPCB && isFollow[1]))
	         {
	            npcb.target = ACTOR;
	         }
	         else
	         {
	            switch(npcb.target)
	            {
	               case NONE:
	                  npcb.target = nearestEnemy(NPCB, npcbpos);
	                  if (npcb.target == NONE)
	                  {
	                     npcb.target = ACTOR;
	                  }
	                  break;
	               case ACTOR:
	                  if (actor.state == DIE)
	                  {
	                     npcb.target = NONE;
	                  }
	                  if ((teammateID[0] == NPCB && !isFollow[0]) || (teammateID[1] == NPCB && !isFollow[1]))
	                  {
	                     npcb.target = nearestEnemy(NPCB, npcbpos);
	                     if (npcb.target == NONE)
	                     {
	                        npcb.target = ACTOR;
	                     }
	                  }
	                  break;
	               case NPCA:
	                  if (npca.state == DIE)
	                  {
	                     npcb.target = NONE;
	                  }
	                  break;
	               case NPCB:
	                  if (npcb.state == DIE)
	                  {
	                     npcb.target = NONE;
	                  }
	                  break;
	               case NPCC:
	                  if (npcc.state == DIE)
	                  {
	                     npcb.target = NONE;
	                  }
	                  break;
	               case NPCD:
	                  if (npcd.state == DIE)
	                  {
	                     npcb.target = NONE;
	                  }
	                  break;
	               case NPCE:
	                  if (npce.state == DIE)
	                  {
	                     npcb.target = NONE;
	                  }
	                  break;
	               case NPCF:
	                  if (npcf.state == DIE)
	                  {
	                     npcb.target = NONE;
	                  }
	                  break;
	               case NPCG:
	                  if (npcg.state == DIE)
	                  {
	                     npcb.target = NONE;
	                  }
	                  break;
	               default:
	                  break;
	            }
	         }
	      }
	      else
	      {
	         switch(npcb.target)
	         {
	            case NONE:
	               npcb.target = nearestFriend(NPCB, npcbpos);
	               break;
	            case ACTOR:
	               if (actor.state == DIE)
	               {
	                  npcb.target = NONE;
	               }
	               break;
	            case NPCA:
	               if (npca.state == DIE)
	               {
	                  npcb.target = NONE;
	               }
	               break;
	            case NPCB:
	               if (npcb.state == DIE)
	               {
	                  npcb.target = NONE;
	               }
	               break;
	            case NPCC:
	               if (npcc.state == DIE)
	               {
	                  npcb.target = NONE;
	               }
	               break;
	            case NPCD:
	               if (npcd.state == DIE)
	               {
	                  npcb.target = NONE;
	               }
	               break;
	            case NPCE:
	               if (npce.state == DIE)
	               {
	                  npcb.target = NONE;
	               }
	               break;
	            case NPCF:
	               if (npcf.state == DIE)
	               {
	                  npcb.target = NONE;
	               }
	               break;
	            case NPCG:
	               if (npcg.state == DIE)
	               {
	                  npcb.target = NONE;
	               }
	               break;
	            default:
	               break;
	         }
	      }
	   }

		/*
			npcb.state:
				0 idle
				1 run
				2 attack1
				3 attack2
				4 damage
				5 die
		*/
		switch(npcb.state)
		{
			case IDLE:
			// combat idle
				npcb.Play(LOOP, (float) skip, FALSE, TRUE);
				switch (npcb.target)
	         {
	            case NONE:
	               break;
	            case ACTOR:
	               if (npcb.isFriend)
	               {
	                  if (myDist(pos, npcbpos) >= 100.0f)
	                  {
	                     npcb.state = RUN;
	                     npcb.SetCurrentAction(NULL, 0, npcb.runID);
	                     npcb.frame = 0;
	                  }
	               }
	               else
	               {
	                  if (myDist(pos, npcbpos) <= 500.0f && myDist(pos, npcbpos) >= 100.0f)
	                  {  
	                     npcb.state = RUN;
	                     npcb.SetCurrentAction(NULL, 0, npcb.runID);
	                     npcb.frame = 0;
	                  }
	                  else if (myDist(pos, npcbpos) < 100.0f  && actor.state != DIE)
	                  {
	                     npcb.wait1++;
	                     npcbfDir[0] = pos[0] - npcbpos[0];
	                     npcbfDir[1] = pos[1] - npcbpos[1];
	                     npcb.SetDirection(npcbfDir, npcbuDir);
	                     if (npcb.wait1 % 50 == 0)
	                     {
	                        npcb.wait2++;
	                        if (npcb.wait2 % 2 == 0)
	                        {
	                           npcb.state = ATTACK2;
	                           npcb.SetCurrentAction(NULL, 0, npcb.attack2ID);
	                           npcb.frame = 0;
	                        }
	                        else
	                        {
	                           npcb.state = ATTACK1;
	                           npcb.SetCurrentAction(NULL, 0, npcb.attack1ID);
	                           npcb.frame = 0;
	                        }
	                     }
	                  }
	               }
	               break;
	            case NPCA:
	               if (myDist(npcbpos, npcapos) <= 600.0f && myDist(npcbpos, npcapos) >= 100.0f)
	               {
	                  npcb.state = RUN;
	                  npcb.SetCurrentAction(NULL, 0, npcb.runID);
	                  npcb.frame = 0;
	               }
	               else if (myDist(npcbpos, npcapos) < 100.0f)
	               {
	                  npcb.wait1++;
	                  npcbfDir[0] = npcapos[0] - npcbpos[0];
	                  npcbfDir[1] = npcapos[1] - npcbpos[1];
	                  npcb.SetDirection(npcbfDir, npcbuDir);
	                  if (npcb.wait1 % 50 == 0)
	                  {
	                     npcb.wait1++;
	                     if (npcb.wait2 % 2 == 0)
	                     {
	                        npcb.state = ATTACK2;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack2ID);
	                        npcb.frame = 0;
	                     }
	                     else
	                     {
	                        npcb.state = ATTACK1;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack1ID);
	                        npcb.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCB:
	               if (myDist(npcbpos, npcbpos) <= 600.0f && myDist(npcbpos, npcbpos) >= 100.0f)
	               {
	                  npcb.state = RUN;
	                  npcb.SetCurrentAction(NULL, 0, npcb.runID);
	                  npcb.frame = 0;
	               }
	               else if (myDist(npcbpos, npcbpos) < 100.0f)
	               {
	                  npcb.wait1++;
	                  npcbfDir[0] = npcbpos[0] - npcbpos[0];
	                  npcbfDir[1] = npcbpos[1] - npcbpos[1];
	                  npcb.SetDirection(npcbfDir, npcbuDir);
	                  if (npcb.wait1 % 50 == 0)
	                  {
	                     npcb.wait1++;
	                     if (npcb.wait2 % 2 == 0)
	                     {
	                        npcb.state = ATTACK2;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack2ID);
	                        npcb.frame = 0;
	                     }
	                     else
	                     {
	                        npcb.state = ATTACK1;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack1ID);
	                        npcb.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCC:
	               if (myDist(npcbpos, npccpos) <= 600.0f && myDist(npcbpos, npccpos) >= 100.0f)
	               {
	                  npcb.state = RUN;
	                  npcb.SetCurrentAction(NULL, 0, npcb.runID);
	                  npcb.frame = 0;
	               }
	               else if (myDist(npcbpos, npccpos) < 100.0f)
	               {
	                  npcb.wait1++;
	                  npcbfDir[0] = npccpos[0] - npcbpos[0];
	                  npcbfDir[1] = npccpos[1] - npcbpos[1];
	                  npcb.SetDirection(npcbfDir, npcbuDir);
	                  if (npcb.wait1 % 50 == 0)
	                  {
	                     npcb.wait1++;
	                     if (npcb.wait2 % 2 == 0)
	                     {
	                        npcb.state = ATTACK2;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack2ID);
	                        npcb.frame = 0;
	                     }
	                     else
	                     {
	                        npcb.state = ATTACK1;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack1ID);
	                        npcb.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCD:
	               if (myDist(npcbpos, npcdpos) <= 600.0f && myDist(npcbpos, npcdpos) >= 100.0f)
	               {
	                  npcb.state = RUN;
	                  npcb.SetCurrentAction(NULL, 0, npcb.runID);
	                  npcb.frame = 0;
	               }
	               else if (myDist(npcbpos, npcdpos) < 100.0f)
	               {
	                  npcb.wait1++;
	                  npcbfDir[0] = npcdpos[0] - npcbpos[0];
	                  npcbfDir[1] = npcdpos[1] - npcbpos[1];
	                  npcb.SetDirection(npcbfDir, npcbuDir);
	                  if (npcb.wait1 % 50 == 0)
	                  {
	                     npcb.wait1++;
	                     if (npcb.wait2 % 2 == 0)
	                     {
	                        npcb.state = ATTACK2;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack2ID);
	                        npcb.frame = 0;
	                     }
	                     else
	                     {
	                        npcb.state = ATTACK1;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack1ID);
	                        npcb.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCE:
	               if (myDist(npcbpos, npcepos) <= 600.0f && myDist(npcbpos, npcepos) >= 100.0f)
	               {
	                  npcb.state = RUN;
	                  npcb.SetCurrentAction(NULL, 0, npcb.runID);
	                  npcb.frame = 0;
	               }
	               else if (myDist(npcbpos, npcepos) < 100.0f)
	               {
	                  npcb.wait1++;
	                  npcbfDir[0] = npcepos[0] - npcbpos[0];
	                  npcbfDir[1] = npcepos[1] - npcbpos[1];
	                  npcb.SetDirection(npcbfDir, npcbuDir);
	                  if (npcb.wait1 % 50 == 0)
	                  {
	                     npcb.wait1++;
	                     if (npcb.wait2 % 2 == 0)
	                     {
	                        npcb.state = ATTACK2;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack2ID);
	                        npcb.frame = 0;
	                     }
	                     else
	                     {
	                        npcb.state = ATTACK1;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack1ID);
	                        npcb.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCF:
	               if (myDist(npcbpos, npcfpos) <= 600.0f && myDist(npcbpos, npcfpos) >= 100.0f)
	               {
	                  npcb.state = RUN;
	                  npcb.SetCurrentAction(NULL, 0, npcb.runID);
	                  npcb.frame = 0;
	               }
	               else if (myDist(npcbpos, npcfpos) < 100.0f)
	               {
	                  npcb.wait1++;
	                  npcbfDir[0] = npcfpos[0] - npcbpos[0];
	                  npcbfDir[1] = npcfpos[1] - npcbpos[1];
	                  npcb.SetDirection(npcbfDir, npcbuDir);
	                  if (npcb.wait1 % 50 == 0)
	                  {
	                     npcb.wait1++;
	                     if (npcb.wait2 % 2 == 0)
	                     {
	                        npcb.state = ATTACK2;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack2ID);
	                        npcb.frame = 0;
	                     }
	                     else
	                     {
	                        npcb.state = ATTACK1;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack1ID);
	                        npcb.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCG:
	               if (myDist(npcbpos, npcgpos) <= 600.0f && myDist(npcbpos, npcgpos) >= 100.0f)
	               {
	                  npcb.state = RUN;
	                  npcb.SetCurrentAction(NULL, 0, npcb.runID);
	                  npcb.frame = 0;
	               }
	               else if (myDist(npcbpos, npcgpos) < 100.0f)
	               {
	                  npcb.wait1++;
	                  npcbfDir[0] = npcgpos[0] - npcbpos[0];
	                  npcbfDir[1] = npcgpos[1] - npcbpos[1];
	                  npcb.SetDirection(npcbfDir, npcbuDir);
	                  if (npcb.wait1 % 50 == 0)
	                  {
	                     npcb.wait1++;
	                     if (npcb.wait2 % 2 == 0)
	                     {
	                        npcb.state = ATTACK2;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack2ID);
	                        npcb.frame = 0;
	                     }
	                     else
	                     {
	                        npcb.state = ATTACK1;
	                        npcb.SetCurrentAction(NULL, 0, npcb.attack1ID);
	                        npcb.frame = 0;
	                     }
	                  }
	               }
	               break;
	            default:
	               break;
	         }
				break;
			case RUN:
			// run
				npcb.Play(LOOP, (float) skip, FALSE, TRUE);

				switch(npcb.target)
	         {
	            case NONE:
	               npcb.state = IDLE;
	               npcb.SetCurrentAction(NULL, 0, npcb.idleID);
	               npcb.frame = 0;
	               break;
	            case ACTOR:
	               npcbfDir[0] = pos[0] - npcbpos[0];
	               npcbfDir[1] = pos[1] - npcbpos[1];
	               npcb.SetDirection(npcbfDir, npcbuDir);
	               npcb.MoveForward(5.0f);
	               if (myDist(pos, npcbpos) < 90.0f)
	               {
	                  npcb.state = IDLE;
	                  npcb.SetCurrentAction(NULL, 0, npcb.idleID);
	                  npcb.frame = 0;
	               }
	               break;
	            case NPCA:
	               npcbfDir[0] = npcapos[0] - npcbpos[0];
	               npcbfDir[1] = npcapos[1] - npcbpos[1];
	               npcb.SetDirection(npcbfDir, npcbuDir);
	               npcb.MoveForward(5.0f);
	               if (myDist(npcapos, npcbpos) < 90.0f)
	               {
	                  npcb.state = IDLE;
	                  npcb.SetCurrentAction(NULL, 0, npcb.idleID);
	                  npcb.frame = 0;
	               }
	               break;
	            case NPCB:
	               npcbfDir[0] = npcbpos[0] - npcbpos[0];
	               npcbfDir[1] = npcbpos[1] - npcbpos[1];
	               npcb.SetDirection(npcbfDir, npcbuDir);
	               npcb.MoveForward(5.0f);
	               if (myDist(npcbpos, npcbpos) < 90.0f)
	               {
	                  npcb.state = IDLE;
	                  npcb.SetCurrentAction(NULL, 0, npcb.idleID);
	                  npcb.frame = 0;
	               }
	               break;
	            case NPCC:
	               npcbfDir[0] = npccpos[0] - npcbpos[0];
	               npcbfDir[1] = npccpos[1] - npcbpos[1];
	               npcb.SetDirection(npcbfDir, npcbuDir);
	               npcb.MoveForward(5.0f);
	               if (myDist(npccpos, npcbpos) < 90.0f)
	               {
	                  npcb.state = IDLE;
	                  npcb.SetCurrentAction(NULL, 0, npcb.idleID);
	                  npcb.frame = 0;
	               }
	               break;
	            case NPCD:
	               npcbfDir[0] = npcdpos[0] - npcbpos[0];
	               npcbfDir[1] = npcdpos[1] - npcbpos[1];
	               npcb.SetDirection(npcbfDir, npcbuDir);
	               npcb.MoveForward(5.0f);
	               if (myDist(npcdpos, npcbpos) < 90.0f)
	               {
	                  npcb.state = IDLE;
	                  npcb.SetCurrentAction(NULL, 0, npcb.idleID);
	                  npcb.frame = 0;
	               }
	               break;
	            case NPCE:
	               npcbfDir[0] = npcepos[0] - npcbpos[0];
	               npcbfDir[1] = npcepos[1] - npcbpos[1];
	               npcb.SetDirection(npcbfDir, npcbuDir);
	               npcb.MoveForward(5.0f);
	               if (myDist(npcepos, npcbpos) < 90.0f)
	               {
	                  npcb.state = IDLE;
	                  npcb.SetCurrentAction(NULL, 0, npcb.idleID);
	                  npcb.frame = 0;
	               }
	               break;
	            case NPCF:
	               npcbfDir[0] = npcfpos[0] - npcbpos[0];
	               npcbfDir[1] = npcfpos[1] - npcbpos[1];
	               npcb.SetDirection(npcbfDir, npcbuDir);
	               npcb.MoveForward(5.0f);
	               if (myDist(npcfpos, npcbpos) < 90.0f)
	               {
	                  npcb.state = IDLE;
	                  npcb.SetCurrentAction(NULL, 0, npcb.idleID);
	                  npcb.frame = 0;
	               }
	               break;
	            case NPCG:
	               npcbfDir[0] = npcgpos[0] - npcbpos[0];
	               npcbfDir[1] = npcgpos[1] - npcbpos[1];
	               npcb.SetDirection(npcbfDir, npcbuDir);
	               npcb.MoveForward(5.0f);
	               if (myDist(npcgpos, npcbpos) < 90.0f)
	               {
	                  npcb.state = IDLE;
	                  npcb.SetCurrentAction(NULL, 0, npcb.idleID);
	                  npcb.frame = 0;
	               }
	               break;
	            default:
	               break;
	         }
				break;
			case ATTACK1:
			// normal attack 1 36 frames
				npcb.Play(ONCE, (float) skip, FALSE, TRUE);
				npcb.frame++;
				if (npcb.frame == 2)
				{
					hP2.Load("npcbAtt.wav");
					hP2.Play(ONCE);
				}
				if (npcb.frame == 17)
				{
					if (npcb.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npcbpos, npcapos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npcbpos, npcbpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npcbpos, npccpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npcbpos, npcdpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npcbpos, npcepos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npcbpos, npcfpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npcbpos, npcgpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npcbpos, pos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npcbpos, npcapos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npcbpos, npcbpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npcbpos, npccpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npcbpos, npcdpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npcbpos, npcepos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npcbpos, npcfpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npcbpos, npcgpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }	
				}
				if (npcb.frame == 35)
				{
					npcb.state = IDLE;
					npcb.SetCurrentAction(NULL, 0, npcb.idleID);	
					npcb.frame = 0;
				}
				break;
			case ATTACK2:
			// normal attack 2 26 frames
	         npcb.Play(ONCE, (float) skip, FALSE, TRUE);
	         npcb.frame++;
			if (npcb.frame == 2)
				{
					hP2.Load("npcbAtt.wav");
					hP2.Play(ONCE);
				}
	         if (npcb.frame == 17)
	         {
	            if (npcb.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npcbpos, npcapos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npcbpos, npcbpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npcbpos, npccpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npcbpos, npcdpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npcbpos, npcepos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npcbpos, npcfpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npcbpos, npcgpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npcbpos, pos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npcbpos, npcapos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npcbpos, npcbpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npcbpos, npccpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npcbpos, npcdpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npcbpos, npcepos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npcbpos, npcfpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npcbpos, npcgpos, npcbfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }  
	         }
	         if (npcb.frame == 25)
	         {
	            npcb.state = IDLE;
	            npcb.SetCurrentAction(NULL, 0, npcb.idleID); 
	            npcb.frame = 0;
	         }
				break;
			case DAMAGE:
			// damage1 16 frames
				npcb.Play(ONCE, (float) skip, FALSE, TRUE);
				npcb.frame++;
				if (npcb.frame == 2)
				{
					hP2.Load("npcbHurt.wav");
					hP2.Play(ONCE);
				}
				if (npcb.frame == 15)
				{
					npcb.state = IDLE;
					npcb.SetCurrentAction(NULL, 0, npcb.idleID);
					npcb.frame = 0;
				}
				break;
			case DIE:
			// die
				npcb.Play(ONCE, (float) skip, FALSE, TRUE);
				if (npcb.frame == 5)
				{
					hP2.Load("npcbDie.wav");
					hP2.Play(ONCE);
				}
				break;
			default:
				break;
		}

		npcb.BB();

		/*------------------------------------------
		 ||||    || |||||||||   ||||||||   ||||||||
		 || ||   || ||      || ||      || ||      ||
		 ||  ||  || |||||||||  ||         ||
		 ||   || || ||         ||      || ||      ||
		 ||    |||| ||          ||||||||   ||||||||
		 ------------------------------------------*/

		if (npcc.blood <= 0 && npcc.state != DIE)
		{
			npcc.state = DIE;
			npcc.SetCurrentAction(NULL, 0, npcc.dieID);
		}

	   if (npcc.state != DIE)
	   {
	      if (npcc.isFriend)
	      {
	         if ((teammateID[0] == NPCC && isFollow[0]) || (teammateID[1] == NPCC && isFollow[1]))
	         {
	            npcc.target = ACTOR;
	         }
	         else
	         {
	            switch(npcc.target)
	            {
	               case NONE:
	                  npcc.target = nearestEnemy(NPCC, npccpos);
	                  if (npcc.target == NONE)
	                  {
	                     npcc.target = ACTOR;
	                  }
	                  break;
	               case ACTOR:
	                  if (actor.state == DIE)
	                  {
	                     npcc.target = NONE;
	                  }
	                  if ((teammateID[0] == NPCC && !isFollow[0]) || (teammateID[1] == NPCC && !isFollow[1]))
	                  {
	                     npcc.target = nearestEnemy(NPCC, npccpos);
	                     if (npcc.target == NONE)
	                     {
	                        npcc.target = ACTOR;
	                     }
	                  }
	                  break;
	               case NPCA:
	                  if (npca.state == DIE)
	                  {
	                     npcc.target = NONE;
	                  }
	                  break;
	               case NPCB:
	                  if (npcb.state == DIE)
	                  {
	                     npcc.target = NONE;
	                  }
	                  break;
	               case NPCC:
	                  if (npcc.state == DIE)
	                  {
	                     npcc.target = NONE;
	                  }
	                  break;
	               case NPCD:
	                  if (npcd.state == DIE)
	                  {
	                     npcc.target = NONE;
	                  }
	                  break;
	               case NPCE:
	                  if (npce.state == DIE)
	                  {
	                     npcc.target = NONE;
	                  }
	                  break;
	               case NPCF:
	                  if (npcf.state == DIE)
	                  {
	                     npcc.target = NONE;
	                  }
	                  break;
	               case NPCG:
	                  if (npcg.state == DIE)
	                  {
	                     npcc.target = NONE;
	                  }
	                  break;
	               default:
	                  break;
	            }
	         }
	      }
	      else
	      {
	         switch(npcc.target)
	         {
	            case NONE:
	               npcc.target = nearestFriend(NPCC, npccpos);
	               break;
	            case ACTOR:
	               if (actor.state == DIE)
	               {
	                  npcc.target = NONE;
	               }
	               break;
	            case NPCA:
	               if (npca.state == DIE)
	               {
	                  npcc.target = NONE;
	               }
	               break;
	            case NPCB:
	               if (npcb.state == DIE)
	               {
	                  npcc.target = NONE;
	               }
	               break;
	            case NPCC:
	               if (npcc.state == DIE)
	               {
	                  npcc.target = NONE;
	               }
	               break;
	            case NPCD:
	               if (npcd.state == DIE)
	               {
	                  npcc.target = NONE;
	               }
	               break;
	            case NPCE:
	               if (npce.state == DIE)
	               {
	                  npcc.target = NONE;
	               }
	               break;
	            case NPCF:
	               if (npcf.state == DIE)
	               {
	                  npcc.target = NONE;
	               }
	               break;
	            case NPCG:
	               if (npcg.state == DIE)
	               {
	                  npcc.target = NONE;
	               }
	               break;
	            default:
	               break;
	         }
	      }
	   }

		/*
			npcc.state:
				0 idle
				1 run
				2 attack1
				3 attack2
				4 damage
				5 die
		*/
		switch(npcc.state)
		{
			case IDLE:
			// combat idle
				npcc.Play(LOOP, (float) skip, FALSE, TRUE);
				switch (npcc.target)
	         {
	            case NONE:
	               break;
	            case ACTOR:
	               if (npcc.isFriend)
	               {
	                  if (myDist(pos, npccpos) >= 100.0f)
	                  {
	                     npcc.state = RUN;
	                     npcc.SetCurrentAction(NULL, 0, npcc.runID);
	                     npcc.frame = 0;
	                  }
	               }
	               else
	               {
	                  if (myDist(pos, npccpos) <= 500.0f && myDist(pos, npccpos) >= 100.0f)
	                  {  
	                     npcc.state = RUN;
	                     npcc.SetCurrentAction(NULL, 0, npcc.runID);
	                     npcc.frame = 0;
	                  }
	                  else if (myDist(pos, npccpos) < 100.0f  && actor.state != DIE)
	                  {
	                     npcc.wait1++;
	                     npccfDir[0] = pos[0] - npccpos[0];
	                     npccfDir[1] = pos[1] - npccpos[1];
	                     npcc.SetDirection(npccfDir, npccuDir);
	                     if (npcc.wait1 % 50 == 0)
	                     {
	                        npcc.wait2++;
	                        if (npcc.wait2 % 2 == 0)
	                        {
	                           npcc.state = ATTACK2;
	                           npcc.SetCurrentAction(NULL, 0, npcc.attack2ID);
	                           npcc.frame = 0;
	                        }
	                        else
	                        {
	                           npcc.state = ATTACK1;
	                           npcc.SetCurrentAction(NULL, 0, npcc.attack1ID);
	                           npcc.frame = 0;
	                        }
	                     }
	                  }
	               }
	               break;
	            case NPCA:
	               if (myDist(npccpos, npcapos) <= 600.0f && myDist(npccpos, npcapos) >= 100.0f)
	               {
	                  npcc.state = RUN;
	                  npcc.SetCurrentAction(NULL, 0, npcc.runID);
	                  npcc.frame = 0;
	               }
	               else if (myDist(npccpos, npcapos) < 100.0f)
	               {
	                  npcc.wait1++;
	                  npccfDir[0] = npcapos[0] - npccpos[0];
	                  npccfDir[1] = npcapos[1] - npccpos[1];
	                  npcc.SetDirection(npccfDir, npccuDir);
	                  if (npcc.wait1 % 50 == 0)
	                  {
	                     npcc.wait1++;
	                     if (npcc.wait2 % 2 == 0)
	                     {
	                        npcc.state = ATTACK2;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack2ID);
	                        npcc.frame = 0;
	                     }
	                     else
	                     {
	                        npcc.state = ATTACK1;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack1ID);
	                        npcc.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCB:
	               if (myDist(npccpos, npcbpos) <= 600.0f && myDist(npccpos, npcbpos) >= 100.0f)
	               {
	                  npcc.state = RUN;
	                  npcc.SetCurrentAction(NULL, 0, npcc.runID);
	                  npcc.frame = 0;
	               }
	               else if (myDist(npccpos, npcbpos) < 100.0f)
	               {
	                  npcc.wait1++;
	                  npccfDir[0] = npcbpos[0] - npccpos[0];
	                  npccfDir[1] = npcbpos[1] - npccpos[1];
	                  npcc.SetDirection(npccfDir, npccuDir);
	                  if (npcc.wait1 % 50 == 0)
	                  {
	                     npcc.wait1++;
	                     if (npcc.wait2 % 2 == 0)
	                     {
	                        npcc.state = ATTACK2;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack2ID);
	                        npcc.frame = 0;
	                     }
	                     else
	                     {
	                        npcc.state = ATTACK1;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack1ID);
	                        npcc.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCC:
	               if (myDist(npccpos, npccpos) <= 600.0f && myDist(npccpos, npccpos) >= 100.0f)
	               {
	                  npcc.state = RUN;
	                  npcc.SetCurrentAction(NULL, 0, npcc.runID);
	                  npcc.frame = 0;
	               }
	               else if (myDist(npccpos, npccpos) < 100.0f)
	               {
	                  npcc.wait1++;
	                  npccfDir[0] = npccpos[0] - npccpos[0];
	                  npccfDir[1] = npccpos[1] - npccpos[1];
	                  npcc.SetDirection(npccfDir, npccuDir);
	                  if (npcc.wait1 % 50 == 0)
	                  {
	                     npcc.wait1++;
	                     if (npcc.wait2 % 2 == 0)
	                     {
	                        npcc.state = ATTACK2;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack2ID);
	                        npcc.frame = 0;
	                     }
	                     else
	                     {
	                        npcc.state = ATTACK1;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack1ID);
	                        npcc.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCD:
	               if (myDist(npccpos, npcdpos) <= 600.0f && myDist(npccpos, npcdpos) >= 100.0f)
	               {
	                  npcc.state = RUN;
	                  npcc.SetCurrentAction(NULL, 0, npcc.runID);
	                  npcc.frame = 0;
	               }
	               else if (myDist(npccpos, npcdpos) < 100.0f)
	               {
	                  npcc.wait1++;
	                  npccfDir[0] = npcdpos[0] - npccpos[0];
	                  npccfDir[1] = npcdpos[1] - npccpos[1];
	                  npcc.SetDirection(npccfDir, npccuDir);
	                  if (npcc.wait1 % 50 == 0)
	                  {
	                     npcc.wait1++;
	                     if (npcc.wait2 % 2 == 0)
	                     {
	                        npcc.state = ATTACK2;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack2ID);
	                        npcc.frame = 0;
	                     }
	                     else
	                     {
	                        npcc.state = ATTACK1;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack1ID);
	                        npcc.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCE:
	               if (myDist(npccpos, npcepos) <= 600.0f && myDist(npccpos, npcepos) >= 100.0f)
	               {
	                  npcc.state = RUN;
	                  npcc.SetCurrentAction(NULL, 0, npcc.runID);
	                  npcc.frame = 0;
	               }
	               else if (myDist(npccpos, npcepos) < 100.0f)
	               {
	                  npcc.wait1++;
	                  npccfDir[0] = npcepos[0] - npccpos[0];
	                  npccfDir[1] = npcepos[1] - npccpos[1];
	                  npcc.SetDirection(npccfDir, npccuDir);
	                  if (npcc.wait1 % 50 == 0)
	                  {
	                     npcc.wait1++;
	                     if (npcc.wait2 % 2 == 0)
	                     {
	                        npcc.state = ATTACK2;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack2ID);
	                        npcc.frame = 0;
	                     }
	                     else
	                     {
	                        npcc.state = ATTACK1;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack1ID);
	                        npcc.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCF:
	               if (myDist(npccpos, npcfpos) <= 600.0f && myDist(npccpos, npcfpos) >= 100.0f)
	               {
	                  npcc.state = RUN;
	                  npcc.SetCurrentAction(NULL, 0, npcc.runID);
	                  npcc.frame = 0;
	               }
	               else if (myDist(npccpos, npcfpos) < 100.0f)
	               {
	                  npcc.wait1++;
	                  npccfDir[0] = npcfpos[0] - npccpos[0];
	                  npccfDir[1] = npcfpos[1] - npccpos[1];
	                  npcc.SetDirection(npccfDir, npccuDir);
	                  if (npcc.wait1 % 50 == 0)
	                  {
	                     npcc.wait1++;
	                     if (npcc.wait2 % 2 == 0)
	                     {
	                        npcc.state = ATTACK2;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack2ID);
	                        npcc.frame = 0;
	                     }
	                     else
	                     {
	                        npcc.state = ATTACK1;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack1ID);
	                        npcc.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCG:
	               if (myDist(npccpos, npcgpos) <= 600.0f && myDist(npccpos, npcgpos) >= 100.0f)
	               {
	                  npcc.state = RUN;
	                  npcc.SetCurrentAction(NULL, 0, npcc.runID);
	                  npcc.frame = 0;
	               }
	               else if (myDist(npccpos, npcgpos) < 100.0f)
	               {
	                  npcc.wait1++;
	                  npccfDir[0] = npcgpos[0] - npccpos[0];
	                  npccfDir[1] = npcgpos[1] - npccpos[1];
	                  npcc.SetDirection(npccfDir, npccuDir);
	                  if (npcc.wait1 % 50 == 0)
	                  {
	                     npcc.wait1++;
	                     if (npcc.wait2 % 2 == 0)
	                     {
	                        npcc.state = ATTACK2;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack2ID);
	                        npcc.frame = 0;
	                     }
	                     else
	                     {
	                        npcc.state = ATTACK1;
	                        npcc.SetCurrentAction(NULL, 0, npcc.attack1ID);
	                        npcc.frame = 0;
	                     }
	                  }
	               }
	               break;
	            default:
	               break;
	         }
				break;
			case RUN:
			// run
				npcc.Play(LOOP, (float) skip, FALSE, TRUE);
				switch(npcc.target)
	         {
	            case NONE:
	               npcc.state = IDLE;
	               npcc.SetCurrentAction(NULL, 0, npcc.idleID);
	               npcc.frame = 0;
	               break;
	            case ACTOR:
	               npccfDir[0] = pos[0] - npccpos[0];
	               npccfDir[1] = pos[1] - npccpos[1];
	               npcc.SetDirection(npccfDir, npccuDir);
	               npcc.MoveForward(5.0f);
	               if (myDist(pos, npccpos) < 90.0f)
	               {
	                  npcc.state = IDLE;
	                  npcc.SetCurrentAction(NULL, 0, npcc.idleID);
	                  npcc.frame = 0;
	               }
	               break;
	            case NPCA:
	               npccfDir[0] = npcapos[0] - npccpos[0];
	               npccfDir[1] = npcapos[1] - npccpos[1];
	               npcc.SetDirection(npccfDir, npccuDir);
	               npcc.MoveForward(5.0f);
	               if (myDist(npcapos, npccpos) < 90.0f)
	               {
	                  npcc.state = IDLE;
	                  npcc.SetCurrentAction(NULL, 0, npcc.idleID);
	                  npcc.frame = 0;
	               }
	               break;
	            case NPCB:
	               npccfDir[0] = npcbpos[0] - npccpos[0];
	               npccfDir[1] = npcbpos[1] - npccpos[1];
	               npcc.SetDirection(npccfDir, npccuDir);
	               npcc.MoveForward(5.0f);
	               if (myDist(npcbpos, npccpos) < 90.0f)
	               {
	                  npcc.state = IDLE;
	                  npcc.SetCurrentAction(NULL, 0, npcc.idleID);
	                  npcc.frame = 0;
	               }
	               break;
	            case NPCC:
	               npccfDir[0] = npccpos[0] - npccpos[0];
	               npccfDir[1] = npccpos[1] - npccpos[1];
	               npcc.SetDirection(npccfDir, npccuDir);
	               npcc.MoveForward(5.0f);
	               if (myDist(npccpos, npccpos) < 90.0f)
	               {
	                  npcc.state = IDLE;
	                  npcc.SetCurrentAction(NULL, 0, npcc.idleID);
	                  npcc.frame = 0;
	               }
	               break;
	            case NPCD:
	               npccfDir[0] = npcdpos[0] - npccpos[0];
	               npccfDir[1] = npcdpos[1] - npccpos[1];
	               npcc.SetDirection(npccfDir, npccuDir);
	               npcc.MoveForward(5.0f);
	               if (myDist(npcdpos, npccpos) < 90.0f)
	               {
	                  npcc.state = IDLE;
	                  npcc.SetCurrentAction(NULL, 0, npcc.idleID);
	                  npcc.frame = 0;
	               }
	               break;
	            case NPCE:
	               npccfDir[0] = npcepos[0] - npccpos[0];
	               npccfDir[1] = npcepos[1] - npccpos[1];
	               npcc.SetDirection(npccfDir, npccuDir);
	               npcc.MoveForward(5.0f);
	               if (myDist(npcepos, npccpos) < 90.0f)
	               {
	                  npcc.state = IDLE;
	                  npcc.SetCurrentAction(NULL, 0, npcc.idleID);
	                  npcc.frame = 0;
	               }
	               break;
	            case NPCF:
	               npccfDir[0] = npcfpos[0] - npccpos[0];
	               npccfDir[1] = npcfpos[1] - npccpos[1];
	               npcc.SetDirection(npccfDir, npccuDir);
	               npcc.MoveForward(5.0f);
	               if (myDist(npcfpos, npccpos) < 90.0f)
	               {
	                  npcc.state = IDLE;
	                  npcc.SetCurrentAction(NULL, 0, npcc.idleID);
	                  npcc.frame = 0;
	               }
	               break;
	            case NPCG:
	               npccfDir[0] = npcgpos[0] - npccpos[0];
	               npccfDir[1] = npcgpos[1] - npccpos[1];
	               npcc.SetDirection(npccfDir, npccuDir);
	               npcc.MoveForward(5.0f);
	               if (myDist(npcgpos, npccpos) < 90.0f)
	               {
	                  npcc.state = IDLE;
	                  npcc.SetCurrentAction(NULL, 0, npcc.idleID);
	                  npcc.frame = 0;
	               }
	               break;
	            default:
	               break;
	         }
				break;
			case ATTACK1:
			// normal attack 1 36 frames
				npcc.Play(ONCE, (float) skip, FALSE, TRUE);
				npcc.frame++;
				if (npcc.frame == 2)
				{
					hP2.Load("npcbAtt.wav");
					hP2.Play(ONCE);
				}
				if (npcc.frame == 17)
				{
					if (npcc.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npccpos, npcapos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npccpos, npcbpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npccpos, npccpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npccpos, npcdpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npccpos, npcepos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npccpos, npcfpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npccpos, npcgpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npccpos, pos, npccfDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npccpos, npcapos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npccpos, npcbpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npccpos, npccpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npccpos, npcdpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npccpos, npcepos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npccpos, npcfpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npccpos, npcgpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }	
				}
				if (npcc.frame == 35)
				{
					npcc.state = IDLE;
					npcc.SetCurrentAction(NULL, 0, npcc.idleID);	
					npcc.frame = 0;
				}
				break;
			case ATTACK2:
			// normal attack 2 26 frames
	         npcc.Play(ONCE, (float) skip, FALSE, TRUE);
	         npcc.frame++;
			if (npcc.frame == 2)
				{
					hP2.Load("npcbAtt.wav");
					hP2.Play(ONCE);
				}
	         if (npcc.frame == 17)
	         {
	            if (npcc.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npccpos, npcapos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npccpos, npcbpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npccpos, npccpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npccpos, npcdpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npccpos, npcepos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npccpos, npcfpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npccpos, npcgpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npccpos, pos, npccfDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npccpos, npcapos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npccpos, npcbpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npccpos, npccpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npccpos, npcdpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npccpos, npcepos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npccpos, npcfpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npccpos, npcgpos, npccfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }  
	         }
	         if (npcc.frame == 25)
	         {
	            npcc.state = IDLE;
	            npcc.SetCurrentAction(NULL, 0, npcc.idleID); 
	            npcc.frame = 0;
	         }
				break;
			case DAMAGE:
			// damage1 16 frames
				npcc.Play(ONCE, (float) skip, FALSE, TRUE);
				npcc.frame++;
				if (npcc.frame == 5)
					{
						hP.Load("npcbHurt.wav");
						hP.Play(ONCE);
					}
				if (npcc.frame == 15)
				{
					npcc.state = IDLE;
					npcc.SetCurrentAction(NULL, 0, npcc.idleID);
					npcc.frame = 0;
				}
				break;
			case DIE:
			// die
				npcc.Play(ONCE, (float) skip, FALSE, TRUE);
				npcc.frame++;
						if (npcc.frame == 5)
					{
						hP.Load("npcbDie.wav");
						hP.Play(ONCE);
					}
				break;
			default:
				break;
		}

		npcc.BB();

		/*------------------------------------------
		 ||||    || |||||||||   ||||||||  ||||||||
		 || ||   || ||      || ||      || ||      ||
		 ||  ||  || |||||||||  ||         ||      ||
		 ||   || || ||         ||      || ||      ||
		 ||    |||| ||          ||||||||  ||||||||
		 ------------------------------------------*/

		if (npcd.blood <= 0 && npcd.state != DIE)
		{
			npcd.state = DIE;
			npcd.SetCurrentAction(NULL, 0, npcd.dieID);
		}

	   
	if (npcd.state != DIE)
	{
	   if (npcd.isFriend)
	   {
	      if ((teammateID[0] == NPCD && isFollow[0]) || (teammateID[1] == NPCD && isFollow[1]))
	      {
	         npcd.target = ACTOR;
	      }
	      else
	      {
	         switch(npcd.target)
	         {
	            case NONE:
	               npcd.target = nearestEnemy(NPCD, npcdpos);
	               if (npcd.target == NONE)
	               {
	                  npcd.target = ACTOR;
	               }
	               break;
	            case ACTOR:
	               if (actor.state == DIE)
	               {
	                  npcd.target = NONE;
	               }
	               if ((teammateID[0] == NPCD && !isFollow[0]) || (teammateID[1] == NPCD && !isFollow[1]))
	               {
	                  npcd.target = nearestEnemy(NPCD, npcdpos);
	                  if (npcd.target == NONE)
	                  {
	                     npcd.target = ACTOR;
	                  }
	               }
	               break;
	            case NPCA:
	               if (npca.state == DIE)
	               {
	                  npcd.target = NONE;
	               }
	               break;
	            case NPCB:
	               if (npcb.state == DIE)
	               {
	                  npcd.target = NONE;
	               }
	               break;
	            case NPCC:
	               if (npcc.state == DIE)
	               {
	                  npcd.target = NONE;
	               }
	               break;
	            case NPCD:
	               if (npcd.state == DIE)
	               {
	                  npcd.target = NONE;
	               }
	               break;
	            case NPCE:
	               if (npce.state == DIE)
	               {
	                  npcd.target = NONE;
	               }
	               break;
	            case NPCF:
	               if (npcf.state == DIE)
	               {
	                  npcd.target = NONE;
	               }
	               break;
	            case NPCG:
	               if (npcg.state == DIE)
	               {
	                  npcd.target = NONE;
	               }
	               break;
	            default:
	               break;
	         }
	      }
	   }
	   else
	   {
	      switch(npcd.target)
	      {
	         case NONE:
	            npcd.target = nearestFriend(NPCD, npcdpos);
	            break;
	         case ACTOR:
	            if (actor.state == DIE)
	            {
	               npcd.target = NONE;
	            }
	            break;
	         case NPCA:
	            if (npca.state == DIE)
	            {
	               npcd.target = NONE;
	            }
	            break;
	         case NPCB:
	            if (npcb.state == DIE)
	            {
	               npcd.target = NONE;
	            }
	            break;
	         case NPCC:
	            if (npcc.state == DIE)
	            {
	               npcd.target = NONE;
	            }
	            break;
	         case NPCD:
	            if (npcd.state == DIE)
	            {
	               npcd.target = NONE;
	            }
	            break;
	         case NPCE:
	            if (npce.state == DIE)
	            {
	               npcd.target = NONE;
	            }
	            break;
	         case NPCF:
	            if (npcf.state == DIE)
	            {
	               npcd.target = NONE;
	            }
	            break;
	         case NPCG:
	            if (npcg.state == DIE)
	            {
	               npcd.target = NONE;
	            }
	            break;
	         default:
	            break;
	      }
	   }
	}

		/*
			npcd.state:
				0 idle
				1 run
				2 attack1
				3 attack2
				4 damage
				5 die
		*/
		switch(npcd.state)
		{
			case IDLE:
			// combat idle
				npcd.Play(LOOP, (float) skip, FALSE, TRUE);
	         switch (npcd.target)
	         {
	            case NONE:
	               break;
	            case ACTOR:
	               if (npcd.isFriend)
	               {
	                  if (myDist(pos, npcdpos) >= 100.0f)
	                  {
	                     npcd.state = RUN;
	                     npcd.SetCurrentAction(NULL, 0, npcd.runID);
	                     npcd.frame = 0;
	                  }
	               }
	               else
	               {
	                  if (myDist(pos, npcdpos) <= 500.0f && myDist(pos, npcdpos) >= 100.0f)
	                  {  
	                     npcd.state = RUN;
	                     npcd.SetCurrentAction(NULL, 0, npcd.runID);
	                     npcd.frame = 0;
	                  }
	                  else if (myDist(pos, npcdpos) < 100.0f  && actor.state != DIE)
	                  {
	                     npcd.wait1++;
	                     npcdfDir[0] = pos[0] - npcdpos[0];
	                     npcdfDir[1] = pos[1] - npcdpos[1];
	                     npcd.SetDirection(npcdfDir, npcduDir);
	                     if (npcd.wait1 % 50 == 0)
	                     {
	                        npcd.wait2++;
	                        if (npcd.wait2 % 2 == 0)
	                        {
	                           npcd.state = ATTACK2;
	                           npcd.SetCurrentAction(NULL, 0, npcd.attack2ID);
	                           npcd.frame = 0;
	                        }
	                        else
	                        {
	                           npcd.state = ATTACK1;
	                           npcd.SetCurrentAction(NULL, 0, npcd.attack1ID);
	                           npcd.frame = 0;
	                        }
	                     }
	                  }
	               }
	               break;
	            case NPCA:
	               if (myDist(npcdpos, npcapos) <= 600.0f && myDist(npcdpos, npcapos) >= 100.0f)
	               {
	                  npcd.state = RUN;
	                  npcd.SetCurrentAction(NULL, 0, npcd.runID);
	                  npcd.frame = 0;
	               }
	               else if (myDist(npcdpos, npcapos) < 100.0f)
	               {
	                  npcd.wait1++;
	                  npcdfDir[0] = npcapos[0] - npcdpos[0];
	                  npcdfDir[1] = npcapos[1] - npcdpos[1];
	                  npcd.SetDirection(npcdfDir, npcduDir);
	                  if (npcd.wait1 % 50 == 0)
	                  {
	                     npcd.wait1++;
	                     if (npcd.wait2 % 2 == 0)
	                     {
	                        npcd.state = ATTACK2;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack2ID);
	                        npcd.frame = 0;
	                     }
	                     else
	                     {
	                        npcd.state = ATTACK1;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack1ID);
	                        npcd.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCB:
	               if (myDist(npcdpos, npcbpos) <= 600.0f && myDist(npcdpos, npcbpos) >= 100.0f)
	               {
	                  npcd.state = RUN;
	                  npcd.SetCurrentAction(NULL, 0, npcd.runID);
	                  npcd.frame = 0;
	               }
	               else if (myDist(npcdpos, npcbpos) < 100.0f)
	               {
	                  npcd.wait1++;
	                  npcdfDir[0] = npcbpos[0] - npcdpos[0];
	                  npcdfDir[1] = npcbpos[1] - npcdpos[1];
	                  npcd.SetDirection(npcdfDir, npcduDir);
	                  if (npcd.wait1 % 50 == 0)
	                  {
	                     npcd.wait1++;
	                     if (npcd.wait2 % 2 == 0)
	                     {
	                        npcd.state = ATTACK2;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack2ID);
	                        npcd.frame = 0;
	                     }
	                     else
	                     {
	                        npcd.state = ATTACK1;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack1ID);
	                        npcd.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCC:
	               if (myDist(npcdpos, npccpos) <= 600.0f && myDist(npcdpos, npccpos) >= 100.0f)
	               {
	                  npcd.state = RUN;
	                  npcd.SetCurrentAction(NULL, 0, npcd.runID);
	                  npcd.frame = 0;
	               }
	               else if (myDist(npcdpos, npccpos) < 100.0f)
	               {
	                  npcd.wait1++;
	                  npcdfDir[0] = npccpos[0] - npcdpos[0];
	                  npcdfDir[1] = npccpos[1] - npcdpos[1];
	                  npcd.SetDirection(npcdfDir, npcduDir);
	                  if (npcd.wait1 % 50 == 0)
	                  {
	                     npcd.wait1++;
	                     if (npcd.wait2 % 2 == 0)
	                     {
	                        npcd.state = ATTACK2;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack2ID);
	                        npcd.frame = 0;
	                     }
	                     else
	                     {
	                        npcd.state = ATTACK1;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack1ID);
	                        npcd.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCD:
	               if (myDist(npcdpos, npcdpos) <= 600.0f && myDist(npcdpos, npcdpos) >= 100.0f)
	               {
	                  npcd.state = RUN;
	                  npcd.SetCurrentAction(NULL, 0, npcd.runID);
	                  npcd.frame = 0;
	               }
	               else if (myDist(npcdpos, npcdpos) < 100.0f)
	               {
	                  npcd.wait1++;
	                  npcdfDir[0] = npcdpos[0] - npcdpos[0];
	                  npcdfDir[1] = npcdpos[1] - npcdpos[1];
	                  npcd.SetDirection(npcdfDir, npcduDir);
	                  if (npcd.wait1 % 50 == 0)
	                  {
	                     npcd.wait1++;
	                     if (npcd.wait2 % 2 == 0)
	                     {
	                        npcd.state = ATTACK2;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack2ID);
	                        npcd.frame = 0;
	                     }
	                     else
	                     {
	                        npcd.state = ATTACK1;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack1ID);
	                        npcd.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCE:
	               if (myDist(npcdpos, npcepos) <= 600.0f && myDist(npcdpos, npcepos) >= 100.0f)
	               {
	                  npcd.state = RUN;
	                  npcd.SetCurrentAction(NULL, 0, npcd.runID);
	                  npcd.frame = 0;
	               }
	               else if (myDist(npcdpos, npcepos) < 100.0f)
	               {
	                  npcd.wait1++;
	                  npcdfDir[0] = npcepos[0] - npcdpos[0];
	                  npcdfDir[1] = npcepos[1] - npcdpos[1];
	                  npcd.SetDirection(npcdfDir, npcduDir);
	                  if (npcd.wait1 % 50 == 0)
	                  {
	                     npcd.wait1++;
	                     if (npcd.wait2 % 2 == 0)
	                     {
	                        npcd.state = ATTACK2;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack2ID);
	                        npcd.frame = 0;
	                     }
	                     else
	                     {
	                        npcd.state = ATTACK1;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack1ID);
	                        npcd.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCF:
	               if (myDist(npcdpos, npcfpos) <= 600.0f && myDist(npcdpos, npcfpos) >= 100.0f)
	               {
	                  npcd.state = RUN;
	                  npcd.SetCurrentAction(NULL, 0, npcd.runID);
	                  npcd.frame = 0;
	               }
	               else if (myDist(npcdpos, npcfpos) < 100.0f)
	               {
	                  npcd.wait1++;
	                  npcdfDir[0] = npcfpos[0] - npcdpos[0];
	                  npcdfDir[1] = npcfpos[1] - npcdpos[1];
	                  npcd.SetDirection(npcdfDir, npcduDir);
	                  if (npcd.wait1 % 50 == 0)
	                  {
	                     npcd.wait1++;
	                     if (npcd.wait2 % 2 == 0)
	                     {
	                        npcd.state = ATTACK2;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack2ID);
	                        npcd.frame = 0;
	                     }
	                     else
	                     {
	                        npcd.state = ATTACK1;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack1ID);
	                        npcd.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCG:
	               if (myDist(npcdpos, npcgpos) <= 600.0f && myDist(npcdpos, npcgpos) >= 100.0f)
	               {
	                  npcd.state = RUN;
	                  npcd.SetCurrentAction(NULL, 0, npcd.runID);
	                  npcd.frame = 0;
	               }
	               else if (myDist(npcdpos, npcgpos) < 100.0f)
	               {
	                  npcd.wait1++;
	                  npcdfDir[0] = npcgpos[0] - npcdpos[0];
	                  npcdfDir[1] = npcgpos[1] - npcdpos[1];
	                  npcd.SetDirection(npcdfDir, npcduDir);
	                  if (npcd.wait1 % 50 == 0)
	                  {
	                     npcd.wait1++;
	                     if (npcd.wait2 % 2 == 0)
	                     {
	                        npcd.state = ATTACK2;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack2ID);
	                        npcd.frame = 0;
	                     }
	                     else
	                     {
	                        npcd.state = ATTACK1;
	                        npcd.SetCurrentAction(NULL, 0, npcd.attack1ID);
	                        npcd.frame = 0;
	                     }
	                  }
	               }
	               break;
	            default:
	               break;
	         }
	         break;
			case RUN:
			// run
				npcd.Play(LOOP, (float) skip, FALSE, TRUE);

	         switch(npcd.target)
	         {
	            case NONE:
	               npcd.state = IDLE;
	               npcd.SetCurrentAction(NULL, 0, npcd.idleID);
	               npcd.frame = 0;
	               break;
	            case ACTOR:
	               npcdfDir[0] = pos[0] - npcdpos[0];
	               npcdfDir[1] = pos[1] - npcdpos[1];
	               npcd.SetDirection(npcdfDir, npcduDir);
	               npcd.MoveForward(5.0f);
	               if (myDist(pos, npcdpos) < 90.0f)
	               {
	                  npcd.state = IDLE;
	                  npcd.SetCurrentAction(NULL, 0, npcd.idleID);
	                  npcd.frame = 0;
	               }
	               break;
	            case NPCA:
	               npcdfDir[0] = npcapos[0] - npcdpos[0];
	               npcdfDir[1] = npcapos[1] - npcdpos[1];
	               npcd.SetDirection(npcdfDir, npcduDir);
	               npcd.MoveForward(5.0f);
	               if (myDist(npcapos, npcdpos) < 90.0f)
	               {
	                  npcd.state = IDLE;
	                  npcd.SetCurrentAction(NULL, 0, npcd.idleID);
	                  npcd.frame = 0;
	               }
	               break;
	            case NPCB:
	               npcdfDir[0] = npcbpos[0] - npcdpos[0];
	               npcdfDir[1] = npcbpos[1] - npcdpos[1];
	               npcd.SetDirection(npcdfDir, npcduDir);
	               npcd.MoveForward(5.0f);
	               if (myDist(npcbpos, npcdpos) < 90.0f)
	               {
	                  npcd.state = IDLE;
	                  npcd.SetCurrentAction(NULL, 0, npcd.idleID);
	                  npcd.frame = 0;
	               }
	               break;
	            case NPCC:
	               npcdfDir[0] = npccpos[0] - npcdpos[0];
	               npcdfDir[1] = npccpos[1] - npcdpos[1];
	               npcd.SetDirection(npcdfDir, npcduDir);
	               npcd.MoveForward(5.0f);
	               if (myDist(npccpos, npcdpos) < 90.0f)
	               {
	                  npcd.state = IDLE;
	                  npcd.SetCurrentAction(NULL, 0, npcd.idleID);
	                  npcd.frame = 0;
	               }
	               break;
	            case NPCD:
	               npcdfDir[0] = npcdpos[0] - npcdpos[0];
	               npcdfDir[1] = npcdpos[1] - npcdpos[1];
	               npcd.SetDirection(npcdfDir, npcduDir);
	               npcd.MoveForward(5.0f);
	               if (myDist(npcdpos, npcdpos) < 90.0f)
	               {
	                  npcd.state = IDLE;
	                  npcd.SetCurrentAction(NULL, 0, npcd.idleID);
	                  npcd.frame = 0;
	               }
	               break;
	            case NPCE:
	               npcdfDir[0] = npcepos[0] - npcdpos[0];
	               npcdfDir[1] = npcepos[1] - npcdpos[1];
	               npcd.SetDirection(npcdfDir, npcduDir);
	               npcd.MoveForward(5.0f);
	               if (myDist(npcepos, npcdpos) < 90.0f)
	               {
	                  npcd.state = IDLE;
	                  npcd.SetCurrentAction(NULL, 0, npcd.idleID);
	                  npcd.frame = 0;
	               }
	               break;
	            case NPCF:
	               npcdfDir[0] = npcfpos[0] - npcdpos[0];
	               npcdfDir[1] = npcfpos[1] - npcdpos[1];
	               npcd.SetDirection(npcdfDir, npcduDir);
	               npcd.MoveForward(5.0f);
	               if (myDist(npcfpos, npcdpos) < 90.0f)
	               {
	                  npcd.state = IDLE;
	                  npcd.SetCurrentAction(NULL, 0, npcd.idleID);
	                  npcd.frame = 0;
	               }
	               break;
	            case NPCG:
	               npcdfDir[0] = npcgpos[0] - npcdpos[0];
	               npcdfDir[1] = npcgpos[1] - npcdpos[1];
	               npcd.SetDirection(npcdfDir, npcduDir);
	               npcd.MoveForward(5.0f);
	               if (myDist(npcgpos, npcdpos) < 90.0f)
	               {
	                  npcd.state = IDLE;
	                  npcd.SetCurrentAction(NULL, 0, npcd.idleID);
	                  npcd.frame = 0;
	               }
	               break;
	            default:
	               break;
	         }
				break;
			case ATTACK1:
			// normal attack 1 31 frames
				npcd.Play(ONCE, (float) skip, FALSE, TRUE);
				npcd.frame++;
				if (npcd.frame == 4)
				{
					hP2.Load("Matt1.wav");
					hP2.Play(ONCE);
				}
				if (npcd.frame == 17)
				{
					if (npcd.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npcdpos, npcapos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npcdpos, npcbpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npcdpos, npccpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npcdpos, npcdpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npcdpos, npcepos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npcdpos, npcfpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npcdpos, npcgpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npcdpos, pos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npcdpos, npcapos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npcdpos, npcbpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npcdpos, npccpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npcdpos, npcdpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npcdpos, npcepos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npcdpos, npcfpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npcdpos, npcgpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
				}
				if (npcd.frame == 30)
				{
					npcd.state = IDLE;
					npcd.SetCurrentAction(NULL, 0, npcd.idleID);	
					npcd.frame = 0;
				}
				break;
			case ATTACK2:
			// normal attack 2 31 frames
	         npcd.Play(ONCE, (float) skip, FALSE, TRUE);
	         npcd.frame++;
			 if (npcd.frame == 0)
				{
					hP2.Load("Mhurt2.wav");
					hP2.Play(ONCE);
				}
	         if (npcd.frame == 17)
	         {
	            if (npcd.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npcdpos, npcapos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npcdpos, npcbpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npcdpos, npccpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npcdpos, npcdpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npcdpos, npcepos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npcdpos, npcfpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npcdpos, npcgpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npcdpos, pos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npcdpos, npcapos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npcdpos, npcbpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npcdpos, npccpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npcdpos, npcdpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npcdpos, npcepos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npcdpos, npcfpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npcdpos, npcgpos, npcdfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	         }
	         if (npcd.frame == 30)
	         {
	            npcd.state = IDLE;
	            npcd.SetCurrentAction(NULL, 0, npcd.idleID); 
	            npcd.frame = 0;
	         }
				break;
			case DAMAGE:
			// no damage
				if (npcd.frame == 0)
				{
					npcd.SetCurrentAction(NULL, 0, npcd.idleID);
					hP2.Load("Mhurt.wav");
					hP2.Play(ONCE);
				}
				npcd.Play(LOOP, (float) skip, FALSE, TRUE);
				npcd.frame++;
				if (npcd.frame == 15)
				{
					npcd.state = IDLE;
					npcd.SetCurrentAction(NULL, 0, npcd.idleID);
					npcd.frame = 0;
				}
				break;
			case DIE:
			// die 46
				npcd.Play(ONCE, (float) skip, FALSE, TRUE);
				if (!npcd.isFriend)
				{
					npcd.frame++;
					if (npcd.frame == 4)
					{

						hP2.Load("Mdie2.wav");
						hP2.Play(ONCE);
					}
					if (npcd.frame == 75)
					{
						if (teammateID[0] == NONE)
						{
							teammateID[0] = NPCD;
							npcd.state = IDLE;
							npcd.SetCurrentAction(NULL, 0, npcd.idleID);
							npcd.frame = 0;
							npcd.blood = npcd.fullBlood;
						}
						else if (teammateID[1] == NONE)
						{
							teammateID[1] = NPCD;
							npcd.state = IDLE;
							npcd.SetCurrentAction(NULL, 0, npcd.idleID);
							npcd.frame = 0;
							npcd.blood = npcd.fullBlood;
						}
						npcd.isFriend = true;
					}
				}
				break;
			default:
				break;
		}

		npcd.BB();

		/*------------------------------------------
		 ||||    || |||||||||   ||||||||  ||||||||||
		 || ||   || ||      || ||      || ||
		 ||  ||  || |||||||||  ||         ||||||||
		 ||   || || ||         ||      || ||
		 ||    |||| ||          ||||||||  ||||||||||
		 ------------------------------------------*/

		if (npce.blood <= 0 && npce.state != DIE)
		{
			npce.state = DIE;
			npce.SetCurrentAction(NULL, 0, npce.dieID);
		}

	   if (npce.state != DIE)
	   {
	      if (npce.isFriend)
	      {
	         if ((teammateID[0] == NPCE && isFollow[0]) || (teammateID[1] == NPCE && isFollow[1]))
	         {
	            npce.target = ACTOR;
	         }
	         else
	         {
	            switch(npce.target)
	            {
	               case NONE:
	                  npce.target = nearestEnemy(NPCE, npcepos);
	                  if (npce.target == NONE)
	                  {
	                     npce.target = ACTOR;
	                  }
	                  break;
	               case ACTOR:
	                  if (actor.state == DIE)
	                  {
	                     npce.target = NONE;
	                  }
	                  if ((teammateID[0] == NPCE && !isFollow[0]) || (teammateID[1] == NPCE && !isFollow[1]))
	                  {
	                     npce.target = nearestEnemy(NPCE, npcepos);
	                     if (npce.target == NONE)
	                     {
	                        npce.target = ACTOR;
	                     }
	                  }
	                  break;
	               case NPCA:
	                  if (npca.state == DIE)
	                  {
	                     npce.target = NONE;
	                  }
	                  break;
	               case NPCB:
	                  if (npcb.state == DIE)
	                  {
	                     npce.target = NONE;
	                  }
	                  break;
	               case NPCC:
	                  if (npcc.state == DIE)
	                  {
	                     npce.target = NONE;
	                  }
	                  break;
	               case NPCD:
	                  if (npcd.state == DIE)
	                  {
	                     npce.target = NONE;
	                  }
	                  break;
	               case NPCE:
	                  if (npce.state == DIE)
	                  {
	                     npce.target = NONE;
	                  }
	                  break;
	               case NPCF:
	                  if (npcf.state == DIE)
	                  {
	                     npce.target = NONE;
	                  }
	                  break;
	               case NPCG:
	                  if (npcg.state == DIE)
	                  {
	                     npce.target = NONE;
	                  }
	                  break;
	               default:
	                  break;
	            }
	         }
	      }
	      else
	      {
	         switch(npce.target)
	         {
	            case NONE:
	               npce.target = nearestFriend(NPCE, npcepos);
	               break;
	            case ACTOR:
	               if (actor.state == DIE)
	               {
	                  npce.target = NONE;
	               }
	               break;
	            case NPCA:
	               if (npca.state == DIE)
	               {
	                  npce.target = NONE;
	               }
	               break;
	            case NPCB:
	               if (npcb.state == DIE)
	               {
	                  npce.target = NONE;
	               }
	               break;
	            case NPCC:
	               if (npcc.state == DIE)
	               {
	                  npce.target = NONE;
	               }
	               break;
	            case NPCD:
	               if (npcd.state == DIE)
	               {
	                  npce.target = NONE;
	               }
	               break;
	            case NPCE:
	               if (npce.state == DIE)
	               {
	                  npce.target = NONE;
	               }
	               break;
	            case NPCF:
	               if (npcf.state == DIE)
	               {
	                  npce.target = NONE;
	               }
	               break;
	            case NPCG:
	               if (npcg.state == DIE)
	               {
	                  npce.target = NONE;
	               }
	               break;
	            default:
	               break;
	         }
	      }
	   }

		/*
			npce.state:
				0 idle
				1 run
				2 attack1
				3 attack2
				4 damage
				5 die
		*/
		switch(npce.state)
		{
			case IDLE:
			// combat idle
				npce.Play(LOOP, (float) skip, FALSE, TRUE);
				switch (npce.target)
	         {
	            case NONE:
	               break;
	            case ACTOR:
	               if (npce.isFriend)
	               {
	                  if (myDist(pos, npcepos) >= 100.0f)
	                  {
	                     npce.state = RUN;
	                     npce.SetCurrentAction(NULL, 0, npce.runID);
	                     npce.frame = 0;
	                  }
	               }
	               else
	               {
	                  if (myDist(pos, npcepos) <= 500.0f && myDist(pos, npcepos) >= 100.0f)
	                  {  
	                     npce.state = RUN;
	                     npce.SetCurrentAction(NULL, 0, npce.runID);
	                     npce.frame = 0;
	                  }
	                  else if (myDist(pos, npcepos) < 100.0f  && actor.state != DIE)
	                  {
	                     npce.wait1++;
	                     npcefDir[0] = pos[0] - npcepos[0];
	                     npcefDir[1] = pos[1] - npcepos[1];
	                     npce.SetDirection(npcefDir, npceuDir);
	                     if (npce.wait1 % 50 == 0)
	                     {
	                        npce.wait2++;
	                        if (npce.wait2 % 2 == 0)
	                        {
	                           npce.state = ATTACK2;
	                           npce.SetCurrentAction(NULL, 0, npce.attack2ID);
	                           npce.frame = 0;
	                        }
	                        else
	                        {
	                           npce.state = ATTACK1;
	                           npce.SetCurrentAction(NULL, 0, npce.attack1ID);
	                           npce.frame = 0;
	                        }
	                     }
	                  }
	               }
	               break;
	            case NPCA:
	               if (myDist(npcepos, npcapos) <= 600.0f && myDist(npcepos, npcapos) >= 100.0f)
	               {
	                  npce.state = RUN;
	                  npce.SetCurrentAction(NULL, 0, npce.runID);
	                  npce.frame = 0;
	               }
	               else if (myDist(npcepos, npcapos) < 100.0f)
	               {
	                  npce.wait1++;
	                  npcefDir[0] = npcapos[0] - npcepos[0];
	                  npcefDir[1] = npcapos[1] - npcepos[1];
	                  npce.SetDirection(npcefDir, npceuDir);
	                  if (npce.wait1 % 50 == 0)
	                  {
	                     npce.wait1++;
	                     if (npce.wait2 % 2 == 0)
	                     {
	                        npce.state = ATTACK2;
	                        npce.SetCurrentAction(NULL, 0, npce.attack2ID);
	                        npce.frame = 0;
	                     }
	                     else
	                     {
	                        npce.state = ATTACK1;
	                        npce.SetCurrentAction(NULL, 0, npce.attack1ID);
	                        npce.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCB:
	               if (myDist(npcepos, npcbpos) <= 600.0f && myDist(npcepos, npcbpos) >= 100.0f)
	               {
	                  npce.state = RUN;
	                  npce.SetCurrentAction(NULL, 0, npce.runID);
	                  npce.frame = 0;
	               }
	               else if (myDist(npcepos, npcbpos) < 100.0f)
	               {
	                  npce.wait1++;
	                  npcefDir[0] = npcbpos[0] - npcepos[0];
	                  npcefDir[1] = npcbpos[1] - npcepos[1];
	                  npce.SetDirection(npcefDir, npceuDir);
	                  if (npce.wait1 % 50 == 0)
	                  {
	                     npce.wait1++;
	                     if (npce.wait2 % 2 == 0)
	                     {
	                        npce.state = ATTACK2;
	                        npce.SetCurrentAction(NULL, 0, npce.attack2ID);
	                        npce.frame = 0;
	                     }
	                     else
	                     {
	                        npce.state = ATTACK1;
	                        npce.SetCurrentAction(NULL, 0, npce.attack1ID);
	                        npce.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCC:
	               if (myDist(npcepos, npccpos) <= 600.0f && myDist(npcepos, npccpos) >= 100.0f)
	               {
	                  npce.state = RUN;
	                  npce.SetCurrentAction(NULL, 0, npce.runID);
	                  npce.frame = 0;
	               }
	               else if (myDist(npcepos, npccpos) < 100.0f)
	               {
	                  npce.wait1++;
	                  npcefDir[0] = npccpos[0] - npcepos[0];
	                  npcefDir[1] = npccpos[1] - npcepos[1];
	                  npce.SetDirection(npcefDir, npceuDir);
	                  if (npce.wait1 % 50 == 0)
	                  {
	                     npce.wait1++;
	                     if (npce.wait2 % 2 == 0)
	                     {
	                        npce.state = ATTACK2;
	                        npce.SetCurrentAction(NULL, 0, npce.attack2ID);
	                        npce.frame = 0;
	                     }
	                     else
	                     {
	                        npce.state = ATTACK1;
	                        npce.SetCurrentAction(NULL, 0, npce.attack1ID);
	                        npce.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCD:
	               if (myDist(npcepos, npcdpos) <= 600.0f && myDist(npcepos, npcdpos) >= 100.0f)
	               {
	                  npce.state = RUN;
	                  npce.SetCurrentAction(NULL, 0, npce.runID);
	                  npce.frame = 0;
	               }
	               else if (myDist(npcepos, npcdpos) < 100.0f)
	               {
	                  npce.wait1++;
	                  npcefDir[0] = npcdpos[0] - npcepos[0];
	                  npcefDir[1] = npcdpos[1] - npcepos[1];
	                  npce.SetDirection(npcefDir, npceuDir);
	                  if (npce.wait1 % 50 == 0)
	                  {
	                     npce.wait1++;
	                     if (npce.wait2 % 2 == 0)
	                     {
	                        npce.state = ATTACK2;
	                        npce.SetCurrentAction(NULL, 0, npce.attack2ID);
	                        npce.frame = 0;
	                     }
	                     else
	                     {
	                        npce.state = ATTACK1;
	                        npce.SetCurrentAction(NULL, 0, npce.attack1ID);
	                        npce.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCE:
	               if (myDist(npcepos, npcepos) <= 600.0f && myDist(npcepos, npcepos) >= 100.0f)
	               {
	                  npce.state = RUN;
	                  npce.SetCurrentAction(NULL, 0, npce.runID);
	                  npce.frame = 0;
	               }
	               else if (myDist(npcepos, npcepos) < 100.0f)
	               {
	                  npce.wait1++;
	                  npcefDir[0] = npcepos[0] - npcepos[0];
	                  npcefDir[1] = npcepos[1] - npcepos[1];
	                  npce.SetDirection(npcefDir, npceuDir);
	                  if (npce.wait1 % 50 == 0)
	                  {
	                     npce.wait1++;
	                     if (npce.wait2 % 2 == 0)
	                     {
	                        npce.state = ATTACK2;
	                        npce.SetCurrentAction(NULL, 0, npce.attack2ID);
	                        npce.frame = 0;
	                     }
	                     else
	                     {
	                        npce.state = ATTACK1;
	                        npce.SetCurrentAction(NULL, 0, npce.attack1ID);
	                        npce.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCF:
	               if (myDist(npcepos, npcfpos) <= 600.0f && myDist(npcepos, npcfpos) >= 100.0f)
	               {
	                  npce.state = RUN;
	                  npce.SetCurrentAction(NULL, 0, npce.runID);
	                  npce.frame = 0;
	               }
	               else if (myDist(npcepos, npcfpos) < 100.0f)
	               {
	                  npce.wait1++;
	                  npcefDir[0] = npcfpos[0] - npcepos[0];
	                  npcefDir[1] = npcfpos[1] - npcepos[1];
	                  npce.SetDirection(npcefDir, npceuDir);
	                  if (npce.wait1 % 50 == 0)
	                  {
	                     npce.wait1++;
	                     if (npce.wait2 % 2 == 0)
	                     {
	                        npce.state = ATTACK2;
	                        npce.SetCurrentAction(NULL, 0, npce.attack2ID);
	                        npce.frame = 0;
	                     }
	                     else
	                     {
	                        npce.state = ATTACK1;
	                        npce.SetCurrentAction(NULL, 0, npce.attack1ID);
	                        npce.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCG:
	               if (myDist(npcepos, npcgpos) <= 600.0f && myDist(npcepos, npcgpos) >= 100.0f)
	               {
	                  npce.state = RUN;
	                  npce.SetCurrentAction(NULL, 0, npce.runID);
	                  npce.frame = 0;
	               }
	               else if (myDist(npcepos, npcgpos) < 100.0f)
	               {
	                  npce.wait1++;
	                  npcefDir[0] = npcgpos[0] - npcepos[0];
	                  npcefDir[1] = npcgpos[1] - npcepos[1];
	                  npce.SetDirection(npcefDir, npceuDir);
	                  if (npce.wait1 % 50 == 0)
	                  {
	                     npce.wait1++;
	                     if (npce.wait2 % 2 == 0)
	                     {
	                        npce.state = ATTACK2;
	                        npce.SetCurrentAction(NULL, 0, npce.attack2ID);
	                        npce.frame = 0;
	                     }
	                     else
	                     {
	                        npce.state = ATTACK1;
	                        npce.SetCurrentAction(NULL, 0, npce.attack1ID);
	                        npce.frame = 0;
	                     }
	                  }
	               }
	               break;
	            default:
	               break;
	         }
				break;
			case RUN:
			// run
				npce.Play(LOOP, (float) skip, FALSE, TRUE);
				switch(npce.target)
	         {
	            case NONE:
	               npce.state = IDLE;
	               npce.SetCurrentAction(NULL, 0, npce.idleID);
	               npce.frame = 0;
	               break;
	            case ACTOR:
	               npcefDir[0] = pos[0] - npcepos[0];
	               npcefDir[1] = pos[1] - npcepos[1];
	               npce.SetDirection(npcefDir, npceuDir);
	               npce.MoveForward(5.0f);
	               if (myDist(pos, npcepos) < 90.0f)
	               {
	                  npce.state = IDLE;
	                  npce.SetCurrentAction(NULL, 0, npce.idleID);
	                  npce.frame = 0;
	               }
	               break;
	            case NPCA:
	               npcefDir[0] = npcapos[0] - npcepos[0];
	               npcefDir[1] = npcapos[1] - npcepos[1];
	               npce.SetDirection(npcefDir, npceuDir);
	               npce.MoveForward(5.0f);
	               if (myDist(npcapos, npcepos) < 90.0f)
	               {
	                  npce.state = IDLE;
	                  npce.SetCurrentAction(NULL, 0, npce.idleID);
	                  npce.frame = 0;
	               }
	               break;
	            case NPCB:
	               npcefDir[0] = npcbpos[0] - npcepos[0];
	               npcefDir[1] = npcbpos[1] - npcepos[1];
	               npce.SetDirection(npcefDir, npceuDir);
	               npce.MoveForward(5.0f);
	               if (myDist(npcbpos, npcepos) < 90.0f)
	               {
	                  npce.state = IDLE;
	                  npce.SetCurrentAction(NULL, 0, npce.idleID);
	                  npce.frame = 0;
	               }
	               break;
	            case NPCC:
	               npcefDir[0] = npccpos[0] - npcepos[0];
	               npcefDir[1] = npccpos[1] - npcepos[1];
	               npce.SetDirection(npcefDir, npceuDir);
	               npce.MoveForward(5.0f);
	               if (myDist(npccpos, npcepos) < 90.0f)
	               {
	                  npce.state = IDLE;
	                  npce.SetCurrentAction(NULL, 0, npce.idleID);
	                  npce.frame = 0;
	               }
	               break;
	            case NPCD:
	               npcefDir[0] = npcdpos[0] - npcepos[0];
	               npcefDir[1] = npcdpos[1] - npcepos[1];
	               npce.SetDirection(npcefDir, npceuDir);
	               npce.MoveForward(5.0f);
	               if (myDist(npcdpos, npcepos) < 90.0f)
	               {
	                  npce.state = IDLE;
	                  npce.SetCurrentAction(NULL, 0, npce.idleID);
	                  npce.frame = 0;
	               }
	               break;
	            case NPCE:
	               npcefDir[0] = npcepos[0] - npcepos[0];
	               npcefDir[1] = npcepos[1] - npcepos[1];
	               npce.SetDirection(npcefDir, npceuDir);
	               npce.MoveForward(5.0f);
	               if (myDist(npcepos, npcepos) < 90.0f)
	               {
	                  npce.state = IDLE;
	                  npce.SetCurrentAction(NULL, 0, npce.idleID);
	                  npce.frame = 0;
	               }
	               break;
	            case NPCF:
	               npcefDir[0] = npcfpos[0] - npcepos[0];
	               npcefDir[1] = npcfpos[1] - npcepos[1];
	               npce.SetDirection(npcefDir, npceuDir);
	               npce.MoveForward(5.0f);
	               if (myDist(npcfpos, npcepos) < 90.0f)
	               {
	                  npce.state = IDLE;
	                  npce.SetCurrentAction(NULL, 0, npce.idleID);
	                  npce.frame = 0;
	               }
	               break;
	            case NPCG:
	               npcefDir[0] = npcgpos[0] - npcepos[0];
	               npcefDir[1] = npcgpos[1] - npcepos[1];
	               npce.SetDirection(npcefDir, npceuDir);
	               npce.MoveForward(5.0f);
	               if (myDist(npcgpos, npcepos) < 90.0f)
	               {
	                  npce.state = IDLE;
	                  npce.SetCurrentAction(NULL, 0, npce.idleID);
	                  npce.frame = 0;
	               }
	               break;
	            default:
	               break;
	         }
				break;
			case ATTACK1:
			// normal attack 1 33 frames
				npce.Play(ONCE, (float) skip, FALSE, TRUE);
				npce.frame++;
				if (npce.frame == 2)
				{
					hP.Load("Matt.wav");
					hP.Play(ONCE);
				}
				if (npce.frame == 17)
				{
					if (npce.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npcepos, npcapos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npcepos, npcbpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npcepos, npccpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npcepos, npcdpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npcepos, npcepos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npcepos, npcfpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npcepos, npcgpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npcepos, pos, npcefDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npcepos, npcapos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npcepos, npcbpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npcepos, npccpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npcepos, npcdpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npcepos, npcepos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npcepos, npcfpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npcepos, npcgpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
				}
				if (npce.frame == 32)
				{
					npce.state = IDLE;
					npce.SetCurrentAction(NULL, 0, npce.idleID);	
					npce.frame = 0;
				}
				break;
			case ATTACK2:
			// no attack2
	         npce.Play(ONCE, (float) skip, FALSE, TRUE);
	         npce.frame++;
			if (npce.frame == 2)
				{
					hP.Load("Matt1.wav");
					hP.Play(ONCE);
				}
	         if (npce.frame == 17)
	         {
	            if (npce.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npcepos, npcapos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npcepos, npcbpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npcepos, npccpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npcepos, npcdpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npcepos, npcepos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npcepos, npcfpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npcepos, npcgpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npcepos, pos, npcefDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npcepos, npcapos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npcepos, npcbpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npcepos, npccpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npcepos, npcdpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npcepos, npcepos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npcepos, npcfpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npcepos, npcgpos, npcefDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	         }
	         if (npce.frame == 32)
	         {
	            npce.state = IDLE;
	            npce.SetCurrentAction(NULL, 0, npce.idleID); 
	            npce.frame = 0;
	         }
				break;
			case DAMAGE:
			// no damage
				if (npce.frame == 0)
				{
					npce.SetCurrentAction(NULL, 0, npce.idleID);
					hP.Load("Mhurt.wav");
					hP.Play(ONCE);
				}
				npce.Play(LOOP, (float) skip, FALSE, TRUE);
				npce.frame++;
				if (npce.frame == 25)
				{
					npce.state = IDLE;
					npce.SetCurrentAction(NULL, 0, npce.idleID);
					npce.frame = 0;
				}
				break;
			case DIE:
			// die 71
				npce.Play(ONCE, (float) skip, FALSE, TRUE);
				if (!npce.isFriend)
				{
					npce.frame++;
					if (npce.frame == 4)
					{

						hP.Load("MDie.wav");
						hP.Play(ONCE);
					}
					if (npce.frame == 100)
					{
						if (teammateID[0] == NONE)
						{
							teammateID[0] = NPCE;
							npce.state = IDLE;
							npce.SetCurrentAction(NULL, 0, npce.idleID);
							npce.frame = 0;
							npce.blood = npce.fullBlood;
						}
						else if (teammateID[1] == NONE)
						{
							teammateID[1] = NPCE;
							npce.state = IDLE;
							npce.SetCurrentAction(NULL, 0, npce.idleID);
							npce.frame = 0;
							npce.blood = npce.fullBlood;
						}					
						npce.isFriend = true;
					}
				}
				break;
			default:
				break;
		}

		npce.BB();

		/*------------------------------------------
		 ||||    || |||||||||   ||||||||  ||||||||||
		 || ||   || ||      || ||      || ||
		 ||  ||  || |||||||||  ||         ||||||||
		 ||   || || ||         ||      || ||
		 ||    |||| ||          ||||||||  ||
		 ------------------------------------------*/

		if (npcf.blood <= 0 && npcf.state != DIE)
		{
			npcf.state = DIE;
			npcf.SetCurrentAction(NULL, 0, npcf.dieID);
		}

	   if (npcf.state != DIE)
	   {
	      if (npcf.isFriend)
	      {
	         if ((teammateID[0] == NPCF && isFollow[0]) || (teammateID[1] == NPCF && isFollow[1]))
	         {
	            npcf.target = ACTOR;
	         }
	         else
	         {
	            switch(npcf.target)
	            {
	               case NONE:
	                  npcf.target = nearestEnemy(NPCF, npcfpos);
	                  if (npcf.target == NONE)
	                  {
	                     npcf.target = ACTOR;
	                  }
	                  break;
	               case ACTOR:
	                  if (actor.state == DIE)
	                  {
	                     npcf.target = NONE;
	                  }
	                  if ((teammateID[0] == NPCF && !isFollow[0]) || (teammateID[1] == NPCF && !isFollow[1]))
	                  {
	                     npcf.target = nearestEnemy(NPCF, npcfpos);
	                     if (npcf.target == NONE)
	                     {
	                        npcf.target = ACTOR;
	                     }
	                  }
	                  break;
	               case NPCA:
	                  if (npca.state == DIE)
	                  {
	                     npcf.target = NONE;
	                  }
	                  break;
	               case NPCB:
	                  if (npcb.state == DIE)
	                  {
	                     npcf.target = NONE;
	                  }
	                  break;
	               case NPCC:
	                  if (npcc.state == DIE)
	                  {
	                     npcf.target = NONE;
	                  }
	                  break;
	               case NPCD:
	                  if (npcd.state == DIE)
	                  {
	                     npcf.target = NONE;
	                  }
	                  break;
	               case NPCE:
	                  if (npce.state == DIE)
	                  {
	                     npcf.target = NONE;
	                  }
	                  break;
	               case NPCF:
	                  if (npcf.state == DIE)
	                  {
	                     npcf.target = NONE;
	                  }
	                  break;
	               case NPCG:
	                  if (npcg.state == DIE)
	                  {
	                     npcf.target = NONE;
	                  }
	                  break;
	               default:
	                  break;
	            }
	         }
	      }
	      else
	      {
	         switch(npcf.target)
	         {
	            case NONE:
	               npcf.target = nearestFriend(NPCF, npcfpos);
	               break;
	            case ACTOR:
	               if (actor.state == DIE)
	               {
	                  npcf.target = NONE;
	               }
	               break;
	            case NPCA:
	               if (npca.state == DIE)
	               {
	                  npcf.target = NONE;
	               }
	               break;
	            case NPCB:
	               if (npcb.state == DIE)
	               {
	                  npcf.target = NONE;
	               }
	               break;
	            case NPCC:
	               if (npcc.state == DIE)
	               {
	                  npcf.target = NONE;
	               }
	               break;
	            case NPCD:
	               if (npcd.state == DIE)
	               {
	                  npcf.target = NONE;
	               }
	               break;
	            case NPCE:
	               if (npce.state == DIE)
	               {
	                  npcf.target = NONE;
	               }
	               break;
	            case NPCF:
	               if (npcf.state == DIE)
	               {
	                  npcf.target = NONE;
	               }
	               break;
	            case NPCG:
	               if (npcg.state == DIE)
	               {
	                  npcf.target = NONE;
	               }
	               break;
	            default:
	               break;
	         }
	      }
	   }

		/*
			npcf.state:
				0 idle
				1 run
				2 attack1
				3 attack2
				4 damage
				5 die
		*/
		switch(npcf.state)
		{
			case IDLE:
			// combat idle
				npcf.Play(LOOP, (float) skip, FALSE, TRUE);
				switch (npcf.target)
	         {
	            case NONE:
	               break;
	            case ACTOR:
	               if (npcf.isFriend)
	               {
	                  if (myDist(pos, npcfpos) >= 100.0f)
	                  {
	                     npcf.state = RUN;
	                     npcf.SetCurrentAction(NULL, 0, npcf.runID);
	                     npcf.frame = 0;
	                  }
	               }
	               else
	               {
	                  if (myDist(pos, npcfpos) <= 500.0f && myDist(pos, npcfpos) >= 100.0f)
	                  {  
	                     npcf.state = RUN;
	                     npcf.SetCurrentAction(NULL, 0, npcf.runID);
	                     npcf.frame = 0;
	                  }
	                  else if (myDist(pos, npcfpos) < 100.0f  && actor.state != DIE)
	                  {
	                     npcf.wait1++;
	                     npcffDir[0] = pos[0] - npcfpos[0];
	                     npcffDir[1] = pos[1] - npcfpos[1];
	                     npcf.SetDirection(npcffDir, npcfuDir);
	                     if (npcf.wait1 % 50 == 0)
	                     {
	                        npcf.wait2++;
	                        if (npcf.wait2 % 2 == 0)
	                        {
	                           npcf.state = ATTACK2;
	                           npcf.SetCurrentAction(NULL, 0, npcf.attack2ID);
	                           npcf.frame = 0;
	                        }
	                        else
	                        {
	                           npcf.state = ATTACK1;
	                           npcf.SetCurrentAction(NULL, 0, npcf.attack1ID);
	                           npcf.frame = 0;
	                        }
	                     }
	                  }
	               }
	               break;
	            case NPCA:
	               if (myDist(npcfpos, npcapos) <= 600.0f && myDist(npcfpos, npcapos) >= 100.0f)
	               {
	                  npcf.state = RUN;
	                  npcf.SetCurrentAction(NULL, 0, npcf.runID);
	                  npcf.frame = 0;
	               }
	               else if (myDist(npcfpos, npcapos) < 100.0f)
	               {
	                  npcf.wait1++;
	                  npcffDir[0] = npcapos[0] - npcfpos[0];
	                  npcffDir[1] = npcapos[1] - npcfpos[1];
	                  npcf.SetDirection(npcffDir, npcfuDir);
	                  if (npcf.wait1 % 50 == 0)
	                  {
	                     npcf.wait1++;
	                     if (npcf.wait2 % 2 == 0)
	                     {
	                        npcf.state = ATTACK2;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack2ID);
	                        npcf.frame = 0;
	                     }
	                     else
	                     {
	                        npcf.state = ATTACK1;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack1ID);
	                        npcf.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCB:
	               if (myDist(npcfpos, npcbpos) <= 600.0f && myDist(npcfpos, npcbpos) >= 100.0f)
	               {
	                  npcf.state = RUN;
	                  npcf.SetCurrentAction(NULL, 0, npcf.runID);
	                  npcf.frame = 0;
	               }
	               else if (myDist(npcfpos, npcbpos) < 100.0f)
	               {
	                  npcf.wait1++;
	                  npcffDir[0] = npcbpos[0] - npcfpos[0];
	                  npcffDir[1] = npcbpos[1] - npcfpos[1];
	                  npcf.SetDirection(npcffDir, npcfuDir);
	                  if (npcf.wait1 % 50 == 0)
	                  {
	                     npcf.wait1++;
	                     if (npcf.wait2 % 2 == 0)
	                     {
	                        npcf.state = ATTACK2;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack2ID);
	                        npcf.frame = 0;
	                     }
	                     else
	                     {
	                        npcf.state = ATTACK1;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack1ID);
	                        npcf.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCC:
	               if (myDist(npcfpos, npccpos) <= 600.0f && myDist(npcfpos, npccpos) >= 100.0f)
	               {
	                  npcf.state = RUN;
	                  npcf.SetCurrentAction(NULL, 0, npcf.runID);
	                  npcf.frame = 0;
	               }
	               else if (myDist(npcfpos, npccpos) < 100.0f)
	               {
	                  npcf.wait1++;
	                  npcffDir[0] = npccpos[0] - npcfpos[0];
	                  npcffDir[1] = npccpos[1] - npcfpos[1];
	                  npcf.SetDirection(npcffDir, npcfuDir);
	                  if (npcf.wait1 % 50 == 0)
	                  {
	                     npcf.wait1++;
	                     if (npcf.wait2 % 2 == 0)
	                     {
	                        npcf.state = ATTACK2;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack2ID);
	                        npcf.frame = 0;
	                     }
	                     else
	                     {
	                        npcf.state = ATTACK1;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack1ID);
	                        npcf.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCD:
	               if (myDist(npcfpos, npcdpos) <= 600.0f && myDist(npcfpos, npcdpos) >= 100.0f)
	               {
	                  npcf.state = RUN;
	                  npcf.SetCurrentAction(NULL, 0, npcf.runID);
	                  npcf.frame = 0;
	               }
	               else if (myDist(npcfpos, npcdpos) < 100.0f)
	               {
	                  npcf.wait1++;
	                  npcffDir[0] = npcdpos[0] - npcfpos[0];
	                  npcffDir[1] = npcdpos[1] - npcfpos[1];
	                  npcf.SetDirection(npcffDir, npcfuDir);
	                  if (npcf.wait1 % 50 == 0)
	                  {
	                     npcf.wait1++;
	                     if (npcf.wait2 % 2 == 0)
	                     {
	                        npcf.state = ATTACK2;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack2ID);
	                        npcf.frame = 0;
	                     }
	                     else
	                     {
	                        npcf.state = ATTACK1;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack1ID);
	                        npcf.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCE:
	               if (myDist(npcfpos, npcepos) <= 600.0f && myDist(npcfpos, npcepos) >= 100.0f)
	               {
	                  npcf.state = RUN;
	                  npcf.SetCurrentAction(NULL, 0, npcf.runID);
	                  npcf.frame = 0;
	               }
	               else if (myDist(npcfpos, npcepos) < 100.0f)
	               {
	                  npcf.wait1++;
	                  npcffDir[0] = npcepos[0] - npcfpos[0];
	                  npcffDir[1] = npcepos[1] - npcfpos[1];
	                  npcf.SetDirection(npcffDir, npcfuDir);
	                  if (npcf.wait1 % 50 == 0)
	                  {
	                     npcf.wait1++;
	                     if (npcf.wait2 % 2 == 0)
	                     {
	                        npcf.state = ATTACK2;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack2ID);
	                        npcf.frame = 0;
	                     }
	                     else
	                     {
	                        npcf.state = ATTACK1;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack1ID);
	                        npcf.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCF:
	               if (myDist(npcfpos, npcfpos) <= 600.0f && myDist(npcfpos, npcfpos) >= 100.0f)
	               {
	                  npcf.state = RUN;
	                  npcf.SetCurrentAction(NULL, 0, npcf.runID);
	                  npcf.frame = 0;
	               }
	               else if (myDist(npcfpos, npcfpos) < 100.0f)
	               {
	                  npcf.wait1++;
	                  npcffDir[0] = npcfpos[0] - npcfpos[0];
	                  npcffDir[1] = npcfpos[1] - npcfpos[1];
	                  npcf.SetDirection(npcffDir, npcfuDir);
	                  if (npcf.wait1 % 50 == 0)
	                  {
	                     npcf.wait1++;
	                     if (npcf.wait2 % 2 == 0)
	                     {
	                        npcf.state = ATTACK2;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack2ID);
	                        npcf.frame = 0;
	                     }
	                     else
	                     {
	                        npcf.state = ATTACK1;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack1ID);
	                        npcf.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCG:
	               if (myDist(npcfpos, npcgpos) <= 600.0f && myDist(npcfpos, npcgpos) >= 100.0f)
	               {
	                  npcf.state = RUN;
	                  npcf.SetCurrentAction(NULL, 0, npcf.runID);
	                  npcf.frame = 0;
	               }
	               else if (myDist(npcfpos, npcgpos) < 100.0f)
	               {
	                  npcf.wait1++;
	                  npcffDir[0] = npcgpos[0] - npcfpos[0];
	                  npcffDir[1] = npcgpos[1] - npcfpos[1];
	                  npcf.SetDirection(npcffDir, npcfuDir);
	                  if (npcf.wait1 % 50 == 0)
	                  {
	                     npcf.wait1++;
	                     if (npcf.wait2 % 2 == 0)
	                     {
	                        npcf.state = ATTACK2;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack2ID);
	                        npcf.frame = 0;
	                     }
	                     else
	                     {
	                        npcf.state = ATTACK1;
	                        npcf.SetCurrentAction(NULL, 0, npcf.attack1ID);
	                        npcf.frame = 0;
	                     }
	                  }
	               }
	               break;
	            default:
	               break;
	         }
				break;
			case RUN:
			// run
				npcf.Play(LOOP, (float) skip, FALSE, TRUE);
				switch(npcf.target)
	         {
	            case NONE:
	               npcf.state = IDLE;
	               npcf.SetCurrentAction(NULL, 0, npcf.idleID);
	               npcf.frame = 0;
	               break;
	            case ACTOR:
	               npcffDir[0] = pos[0] - npcfpos[0];
	               npcffDir[1] = pos[1] - npcfpos[1];
	               npcf.SetDirection(npcffDir, npcfuDir);
	               npcf.MoveForward(5.0f);
	               if (myDist(pos, npcfpos) < 90.0f)
	               {
	                  npcf.state = IDLE;
	                  npcf.SetCurrentAction(NULL, 0, npcf.idleID);
	                  npcf.frame = 0;
	               }
	               break;
	            case NPCA:
	               npcffDir[0] = npcapos[0] - npcfpos[0];
	               npcffDir[1] = npcapos[1] - npcfpos[1];
	               npcf.SetDirection(npcffDir, npcfuDir);
	               npcf.MoveForward(5.0f);
	               if (myDist(npcapos, npcfpos) < 90.0f)
	               {
	                  npcf.state = IDLE;
	                  npcf.SetCurrentAction(NULL, 0, npcf.idleID);
	                  npcf.frame = 0;
	               }
	               break;
	            case NPCB:
	               npcffDir[0] = npcbpos[0] - npcfpos[0];
	               npcffDir[1] = npcbpos[1] - npcfpos[1];
	               npcf.SetDirection(npcffDir, npcfuDir);
	               npcf.MoveForward(5.0f);
	               if (myDist(npcbpos, npcfpos) < 90.0f)
	               {
	                  npcf.state = IDLE;
	                  npcf.SetCurrentAction(NULL, 0, npcf.idleID);
	                  npcf.frame = 0;
	               }
	               break;
	            case NPCC:
	               npcffDir[0] = npccpos[0] - npcfpos[0];
	               npcffDir[1] = npccpos[1] - npcfpos[1];
	               npcf.SetDirection(npcffDir, npcfuDir);
	               npcf.MoveForward(5.0f);
	               if (myDist(npccpos, npcfpos) < 90.0f)
	               {
	                  npcf.state = IDLE;
	                  npcf.SetCurrentAction(NULL, 0, npcf.idleID);
	                  npcf.frame = 0;
	               }
	               break;
	            case NPCD:
	               npcffDir[0] = npcdpos[0] - npcfpos[0];
	               npcffDir[1] = npcdpos[1] - npcfpos[1];
	               npcf.SetDirection(npcffDir, npcfuDir);
	               npcf.MoveForward(5.0f);
	               if (myDist(npcdpos, npcfpos) < 90.0f)
	               {
	                  npcf.state = IDLE;
	                  npcf.SetCurrentAction(NULL, 0, npcf.idleID);
	                  npcf.frame = 0;
	               }
	               break;
	            case NPCE:
	               npcffDir[0] = npcepos[0] - npcfpos[0];
	               npcffDir[1] = npcepos[1] - npcfpos[1];
	               npcf.SetDirection(npcffDir, npcfuDir);
	               npcf.MoveForward(5.0f);
	               if (myDist(npcepos, npcfpos) < 90.0f)
	               {
	                  npcf.state = IDLE;
	                  npcf.SetCurrentAction(NULL, 0, npcf.idleID);
	                  npcf.frame = 0;
	               }
	               break;
	            case NPCF:
	               npcffDir[0] = npcfpos[0] - npcfpos[0];
	               npcffDir[1] = npcfpos[1] - npcfpos[1];
	               npcf.SetDirection(npcffDir, npcfuDir);
	               npcf.MoveForward(5.0f);
	               if (myDist(npcfpos, npcfpos) < 90.0f)
	               {
	                  npcf.state = IDLE;
	                  npcf.SetCurrentAction(NULL, 0, npcf.idleID);
	                  npcf.frame = 0;
	               }
	               break;
	            case NPCG:
	               npcffDir[0] = npcgpos[0] - npcfpos[0];
	               npcffDir[1] = npcgpos[1] - npcfpos[1];
	               npcf.SetDirection(npcffDir, npcfuDir);
	               npcf.MoveForward(5.0f);
	               if (myDist(npcgpos, npcfpos) < 90.0f)
	               {
	                  npcf.state = IDLE;
	                  npcf.SetCurrentAction(NULL, 0, npcf.idleID);
	                  npcf.frame = 0;
	               }
	               break;
	            default:
	               break;
	         }
				break;
			case ATTACK1:
			// normal attack 1 35 frames
				npcf.Play(ONCE, (float) skip, FALSE, TRUE);
				npcf.frame++;
				if (npcf.frame == 3)
				{
					hP.Load("Gatt.wav");
					hP.Play(ONCE);
				}
				if (npcf.frame == 17)
				{
					if (npcf.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npcfpos, npcapos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npcfpos, npcbpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npcfpos, npccpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npcfpos, npcdpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npcfpos, npcepos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npcfpos, npcfpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npcfpos, npcgpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npcfpos, pos, npcffDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npcfpos, npcapos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npcfpos, npcbpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npcfpos, npccpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npcfpos, npcdpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npcfpos, npcepos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npcfpos, npcfpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npcfpos, npcgpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
				}
				if (npcf.frame == 34)
				{
					npcf.state = IDLE;
					npcf.SetCurrentAction(NULL, 0, npcf.idleID);	
					npcf.frame = 0;
				}
				break;
			case ATTACK2:
			// normal attack 2 39 frames
	         npcf.Play(ONCE, (float) skip, FALSE, TRUE);
	         npcf.frame++;
			 if (npcf.frame == 3)
				{
					hP.Load("Gatt2.wav");
					hP.Play(ONCE);
				}
	         if (npcf.frame == 17)
	         {
	            if (npcf.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npcfpos, npcapos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npcfpos, npcbpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npcfpos, npccpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npcfpos, npcdpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npcfpos, npcepos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npcfpos, npcfpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npcfpos, npcgpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npcfpos, pos, npcffDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npcfpos, npcapos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npcfpos, npcbpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npcfpos, npccpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npcfpos, npcdpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npcfpos, npcepos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npcfpos, npcfpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npcfpos, npcgpos, npcffDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	         }
	         if (npcf.frame == 38)
	         {
	            npcf.state = IDLE;
	            npcf.SetCurrentAction(NULL, 0, npcf.idleID); 
	            npcf.frame = 0;
	         }
				break;
			case DAMAGE:
			// no damage
				if (npcf.frame == 0)
				{	
					npcf.SetCurrentAction(NULL, 0, npcf.idleID);
					hP.Load("Ghurt.wav");
					hP.Play(ONCE);
				}
				npcf.Play(LOOP, (float) skip, FALSE, TRUE);
				npcf.frame++;

				if (npcf.frame == 25)
				{
					npcf.state = IDLE;
					npcf.SetCurrentAction(NULL, 0, npcf.idleID);
					npcf.frame = 0;
				}
				break;
			case DIE:
			// die 58
				npcf.Play(ONCE, (float) skip, FALSE, TRUE);
				if (!npcf.isFriend)
				{
					if (npcf.frame == 4){
						hP.Load("Gdie.wav");
						hP.Play(ONCE);
					}
					npcf.frame++;
					if (npcf.frame == 87)
					{
						if (teammateID[0] == NONE)
						{
							teammateID[0] = NPCF;
							npcf.state = IDLE;
							npcf.SetCurrentAction(NULL, 0, npcf.idleID);
							npcf.frame = 0;
							npcf.blood = npcf.fullBlood;
						}
						else if (teammateID[1] == NONE)
						{	
							npcf.state = IDLE;
							npcf.SetCurrentAction(NULL, 0, npcf.idleID);
							npcf.frame = 0;
							npcf.blood = npcf.fullBlood;
							teammateID[1] = NPCF;
						}
						npcf.isFriend = true;
					}
				}
				break;
			default:
				break;
		}

		npcf.BB();

		/*------------------------------------------
		 ||||    || |||||||||   ||||||||   ||||||||
		 || ||   || ||      || ||      || ||
		 ||  ||  || |||||||||  ||         ||    |||
		 ||   || || ||         ||      || ||      ||
		 ||    |||| ||          ||||||||   ||||||||
		 ------------------------------------------*/

		if (npcg.blood <= 0 && npcg.state != DIE)
		{
			npcg.state = DIE;
			npcg.SetCurrentAction(NULL, 0, npcg.dieID);
		}

	   if (npcg.state != DIE)
	   {
	      if (npcg.isFriend)
	      {
	         if ((teammateID[0] == NPCG && isFollow[0]) || (teammateID[1] == NPCG && isFollow[1]))
	         {
	            npcg.target = ACTOR;
	         }
	         else
	         {
	            switch(npcg.target)
	            {
	               case NONE:
	                  npcg.target = nearestEnemy(NPCG, npcgpos);
	                  if (npcg.target == NONE)
	                  {
	                     npcg.target = ACTOR;
	                  }
	                  break;
	               case ACTOR:
	                  if (actor.state == DIE)
	                  {
	                     npcg.target = NONE;
	                  }
	                  if ((teammateID[0] == NPCG && !isFollow[0]) || (teammateID[1] == NPCG && !isFollow[1]))
	                  {
	                     npcg.target = nearestEnemy(NPCG, npcgpos);
	                     if (npcg.target == NONE)
	                     {
	                        npcg.target = ACTOR;
	                     }
	                  }
	                  break;
	               case NPCA:
	                  if (npca.state == DIE)
	                  {
	                     npcg.target = NONE;
	                  }
	                  break;
	               case NPCB:
	                  if (npcb.state == DIE)
	                  {
	                     npcg.target = NONE;
	                  }
	                  break;
	               case NPCC:
	                  if (npcc.state == DIE)
	                  {
	                     npcg.target = NONE;
	                  }
	                  break;
	               case NPCD:
	                  if (npcd.state == DIE)
	                  {
	                     npcg.target = NONE;
	                  }
	                  break;
	               case NPCE:
	                  if (npce.state == DIE)
	                  {
	                     npcg.target = NONE;
	                  }
	                  break;
	               case NPCF:
	                  if (npcf.state == DIE)
	                  {
	                     npcg.target = NONE;
	                  }
	                  break;
	               case NPCG:
	                  if (npcg.state == DIE)
	                  {
	                     npcg.target = NONE;
	                  }
	                  break;
	               default:
	                  break;
	            }
	         }
	      }
	      else
	      {
	         switch(npcg.target)
	         {
	            case NONE:
	               npcg.target = nearestFriend(NPCG, npcgpos);
	               break;
	            case ACTOR:
	               if (actor.state == DIE)
	               {
	                  npcg.target = NONE;
	               }
	               break;
	            case NPCA:
	               if (npca.state == DIE)
	               {
	                  npcg.target = NONE;
	               }
	               break;
	            case NPCB:
	               if (npcb.state == DIE)
	               {
	                  npcg.target = NONE;
	               }
	               break;
	            case NPCC:
	               if (npcc.state == DIE)
	               {
	                  npcg.target = NONE;
	               }
	               break;
	            case NPCD:
	               if (npcd.state == DIE)
	               {
	                  npcg.target = NONE;
	               }
	               break;
	            case NPCE:
	               if (npce.state == DIE)
	               {
	                  npcg.target = NONE;
	               }
	               break;
	            case NPCF:
	               if (npcf.state == DIE)
	               {
	                  npcg.target = NONE;
	               }
	               break;
	            case NPCG:
	               if (npcg.state == DIE)
	               {
	                  npcg.target = NONE;
	               }
	               break;
	            default:
	               break;
	         }
	      }
	   }

		/*
			npcg.state:
				0 idle
				1 run
				2 attack1
				3 attack2
				4 damage
				5 die
		*/
		switch(npcg.state)
		{
			case IDLE:
			// combat idle
				npcg.Play(LOOP, (float) skip, FALSE, TRUE);
				switch (npcg.target)
	         {
	            case NONE:
	               break;
	            case ACTOR:
	               if (npcg.isFriend)
	               {
	                  if (myDist(pos, npcgpos) >= 100.0f)
	                  {
	                     npcg.state = RUN;
	                     npcg.SetCurrentAction(NULL, 0, npcg.runID);
	                     npcg.frame = 0;
	                  }
	               }
	               else
	               {
	                  if (myDist(pos, npcgpos) <= 500.0f && myDist(pos, npcgpos) >= 100.0f)
	                  {  
	                     npcg.state = RUN;
	                     npcg.SetCurrentAction(NULL, 0, npcg.runID);
	                     npcg.frame = 0;
	                  }
	                  else if (myDist(pos, npcgpos) < 100.0f  && actor.state != DIE)
	                  {
	                     npcg.wait1++;
	                     npcgfDir[0] = pos[0] - npcgpos[0];
	                     npcgfDir[1] = pos[1] - npcgpos[1];
	                     npcg.SetDirection(npcgfDir, npcguDir);
	                     if (npcg.wait1 % 50 == 0)
	                     {
	                        npcg.wait2++;
	                        if (npcg.wait2 % 2 == 0)
	                        {
	                           npcg.state = ATTACK2;
	                           npcg.SetCurrentAction(NULL, 0, npcg.attack2ID);
	                           npcg.frame = 0;
	                        }
	                        else
	                        {
	                           npcg.state = ATTACK1;
	                           npcg.SetCurrentAction(NULL, 0, npcg.attack1ID);
	                           npcg.frame = 0;
	                        }
	                     }
	                  }
	               }
	               break;
	            case NPCA:
	               if (myDist(npcgpos, npcapos) <= 600.0f && myDist(npcgpos, npcapos) >= 100.0f)
	               {
	                  npcg.state = RUN;
	                  npcg.SetCurrentAction(NULL, 0, npcg.runID);
	                  npcg.frame = 0;
	               }
	               else if (myDist(npcgpos, npcapos) < 100.0f)
	               {
	                  npcg.wait1++;
	                  npcgfDir[0] = npcapos[0] - npcgpos[0];
	                  npcgfDir[1] = npcapos[1] - npcgpos[1];
	                  npcg.SetDirection(npcgfDir, npcguDir);
	                  if (npcg.wait1 % 50 == 0)
	                  {
	                     npcg.wait1++;
	                     if (npcg.wait2 % 2 == 0)
	                     {
	                        npcg.state = ATTACK2;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack2ID);
	                        npcg.frame = 0;
	                     }
	                     else
	                     {
	                        npcg.state = ATTACK1;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack1ID);
	                        npcg.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCB:
	               if (myDist(npcgpos, npcbpos) <= 600.0f && myDist(npcgpos, npcbpos) >= 100.0f)
	               {
	                  npcg.state = RUN;
	                  npcg.SetCurrentAction(NULL, 0, npcg.runID);
	                  npcg.frame = 0;
	               }
	               else if (myDist(npcgpos, npcbpos) < 100.0f)
	               {
	                  npcg.wait1++;
	                  npcgfDir[0] = npcbpos[0] - npcgpos[0];
	                  npcgfDir[1] = npcbpos[1] - npcgpos[1];
	                  npcg.SetDirection(npcgfDir, npcguDir);
	                  if (npcg.wait1 % 50 == 0)
	                  {
	                     npcg.wait1++;
	                     if (npcg.wait2 % 2 == 0)
	                     {
	                        npcg.state = ATTACK2;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack2ID);
	                        npcg.frame = 0;
	                     }
	                     else
	                     {
	                        npcg.state = ATTACK1;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack1ID);
	                        npcg.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCC:
	               if (myDist(npcgpos, npccpos) <= 600.0f && myDist(npcgpos, npccpos) >= 100.0f)
	               {
	                  npcg.state = RUN;
	                  npcg.SetCurrentAction(NULL, 0, npcg.runID);
	                  npcg.frame = 0;
	               }
	               else if (myDist(npcgpos, npccpos) < 100.0f)
	               {
	                  npcg.wait1++;
	                  npcgfDir[0] = npccpos[0] - npcgpos[0];
	                  npcgfDir[1] = npccpos[1] - npcgpos[1];
	                  npcg.SetDirection(npcgfDir, npcguDir);
	                  if (npcg.wait1 % 50 == 0)
	                  {
	                     npcg.wait1++;
	                     if (npcg.wait2 % 2 == 0)
	                     {
	                        npcg.state = ATTACK2;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack2ID);
	                        npcg.frame = 0;
	                     }
	                     else
	                     {
	                        npcg.state = ATTACK1;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack1ID);
	                        npcg.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCD:
	               if (myDist(npcgpos, npcdpos) <= 600.0f && myDist(npcgpos, npcdpos) >= 100.0f)
	               {
	                  npcg.state = RUN;
	                  npcg.SetCurrentAction(NULL, 0, npcg.runID);
	                  npcg.frame = 0;
	               }
	               else if (myDist(npcgpos, npcdpos) < 100.0f)
	               {
	                  npcg.wait1++;
	                  npcgfDir[0] = npcdpos[0] - npcgpos[0];
	                  npcgfDir[1] = npcdpos[1] - npcgpos[1];
	                  npcg.SetDirection(npcgfDir, npcguDir);
	                  if (npcg.wait1 % 50 == 0)
	                  {
	                     npcg.wait1++;
	                     if (npcg.wait2 % 2 == 0)
	                     {
	                        npcg.state = ATTACK2;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack2ID);
	                        npcg.frame = 0;
	                     }
	                     else
	                     {
	                        npcg.state = ATTACK1;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack1ID);
	                        npcg.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCE:
	               if (myDist(npcgpos, npcepos) <= 600.0f && myDist(npcgpos, npcepos) >= 100.0f)
	               {
	                  npcg.state = RUN;
	                  npcg.SetCurrentAction(NULL, 0, npcg.runID);
	                  npcg.frame = 0;
	               }
	               else if (myDist(npcgpos, npcepos) < 100.0f)
	               {
	                  npcg.wait1++;
	                  npcgfDir[0] = npcepos[0] - npcgpos[0];
	                  npcgfDir[1] = npcepos[1] - npcgpos[1];
	                  npcg.SetDirection(npcgfDir, npcguDir);
	                  if (npcg.wait1 % 50 == 0)
	                  {
	                     npcg.wait1++;
	                     if (npcg.wait2 % 2 == 0)
	                     {
	                        npcg.state = ATTACK2;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack2ID);
	                        npcg.frame = 0;
	                     }
	                     else
	                     {
	                        npcg.state = ATTACK1;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack1ID);
	                        npcg.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCF:
	               if (myDist(npcgpos, npcfpos) <= 600.0f && myDist(npcgpos, npcfpos) >= 100.0f)
	               {
	                  npcg.state = RUN;
	                  npcg.SetCurrentAction(NULL, 0, npcg.runID);
	                  npcg.frame = 0;
	               }
	               else if (myDist(npcgpos, npcfpos) < 100.0f)
	               {
	                  npcg.wait1++;
	                  npcgfDir[0] = npcfpos[0] - npcgpos[0];
	                  npcgfDir[1] = npcfpos[1] - npcgpos[1];
	                  npcg.SetDirection(npcgfDir, npcguDir);
	                  if (npcg.wait1 % 50 == 0)
	                  {
	                     npcg.wait1++;
	                     if (npcg.wait2 % 2 == 0)
	                     {
	                        npcg.state = ATTACK2;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack2ID);
	                        npcg.frame = 0;
	                     }
	                     else
	                     {
	                        npcg.state = ATTACK1;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack1ID);
	                        npcg.frame = 0;
	                     }
	                  }
	               }
	               break;
	            case NPCG:
	               if (myDist(npcgpos, npcgpos) <= 600.0f && myDist(npcgpos, npcgpos) >= 100.0f)
	               {
	                  npcg.state = RUN;
	                  npcg.SetCurrentAction(NULL, 0, npcg.runID);
	                  npcg.frame = 0;
	               }
	               else if (myDist(npcgpos, npcgpos) < 100.0f)
	               {
	                  npcg.wait1++;
	                  npcgfDir[0] = npcgpos[0] - npcgpos[0];
	                  npcgfDir[1] = npcgpos[1] - npcgpos[1];
	                  npcg.SetDirection(npcgfDir, npcguDir);
	                  if (npcg.wait1 % 50 == 0)
	                  {
	                     npcg.wait1++;
	                     if (npcg.wait2 % 2 == 0)
	                     {
	                        npcg.state = ATTACK2;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack2ID);
	                        npcg.frame = 0;
	                     }
	                     else
	                     {
	                        npcg.state = ATTACK1;
	                        npcg.SetCurrentAction(NULL, 0, npcg.attack1ID);
	                        npcg.frame = 0;
	                     }
	                  }
	               }
	               break;
	            default:
	               break;
	         }
				break;
			case RUN:
			// run
				npcg.Play(LOOP, (float) skip, FALSE, TRUE);
				switch(npcg.target)
	         {
	            case NONE:
	               npcg.state = IDLE;
	               npcg.SetCurrentAction(NULL, 0, npcg.idleID);
	               npcg.frame = 0;
	               break;
	            case ACTOR:
	               npcgfDir[0] = pos[0] - npcgpos[0];
	               npcgfDir[1] = pos[1] - npcgpos[1];
	               npcg.SetDirection(npcgfDir, npcguDir);
	               npcg.MoveForward(5.0f);
	               if (myDist(pos, npcgpos) < 90.0f)
	               {
	                  npcg.state = IDLE;
	                  npcg.SetCurrentAction(NULL, 0, npcg.idleID);
	                  npcg.frame = 0;
	               }
	               break;
	            case NPCA:
	               npcgfDir[0] = npcapos[0] - npcgpos[0];
	               npcgfDir[1] = npcapos[1] - npcgpos[1];
	               npcg.SetDirection(npcgfDir, npcguDir);
	               npcg.MoveForward(5.0f);
	               if (myDist(npcapos, npcgpos) < 90.0f)
	               {
	                  npcg.state = IDLE;
	                  npcg.SetCurrentAction(NULL, 0, npcg.idleID);
	                  npcg.frame = 0;
	               }
	               break;
	            case NPCB:
	               npcgfDir[0] = npcbpos[0] - npcgpos[0];
	               npcgfDir[1] = npcbpos[1] - npcgpos[1];
	               npcg.SetDirection(npcgfDir, npcguDir);
	               npcg.MoveForward(5.0f);
	               if (myDist(npcbpos, npcgpos) < 90.0f)
	               {
	                  npcg.state = IDLE;
	                  npcg.SetCurrentAction(NULL, 0, npcg.idleID);
	                  npcg.frame = 0;
	               }
	               break;
	            case NPCC:
	               npcgfDir[0] = npccpos[0] - npcgpos[0];
	               npcgfDir[1] = npccpos[1] - npcgpos[1];
	               npcg.SetDirection(npcgfDir, npcguDir);
	               npcg.MoveForward(5.0f);
	               if (myDist(npccpos, npcgpos) < 90.0f)
	               {
	                  npcg.state = IDLE;
	                  npcg.SetCurrentAction(NULL, 0, npcg.idleID);
	                  npcg.frame = 0;
	               }
	               break;
	            case NPCD:
	               npcgfDir[0] = npcdpos[0] - npcgpos[0];
	               npcgfDir[1] = npcdpos[1] - npcgpos[1];
	               npcg.SetDirection(npcgfDir, npcguDir);
	               npcg.MoveForward(5.0f);
	               if (myDist(npcdpos, npcgpos) < 90.0f)
	               {
	                  npcg.state = IDLE;
	                  npcg.SetCurrentAction(NULL, 0, npcg.idleID);
	                  npcg.frame = 0;
	               }
	               break;
	            case NPCE:
	               npcgfDir[0] = npcepos[0] - npcgpos[0];
	               npcgfDir[1] = npcepos[1] - npcgpos[1];
	               npcg.SetDirection(npcgfDir, npcguDir);
	               npcg.MoveForward(5.0f);
	               if (myDist(npcepos, npcgpos) < 90.0f)
	               {
	                  npcg.state = IDLE;
	                  npcg.SetCurrentAction(NULL, 0, npcg.idleID);
	                  npcg.frame = 0;
	               }
	               break;
	            case NPCF:
	               npcgfDir[0] = npcfpos[0] - npcgpos[0];
	               npcgfDir[1] = npcfpos[1] - npcgpos[1];
	               npcg.SetDirection(npcgfDir, npcguDir);
	               npcg.MoveForward(5.0f);
	               if (myDist(npcfpos, npcgpos) < 90.0f)
	               {
	                  npcg.state = IDLE;
	                  npcg.SetCurrentAction(NULL, 0, npcg.idleID);
	                  npcg.frame = 0;
	               }
	               break;
	            case NPCG:
	               npcgfDir[0] = npcgpos[0] - npcgpos[0];
	               npcgfDir[1] = npcgpos[1] - npcgpos[1];
	               npcg.SetDirection(npcgfDir, npcguDir);
	               npcg.MoveForward(5.0f);
	               if (myDist(npcgpos, npcgpos) < 90.0f)
	               {
	                  npcg.state = IDLE;
	                  npcg.SetCurrentAction(NULL, 0, npcg.idleID);
	                  npcg.frame = 0;
	               }
	               break;
	            default:
	               break;
	         }
				break;
			case ATTACK1:
			// normal attack 1 31 frames
				npcg.Play(ONCE, (float) skip, FALSE, TRUE);
				npcg.frame++;
				if (npcg.frame == 1)
				{
					hP.Load("Gatt3.wav");
					hP.Play(ONCE);
				}
				if (npcg.frame == 17)
				{
					if (npcg.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npcgpos, npcapos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npcgpos, npcbpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npcgpos, npccpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npcgpos, npcdpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npcgpos, npcepos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npcgpos, npcfpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npcgpos, npcgpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npcgpos, pos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npcgpos, npcapos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npcgpos, npcbpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npcgpos, npccpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npcgpos, npcdpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npcgpos, npcepos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npcgpos, npcfpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npcgpos, npcgpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
				}
				if (npcg.frame == 30)
				{
					npcg.state = IDLE;
					npcg.SetCurrentAction(NULL, 0, npcg.idleID);	
					npcg.frame = 0;
				}
				break;
			case ATTACK2:
			// normal attack 2 48 frames
	         npcg.Play(ONCE, (float) skip, FALSE, TRUE);
	         npcg.frame++;
			 if (npcg.frame == 1)
				{
					hP.Load("Gatt2.wav");
					hP.Play(ONCE);
				}
	         if (npcg.frame == 17)
	         {
	            if (npcg.isFriend)
	            {
	               if (npca.state != DIE && !npca.isFriend)
	               {
	                  if(isHit(npcgpos, npcapos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && !npcb.isFriend)
	               {
	                  if(isHit(npcgpos, npcbpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && !npcc.isFriend)
	               {
	                  if(isHit(npcgpos, npccpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && !npcd.isFriend)
	               {
	                  if(isHit(npcgpos, npcdpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && !npce.isFriend)
	               {
	                  if(isHit(npcgpos, npcepos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && !npcf.isFriend)
	               {
	                  if(isHit(npcgpos, npcfpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && !npcg.isFriend)
	               {
	                  if(isHit(npcgpos, npcgpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	            else
	            {
	               if (actor.state != DIE)
	               {
	                  if(isHit(npcgpos, pos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     actor.blood -= 30;
	                     actor.state = DAMAGE;
	                     actor.SetCurrentAction(NULL, 0, heavyDamagedID);
	                     actor.frame = 0;
	                     attackKeyLocked = true;
	                     movementKeyLocked = true;
	                  }     
	               }
	               if (npca.state != DIE && npca.isFriend)
	               {
	                  if(isHit(npcgpos, npcapos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npca.blood -= 30;
	                     npca.state = DAMAGE;
	                     npca.SetCurrentAction(NULL, 0, npca.damageID);
	                     npca.frame = 0;
	                  }
	               }
	               if (npcb.state != DIE && npcb.isFriend)
	               {
	                  if(isHit(npcgpos, npcbpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcb.blood -= 30;
	                     npcb.state = DAMAGE;
	                     npcb.SetCurrentAction(NULL, 0, npcb.damageID);
	                     npcb.frame = 0;
	                  }
	               }
	               if (npcc.state != DIE && npcc.isFriend)
	               {
	                  if(isHit(npcgpos, npccpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcc.blood -= 30;
	                     npcc.state = DAMAGE;
	                     npcc.SetCurrentAction(NULL, 0, npcc.damageID);
	                     npcc.frame = 0;
	                  }
	               }
	               if (npcd.state != DIE && npcd.isFriend)
	               {
	                  if(isHit(npcgpos, npcdpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcd.blood -= 30;
	                     npcd.state = DAMAGE;
	                     npcd.SetCurrentAction(NULL, 0, npcd.damageID);
	                     npcd.frame = 0;
	                  }
	               }
	               if (npce.state != DIE && npce.isFriend)
	               {
	                  if(isHit(npcgpos, npcepos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npce.blood -= 30;
	                     npce.state = DAMAGE;
	                     npce.SetCurrentAction(NULL, 0, npce.damageID);
	                     npce.frame = 0;
	                  }
	               }
	               if (npcf.state != DIE && npcf.isFriend)
	               {
	                  if(isHit(npcgpos, npcfpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcf.blood -= 30;
	                     npcf.state = DAMAGE;
	                     npcf.SetCurrentAction(NULL, 0, npcf.damageID);
	                     npcf.frame = 0;
	                  }
	               }
	               if (npcg.state != DIE && npcg.isFriend)
	               {
	                  if(isHit(npcgpos, npcgpos, npcgfDir, 120.0f, 40.0f))
	                  {
	                     npcg.blood -= 30;
	                     npcg.state = DAMAGE;
	                     npcg.SetCurrentAction(NULL, 0, npcg.damageID);
	                     npcg.frame = 0;
	                  }
	               }
	            }
	         }
	         if (npcg.frame == 47)
	         {
	            npcg.state = IDLE;
	            npcg.SetCurrentAction(NULL, 0, npcg.idleID); 
	            npcg.frame = 0;
	         }
				break;
			case DAMAGE:
			// no damage
				if (npcg.frame == 0)
				{
					npcg.SetCurrentAction(NULL, 0, npcg.idleID);
					hP.Load("Ghurt2.wav");
					hP.Play(ONCE);
				}
				npcg.Play(LOOP, (float) skip, FALSE, TRUE);
				npcg.frame++;
				if (npcg.frame == 25)
				{
					npcg.state = IDLE;
					npcg.SetCurrentAction(NULL, 0, npcg.idleID);
					npcg.frame = 0;
				}
				break;
			case DIE:
			// die 31
				npcg.Play(ONCE, (float) skip, FALSE, TRUE);
				if (!npcg.isFriend)
				{
					npcg.frame++;
					if (npcg.frame == 5)
					{
						hP.Load("Gdie2.wav");
						hP.Play(ONCE);
					}
					if (npcg.frame == 60)
					{
						if (teammateID[0] == NONE)
						{
							teammateID[0] = NPCG;
							npcg.state = IDLE;
							npcg.SetCurrentAction(NULL, 0, npcg.idleID);
							npcg.frame = 0;
							npcg.blood = npcg.fullBlood;
						}
						else if (teammateID[1] == NONE)
						{
							teammateID[1] = NPCG;
							npcg.state = IDLE;
							npcg.SetCurrentAction(NULL, 0, npcg.idleID);
							npcg.frame = 0;
							npcg.blood = npcg.fullBlood;
						}
						npcg.isFriend = true;
					}
				}
				break;
			default:
				break;
		}

		npcg.BB();


			// play game FX
			   if (gFXID != FAILED_ID) {
				  FnGameFXSystem gxS(gFXID);
				  BOOL4 beOK = gxS.Play((float) skip, ONCE);
				  if (!beOK) {
					 FnScene scene(sID);
					 scene.DeleteGameFXSystem(gFXID);
					 gFXID = FAILED_ID;
				  }
			   }

						// play game FX
			   if (dFXID != FAILED_ID) {
				  FnGameFXSystem dxS(dFXID);
				  BOOL4 beOK = dxS.Play((float) skip, ONCE);
				  if (!beOK) {
					 FnScene scene(sID);
					 scene.DeleteGameFXSystem(dFXID);
					 dFXID = FAILED_ID;
				  }
			   }

		/*
			// play game FX
			   if (gFXID != FAILED_ID) {
				  FnGameFXSystem dxS(gFXID);
				  BOOL4 beOK = dxS.Play((float) skip, ONCE);
				  if (!beOK) {
					 FnScene scene(sID);
					 scene.DeleteGameFXSystem(gFXID);
					 gFXID = FAILED_ID;
				  }
			   }
	*/

		
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
				if(d_oa < 700.0f)
				{
					object.MoveForward(-10.0f);
					object.GetPosition(opos);
					d_oa = myDist(opos, pos);
					if(d_oa > 700.0f){
						opos[2] = cameraHieght(700.0f);
						opos[0] = pos[0] - sqrt(490000.0f / (1 + (cfDir[1] * cfDir[1]) / (cfDir[0] * cfDir[0]))) * cfDir[0] / fabs(cfDir[0]);
						opos[1] = pos[1] - sqrt(490000.0f / (1 + (cfDir[0] * cfDir[0]) / (cfDir[1] * cfDir[1]))) * cfDir[1] / fabs(cfDir[1]);
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
}

void RenderIt(int skip)
{
	FnViewport vp;

	// render the whole scene
	vp.ID(vID);
	vp.Render3D(cID, TRUE, TRUE);

	if(pause){
   		vp.RenderSprites(sID2, TRUE, TRUE);
   	}

	// get camera's data
	FnCamera camera;
	camera.ID(cID);

	FnObject object;
	object.ID(oID);

	float cpos[3], cfDir[3], cuDir[3], pos[3], npcapos[3], npcbpos[3];

	camera.GetPosition(cpos);
	camera.GetDirection(cfDir, cuDir);

 	actor.GetPosition(pos);
 	npca.GetPosition(npcapos);
 	npcb.GetPosition(npcbpos);

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
		
	}

	FnText text;
	text.ID(textID);

	text.Begin(vID);
	text.Write(string, 20, 20, 255, 0, 0);

	char posS[256], npcaposS[256], npcbposS[256], distS[256];
	char cposS[256], cfDirS[256], cuDirS[256];
	char actorBloodS[256], npcaBloodS[256], npcbBloodS[256];
	char frameS[256];

	sprintf(cposS, "cpos: %8.3f %8.3f %8.3f", cpos[0], cpos[1], cpos[2]);
	sprintf(cfDirS, "cfacing: %8.3f %8.3f %8.3f", cfDir[0], cfDir[1], cfDir[2]);
	sprintf(cuDirS, "cup: %8.3f %8.3f %8.3f", cuDir[0], cuDir[1], cuDir[2]);
	sprintf(posS, "pos: %8.3f %8.3f %8.3f", pos[0], pos[1], pos[2]);
	sprintf(npcaposS, "npcapos: %8.3f %8.3f %8.3f", npcapos[0], npcapos[1], npcapos[2]);
	sprintf(npcbposS, "npcbpos: %8.3f %8.3f %8.3f", npcbpos[0], npcbpos[1], npcbpos[2]);
	sprintf(distS, "dist: %8.3f", myDist(cpos, pos));
	sprintf(actorBloodS, "Actor HP: %d / %d", actor.blood, actor.fullBlood);
	sprintf(npcaBloodS, "Npc A HP: %d / %d", npca.blood, npca.fullBlood);
	sprintf(npcbBloodS, "Npc B HP: %d / %d", npcb.blood, npcb.fullBlood);
	sprintf(frameS, "actor frame: %d", actor.frame);

	text.Write(cposS, 20, 35, 255, 255, 0);
	text.Write(cfDirS, 20, 50, 255, 255, 0);
	text.Write(cuDirS, 20, 65, 255, 255, 0);
	text.Write(distS, 20, 95, 255, 255, 0);
	text.Write(posS, 20, 110, 255, 255, 0);
	text.Write(npcaposS, 20, 125, 255, 255, 0);
	text.Write(npcbposS, 20, 140, 255, 255, 0);
	text.Write(actorBloodS, 20, 155, 255, 255, 0);
	text.Write(npcaBloodS, 20, 170, 255, 255, 0);
	text.Write(npcbBloodS, 20, 185, 255, 255, 0);
	text.Write(frameS, 20, 200, 255, 255, 0);

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
		if(code == FY_1){
			cameraRotateState = 1;
		}
		else if(code == FY_2){
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
	if (value)
	{
		attackKeyState = true;
	}
	else
	{
		attackKeyState = false;
	}	
}

void NpcControl(BYTE code, BOOL4 value){
	if (value)
	{
		if (code == FY_A)
		{
			isFollow[0] = false;
		}
		else if (code == FY_S)
		{
			isFollow[0] = true;
		}
		else if (code == FY_D)
		{
			isFollow[1] = false;
		}
		else if (code == FY_F)
		{
			isFollow[1] = true;
		}
	}
}

void Reset(BYTE code, BOOL4 value)
{
	if (value)
	{
		actor.blood = actor.fullBlood;
		npca.blood = npca.fullBlood;
		npcb.blood = npcb.fullBlood;
		npcc.blood = npcc.fullBlood;
		npcd.blood = npcd.fullBlood;
		npce.blood = npce.fullBlood;
		npcf.blood = npcf.fullBlood;
		npcg.blood = npcg.fullBlood;
		actor.state = IDLE;
		npca.state = IDLE;
		npcb.state = IDLE;
		npcc.state = IDLE;
		npcd.state = IDLE;
		npce.state = IDLE;
		npcf.state = IDLE;
		npcg.state = IDLE;
		actor.SetCurrentAction(NULL, 0, idleID);
		npca.SetCurrentAction(NULL, 0, npca.idleID);
		npcb.SetCurrentAction(NULL, 0, npcb.idleID);
		npcc.SetCurrentAction(NULL, 0, npcc.idleID);
		npcd.SetCurrentAction(NULL, 0, npcd.idleID);
		npce.SetCurrentAction(NULL, 0, npce.idleID);
		npcf.SetCurrentAction(NULL, 0, npcf.idleID);
		npcg.SetCurrentAction(NULL, 0, npcg.idleID);
		actor.frame = 0;
		npca.frame = 0;
		npcb.frame = 0;
		npcc.frame = 0;
		npcd.frame = 0;
		npce.frame = 0;
		npcf.frame = 0;
		npcg.frame = 0;

		attackKeyLocked = false;
		movementKeyLocked = false;
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

void PauseGame(BYTE code, BOOL4 value){
	if(value){
		pauseID = FyCreateAudio();
		FnAudio pauseP;
		pauseP.Object(pauseID);
		pauseP.Load("dominating.wav");
		pauseP.Play(ONCE);

		FnAudio mP;
		mP.Object(mmID);
		
		if(pause) {
			mP.Play(LOOP);
			pause = false;
		}
		else{
			mP.Pause();
			pause = true;
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