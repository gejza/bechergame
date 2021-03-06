
#include "../StdAfx.h"
#include "../becher.h"
#include "../buildings.h"
//#include "elements.h"
#include "editor_map.h"
//#include "plugin_editlevel.h"
#include "../sysobjs.h"
#include "../elements.h"


void EditorMap::CreateNew(uint sizeX, uint sizeY)
{

	m_numX = sizeX;
	m_numY = sizeY;

	m_sizeX = m_numX * 20.f;
	m_sizeY = m_numY * 20.f;
	m_distX = m_sizeX / m_numX;
	m_distY = m_sizeY / m_numY;
	CreateScene();
	GetEngine()->SetActiveScene(m_scene);
	m_mapfilepath = wxT("");
	SetTerrainData();
	m_terrain->Create(m_sizeX, m_sizeY,m_numX,m_numY);

	// vytvorit
	for (uint x=0;x < m_numX;x++)
	for (uint y=0;y < m_numY;y++)
	{
		IHoeEnv::GridSurface::TGridDesc desc;
		m_terrain->GetGridDesc(x,y,&desc);		
		desc.x2 = 0;
		desc.y2 = 0;
		desc.tex2 = 0xff;
		m_terrain->SetGridDesc(x,y,&desc);
	}

	m_terrain->Load();
	// textures
}

bool EditorMap::LoadMap(const wxString &path)
{
	// smazani objektu
	//TODO dodelat mazani levelu
	m_obj.Delete();

	HoeEditor::EditorFile file;
	if (!file.OpenRead(path))
		return false;
	BecherGameLoad r(&file);
	CreateScene();
	GetEngine()->SetActiveScene(m_scene);
	SetTerrainData();
	
	// nahrat mapu
	// nejdriv hlavicu
	if (!r.ReadHeader())
		return false;
	// postupne nacitani chunku
	int pos = sizeof(BechSaveHeader);
	while (1)
	{
		r.Seek(pos);
		if (!r.ReadNext())
			return false;
		if (r.Chunk().chunk == ID_CHUNK('e','n','d',' '))
			break;
		// sejf value
		LoadMapChunk(r);
		pos += r.Chunk().size + sizeof(MapChunk); 
	}

	return true;
}

bool EditorMap::SaveMap(const wxString &path)
{
	// open file for write
	HoeEditor::EditorFile file;
	if (!file.OpenWrite(path))
	{
		wxLogMessage(_("Failed open file %s for writing."), path.c_str());
		return false;
	}

	BecherGameSave w(&file);

	BechSaveHeader head;
	// zapsat prazdnou hlavicku
	memset(&head, 0, sizeof(head));
	head.id = ID_CHUNK('A','L','C','E');
	head.version = ID_BECHERVER;
	w.Write(&head, sizeof(head));
	// info o levelu
	/*w.WriteChunk(ID_CHUNK('i','n','f','o'));
	ChunkDictWrite info();
	info.Reset();
	info.End();
	w.WriteChunkEnd();*/

	// terrain dump
	w.WriteChunk(ID_CHUNK('t','e','r','r'));
	m_terrain->Dump(&w);
	w.WriteChunkEnd();

	for (int i=0;i < m_addon.Count();i++)
	{
		w.WriteChunk(ID_CHUNK('a','d','o','n'));
		ChunkDictWrite dict(w);
		dict.Begin();
		dict.Key("type", (int)m_addon[i]->GetType());
		dict.Key("x", (float)m_addon[i]->GetPosX());
		dict.Key("y", (float)m_addon[i]->GetPosY());
		dict.Key("angle", (float)m_addon[i]->GetAngle());
		dict.End();
		w.WriteChunkEnd();
	}

	// ulozit systemove objekty
	for (int i=0;i < m_sysobj.Count();i++)
	{
		// uklada se jen typ
		w.WriteChunk(ID_CHUNK('s','y','s','o'));
		ChunkDictWrite dict(w);
		dict.Begin();
		dict.Key("type", (int)m_sysobj[i]->GetType());
		dict.Key("x", (float)m_sysobj[i]->GetPosX());
		dict.Key("y", (float)m_sysobj[i]->GetPosY());
		m_sysobj[i]->Save(dict);
		dict.End();
		w.WriteChunkEnd();
	}

	// objekty
	SaveAllObjects(w);
	
	// ulozit parametry objektu

	w.WriteChunk(ID_CHUNK('e','n','d',' '));
	w.GetStringMap()->WriteToFile(&head, w);

	// ulozit key map
	w.GetFile()->Seek(0);
	w.Write(&head, sizeof(head));

	return true;
}

bool EditorMap::SaveMap()
{
	assert(!m_mapfilepath.IsEmpty());
	return SaveMap(m_mapfilepath);
}

void EditorMap::SelectObject(const int x, const int y)
{
	BecherObject * o = GetObject(x,y);
	if (o == m_select)
		return;
	if (m_select) 
		m_select->GameMsg(BMSG_Unselect,0,0,NULL);
	if (o)
		o->GameMsg(BMSG_Select,0,0,NULL);
	else
		GetProp()->Begin(NULL);
	m_select = o;
}

void EditorMap::SetFilePath(const wxString &path)
{
	m_mapfilepath = path;
}

wxString EditorMap::GetFilePath()
{
	return this->m_mapfilepath;
}

wxString EditorMap::GetTitle()
{
	return this->m_mapfilepath;
}

void EditorMap::Resize(int top, int bottom, int left, int right)
{
	// ulozeni stareho
	IHoeEnv::GridSurface::TGridDesc * desc = new IHoeEnv::GridSurface::TGridDesc[m_numX*m_numY];
	for (uint y=0;y < m_numY;y++)
		for (uint x=0;x < m_numX;x++)
			m_terrain->GetGridDesc(x,y, desc + m_numX * y + x);
	
	const uint nx = left + right + m_numX;
	const uint ny = top + bottom + m_numY;
	const float posx = (nx - m_numX) * m_distX;
	const float posy = (ny - m_numY) * m_distY;
	m_sizeX += posx; m_sizeY += posy;

	m_terrain->Create(m_sizeX, m_sizeY, nx, ny);
	for (uint y=0;y < ny;y++)
		for (uint x=0;x < nx;x++)
		{
			const uint px = left + x;
			const uint py = top + y;
			if (px >= 0 && px < m_numX && py >= 0 && py < m_numY)
				m_terrain->SetGridDesc(x,y, desc + m_numX * py + px);
			else
			{
				IHoeEnv::GridSurface::TGridDesc d;
				memset(&d, 0, sizeof d);
				d.tex2 = 1;
				m_terrain->SetGridDesc(x,y, &d);
			}
		}
	m_numX = nx;
	m_numY = ny;
	delete [] desc;

	// posunout vsechny na spravne misto
	for (int i=0;i<GetNumObj();i++)
	{
		BecherObject & o = *GetObj(i);
		o.SetPosition(o.GetPosX()-posx*0.5f, o.GetPosY()-posy*0.5f,0);
	}
}

void EditorMap::ShowSystemObjects(bool show)
{
	// prolezt a zapnout, nebo vypnout
	for (int i=0;i<m_sysobj.Count();i++)
		m_sysobj[i]->Show(show);
}

void EditorMap::ShowObjects(bool show, bool wire)
{
	for (int i=0;i<m_obj.Count();i++)
	{
		m_obj[i]->Show(show);
		if (wire)
			m_obj[i]->GetCtrl()->SetFlags(HOF_WIRE);
		else
			m_obj[i]->GetCtrl()->UnsetFlags(HOF_WIRE);
	}
}

void EditorMap::ModelHeightUpdate()
{
	int i;
	for (i=0;i<m_obj.Count();i++)
	{
		const float x = m_obj[i]->GetPosX();
		const float y = m_obj[i]->GetPosY();
		m_obj[i]->SetPosition(x,y,m_scene->GetScenePhysics()->GetHeight(x,y));
	}	
	for (i=0;i<m_sysobj.Count();i++)
	{
		const float x = m_sysobj[i]->GetPosX();
		const float y = m_sysobj[i]->GetPosY();
		m_sysobj[i]->SetPosition(x,y,m_scene->GetScenePhysics()->GetHeight(x,y));
	}
}

void EditorMap::DeleteSelect()
{
	if (m_select == NULL)
		return;
	//m_select->Unselect();
	if (dynamic_cast<BecherSystemObject*>(m_select))
	{
		BecherSystemObject * sb = dynamic_cast<BecherSystemObject*>(m_select);
		m_sysobj.Remove(sb);
		m_select = NULL;
		delete sb;
	}
	if (dynamic_cast<Addon*>(m_select))
	{
		Addon * sb = dynamic_cast<Addon*>(m_select);
		m_addon.Remove(sb);
		m_select = NULL;
		delete sb;
	}
	else
	{
		m_obj.Remove(m_select);
		delete m_select;
		m_select = NULL;
	}
}


