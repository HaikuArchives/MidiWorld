// ***********************************************
// Brendan Allen  -- darkmoon96@tcon.net
// Copyright 1999
// You can use this code freely in your program
// as long as you give credit to the author (ie
// me) somewhere in the documentation or the about
// box of the application AND leave these comments
// in all of the files you use.
// ***********************************************

// DeskbarView.cpp
//This should be a simplied version of MidiView.cpp because
//we are going to have this in the deskbar which will have
//less features than the application
//Lets see we will have: Transpose
//Tempo only by certain percents -- maybe 4 or 5 choices
//Play, stop, pause, resume
//Feature here not in regular app -- Will be able to switch playlists
// in a menu for all the ones found on Be partitions
//When you click on the icon it bring up one menu that will
//list all the midis and have submenus of other stuff
//If they select a midi it will play =)

#include "DeskbarView.h"
#include "MidiView.h"
#include "constants.h"
#include <Deskbar.h>
#include <Mime.h> //for MimeType class
#include <Screen.h>
#include <Dragger.h>
#include <MenuItem.h>
#include <MenuField.h>
#include <Alert.h>
#include <string.h> //for strcmp
#include <String.h> //for BString
#include <stdio.h>
#include <StorageKit.h>
#include <Roster.h>

const char* DESKBAR_SIGNATURE = "application/x-vnd.Be-TSKB";


int32 load_db_prefs(void *data);

int32 load_db_prefs(void *data)
{
	DeskbarView *daView = (DeskbarView *)data;
	if(daView->LockLooper())
	{	daView->LoadPrefs();
		daView->AddPlaylists();
		daView->SavePrefs();
		daView->UnlockLooper();
		return B_OK;
	}
	else
		return B_ERROR;
}

DeskbarView::DeskbarView(BRect frame)
		: BView(frame, "MidiDeskbarView",B_FOLLOW_ALL, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE| B_WILL_ACCEPT_FIRST_CLICK | B_PULSE_NEEDED)
{
	regular=true;
	DBMenu = new BPopUpMenu("DeskbarMenu",false);
	InitStuff();
	BMenuItem *about=new BMenuItem("About MidiWorld Replicant!", new BMessage(B_ABOUT_REQUESTED));
	BMenuItem *add_me=new BMenuItem("Add to Deskbar", new BMessage(DB_ADD));
	DBMenu->AddItem(about);
	DBMenu->AddItem(add_me);
	BRect rect(Bounds());
	rect.OffsetTo(B_ORIGIN);
	rect.top = rect.bottom - 7;
	rect.left = rect.right - 7;
	AddChild(new BDragger(rect, this));
}

DeskbarView::DeskbarView(BMessage *message) : BView(message)
{
	regular=false;
	//create menu and add stuff it
	find_directory(B_USER_SETTINGS_DIRECTORY, &last_playlist);
	last_playlist.Append("MidiWorld/Default");
	InitStuff();//nonspecific to the BMessage version
	InitMoreStuff();
	BMenuItem *remove_me=new BMenuItem("Remove from Deskbar", new BMessage(DB_REMOVE));
	DBMenu->AddItem(remove_me);
	LoadPrefs();
	AddPlaylists();
	SavePrefs();
}

void DeskbarView::InitStuff()
{
	paused = false;
	BRect buttonRect(0, 0, 15, 15);
	icon = new BBitmap(buttonRect, B_CMAP8);
	BMimeType mime("application/x-vnd.ISLE-BUG.MidiWorld");
	mime.GetIcon(icon, B_MINI_ICON);
}

void DeskbarView::InitMoreStuff()
{
	//create the main menu item
	be_synth->SetSynthVolume(1.0);
	double scale_by = (double)(2/2.8);
	be_synth->SetSynthVolume(scale_by);
	play = new BMenu("Play");
	play->SetRadioMode(true);
	stop = new BMenuItem("Stop", new BMessage(DB_STOP));
	stop->SetEnabled(false);
	pause=new BMenuItem("Pause",new BMessage(DB_PAUSE));
	pause->SetEnabled(false);
	BMenuItem *open_pl=new BMenuItem("Open Playlist", new BMessage(OPEN_PLAYLIST));
	BMenuItem *add_midis=new BMenuItem("Add Midis", new BMessage(OPEN_PLAYLIST));
	BMenuItem *launch_mw=new BMenuItem("Launch MidiWorld!", new BMessage(DB_LAUNCH_MW));
	BMenuItem *about=new BMenuItem("About MidiWorld Replicant!", new BMessage(B_ABOUT_REQUESTED));
	//add the tempo menu
	tempo_m=new BMenu("Tempo");//have 50%, 150%, %200, %250, %300
	tempo_m->SetRadioMode(true);
	BMenuItem *treset=new BMenuItem("Normal", new BMessage(DB_T_RESET));
	treset->SetMarked(true);
	BMenuItem *t50=new BMenuItem("50%", new BMessage(DB_T50));
	BMenuItem *t150=new BMenuItem("150%", new BMessage(DB_T150));
	BMenuItem *t200=new BMenuItem("200%", new BMessage(DB_T200));
	BMenuItem *t250=new BMenuItem("250%", new BMessage(DB_T250));
	BMenuItem *t300=new BMenuItem("300%", new BMessage(DB_T300));
	tempo_m->AddItem(treset);
	tempo_m->AddItem(t50);
	tempo_m->AddItem(t150);
	tempo_m->AddItem(t200);
	tempo_m->AddItem(t250);
	tempo_m->AddItem(t300);
	//add transpose menu
	transpose=new BMenu("Transpose"); //up, down, reset
	BMenuItem *tr_up=new BMenuItem("Up", new BMessage(DB_TR_UP));
	BMenuItem *tr_down=new BMenuItem("Down", new BMessage(DB_TR_DOWN));
	BMenuItem *tr_reset=new BMenuItem("Reset", new BMessage(DB_TR_RESET));
	transpose->AddItem(tr_up);
	transpose->AddItem(tr_down);
	transpose->AddItem(tr_reset);
	transpose->SetEnabled(false);
	//add reverb menu
	reverb=new BMenu("Reverb");
	reverb->SetRadioMode(true);
	//no reverb, closet, garage, ballroom, cavern, dungeon
	BMenuItem *none=new BMenuItem("No reverb", new BMessage(DB_RV_NONE));
	none->SetMarked(true);
	BMenuItem *closet=new BMenuItem("Closet", new BMessage(DB_RV_CLOSET));
	BMenuItem *garage=new BMenuItem("Garage", new BMessage(DB_RV_GARAGE));
	BMenuItem *ballroom=new BMenuItem("Ballroom", new BMessage(DB_RV_BALLROOM));
	BMenuItem *cavern=new BMenuItem("Cavern", new BMessage(DB_RV_CAVERN));
	BMenuItem *dungeon=new BMenuItem("Dungeon", new BMessage(DB_RV_DUNGEON));
	reverb->AddItem(none);
	reverb->AddItem(closet);
	reverb->AddItem(garage);
	reverb->AddItem(ballroom);
	reverb->AddItem(cavern);
	reverb->AddItem(dungeon);
	//add volume
	volume=new BMenu("Volume");
	volume->SetRadioMode(true);
	BMenuItem *db_v1=new BMenuItem("1", new BMessage(DB_V_1));
	BMenuItem *db_v2=new BMenuItem("2", new BMessage(DB_V_2));
	db_v2->SetMarked(true);
	BMenuItem *db_v3=new BMenuItem("3", new BMessage(DB_V_3));
	BMenuItem *db_v4=new BMenuItem("4", new BMessage(DB_V_4));
	BMenuItem *db_v5=new BMenuItem("5", new BMessage(DB_V_5));
	BMenuItem *db_v6=new BMenuItem("6", new BMessage(DB_V_6));
	BMenuItem *db_v7=new BMenuItem("7", new BMessage(DB_V_7));
	volume->AddItem(db_v1);
	volume->AddItem(db_v2);
	volume->AddItem(db_v3);
	volume->AddItem(db_v4);
	volume->AddItem(db_v5);
	volume->AddItem(db_v6);
	volume->AddItem(db_v7);
	//add everything to the pop up menu
	DBMenu = new BPopUpMenu("DeskbarMenu",false);
	DBMenu->AddItem(play);
	DBMenu->AddItem(stop);
	DBMenu->AddItem(pause);
	DBMenu->AddSeparatorItem();
	DBMenu->AddItem(open_pl);
	DBMenu->AddItem(add_midis);
	DBMenu->AddSeparatorItem();
	DBMenu->AddItem(about);
	DBMenu->AddItem(launch_mw);
	DBMenu->AddSeparatorItem();
	DBMenu->AddItem(volume);
	DBMenu->AddItem(tempo_m);
	tempo_m->SetEnabled(false);
	DBMenu->AddItem(reverb);
	DBMenu->AddItem(transpose);
	DBMenu->AddSeparatorItem();
}

DeskbarView::~DeskbarView()
{
	if(regular==false)
	{	//SavePrefs();
		CurrentPlayingMidi.Fade();
		CurrentPlayingMidi.UnloadFile();
	}
}

void DeskbarView::AttachedToWindow(void)
{
	//set the target for the menus to this view
	if(regular==false)
	{
		play->SetTargetForItems(this);
		tempo_m->SetTargetForItems(this);
		reverb->SetTargetForItems(this);
		transpose->SetTargetForItems(this);
		volume->SetTargetForItems(this);
	}
	DBMenu->SetTargetForItems(this);
	DBMenu->SetRadioMode(false);

	if (Parent())
		SetViewColor(Parent()->ViewColor());

	BView::AttachedToWindow();
}

DeskbarView *DeskbarView::Instantiate(BMessage *data)
{
	if(!validate_instantiation(data, "DeskbarView"))
		return NULL;
	return new DeskbarView(data);
}

status_t DeskbarView::Archive(BMessage *data, bool deep) const
{
	BView::Archive(data, deep);
	data->AddString("class", "DeskbarView");
	data->AddString("add_on", "application/x-vnd.ISLE-BUG.MidiWorld");
	return B_OK;
}

void DeskbarView::MouseDown(BPoint point)
{
	//display the menu here
	int32 buttons = 0;

	Window()->CurrentMessage()->FindInt32("buttons", &buttons);

	GetMouse(&point,(uint32 *)&buttons, true);
	if(buttons==B_PRIMARY_MOUSE_BUTTON || buttons==B_SECONDARY_MOUSE_BUTTON)
	{
		ConvertToScreen(&point);
		point.x-=1;
		point.y-=1;
		DBMenu->Go(point,true);
	}
}

void DeskbarView::OpenPlaylist()
{
	BFilePanel *OpenObject=new BFilePanel(B_OPEN_PANEL,new BMessenger(this));
	OpenObject->Show();
}

void DeskbarView::AddPlaylists()
{
	status_t err;
	BFile *pFile = new BFile();
	err = pFile->SetTo(last_playlist.Path(), B_READ_WRITE);
	//Now we check to see if it is there, if it isn't then we don't want to
	//do anything else, or if the list is not empty that means that
	//someone opened another playlist we don't want both lists
	if(err == B_ENTRY_NOT_FOUND)
	{
		delete pFile;
		return;
	}
	OpenFile(pFile);
}

void DeskbarView::OpenFile(BFile *pFile)
{
	//Now the entry_ref will be used to get each one in the Flattened
	//BMessage which we did above
	entry_ref midiRef;
	BMessage message;
	BMenu *da_list=play;
	//Now when we unflatten the file we now have the
	//BMessage we stored above in SavePreferences()
	message.Unflatten(pFile);
	//Now lets get all the midi's and add them to our list!
	int32 i = 0;
	while (message.FindRef("midis", i++, &midiRef) == B_OK)
	{
		if(BEntry(&midiRef).Exists())
		{
			BMessage *play_msg = new BMessage(DB_PLAY);
			play_msg->AddRef("midi", &midiRef);
			BMenuItem *bla = new BMenuItem(midiRef.name, play_msg);
			bla->SetTarget(this);
			da_list->AddItem(bla);
		}
	}
}

void DeskbarView::Draw(BRect rect)
{
	//just draw the icon here? probably
	BView::Draw(rect);
	SetDrawingMode(B_OP_OVER);
	DrawBitmap(icon);
}

void DeskbarView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case B_REFS_RECEIVED:
		{
			int i=0;
			entry_ref my_midi_ref;
			while (message->FindRef("refs", i++, &my_midi_ref) == B_OK)
	    	{
				//check to see if the midiplaylist exists first
				if(BEntry(&my_midi_ref).Exists())
				{
					BNode mynode(&my_midi_ref);
					char type[B_MIME_TYPE_LENGTH];
					(new BNodeInfo(&mynode))->GetType(type);
					BMimeType mime(type);
					if(mime == "text/x-midiworld-playlist")
					{
						DBMenu->RemoveItem(play);
						delete play;
						play = new BMenu("Play");
						play->SetRadioMode(true);
						play->SetTargetForItems(this);
						DBMenu->AddItem(play, 0);
						last_playlist.SetTo(new BEntry(&my_midi_ref));
						SavePrefs();
						OpenFile(new BFile(&my_midi_ref, B_READ_ONLY));
					}
					else if (mime=="audio/midi" || mime=="audio/x-midi")
					{
						BMessage *play_msg = new BMessage(DB_PLAY);
						play_msg->AddRef("midi", &my_midi_ref);
						BMenuItem *bla = new BMenuItem(my_midi_ref.name, play_msg);
						bla->SetTarget(this);
						play->AddItem(bla);
					}
					else
						(new BAlert("", "That is not a midi file/playlist or it is not the right mimetype.", "OK"))->Go();
				}
			}
			break;
		}
		case B_ABOUT_REQUESTED:
		{
			char bla[] = "MidiWorld! created by Brendan Allen\n"
								"Deskbar and Desktop replicant! Hope you enjoy it!";
			(new BAlert("", bla, "OK COOL!"))->Go();
			break;
		}
		case DB_PLAY:
		{
			entry_ref midi_ref;
			message->FindRef("midi", &midi_ref);
			CurrentPlayingMidi.Fade();
			CurrentPlayingMidi.UnloadFile();
			if(CurrentPlayingMidi.LoadFile(&midi_ref) == B_OK)
			{
				tempo = CurrentPlayingMidi.Tempo();
				CurrentPlayingMidi.Start();
				tempo_m->SetEnabled(true);
				transpose->SetEnabled(true);
				pause->SetEnabled(true);
				stop->SetEnabled(true);
			}
			break;
		}
		case DB_STOP:
		{
			CurrentPlayingMidi.Fade();
			tempo_m->SetEnabled(false);
			transpose->SetEnabled(false);
			play->FindMarked()->SetMarked(false);
			pause->SetEnabled(false);
			stop->SetEnabled(false);
			break;
		}
		case DB_PAUSE:
		{
			if(paused == false)
			{
				CurrentPlayingMidi.Pause();
				pause->SetLabel("Resume");
			}
			else
			{
				CurrentPlayingMidi.Resume();
				pause->SetLabel("Pause");
			}

			paused = !paused;
			break;
		}
		//tempo menu stuff
		case DB_T_RESET:
		{
			CurrentPlayingMidi.SetTempo(tempo);
			CurrentPlayingMidi.ScaleTempoBy(1.0);
			break;
		}
		case DB_T50:
		{
			CurrentPlayingMidi.SetTempo(tempo);
			CurrentPlayingMidi.ScaleTempoBy(0.5);
			break;
		}
		case DB_T150:
		{
			CurrentPlayingMidi.SetTempo(tempo);
			CurrentPlayingMidi.ScaleTempoBy(1.5);
			break;
		}
		case DB_T200:
		{
			CurrentPlayingMidi.SetTempo(tempo);
			CurrentPlayingMidi.ScaleTempoBy(2.0);
			break;
		}
		case DB_T250:
		{
			CurrentPlayingMidi.SetTempo(tempo);
			CurrentPlayingMidi.ScaleTempoBy(2.5);
			break;
		}
		case DB_T300:
		{
			CurrentPlayingMidi.SetTempo(tempo);
			CurrentPlayingMidi.ScaleTempoBy(3.0);
			break;
		}
		//transpose stuff
		case DB_TR_UP:
		{
			if(CurrentPlayingMidi.Transposition() >= 12)
				return;
			CurrentPlayingMidi.SetTransposition(CurrentPlayingMidi.Transposition()+1);
			break;
		}
		case DB_TR_DOWN:
		{
			if(CurrentPlayingMidi.Transposition() <= -12)
				return;
			CurrentPlayingMidi.SetTransposition(CurrentPlayingMidi.Transposition()-1);
			break;
		}
		case DB_TR_RESET:
		{
			CurrentPlayingMidi.SetTransposition(0);
			break;
		}
		//reverb stuff
		case DB_RV_NONE:
		{
			be_synth->EnableReverb(false);
			break;
		}
		case DB_RV_DUNGEON:
		{
			if(be_synth->IsReverbEnabled()==false)
				be_synth->EnableReverb(true);
			be_synth->SetReverb(B_REVERB_DUNGEON);
			break;
		}
		case DB_RV_CAVERN:
		{
			if(be_synth->IsReverbEnabled()==false)
				be_synth->EnableReverb(true);
			be_synth->SetReverb(B_REVERB_CAVERN);
			break;
		}
		case DB_RV_BALLROOM:
		{
			if(be_synth->IsReverbEnabled()==false)
				be_synth->EnableReverb(true);
			be_synth->SetReverb(B_REVERB_BALLROOM);
			break;
		}
		case DB_RV_GARAGE:
		{
			if(be_synth->IsReverbEnabled()==false)
				be_synth->EnableReverb(true);
			be_synth->SetReverb(B_REVERB_GARAGE);
			break;
		}
		case DB_RV_CLOSET:
		{
			if(be_synth->IsReverbEnabled()==false)
				be_synth->EnableReverb(true);
			be_synth->SetReverb(B_REVERB_CLOSET);
			break;
		}
		case OPEN_PLAYLIST:
		{
			OpenPlaylist();
			SavePrefs();
			break;
		}
		case DB_REMOVE:
		{
			BDeskbar db;
			db.RemoveItem("MidiDeskbarView");
			break;
		}
		case DB_ADD:
		{
			MidiView *bla=(MidiView *)Parent();
			bla->SaveDefault();
			install_view1();
		}
		case DB_LAUNCH_MW:
		{
			be_roster->Launch("application/x-vnd.ISLE-BUG.MidiWorld");
			break;
		}
		case DB_V_1:
		{
			be_synth->SetSynthVolume(1.0);
			double scale_by = (double)(1/2.8);
			be_synth->SetSynthVolume(scale_by);
			break;
		}
		case DB_V_2:
		{
			be_synth->SetSynthVolume(1.0);
			double scale_by = (double)(2/2.8);
			be_synth->SetSynthVolume(scale_by);
			break;
		}
		case DB_V_3:
		{
			be_synth->SetSynthVolume(1.0);
			double scale_by = (double)(3/2.8);
			be_synth->SetSynthVolume(scale_by);
			break;
		}
		case DB_V_4:
		{
			be_synth->SetSynthVolume(1.0);
			double scale_by = (double)(4/2.8);
			be_synth->SetSynthVolume(scale_by);
			break;
		}
		case DB_V_5:
		{
			be_synth->SetSynthVolume(1.0);
			double scale_by = (double)(5/2.8);
			be_synth->SetSynthVolume(scale_by);
			break;
		}
		case DB_V_6:
		{
			be_synth->SetSynthVolume(1.0);
			double scale_by = (double)(6/2.8);
			be_synth->SetSynthVolume(scale_by);
			break;
		}
		case DB_V_7:
		{
			be_synth->SetSynthVolume(1.0);
			double scale_by = (double)(7/2.8);
			be_synth->SetSynthVolume(scale_by);
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}

void DeskbarView::Pulse()
{
	if(regular==true)
		return;
	if(CurrentPlayingMidi.IsFinished() == true)
	{
		CurrentPlayingMidi.UnloadFile();
		BMenuItem *m_item;

		if(play->CountItems() < 1)
			return;
		m_item=play->FindMarked();
		if(m_item == NULL)
			return;

		if(play->IndexOf(m_item)+1 != play->CountItems())
		{
			m_item->SetMarked(false);	m_item=play->ItemAt(play->IndexOf(m_item)+1);
			m_item->SetMarked(true);
			//find the midi's path
			entry_ref midi;
			m_item->Message()->FindRef("midi", &midi);
			CurrentPlayingMidi.LoadFile(&midi);
			tempo = CurrentPlayingMidi.Tempo();
			//find the current tempo item selected and post the message
			//this next line of code is genius isn't it? =)
			Looper()->PostMessage(tempo_m->FindMarked()->Message(), this);
			//lets start playing and tell the view and the item we are playing
			CurrentPlayingMidi.Start();
			paused=false;
		}
		else
		{	//no more files to play start at beginning?
			//Set it to beginning, get the first item and the item that was playing
			m_item->SetMarked(false);	m_item=play->ItemAt(0);
			m_item->SetMarked(true);
			entry_ref midi;
			m_item->Message()->FindRef("midi", &midi);
			CurrentPlayingMidi.LoadFile(&midi);
			tempo = CurrentPlayingMidi.Tempo();
			Looper()->PostMessage(tempo_m->FindMarked()->Message());
			CurrentPlayingMidi.Start();
			paused=false;
		}
	}
}

void DeskbarView::SavePrefs()
{
	//things to save -- which playlist was open and where
	//volume & reverb
	status_t err;
	//Lets find the path to our preferences and set it
	BPath pPath;
	BFile *pFile = new BFile();
	BDirectory this_dir;
	BMessage prefs;

	find_directory(B_USER_SETTINGS_DIRECTORY, &pPath);
	pPath.Append("MidiWorld");
	BDirectory().CreateDirectory(pPath.Path(), &this_dir);
	pPath.Append("db_preferences");
	err = pFile->SetTo(pPath.Path(), B_READ_WRITE | B_CREATE_FILE );
	//Lets store as BMessage
	prefs.AddInt32("reverb", reverb->IndexOf(reverb->FindMarked()) );
	prefs.AddInt32("volume", volume->IndexOf(volume->FindMarked()) );
	prefs.AddString("last_playlist", last_playlist.Path() );
	prefs.Flatten(pFile);
	delete pFile;

}

void DeskbarView::LoadPrefs()
{
	status_t err;

	BPath pPath;
	BFile *pFile = new BFile();

	find_directory(B_USER_SETTINGS_DIRECTORY, &pPath);
	pPath.Append("MidiWorld/db_preferences");
	err = pFile->SetTo(pPath.Path(), B_READ_WRITE);

	BMessage prefs;
	prefs.Unflatten(pFile);

	if(err==B_OK)
	{//Retrieve our attributes from our preferences file
	BMenuItem *bla;
	bla=reverb->ItemAt(prefs.FindInt32("reverb"));
	bla->SetMarked(true);
	bla=volume->ItemAt(prefs.FindInt32("volume"));
	bla->SetMarked(true);
	Looper()->PostMessage(bla->Message(), this);
	last_playlist.Unset();
	last_playlist.SetTo(prefs.FindString("last_playlist"));
	}
	delete pFile;
}

void install_view1()
{
	BDeskbar da_db;
	da_db.AddItem(new DeskbarView(BRect(0, 0, 15, 15)));
}
