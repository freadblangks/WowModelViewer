#include "animcontrol.h"

#include <wx/wx.h>

#include "logger/Logger.h"
#include "CASCFolder.h"
#include "FileTreeItem.h"
#include "globalvars.h"
#include "GameDatabase.h"
#include "util.h"
#include "UserSkins.h"

IMPLEMENT_CLASS(AnimControl, wxWindow)

BEGIN_EVENT_TABLE(AnimControl, wxWindow)
	EVT_COMBOBOX(ID_ANIM, AnimControl::OnAnim)
	EVT_COMBOBOX(ID_ANIM_SECONDARY, AnimControl::OnAnim)
	EVT_TEXT_ENTER(ID_ANIM_SECONDARY_TEXT, AnimControl::OnButton)
	EVT_COMBOBOX(ID_ANIM_MOUTH, AnimControl::OnAnim)

	EVT_COMBOBOX(ID_LOOPS, AnimControl::OnLoop)
	EVT_COMBOBOX(ID_SKIN, AnimControl::OnSkin)
	EVT_COMBOBOX(ID_ITEMSET, AnimControl::OnItemSet)

	EVT_CHECKBOX(ID_OLDSTYLE, AnimControl::OnCheck)
	EVT_CHECKBOX(ID_ANIM_LOCK, AnimControl::OnCheck)
	EVT_CHECKBOX(ID_ANIM_NEXT, AnimControl::OnCheck)

	EVT_BUTTON(ID_PLAY, AnimControl::OnButton)
	EVT_BUTTON(ID_PAUSE, AnimControl::OnButton)
	EVT_BUTTON(ID_STOP, AnimControl::OnButton)
	EVT_BUTTON(ID_ADDANIM, AnimControl::OnButton)
	EVT_BUTTON(ID_CLEARANIM, AnimControl::OnButton)
	EVT_BUTTON(ID_PREVANIM, AnimControl::OnButton)
	EVT_BUTTON(ID_NEXTANIM, AnimControl::OnButton)

	EVT_SLIDER(ID_SPEED, AnimControl::OnSliderUpdate)
	EVT_SLIDER(ID_SPEED_MOUTH, AnimControl::OnSliderUpdate)
	EVT_SLIDER(ID_FRAME, AnimControl::OnSliderUpdate)
END_EVENT_TABLE()

AnimControl::AnimControl(wxWindow* parent, wxWindowID id)
{
	LOG_INFO << "Creating Anim Control...";

	if(Create(parent, id, wxDefaultPosition, wxSize(700,120), 0, wxT("AnimControlFrame")) == false) {
		wxMessageBox(wxT("Failed to create a window for our AnimControl!"), wxT("Error"));
		LOG_ERROR << "Failed to create a window for our AnimControl!";
		return;
	}

	const wxString strLoops[10] = {wxT("0"), wxT("1"), wxT("2"), wxT("3"), wxT("4"), wxT("5"), wxT("6"), wxT("7"), wxT("8"), wxT("9")};
	
	animCList = new wxComboBox(this, ID_ANIM, _("Animation"), wxPoint(10,10), wxSize(150,16), 0, NULL, wxCB_READONLY|wxCB_SORT, wxDefaultValidator, wxT("Animation")); //|wxCB_SORT); //wxPoint(66,10)
	animCList2 = new wxComboBox(this, ID_ANIM_SECONDARY, _("Secondary"), wxPoint(10,95), wxSize(150,16), 0, NULL, wxCB_READONLY|wxCB_SORT, wxDefaultValidator, wxT("Secondary")); //|wxCB_SORT); //wxPoint(66,10)
	animCList2->Enable(false);
	animCList2->Show(false);

	lockText = new wxTextCtrl(this, ID_ANIM_SECONDARY_TEXT, wxEmptyString, wxPoint(300, 64), wxSize(20, 20), wxTE_PROCESS_ENTER, wxDefaultValidator);
	lockText->SetValue(wxString::Format(wxT("%d"), UPPER_BODY_BONES));
	lockText->Enable(false);
	lockText->Show(false);

	// Our hidden head/mouth related controls
	animCList3 = new wxComboBox(this, ID_ANIM_MOUTH, _("Mouth"), wxPoint(170,95), wxSize(150,16), 0, NULL, wxCB_READONLY|wxCB_SORT, wxDefaultValidator, wxT("Secondary")); //|wxCB_SORT); //wxPoint(66,10)
	animCList3->Enable(false);
	animCList3->Show(false);

	//btnPauseMouth = new wxButton(this, ID_PAUSE_MOUTH, wxT("Pause"), wxPoint(160,100), wxSize(45,20));
	//btnPauseMouth->Show(false);

	speedMouthLabel = new wxStaticText(this, -1, wxT("Speed: 1.0x"), wxPoint(340,95), wxDefaultSize);
	speedMouthLabel->Show(false);

	speedMouthSlider = new wxSlider(this, ID_SPEED_MOUTH, 10, 0, 40, wxPoint(415,95), wxSize(100,38), wxSL_AUTOTICKS);
	speedMouthSlider->SetTickFreq(10, 1);
	speedMouthSlider->Show(false);

	// ---

	loopList = new wxComboBox(this, ID_LOOPS, wxT("0"), wxPoint(330, 10), wxSize(40,16), 10, strLoops, wxCB_READONLY, wxDefaultValidator, wxT("Loops")); //|wxCB_SORT); //wxPoint(66,10)
	btnAdd = new wxButton(this, ID_ADDANIM, _("Add"), wxPoint(380, 10), wxSize(45,20));

	skinList = new wxComboBox(this, ID_SKIN, _("Skin"), wxPoint(170,10), wxSize(150,16), 0, NULL, wxCB_READONLY);
	skinList->Show(FALSE);
	randomSkins = true;
	defaultDoodads = true;

	wmoList = new wxComboBox(this, ID_ITEMSET, _("Item set"), wxPoint(220,10), wxSize(128,16), 0, NULL, wxCB_READONLY);
	wmoList->Show(FALSE);
	wmoLabel = new wxStaticText(this, -1, wxEmptyString, wxPoint(10,15), wxSize(192,16));
	wmoLabel->Show(FALSE);

	speedSlider = new wxSlider(this, ID_SPEED, 10, 1, 40, wxPoint(490,56), wxSize(100,38), wxSL_AUTOTICKS);
	speedSlider->SetTickFreq(10, 1);
	speedLabel = new wxStaticText(this, -1, wxT("Speed: 1.0x"), wxPoint(490,40), wxDefaultSize);

	frameLabel = new wxStaticText(this, -1, wxT("Frame: 0"), wxPoint(330,40), wxDefaultSize);
	frameSlider = new wxSlider(this, ID_FRAME, 1, 1, 10, wxPoint(330,56), wxSize(160,38), wxSL_AUTOTICKS);
	frameSlider->SetTickFreq(2, 1);

	btnPlay = new wxButton(this, ID_PLAY, _("Play"), wxPoint(10,40), wxSize(45,20));
	btnPause = new wxButton(this, ID_PAUSE, _("Pause"), wxPoint(62,40), wxSize(45,20));
	btnStop = new wxButton(this, ID_STOP, _("Stop"), wxPoint(115,40), wxSize(45,20));
	
	btnClear = new wxButton(this, ID_CLEARANIM, _("Clear"), wxPoint(10,64), wxSize(45,20));
	btnPrev = new wxButton(this, ID_PREVANIM, wxT("<<"), wxPoint(62,64), wxSize(45,20));
	btnNext = new wxButton(this, ID_NEXTANIM, wxT(">>"), wxPoint(115,64), wxSize(45,20));
	
	lockAnims = new wxCheckBox(this, ID_ANIM_LOCK, _("Lock Animations"), wxPoint(170,64), wxDefaultSize, 0);
	bLockAnims = true;
	lockAnims->SetValue(bLockAnims);

	oldStyle = new wxCheckBox(this, ID_OLDSTYLE, _("Auto Animate"), wxPoint(170,40), wxDefaultSize, 0);
	bOldStyle = true;
	oldStyle->SetValue(bOldStyle);
	nextAnims = new wxCheckBox(this, ID_ANIM_NEXT, _("Next Animations"), wxPoint(430,10), wxDefaultSize, 0);
	bNextAnims = false;
	nextAnims->SetValue(bNextAnims);

}

AnimControl::~AnimControl()
{
	// Free the memory the was allocated (fixed: memory leak)
	for (size_t i=0; i<skinList->GetCount(); i++) {
		TextureGroup *grp = (TextureGroup *)skinList->GetClientData((unsigned int)i);
		wxDELETE(grp);
	}

	animCList->Clear();
	animCList2->Clear();
	animCList3->Clear();
	skinList->Clear();
	
	animCList->Destroy();
	animCList2->Destroy();
	animCList3->Destroy();
	skinList->Destroy();
}

void AnimControl::UpdateModel(WoWModel *m)
{
  if (!m)
    return;

  // Clear skin/texture data from previous model - if there is any.
  if (g_selModel)
  {
    for (size_t i=0; i<skinList->GetCount(); i++)
    {
      TextureGroup *grp = (TextureGroup *)skinList->GetClientData((unsigned int)i);
      wxDELETE(grp);
    }
  }
  // --

  LOG_INFO << "Update model:" << m->itemName().c_str();

  g_selModel = m;

  selectedAnim = 0;
  selectedAnim2 = -1;
  selectedAnim3 = -1;

  animCList->Clear();
  animCList2->Clear();
  animCList3->Clear();

  skinList->Clear();

  ssize_t useanim = -1;

  // Find any textures that exist for the model
  bool res = false;

  wxString fn = m->itemName();
  fn = fn.Lower();
  LOG_INFO << __FUNCTION__ << fn.c_str();

  if (fn.substr(0,4) != wxT("char"))
  {
  	LOG_INFO << __FUNCTION__ << "NON CHAR";
    if (fn.substr(0,8) == wxT("creature"))
    {
    	LOG_INFO << __FUNCTION__ << "CREATURE";
      res = UpdateCreatureModel(m);
    }
    else if (fn.substr(0,4) == wxT("item"))
    {
    	LOG_INFO << __FUNCTION__ << "ITEM";
      res = UpdateItemModel(m);
    }
  }

  skinList->Show(res);

  // A small attempt at keeping the 'previous' animation that was selected when changing
  // the selected model via the model control.
  /*
	// Alfred 2009.07.19 keep currentAnim may crash others if it doesn't have, we should save the animID, not currentAnim
	if (g_selModel->currentAnim > 0)
		useanim = g_selModel->currentAnim;
   */

  /*
	if (g_selModel->charModelDetails.isChar) { // only display the "secondary" animation list if its a character
		animCList2->Select(useanim);
		animCList2->Show(true);
		lockAnims->Show(true);
		loopList->Show(true);
		btnAdd->Show(true);
	} else {
		animCList2->Show(false);
		lockAnims->Show(true);
		loopList->Show(false);
		btnAdd->Show(false);
	}
   */

  // Animation stuff
  LOG_INFO << __FUNCTION__ << m->animated << m->anims;
  if (m->animated && m->anims)
  {
    wxString strName;
    wxString strStand;
    int selectAnim = 0;

    map<int, string> animsVal = m->getAnimsMap();

    for (size_t i=0; i<m->header.nAnimations; i++)
    {
      std::stringstream label;
      label << animsVal[m->anims[i].animID];
      label << " [";
      label << i;
      label << "]";
      wxString strName=label.str().c_str();

      if (g_selModel->anims[i].animID == ANIM_STAND && useanim == -1)
      {
        strStand = strName;
        useanim = i;
      }

      animCList->Append(strName);
      animCList2->Append(strName);
      animCList3->Append(strName);
    }

    if (useanim != -1)
    {
      for(unsigned int i=0; i<animCList->GetCount(); i++)
      {
        strName = animCList->GetString(i);
        if (strName == strStand)
        {
          selectAnim = i;
          break;
        }
      }
    }

    if (useanim==-1)
      useanim = 0;
    //return;

    g_selModel->currentAnim = useanim; // anim position in anims
    animCList->Select(selectAnim); // anim position in selection
    animCList->Show(true);

    frameSlider->SetRange(g_selModel->anims[useanim].timeStart, g_selModel->anims[useanim].timeEnd);
    frameSlider->SetTickFreq(g_selModel->anims[useanim].playSpeed, 1);

    g_selModel->animManager->SetAnim(0, useanim, 0);
    if (bNextAnims && g_selModel)
    {
      int NextAnimation = useanim;
      for(size_t i=1; i<4; i++)
      {
        NextAnimation = g_selModel->anims[NextAnimation].NextAnimation;
        if (NextAnimation >= 0)
          g_selModel->animManager->AddAnim(NextAnimation, loopList->GetSelection());
        else
          break;
      }
    }
    g_selModel->animManager->Play();
  }
  wmoList->Show(false);
  wmoLabel->Show(false);
}

void AnimControl::UpdateWMO(WMO *w, int group)
{
	if (!w || w->itemName().size()==0)
		return;

	bool newwmo = (oldname != w->itemName());
	oldname = w->itemName();

	//Model *m = static_cast<Model*>(canvas->root->children[0]);

	//if (!m || m->anims==NULL)
	//	return;

	//m->animManager->Reset();
	g_selWMO = w;


	frameSlider->SetRange(0, 10);
	frameSlider->SetTickFreq(2, 1);
	
	animCList->Show(false);
	skinList->Show(false);
	loopList->Show(false);
	btnAdd->Show(false);
	
	if (newwmo) {
		// build itemset list
		wmoList->Clear();
		wmoList->Append(wxT("(No doodads)"));

		for (size_t i=0; i<g_selWMO->doodadsets.size(); i++) {
			wmoList->Append(wxString(g_selWMO->doodadsets[i].name, *wxConvCurrent));
		}

		int sel = defaultDoodads ? 1 : 0;
		g_selWMO->includeDefaultDoodads = defaultDoodads;
		wmoList->Select(sel);
		g_selWMO->showDoodadSet(sel-1);
	}
	wmoList->Show(TRUE);

	// get wmo name or current wmogroup name/descr
	if (group>=-1 && group<(int)g_selWMO->nGroups) {
		wxString label = w->itemName();
		label = label.AfterLast(MPQ_SLASH);
		if (group>=0) {
			label += wxT(" - ") + g_selWMO->groups[group].name;
			if (g_selWMO->groups[group].desc.length()) {
				label += wxT(" - ") + g_selWMO->groups[group].desc;
			}
		}
		wmoLabel->SetLabel(label);
	} else {
		wmoLabel->SetLabel(wxT("This group has been removed from the WMO"));
	}
	wmoLabel->Show(TRUE);
}

QString sFilterDir;
bool filterDir(QString fn)
{
	QString tmp = fn;
	tmp = tmp.toLower();
	return (tmp.startsWith(sFilterDir) && tmp.endsWith("blp"));
}


bool AnimControl::UpdateCreatureModel(WoWModel *m)
{
  std::set<wxString> alreadyUsedTextures;
  TextureSet skins;

  // see if this model has skins
  LOG_INFO << "Searching skins for" << m->itemName().c_str();

  wxString fn = m->itemName();

  // remove extension
  fn = fn.BeforeLast(wxT('.'));

  QString query = QString("SELECT Texture1, Texture2, Texture3, FileData.path FROM CreatureDisplayInfo \
		                   LEFT JOIN CreatureModelData ON CreatureDisplayInfo.ModelID = CreatureModelData.ID \
		                   LEFT JOIN FileData ON CreatureModelData.FileDataID = FileData.ID WHERE FileData.name LIKE \"%1\"").arg( wxString(m->itemName()).AfterLast(SLASH).c_str());

  sqlResult r = GAMEDATABASE.sqlQuery(query);

  if(r.valid && !r.values.empty())
  {
    for(size_t i = 0 ; i < r.values.size() ; i++)
    {
      TextureGroup grp;
      int count = 0;
      for (size_t skin=0; skin<TextureGroup::num; skin++)
      {
        if(!r.values[i][skin].isEmpty())
        {
          std::string texfullname = r.values[i][3].toStdString() + r.values[i][skin].toStdString() + ".blp";
          wxString texture(texfullname.c_str());
          alreadyUsedTextures.insert(texture.Lower());
          grp.tex[skin] = texture;
          count++;
        }
      }

      grp.base = TEXTURE_GAMEOBJECT1;
      grp.count = count;

      if(grp.tex[0].length() > 0 && std::find(skins.begin(),skins.end(),grp) == skins.end())
        skins.insert(grp);
    }
  }

  int count = (int)skins.size();

  LOG_INFO << "Found" << skins.size() << "skins (Database)";

  // if aready found some info from database, then check if model is a multi textured one
  // if it is the case, does not make sense to search for specific texture (as alone, rendering will be bad)
  // we search for file in folder onyl if :
  // - we didn't find anything yet (from database)
  // - we found something AND what we found only use 1 texture
  if(count == 0 || (count != 0 && (skins.begin())->count == 1))
  {
    // Search the same directory for BLPs
    std::set<FileTreeItem> filelist;
    sFilterDir = QString::fromStdString(m->itemName());
    sFilterDir.toLower();
    CASCFOLDER.filterFileList(filelist,filterDir);
    if (filelist.begin() != filelist.end())
    {
      TextureGroup grp;
      grp.base = TEXTURE_GAMEOBJECT1;
      grp.count = 1;
      for (std::set<FileTreeItem>::iterator it = filelist.begin(); it != filelist.end(); ++it)
      {
        wxString texname = (*it).displayName.toStdString();

        // use this alone texture only if not already used from database infos
        if(grp.tex[0].length() > 0 && std::find(alreadyUsedTextures.begin(),alreadyUsedTextures.end(),texname.Lower()) == alreadyUsedTextures.end())
        {
          grp.tex[0] =  texname;
          skins.insert(grp);
        }
      }
    }
  }
  bool ret = false;

  if (!skins.empty())
  {
    LOG_INFO << "Found" << skins.size() << "skins (total)";
    ret = FillSkinSelector(skins);

    if (count == 0) // No entries on .dbc and skins.txt
      count = (int)skins.size();

    if (ret)
    { // Don't call SetSkin without a skin
      int mySkin = randomSkins ? randint(0, (int)count-1) : 0;
      SetSkin(mySkin);
    }
  }

  return ret;
}

bool AnimControl::UpdateItemModel(WoWModel *m)
{
  std::set<wxString> alreadyUsedTextures;
  TextureSet skins;

  LOG_INFO << "Searching skins for" << m->itemName().c_str();

	wxString fn = m->itemName();

	// change M2 to mdx
	fn = fn.BeforeLast(wxT('.')) + wxT(".mdx");

	// Check to see if its a helmet model, if so cut off the race
	// and gender specific part of the filename off
	if (fn.Find(wxT("\\head\\")) > wxNOT_FOUND || fn.Find(wxT("\\Head\\")) > wxNOT_FOUND) {
		fn = fn.BeforeLast('_') + wxT(".mdx");
	}

	// just get the file name, exclude the path.
	fn = fn.AfterLast(MPQ_SLASH);
	
	// query textures for model1
	QString query= QString("SELECT DISTINCT path,name FROM ItemDisplayInfo  LEFT JOIN TextureFileData ON TextureItemID1 = TextureFileData.TextureItemID LEFT JOIN FileData ON TextureFileData.FileDataID = FileData.id WHERE Model1 = \"%1\"").arg(fn.mb_str());
	sqlResult r = GAMEDATABASE.sqlQuery(query);

	if(r.valid && !r.empty())
	{
	  for(size_t i = 0 ; i < r.values.size() ; i++)
	  {
	    TextureGroup grp;
	    grp.base = TEXTURE_ITEM;
	    grp.count = 1;
	    std::string tex = r.values[i][0].toStdString() + r.values[i][1].toStdString();
	    wxString skin = tex.c_str();
	    alreadyUsedTextures.insert(skin.Lower());
	    grp.tex[0] = skin;
	    if (grp.tex[0].length() > 0)
	      skins.insert(grp);
	  }
	}

	// do the same for model2
	query= QString("SELECT DISTINCT path,name FROM ItemDisplayInfo  LEFT JOIN TextureFileData ON TextureItemID2 = TextureFileData.TextureItemID LEFT JOIN FileData ON TextureFileData.FileDataID = FileData.id WHERE Model2 = \"%1\"").arg(fn.mb_str());
	r = GAMEDATABASE.sqlQuery(query);

	if(r.valid && !r.empty())
	{
	  for(size_t i = 0 ; i < r.values.size() ; i++)
	  {
	    TextureGroup grp;
	    grp.base = TEXTURE_ITEM;
	    grp.count = 1;
	    std::string tex = r.values[i][0].toStdString() + r.values[i][1].toStdString();
	    wxString skin = tex.c_str();
	    alreadyUsedTextures.insert(skin.Lower());
	    grp.tex[0] = skin;
	    if (grp.tex[0].length() > 0)
	      skins.insert(grp);
	  }
	}

	LOG_INFO << "Found" << skins.size() << "skins (Database)";

	// Search the same directory for BLPs
	std::set<FileTreeItem> filelist;
	sFilterDir = QString::fromStdString(m->itemName());
	sFilterDir.toLower();
	CASCFOLDER.filterFileList(filelist,filterDir);
	if (filelist.begin() != filelist.end())
	{
		TextureGroup grp;
		grp.base = TEXTURE_ITEM;
		grp.count = 1;
		for (std::set<FileTreeItem>::iterator it = filelist.begin(); it != filelist.end(); ++it)
		{
		  wxString texname = (*it).displayName.toStdString();
		  // use this alone texture only if not already used from database infos
		  if(grp.tex[0].length() > 0 && std::find(alreadyUsedTextures.begin(),alreadyUsedTextures.end(),texname.Lower()) == alreadyUsedTextures.end())
		  {
		    grp.tex[0] = texname;
		    skins.insert(grp);
		  }
		}
	}
	bool ret = false;

	if (!skins.empty())
	{
	  LOG_INFO << "Found" << skins.size() << "skins (Total)";
		ret = FillSkinSelector(skins);

		if (ret)
		{ // Don't call SetSkin without a skin
			int mySkin = randomSkins ? randint(0, (int)skins.size()-1) : 0;
			SetSkin(mySkin);
		}
	}

	return ret;
}


bool AnimControl::FillSkinSelector(TextureSet &skins)
{
	if (skins.size() > 0) {
		int num = 0;
		// fill our skin selector
		for (TextureSet::iterator it = skins.begin(); it != skins.end(); ++it) {
			wxString texname = it->tex[0];
			skinList->Append(texname.AfterLast(MPQ_SLASH).BeforeLast('.'));
			texname = g_selModel->itemName();
			texname = texname.BeforeLast(MPQ_SLASH) + MPQ_SLASH + texname + wxT(".blp");
			LOG_INFO << "Added" << texname.c_str() << "to the TextureList[" << g_selModel->TextureList.size() << "] via FillSkinSelector.";
			g_selModel->TextureList.push_back(texname.c_str());
			TextureGroup *grp = new TextureGroup(*it);
			skinList->SetClientData(num++, grp);
		}

		bool ret = (skins.size() > 0);
		//skins.clear();
		return ret;
	} else 
		return false;
}


void AnimControl::OnButton(wxCommandEvent &event)
{
	if (!g_selModel)
		return;

	int id = event.GetId();

	if (id == ID_PLAY) {
		g_selModel->currentAnim = g_selModel->animManager->GetAnim();
		g_selModel->animManager->Play();
	} else if (id == ID_PAUSE) {
		g_selModel->animManager->Pause();
	} else if (id == ID_STOP) {
		g_selModel->animManager->Stop();
	} else if (id == ID_CLEARANIM) {
		g_selModel->animManager->Clear();
	} else if (id == ID_ADDANIM) {
		g_selModel->animManager->AddAnim(selectedAnim, loopList->GetSelection());
	} else if (id == ID_PREVANIM) {
		g_selModel->animManager->PrevFrame();
		SetAnimFrame(g_selModel->animManager->GetFrame());
	} else if (id == ID_NEXTANIM) {
		g_selModel->animManager->NextFrame();
		SetAnimFrame(g_selModel->animManager->GetFrame());
	} else if (id == ID_ANIM_SECONDARY_TEXT) {
		int count = wxAtoi(lockText->GetValue());
		if (count < 0)
			count = UPPER_BODY_BONES;
		if (count > BONE_MAX)
			count = BONE_MAX;
		g_selModel->animManager->SetSecondaryCount(count);
	}
}

void AnimControl::OnCheck(wxCommandEvent &event)
{
	if (event.GetId() == ID_OLDSTYLE)
		bOldStyle = event.IsChecked();
	else if (event.GetId() == ID_ANIM_LOCK) {
		bLockAnims = event.IsChecked();

		if (bLockAnims == false) {
			animCList2->Enable(true);
			animCList2->Show(true);
			lockText->Enable(true);
			lockText->Show(true);
			animCList3->Enable(true);
			animCList3->Show(true);
			speedMouthSlider->Show(true);
			speedMouthLabel->Show(true);
			//btnPauseMouth->Show(true);
		} else {
			if (g_selModel)
				g_selModel->animManager->ClearSecondary();
			animCList2->Enable(false);
			animCList2->Show(false);
			lockText->Enable(false);
			lockText->Show(false);
			animCList3->Enable(false);
			animCList3->Show(false);
			speedMouthSlider->Show(false);
			speedMouthLabel->Show(false);
			//btnPauseMouth->Show(false);
		}
	} else if  (event.GetId() == ID_ANIM_NEXT) {
		bNextAnims = event.IsChecked();
		if (bNextAnims && g_selModel) {
			int NextAnimation = selectedAnim;
			for(size_t i=1; i<4; i++) {
				NextAnimation = g_selModel->anims[NextAnimation].NextAnimation;
				if (NextAnimation >= 0)
					g_selModel->animManager->AddAnim(NextAnimation, loopList->GetSelection());
				else
					break;
			}
		} else {
			g_selModel->animManager->SetCount(1);
		}
	}
}

void AnimControl::OnAnim(wxCommandEvent &event)
{
	if (event.GetId() == ID_ANIM) {
		if (g_selModel) {
			wxString val = animCList->GetValue();
			int first = val.Find('[')+1;
			int last = val.Find(']');
			selectedAnim = wxAtoi(val.Mid(first, last-first));
			
			if (bLockAnims) {
				//selectedAnim2 = -1;
				animCList2->SetSelection(event.GetSelection());
			}

			if (bOldStyle == true) {
				g_selModel->currentAnim = selectedAnim;
				g_selModel->animManager->Stop();
				g_selModel->animManager->SetAnim(0, selectedAnim, loopList->GetSelection());
				if (bNextAnims && g_selModel) {
					int NextAnimation = selectedAnim;
					for(size_t i=1; i<4; i++) {
						NextAnimation = g_selModel->anims[NextAnimation].NextAnimation;
						if (NextAnimation >= 0)
							g_selModel->animManager->AddAnim(NextAnimation, loopList->GetSelection());
						else
							break;
					}
				}
				g_selModel->animManager->Play();
				
				frameSlider->SetRange(g_selModel->anims[selectedAnim].timeStart, g_selModel->anims[selectedAnim].timeEnd);
				frameSlider->SetTickFreq(g_selModel->anims[selectedAnim].playSpeed, 1);
			}
		}

		//canvas->resetTime();
	} else if (event.GetId() == ID_ANIM_SECONDARY) {
		wxString val = animCList2->GetValue();
		int first = val.Find('[')+1;
		int last = val.Find(']');
		selectedAnim2 = wxAtoi(val.Mid(first, last-first));

		g_selModel->animManager->SetSecondary(selectedAnim2);
	} else if (event.GetId() == ID_ANIM_MOUTH) {
		wxString val = animCList3->GetValue();
		int first = val.Find('[')+1;
		int last = val.Find(']');
		selectedAnim3 = wxAtoi(val.Mid(first, last-first));

		//canvas->g_selModel->animManager->SetSecondary(selectedAnim2);
		g_selModel->animManager->SetMouth(event.GetSelection());
	}
}

void AnimControl::OnSkin(wxCommandEvent &event)
{
	if (g_selModel) {
		int sel = event.GetSelection();
		SetSkin(sel);
	}
}

void AnimControl::OnItemSet(wxCommandEvent &event)
{
	if (g_selWMO) {
		int sel = event.GetSelection();
		// -1 for no doodads
		g_selWMO->showDoodadSet(sel - 1);
	}
}

void AnimControl::OnSliderUpdate(wxCommandEvent &event)
{
	if (event.GetId() == ID_SPEED) {
		SetAnimSpeed(speedSlider->GetValue() / 10.0f);

	} else if (event.GetId() == ID_SPEED_MOUTH) {
		if (!g_selModel || !g_selModel->animManager)
			return;
		
		float speed = speedMouthSlider->GetValue() / 10.0f;
		g_selModel->animManager->SetMouthSpeed(speed);
		speedMouthLabel->SetLabel(wxString::Format(_("Speed: %.1fx"), speed));

	} else if (event.GetId() == ID_FRAME)
		SetAnimFrame(frameSlider->GetValue());

}

void AnimControl::OnLoop(wxCommandEvent &)
{
	if (bOldStyle == true) {
		g_selModel->animManager->Stop();
		g_selModel->animManager->SetAnim(0, selectedAnim, loopList->GetSelection());
		if (bNextAnims && g_selModel) {
			int NextAnimation = selectedAnim;
			for(size_t i=1; i<4; i++) {
				NextAnimation = g_selModel->anims[NextAnimation].NextAnimation;
				if (NextAnimation >= 0)
					g_selModel->animManager->AddAnim(NextAnimation, loopList->GetSelection());
				else
					break;
			}
		}
		g_selModel->animManager->Play();
	} 
}

void AnimControl::SetSkin(int num)
{
	TextureGroup *grp = (TextureGroup*) skinList->GetClientData(num);
	for (size_t i=0; i<grp->count; i++)
	{
		int base = grp->base + i;
		if (g_selModel->useReplaceTextures[base])
		{
			wxString skin = grp->tex[i];
			// refresh TextureList for further use
			for (ssize_t j=0; j<TEXTURE_MAX; j++)
			{
				if (base == g_selModel->specialTextures[j])
				{
					g_selModel->TextureList[j] = skin;
					break;
				}
			}
			g_selModel->replaceTextures[grp->base+i] = texturemanager.add(skin.c_str());
		}
	}

	skinList->Select(num);
}

int AnimControl::AddSkin(TextureGroup grp)
{
	skinList->Append(wxT("Custom"));
	int count = skinList->GetCount() - 1;
	TextureGroup *group = new TextureGroup(grp);
	skinList->SetClientData(count, group);
	SetSkin(count);
	return count;
}

void AnimControl::SetAnimSpeed(float speed)
{
	if (!g_selModel || !g_selModel->animManager)
		return;

	g_selModel->animManager->SetSpeed(speed);
	
	speedLabel->SetLabel(wxString::Format(_("Speed: %.1fx"), speed));
}

void AnimControl::SetAnimFrame(size_t frame)
{
	if (!g_selModel || !g_selModel->animManager)
		return;

	g_selModel->animManager->SetFrame(frame);
	
	size_t frameNum = (frame - g_selModel->anims[g_selModel->currentAnim].timeStart);

	frameLabel->SetLabel(wxString::Format(_("Frame: %i"), frameNum));
	frameSlider->SetValue((int)frame);
}
