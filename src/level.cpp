
#include "StdAfx.h"
#include "becher.h"
#include "buildings.h"
#include "elements.h"
#include "level.h"
#include "game.h"
#include "dialogs.h"
#include "troll.h"
#include "obj_sugar.h"
#include "obj_destilate.h"
#include "obj_factory.h"
#include "obj_store.h"
#include "obj_waterhole.h"
#include "obj_farm.h"
#include "phys_prov.h"
#include "b_msg.h"

enum 
{
	ESC_move_speed = 0,
	ESC_track_speed,
	ESC_zoom_speed,
	ESC_rotate_speed,
	ESC_max
};
static const THoeVarIndex idx_camera[] = { 
	"move_speed", ESC_move_speed, TVAR_FLOAT, NULL,
	"track_speed", ESC_track_speed, TVAR_FLOAT, NULL,
	"zoom_speed", ESC_zoom_speed, TVAR_FLOAT, NULL,
	"rotate_speed", ESC_rotate_speed, TVAR_FLOAT, NULL,
	  NULL };

static HoeGame::CVarValue vv_camera[ESC_max] = { 60.f, 2.f, 40.f, 1.f };
static CVar v_camera("camera", idx_camera, vv_camera, ESC_max);

BecherLevel::BecherLevel()
{
	this->m_build = NULL;
	m_filename[0] = '\0';
	m_select = NULL;
	m_mselect = NULL;
	m_dlg = NULL;
	//m_info.Init(10.f,550.f, 50.f);
	//m_controls.Init();

}

void BecherLevel::_Paint(IHoe2D * h2d)
{
	// hud
	m_hud.Draw(h2d);
	if (m_dlg)
		m_dlg->Draw(h2d);

	if (GetBecher()->GetConDlg()->IsActive())
        GetBecher()->GetConDlg()->Draw(h2d);

}

void BecherLevel::OnSet()
{
	HoeGetRef(GetEngine())->SetBackgroundColor(0x00000000);
	HoeGetInput(GetEngine())->RegisterKeyboard(this);
		//if (HoeGetRef(GetEngine())->IsFullscreen())
		//	HoeGetInput(GetEngine())->RegisterMouse(IHoeInput::MT_Cursored, this);
		//else
	HoeGetInput(GetEngine())->RegisterMouse(IHoeInput::MT_Background, this);
	HoeGetInput(GetEngine())->SetWindowRect(800,600);

	GetScene()->Set2DCallback(this);
	GetEngine()->SetActiveScene(GetScene());	

}

void BecherLevel::Update(float time)
{
	if (IsPaused())
	{
		GetBecher()->GetConDlg()->Update(time);
		return;
	}
	else
	{
		m_timer.Update(time);
		m_mjobs.Update(time);
		GetView()->Update(time);
		// update objects
		for (uint i=0;i < this->m_obj.Count();i++)
		{
			this->m_obj[i]->Update(time);
		}
	}

	if (!GetBecher()->GetConDlg()->IsActive())
	{
		if (this->IsKeyDown(HK_UP))
			GetView()->Move(time * v_camera.GetFloat(ESC_move_speed),0);
		if (this->IsKeyDown(HK_DOWN))
			GetView()->Move(-time * v_camera.GetFloat(ESC_move_speed),0);
		if (this->IsKeyDown(HK_RIGHT))
			GetView()->Move(0,time * v_camera.GetFloat(ESC_move_speed));
		if (this->IsKeyDown(HK_LEFT))
			GetView()->Move(0,-time * v_camera.GetFloat(ESC_move_speed));

		if (IsKeyDown(HK_MINUS))
		{
			GetView()->Zoom(v_camera.GetFloat(ESC_zoom_speed) * time);
		}
		if (IsKeyDown(HK_EQUALS))
			GetView()->Zoom(-v_camera.GetFloat(ESC_zoom_speed) * time);
			
		if (IsKeyDown(HK_COMMA))
			GetView()->Rotate(-time * v_camera.GetFloat(ESC_rotate_speed));
		if (IsKeyDown(HK_PERIOD))
			GetView()->Rotate(time * v_camera.GetFloat(ESC_rotate_speed));
	}
	else
	{
		GetBecher()->GetConDlg()->Update(time);
	}
	if (m_hud.GetActMap())
	{
		float x,y;
		GetView()->GetTargetPosition(&x,&y);
		m_hud.GetActMap()->SetCameraPosition(x, y);
	}
}

void BecherLevel::MouseUpdate(float x, float y)
{
	if (m_build)
	{
		float X,Y;
		m_build->Show(true);
		if (GetView()->GetPick(x,y,&X,&Y))
		{
			// zda postavit nebo nestavit
			// ukazatel cervene
			PAR_BuildPlace bp = {X,Y,NULL,0,0};
			m_build->SetAngle(1);
			SendGameMsg(m_build, BMSG_SelectPlace, 0, &bp, 5);
		}
	}
	else // show select
	{
		// vybrat objekt pro zobrazeni
	}
}

void BecherLevel::MouseLeave() 
{
	if (m_build)
		m_build->Show(false);
}

void BecherLevel::SetBuildObject(BecherBuilding * bo, int gold, int wood, int stone)
{
	LeaveBuild();
	m_build = bo;
	m_buildgold = gold;
	m_buildwood = wood;
	m_buildstone = stone;
	//m_buildinit = m_select;
	//todomsg bo->SetMode(EBM_Select);
	bo->Show(false);
}

void BecherLevel::LeaveBuild()
{
	if (m_build)
	{
		delete m_build;
		m_build = NULL;
	}
}

void BecherLevel::Start()
{
	m_timer.Set(HoeGame::Timer::tmClock, 0);
	m_timer.Start();
	IHoeLight * light = m_scene->CreateLight(true);
}

void BecherLevel::MouseLeftDown(float x, float y)
{
	if (m_build)
	{
		//m_buildcandidate.wood,m_buildcandidate.stone);
		//assert((this->GetSelectionCount() == 1));
		//assert(this->GetSelectedObject(0)->GetType() == EBO_Troll);
		//reinterpret_cast<Troll*>(this->GetSelectedObject(0))->Build(reinterpret_cast<BecherBuilding*>(o));
		float X,Y;
		m_build->Show(true);
		if (GetView()->GetPick(x,y,&X,&Y))
		{
			PAR_BuildPlace bp = {X,Y,NULL,0,0};
			if (!SendGameMsg(m_build, BMSG_StartBuilding,0,&bp,5)) 
			//if (dynamic_cast<BecherBuilding*>(m_build)->StartBuilding(m_buildgold, m_buildwood, m_buildstone))
			{
				this->AddObject(m_build);
				//if (m_buildinit && m_buildinit->GetType() == EBO_Troll)
				//	dynamic_cast<BecherBuilding*>(m_build)->Assign(dynamic_cast<Troll*>(m_buildinit));
				m_build = NULL;
			}
		}
		return;
	}

	HoeGame::Strategy::StgObject * o = GetView()->SelObject(x,y);
	if (o)

		SelectObject(dynamic_cast<BecherObject *>(o));
	else
		SelectObject(NULL);
}

void BecherLevel::SelectObject(BecherObject * so)
{
	if (m_select && m_select != so)
	{
		SendGameMsg(m_select, BMSG_Unselect, NULL, 0);
		m_hud.SetObjectHud(NULL,NULL);
		m_hud.ShowReset();
	}
	if (so)
	{
		SendGameMsg(so, BMSG_Select, NULL,0);
		GetView()->SetTrack(so->GetPosX(), so->GetPosY(), v_camera.GetFloat(ESC_track_speed));
		m_select = so;
	}
	else
		m_select = NULL;
}

bool BecherLevel::SaveGame(const char * path)
{
	/*HoeGame::HoeFile file;
	if (!file.Open(path, hftWrite))
		return false;
	BecherGameSave w(&file);

	w.WriteChunk(ID_BSAVE, ID_BECHERVER);
	w.WriteChunk(ID_CHUNK('m','a','p',' '),0);
	// chunks
	// save map info (ktera mapa)
	size_t sfn = strlen(m_filename);
	w.Write(m_filename, sfn);
	w.WriteChunkEnd();

	// save timer
	//m_cash.Save(w);
	//m_timer.Save(w);

	// save view
	/*float view[4];
	GetView()->GetTargetPosition( &view[0], &view[1]);
	view[2] = GetView()->GetAngle();
	view[3] = GetView()->GetDistance();
	w.Write(view, sizeof(view));

	// save objects
	SaveAllObjects(w);*/

	return true;
}

bool BecherLevel::LoadGame(const char *path)
{
	// unload stare
	Create(CreateScene());

	if (!m_mjobs.Init())
		return false;
	m_hud.Load("scripts/hud.menu");
	this->m_cash.Link(dynamic_cast<HoeGame::Gui::DigiCounter *>(m_hud.ReqItem("cash", HoeGame::Gui::EDigiCounter)));
	m_builddlg.Load("scripts/build.menu");

	SetTerrainData();
    TRACE;
	HoeGame::HoeFile file;
	if (!file.Open(path))
	{
		GetCon()->Printf("Open file %s failed.", path);
		return false;
	}
  TRACE;
	BecherGameLoad r(&file);
	// nahrat mapu
	// nejdriv hlavicu
	if (!r.ReadHeader())
		return false;
	// postupne nacitani chunku
    TRACE;
	int pos = sizeof(BechSaveHeader);
	while (1)
	{
        TRACE;
		r.Seek(pos);
		if (!r.ReadNext())
			return false;
		if (r.Chunk().chunk == ID_CHUNK('e','n','d',' '))
			break;
		// sejf value
		LoadMapChunk(r);
		pos += r.Chunk().size + sizeof(MapChunk); 
	}
  TRACE;
	/*if (!r.IsSaveGame())
	{
		strncpy(m_filename, path, sizeof(m_filename));
		if (!Load( r, false))
			return false;		
	}
	else 
	{
		// jmeno hry
		if (!r.ReadNext())
			return false;

		if (r.Chunk().chunk == ID_CHUNK('n','a','m','e'))
		{
			r.Skip(r.Chunk().size);
			// prvni musi byt chunk s nazvem hry
			
			if (r.Read<dword>() != 123456789 || !r.ReadNext())
				return false;
		}
		if (r.Chunk().chunk != ID_CHUNK('m','a','p',' '))
			return false;

		// zjistit prvni chunk a nahrat mapu
		char mapname[1024] = {0};
		r.Read(mapname, r.Chunk().size);
		if (r.Read<dword>() != 123456789)
				return false;
		// nacist mapu
		HoeGame::HoeFile filemap;
		if (!filemap.Open(mapname))
		{
			GetCon()->Printf("Open file %s failed.", path);
			return false;
		}

		BecherGameLoad rr(&filemap);
		if (!rr.ReadHeader())
			return false;

		if (!Load( rr, true))
				return false;		

		if (!Load(r, false))
			return false;
	}*/

	// 
	m_land.Create(this);
	// load minimaps
	m_watermap.Load(this);
	m_coalmap.Load(this);
	m_stonemap.Load(this);
	//m_termap.Load(this);


	m_hud.SetMap(&m_termap);

	IHoeModel * m = (IHoeModel*)GetEngine()->Create("model voda");
	// voda
	for (uint x=0; x < this->m_numX;x+=1)
	for (uint y=0; y < this->m_numY;y+=1)
	{
		if (!Phys::Get()->IsWater(x,y))
			continue;
		XHoeObject * obj = new XHoeObject;
		m_scene->RegisterObject(obj);
		obj->SetModel(m);
		obj->SetPosition( x * 20.f + 10.f - this->m_sizeX * 0.5f, -15, y * 20.f + 10.f - this->m_sizeY * 0.5f);
		obj->Show(true);
	}

	return true;
}

////////////////////////////////////////////////
bool BecherTime::Save(HoeFileWriter &w)
{
	return true;
}

bool BecherTime::Load(int ver, HoeFileReader &r)
{
	return true;
}

BecherCash::BecherCash()
{
	Set(-1000,-100000);
}

void BecherCash::Set(int cash, int limit)
{
	m_limit = limit;
	m_cash = cash;
}

bool BecherCash::Add(int m)
{ 
	if (GetLimitCash() < -m) 
		return false; 
	m_cash += m; 
	return true; 
}

bool BecherCash::Save(HoeFileWriter &w)
{
	//w.Write<int>(m_cash);
	//w.Write<int>(m_limit);
	return true;
}

bool BecherCash::Load(int ver, HoeFileReader &r)
{
	//m_cash = r.Read<int>();
	//m_limit = r.Read<int>();
	Set(0,-100000);
	return true;
}

void BecherLevel::OnKeyDown(int key)
{
	if (key == HK_GRAVE)
		GetBecher()->GetConDlg()->Open();
	else if (key == HK_F9)
	{
			if (SaveGame("a.sav"))
				m_hud.GetInfo()->Add("Game saved.");
			else
				assert(!"game save failed");
	}
	else if (key == HK_SPACE)
	{
		SetDialog(&m_builddlg);
	}
	else if (key == HK_DELETE)
	{
		if (m_select)
		{
			BecherObject * o = m_select;
			SelectObject(NULL);
			DeleteObject(o);
		}
	}
	/*if (GetCon()->IsActive())
		GetCon()->*/
	//m_info.Addf("key down %d",key);
}

void BecherLevel::SetDialog(BaseDialog * dlg)
{ 
	if (m_dlg == dlg)
		return;
	if (dlg) LeaveBuild();
	// neco jako set?
	if (m_dlg) m_dlg->OnHide();
	if (dlg) dlg->OnShow();
	m_dlg = dlg; 
}

void BecherLevel::OnMouseMove(float X, float Y)
{
	bool select = false;

	// select 2d
	/*select = this->m_controls.MouseMove(X,Y);
	if (select)
        MouseLeave();
	else*/
	BecherObject * o = NULL;
	bool act = false;
	if (m_dlg)
		m_dlg->Move(X,Y, act);
	m_hud.Move(X,Y,act);
	if (!act)
	{
		HoeGame::Strategy::StgObject * obj = GetView()->SelObject(X,Y);
		if (obj)
		{
			o = dynamic_cast<BecherObject *>(obj);
			act = true;
		}
	}

	if (m_mselect && o != m_mselect)
	{
		SendGameMsg(m_mselect, BMSG_CursorInactive);
		m_mselect = NULL;
	}

	if (!m_build)
	{
		if (o && o != m_select)
		{
			SendGameMsg(m_mselect, BMSG_CursorActive);
		}
		m_mselect = o;
	}
	MouseUpdate(X,Y);
}

void BecherLevel::OnWheel(long p)
{
	if (IsShiftKeyDown())
		GetView()->Zoom(p * 0.1f);
	else
		GetView()->Rotate(p * 0.001f);
}

void BecherLevel::OnLeftButtonUp()
{
	if (m_dlg && m_dlg->Click(GetMouseX(), GetMouseY()))
	{
		return;
	}
	if (m_hud.Click(GetMouseX(), GetMouseY()))
	{
		return;
	}
	else
	//this->m_controls.ShowReset();
	MouseLeftDown(GetMouseX(), GetMouseY());
}

void BecherLevel::OnLeftButtonDown()
{
	//m_info.Addf("button up %d",p);

}

void BecherLevel::OnRightButtonUp()
{
	if (GetSelectedObject() && GetSelectedObject()->GetType() ==  EBO_Troll)
	{
		float x,y;
		if (GetView()->GetPick(GetMouseX(), GetMouseY(), &x, &y))
		{
			HoeMath::Vector2 vec(x,y);
			SendGameMsg(GetSelectedObject(),BMSG_Go,0,&vec,2);
		}
	}
}

void BecherLevel::OnRightButtonDown()
{
	//m_info.Addf("right button up");
}

void BecherLevel::AddBuildObject(unsigned long id, int gold, int wood, int stone)
{
	if (GetCash()->GetLimitCash() < gold)
	{
		GetLevel()->GetPanel()->GetInfo()->Add(GetLang()->GetString(104));
		// todo -> limit pro penize
		return;
	}

	assert_obj(id);
	BecherObject * bo = CreateObject((EObjType)id);
	BecherBuilding * bb = dynamic_cast<BecherBuilding*>(bo);
	if (bb)
		SetBuildObject( bb, gold, wood, stone);
}

Troll * BecherLevel::CreateTroll()
{
	Troll * t = (Troll*)GetLevel()->CreateObject(EBO_Troll);
	t->SetPosition(0, 0, 0);
	// TODO nastavit na pozici kde se ma objevovat podle mapy
	GetLevel()->AddObject(t);
	return t;
}

/////////////////////////////////////////////
// Path finding
bool BecherLevel::FindPath(const HoeMath::Vector2 &from, const HoeMath::Vector2 &to, TrollPath &path)
{
	return m_land.FindPath(from, to, path);
}

//////////////////////////////////////////////
// Lua
int BecherLevel::l_SetBuilding(lua_State * L)
{
	HoeGame::LuaParam lp(L);
	if (lp.CheckPar(4,"nnnn", "SetBuilding"))
	{
		GetLevel()->AddBuildObject(lp.GetNum(-4), lp.GetNum(-3), lp.GetNum(-2), lp.GetNum(-1));
	}
	return 0;
}

int BecherLevel::l_AddCash(lua_State * L)
{
	HoeGame::LuaParam lp(L);
	if (lp.CheckPar(1, "n", "AddCash"))
	{
		GetLevel()->GetCash()->Add(lp.GetNum(-1));
	}
	return 0;
}

int BecherLevel::l_GetCash(lua_State * L)
{
	HoeGame::LuaParam lp(L);
	if (lp.CheckPar(0,0, "GetCash"))
		lp.PushNum(GetLevel()->GetCash()->GetLimitCash());
	return 1;
}

int BecherLevel::l_Camera(lua_State * L)
{
	BecherLevel *level = GetLevel();
	HoeGame::LuaParam lp(L);
	if (lp.GetNumParam() == 1 && lp.CheckPar(1,"n", "Camera"))
	{
		BecherObject * bo = level->GetObj(lp.GetNum(-1));
		if (bo)
			level->GetView()->SetTrack(bo->GetPosX(), bo->GetPosY(), v_camera.GetFloat(ESC_track_speed));
	}
	else if (lp.GetNumParam() == 2 && lp.CheckPar(2,"nn", "Camera"))
	{
		level->GetView()->SetTrack(lp.GetFloat(-2), lp.GetFloat(-1), v_camera.GetFloat(ESC_track_speed));
	}
	return 0;
}

/////////////////////////////////////////////
// Path finding
bool BecherLand::Create(BecherMap * map)
{
	m_tiles.Create(map->m_numX, map->m_numY);
	IHoeEnv::GridSurface * grid = map->GetTerrain();
	// nejak vypiskovat
	IHoeEnv::GridSurface::TGridDesc desc;
	for (int y=0;y < map->m_numY;y++)
	{
		for (int x=0;x < map->m_numX;x++)
		{
			if (grid->GetGridModel(x,y) != -1)
			{
				m_tiles.Set(x,y,0);
				continue;
			}
			float min = 0.f;
			grid->GetAvgHeight(x,y,&min,NULL);
			m_tiles.Set(x,y, (min >= -15.f) ? 1:0);
		}
	}
	THoeRect rect;
	rect.top = - (map->m_sizeY*0.5f);
	rect.left = - (map->m_sizeX*0.5f);
	rect.right = (map->m_sizeX*0.5f);
	rect.bottom = (map->m_sizeY*0.5f);
	m_land.Create(map->m_numX,map->m_numY, rect);
	m_land.Preprocess(m_tiles);
	return true;
}

bool BecherLand::FindPath(const HoeMath::Vector2 &from, const HoeMath::Vector2 &to, TrollPath &path)
{
	return m_land.Find(from,to,path);
}









