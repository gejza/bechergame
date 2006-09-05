
#include "StdAfx.h"
#include "becher.h"
#include "game.h"
#include "troll.h"
#include "obj_store.h"

float getheight(IHoeModel * m);



bool Store::SetToWork(Troll * t)
{
	return false;
}

void Store::UnsetFromWork(Troll * t)
{
}

////////////////////////////////////////////////////////////
Store::Store(IHoeScene * scn) : BecherBuilding(scn), 
	m_stone(EBS_Stone), m_wood(EBS_Wood), m_sugar(EBS_Sugar), m_water(EBS_Water),
	m_becher(EBS_Becher), m_alcohol(EBS_Alco), m_cane(EBS_Cane)
{
	// set owners
	SetModel((IHoeModel*)GetResMgr()->ReqResource(ID_STORE));
	GetCtrl()->SetFlags(HOF_ADVSHOW);
	m_stone.SetOwner(this); CRR::Get()->Register(&m_stone);
	m_wood.SetOwner(this); CRR::Get()->Register(&m_wood);
	m_sugar.SetOwner(this); CRR::Get()->Register(&m_sugar);
	m_water.SetOwner(this); CRR::Get()->Register(&m_water);
	m_becher.SetOwner(this); CRR::Get()->Register(&m_becher);
	m_alcohol.SetOwner(this); CRR::Get()->Register(&m_alcohol);
	m_cane.SetOwner(this); CRR::Get()->Register(&m_cane);
}

void Store::AdvPaint(IHoePaint3D * h3)
{
	// info
	if (this->IsCurActive() || this->IsSelected())
	{
		static IHoeModel * mod_i = dynamic_cast<IHoeModel*>(GetEngine()->Create("model info"));
		h3->ResetPos();
		h3->Scale(2.5, 0, 2.5);
		h3->Move(0,2,0);
		h3->Paint(mod_i);
	}

	static IHoeModel * mod[] = {dynamic_cast<IHoeModel*>(GetEngine()->Create("model miska")),
		dynamic_cast<IHoeModel*>(GetEngine()->Create("model sud")),
		dynamic_cast<IHoeModel*>(GetEngine()->Create("model kblok")),
		dynamic_cast<IHoeModel*>(GetEngine()->Create("model bedna_becher")),
		dynamic_cast<IHoeModel*>(GetEngine()->Create("model bedna_cukr")),
		dynamic_cast<IHoeModel*>(GetEngine()->Create("model bedna_dul")),
		dynamic_cast<IHoeModel*>(GetEngine()->Create("model kmen")),
		dynamic_cast<IHoeModel*>(GetEngine()->Create("model vedro"))};
	static float p = getheight(this->GetModel());
	int m = 0;
	for (int i=0;i<4;i++)
		for(int j=0;j<4;j++)
		{
			IHoeModel * model = mod[(m++)%8];
			if (model)
			{
				h3->ResetPos();
				//h3->RotateY(GetEngine()->SysFloatTime()*2);
				h3->Move((float)8*i-12, p, (float)8*j-12);
				h3->Paint(model);
			}
		}

}

bool Store::Save(BecherGameSave &w)
{
	BecherBuilding::Save(w);
	w.WriteRI(m_stone);
	w.WriteRI(m_wood);
	w.WriteRI(m_sugar);
	w.WriteRI(m_water);
	w.WriteRI(m_becher);
	w.WriteRI(m_alcohol);
	return true;
}

bool Store::Load(BecherGameLoad &r)
{
	BecherBuilding::Load(r);
	r.ReadRI(m_stone);
	r.ReadRI(m_wood);
	r.ReadRI(m_sugar);
	r.ReadRI(m_water);
	r.ReadRI(m_becher);
	r.ReadRI(m_alcohol);
	uint n = 10000;
	m_cane.Add(&n,10000);
	return true;
}

#ifndef BECHER_EDITOR

ResourceExp * Store::EBSToPointer(ESurType type)
{
	switch (type)
	{
	case EBS_Becher:
		return &m_becher;
	case EBS_Cane:
		return &m_cane;
	case EBS_Sugar:
		return &m_sugar;
	case EBS_Alco:
		return &m_alcohol;
	case EBS_Stone:
		return &m_stone;
	case EBS_Wood:
		return &m_wood;
	case EBS_Water:
		return &m_water;
	default:
		assert(!"Unknown sur. type");
	}
	return NULL;
}

bool Store::InsertSur(ESurType type, uint *s)
{
	return EBSToPointer(type)->Add(s, 10000000);
}

void Store::Update(const double t)
{
}

int Store::GetStatus(ESurType type)
{
		if (type == EBS_Cane)
		return 100;
	return EBSToPointer(type)->GetNum();
}

bool Store::Idiot(Job *t)
{
	return false;
}

bool Store::Select()
{
	if (!IsBuildMode())
        GetLua()->func("s_sklad");
	return true;
}

#else
bool Store::Select()
{
	GetProp()->Begin(this);
	GetProp()->AppendCategory(_("Materials"));
	GetProp()->AppendLong(0, _("Wood"), m_wood.GetNum());
	GetProp()->AppendLong(1, _("Stone"), m_stone.GetNum());
	GetProp()->AppendLong(2, _("Water"), m_water.GetNum());
	GetProp()->AppendLong(3, _("Sugar"), m_sugar.GetNum());
	GetProp()->AppendLong(4, _("Alcohol"), m_alcohol.GetNum());
	GetProp()->AppendLong(5, _("Bechers"), m_becher.GetNum());
	GetProp()->AppendCategory(_("Store"));
	GetProp()->AppendLong(6, _("Limit"), 100);
	GetProp()->End();	
	return true;
}

void Store::OnChangeProp(int id, const HoeEditor::PropItem & pi)
{
	switch (id)
	{
	case 0:
		m_wood.SetNum(pi.GetLong());
		break;
	case 1:
		m_stone.SetNum(pi.GetLong());
		break;
	case 2:
		m_water.SetNum(pi.GetLong());
		break;
	case 3:
		m_sugar.SetNum(pi.GetLong());
		break;
	case 4:
		m_alcohol.SetNum(pi.GetLong());
		break;
	case 5:
		m_becher.SetNum(pi.GetLong());
		break;
	};
}



#endif


