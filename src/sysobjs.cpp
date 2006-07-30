
#include "StdAfx.h"
#include "becher.h"
#include "sysobjs.h"

SystemObjectSound::SystemObjectSound(IHoeScene * scn) : BecherSystemObject(scn) 
{
#ifdef BECHER_EDITOR
	SetModel(GetModelForSound());
#endif
}


bool SystemObjectSound::Load(BecherGameLoad &r)
{
	float x,y;
	x = r.Read<float>();
	y = r.Read<float>();
#ifdef BECHER_EDITOR
	this->SetPosition(x,y);
	this->Show(true);
#endif
	return true;
}

#ifndef BECHER_EDITOR

bool SystemObjectSound::Save(BecherGameSave &w)
{
	assert(!"this code not run");
	return true;
}

#else

bool SystemObjectSound::Save(BecherGameSave &w)
{
	// ulozit pozici
	w.WriteValue<float>(this->GetPosX());
	w.WriteValue<float>(this->GetPosY());
	return true;
}

IHoeModel * SystemObjectSound::GetModelForSound()
{
	static IHoeModel * mod = NULL;
	if (mod == NULL)
	{
		mod = (IHoeModel *)GetEngine()->Create("generate model box 10");
	}
	return mod;
}

bool SystemObjectSound::Select()
{
	GetProp()->Begin(this);
	GetProp()->AppendCategory(_("Sound"));
	GetProp()->AppendBool(0, _("Loop"), false);
	GetProp()->AppendCategory(_("Test"));
	GetProp()->AppendBool(0, _("Play"), false);
	GetProp()->End();	
	return true;
}

void SystemObjectSound::OnChangeProp(int id, const HoeEditor::PropItem & pi)
{
}

#endif
