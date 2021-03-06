
#include "StdAfx.h"
#include "becher.h"
#include "troll.h"
#include "obj_farm.h"

static CVar v_speed("farm_speed", 0.02f, TVAR_SAVE); // rychlost rustu
static CVar v_farma("farm_cost", 130, TVAR_SAVE);
static CVar v_farma_drevo("farm_cost_wood", 60, TVAR_SAVE);
static CVar v_farma_kamen("farm_cost_stone", 20, TVAR_SAVE);
static CVar v_num("farm_num", 20, TVAR_SAVE); // pocet trtiny z urody
static CVar v_build("farm_build", "1.4:K1+D1=0.011", TVAR_SAVE); // recept pro staveni

////////////////////////////////////////////////////////////
Farm::Farm(IHoeScene * scn) : ProductionBuilding(scn, v_build), m_cane(EBS_Cane)
{
	SetModel((IHoeModel*)GetResMgr()->ReqResource(model_FARM));
	//SetRingParam(4.5f,3.5f,2.f);
	m_cane.SetOwner(this); CRR::Get()->Register(&m_cane);
	//worked = NULL;
	//rust = 0.f;
	//skliz = 0;
	memset(&m_growinfo,0, sizeof(m_growinfo));
	m_growinfo.model = (IHoeModel*)GetResMgr()->ReqResource(model_FARM_POLE);
	m_growinfo.pos.Translate(0, -5.f,0);
	GetCtrl()->Link(THoeSubObject::Object, &m_growinfo);
	m_grow = 0.f;
	m_growinfo.pos.Translate(0,5.f * m_grow - 5.f,0);
	m_work = NULL;
}

bool Farm::Save(ChunkDictWrite &w)
{
	ProductionBuilding::Save(w);
	// ulozit rust
	/*m_cane.Save(w);
	w.Write<float>(m_grow);
	w.WriteValue<uint>(m_work ? m_work->GetID():0);
	*/
	return true;
}

bool Farm::Load(const ChunkDictRead &r)
{
	ProductionBuilding::Load(r);
	//m_cane.Load(r);
	//m_grow = r.Read<float>();
	//m_growinfo.pos.Translate(0,5.f * m_grow - 5.f,0);
	//uint it = r.Read<uint>();
	//m_work = dynamic_cast<Troll*>(GetLevel()->GetObjFromID(it));
	// 
	OnUpdateSur();
	return true;
}

int Farm::GetInfo(int type, char * str, size_t n)
{
	register int ret = 0;
	if (type==BINFO_Custom && str)
	{
		type = DefaultCustomInfo(str);
	}
	switch (type)
	{
	case BINFO_Production:
		ret = int(this->m_grow * 100.f);
		break;
	case BINFO_NumSur|EBS_Cane:
		ret = int(m_cane.GetNum());
		break;
	default:
		return ProductionBuilding::GetInfo(type, str, n);
	};

	if (str)
		snprintf(str, n, "%d", ret);
	return ret;
}

int Farm::GameMsg(int msg, int par1, void * par2, uint npar2)
{
#ifndef BECHER_EDITOR
	switch (msg)
	{
	case BMSG_Select:
		Select();
		break;
	case BMSG_GetSur: {
		PAR_Load * l = (PAR_Load *)par2;
		if (l->sur == EBS_Cane)
		{
			m_cane.Unlock(par1);
			return this->m_cane.Get(l->num, true);
		}
		} break;
	case BMSG_LockSur: {
			PAR_Favour * f = (PAR_Favour *)par2;
			if (f->sur == EBS_Cane)
				return m_cane.Lock(f->num);
			break;
		} 
	}
#endif
	return BecherBuilding::GameMsg(msg, par1, par2, npar2);
}

#ifndef BECHER_EDITOR

void Farm::Update(const float dtime)
{
	BecherObject::Update(dtime);
	m_grow += v_speed.GetFloat() * dtime;
	if (m_grow >= 1.f)
	{
		m_cane.SetNum(v_num.GetInt());
		m_grow = 0.f;
	}
	m_growinfo.pos.Translate(0,5.f * m_grow - 5.f,0);
}

bool Farm::Select()
{
	//ProductionBuilding::Select();
	GetLevel()->GetPanel()->SetObjectHud("scripts/farm.menu",this);
	GetLua()->func("s_farma");
	return true;
}

#else // BECHER_OBJECT
void Farm::Update(const float t)
{
}

bool Farm::Select()
{
	//ProductionBuilding::Select();
	return true;
}

#endif // BECHER_OBJECT



