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

VIEWPORTid vID;                 // the major viewport
SCENEid sID;                    // the 3D scene
OBJECTid cID, tID, oID;         // the main camera and the terrain for terrain following
CHARACTERid actorID, npcaID, npcbID; // the major character
ACTIONid idleID, runID, curPoseID, idleaID, runaID, curPoseaID, idlebID, runbID, curPosebID, attackID; // two actions
ROOMid terrainRoomID = FAILED_ID;
TEXTid textID = FAILED_ID;

// some globals
int frame = 0;
int oldX, oldY, oldXM, oldYM, oldXMM, oldYMM;
int actorHP = MAX_HP, npcaHP = MAX_HP, npcbHP = MAX_HP;

// hotkey callbacks
void QuitGame(BYTE, BOOL4);
void Movement(BYTE, BOOL4);

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

   // load the character
   FySetModelPath("Data\\NTU5\\Characters");
   FySetTexturePath("Data\\NTU5\\Characters");
   FySetCharacterPath("Data\\NTU5\\Characters");
   actorID = scene.LoadCharacter("Lyubu2");
   npcaID = scene.LoadCharacter("Donzo2");
   npcbID = scene.LoadCharacter("Robber02");
   room.AddObject(actorID);
   room.AddObject(npcaID);
   room.AddObject(npcbID);

   // put the character on terrain
   float pos[3], fDir[3], uDir[3];
   FnCharacter actor, npca, npcb;
   actor.ID(actorID);
   pos[0] = 3569.0f; pos[1] = -3208.0f; pos[2] = 1000.0f;
   fDir[0] = 1.0f; fDir[1] = 0.0f; fDir[2] = 0.0f;
   uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
   actor.SetDirection(fDir, uDir);

   actor.SetTerrainRoom(terrainRoomID, 10.0f);
   beOK = actor.PutOnTerrain(pos);

   // Get two character actions pre-defined at Lyubu2
   idleID = actor.GetBodyAction(NULL, "Idle");
   runID = actor.GetBodyAction(NULL, "Run");
   attackID = actor.GetBodyAction(NULL, "NormalAttack1");

   // set the character to idle action
   curPoseID = idleID;
   actor.SetCurrentAction(NULL, 0, curPoseID);
   actor.Play(START, 0.0f, FALSE, TRUE);

   //npca
   npca.ID(npcaID);
   pos[0] = 3769.0f; pos[1] = -3208.0f; pos[2] = 1000.0f;
   fDir[0] = 1.0f; fDir[1] = 0.0f; fDir[2] = 0.0f;
   uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
   npca.SetDirection(fDir, uDir);

   npca.SetTerrainRoom(terrainRoomID, 10.0f);
   beOK = npca.PutOnTerrain(pos);

   // Get two character actions pre-defined at Donzo2
   idleaID = npca.GetBodyAction(NULL, "Idle");
   runaID = npca.GetBodyAction(NULL, "Run");

   // set the character to idle action
   curPoseaID = idleaID;
   npca.SetCurrentAction(NULL, 0, curPoseaID);
   npca.Play(START, 0.0f, FALSE, TRUE);

    //npcb
   npcb.ID(npcbID);
   pos[0] = 3669.0f; pos[1] = -3208.0f; pos[2] = 1000.0f;
   fDir[0] = 1.0f; fDir[1] = 0.0f; fDir[2] = 0.0f;
   uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
   npcb.SetDirection(fDir, uDir);

   npcb.SetTerrainRoom(terrainRoomID, 10.0f);
   beOK = npcb.PutOnTerrain(pos);

   // Get two character actions pre-defined at Robber02
   idlebID = npcb.GetBodyAction(NULL, "CombatIdle");
   runbID = npcb.GetBodyAction(NULL, "Run");

   // set the character to idle action
   curPosebID = idlebID;
   npcb.SetCurrentAction(NULL, 0, curPosebID);
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

   // create a text object for displaying messages on screen
   textID = FyCreateText("Trebuchet MS", 18, FALSE, FALSE);

   // set Hotkeys
   FyDefineHotKey(FY_ESCAPE, QuitGame, FALSE);  // escape for quiting the game
   FyDefineHotKey(FY_UP, Movement, FALSE);      // Up for moving forward
   FyDefineHotKey(FY_RIGHT, Movement, FALSE);   // Right for turning right
   FyDefineHotKey(FY_LEFT, Movement, FALSE);    // Left for turning left
   FyDefineHotKey(FY_DOWN, Movement, FALSE);    // Down for moving back
   FyDefineHotKey(FY_Z, Movement, FALSE);

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


/*-------------------------------------------------------------
  30fps timer callback in fixed frame rate for major game loop
  C.Wang 1103, 2007
 --------------------------------------------------------------*/
void GameAI(int skip)
{
	FnCharacter actor, npca, npcb;
	FnCamera camera;
	FnObject object, terrain;

	bool walk = false;
	 
	float afDir[3], auDir[3], cfDir[3], cuDir[3], ofDir[3], ouDir[3], obpos[3], acpos[3], ohitdir[3];

    ohitdir[0] = 0.0f; ohitdir[1] = 0.0f; ohitdir[2] = -1.0f;

	object.ID(oID);
	camera.ID(cID);
	terrain.ID(tID);
	
	// play character pose
	actor.ID(actorID);
	actor.Play(LOOP, (float) skip, FALSE, TRUE);

	npca.ID(npcaID);
	npca.Play(LOOP, (float) skip, FALSE, TRUE);
	
	npcb.ID(npcbID);
	npcb.Play(LOOP, (float) skip, FALSE, TRUE);

	object.GetPosition(obpos);
	actor.GetPosition(acpos);

	float d_oa = oadistance(obpos, acpos);

	float TURN_A = acos((d_oa*d_oa + d_oa*d_oa - 100.0f) / (d_oa*d_oa + d_oa*d_oa)) * 180.0f / M_PI;

	actor.GetDirection(afDir, auDir);
	object.GetDirection(ofDir, ouDir);

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

	object.GetPosition(obpos);
	actor.GetPosition(acpos);
	camera.GetDirection(cfDir, cuDir);

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

   FnCharacter actor;
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
	FnCharacter actor;
	actor.ID(actorID);

	if( FyCheckHotKeyStatus(code) ) {
		actor.SetCurrentAction(NULL, 0, runID);
	}
	else if(!FyCheckHotKeyStatus(FY_UP) && !FyCheckHotKeyStatus(FY_RIGHT) && !FyCheckHotKeyStatus(FY_LEFT) && !FyCheckHotKeyStatus(FY_DOWN)) {
		actor.SetCurrentAction(NULL, 0, idleID);
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
