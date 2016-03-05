// ***********************************************
// Brendan Allen  -- darkmoon96@tcon.net
// Copyright 1999
// You can use this code freely in your program
// as long as you give credit to the author (ie
// me) somewhere in the documentation or the about
// box of the application AND leave these comments
// in all of the files you use.
// ***********************************************

// ----------------------------------------------------------------------- 
// MidiView.cpp 
// ----------------------------------------------------------------------- 
#include "MidiView.h"
#include "MidiApp.h"
#include "MidiItem.h"
#include "MidiWin.h"
#include "DeskbarView.h"
#include "Colors.h"
#include "constants.h"
#include <ScrollView.h>
#include <Application.h>
#include <Mime.h>
#include <Node.h>
#include <NodeInfo.h>
#include <Alert.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <Path.h>
#include <FilePanel.h>
#include <Messenger.h>
#include <Roster.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <OS.h>

int32 load_prefs(void *data);
int32 stop_midi_thread(void *data);
//load prefs thread function
int32 load_prefs(void *data)
{
	MidiView *daView = (MidiView *)data;
	daView->LockLooper();
	daView->OpenPreferences();
	daView->OpenDefault();
	daView->Window()->PostMessage(PREFS_DONE_LOADING, daView);
	daView->UnlockLooper();
	return B_OK;
}

int32 stop_midi_thread(void *data)
{
	MidiView *daView = (MidiView *)data;
	//the next basically fades it down until it stops
	if(daView->CurrentPlayingMidi.IsFinished() == false)
		daView->CurrentPlayingMidi.Fade();
	daView->CurrentPlayingMidi.UnloadFile();
	return B_OK;
}

MidiView::MidiView(BRect frame) 
	: BView(frame, "MidiView", B_FOLLOW_ALL, B_WILL_DRAW | B_PULSE_NEEDED) 
{
	current=NULL; //the current MidiItem
	saveFile=NULL;
	openFile=NULL;
	is_playing = false; //nothing is playing when we start
	looping = play_on_start = auto_save = true;
//The following is for the replicant but I am thinking we don't need it
	BRect rect(Bounds());
	rect.OffsetTo(B_ORIGIN);
	rect.top = rect.bottom - 15;
	rect.left = rect.right - 15;
	DeskbarView *replicant = new DeskbarView(rect);
	AddChild(replicant);
//This is the ListView that holds the midis
//also gotta have a ScrollView
	BRect bla(10,25,170,205);
	MidiList = new MidiListView(bla);
	midi_scroll=new BScrollView("MidiScroll",MidiList,B_FOLLOW_ALL,0,false,true,B_FANCY_BORDER);
//Make the main menu bar	
	main_menu_bar = new BMenuBar(BRect(0,0,this->Frame().Width(),19), "main");
//Add the reverb menu
	reverb_menu = new BMenu("Reverb");
	reverb[0] = new BMenuItem("No Reverb", new BMessage(REVERB_NONE));
	reverb[0]->SetMarked(true);
	reverb[1] = new BMenuItem("Closet", new BMessage(REVERB_CLOSET));
	reverb[2] = new BMenuItem("Garage", new BMessage(REVERB_GARAGE));
	reverb[3] = new BMenuItem("Ballroom", new BMessage(REVERB_BALLROOM));
	reverb[4] = new BMenuItem("Cavern", new BMessage(REVERB_CAVERN));
	reverb[5] = new BMenuItem("Dungeon", new BMessage(REVERB_DUNGEON));
	for(int i=0; i<=5; i++)
		reverb_menu->AddItem(reverb[i]);
	reverb_menu->SetRadioMode(true);
//Add Transpose menu here
	transpose_menu = new BMenu("Transpose");
	transpose[0] = new BMenuItem("Down", new BMessage(TRANSPOSE_DOWN));
	transpose[1] = new BMenuItem("Up", new BMessage(TRANSPOSE_UP));
	transpose[2] = new BMenuItem("Reset", new BMessage(TRANSPOSE_DEFAULT));
	for(int j=0; j<=2; j++)
		transpose_menu->AddItem(transpose[j]);
//create menu items for file menu
	about= new BMenuItem("About MidiWorld!", new BMessage(B_ABOUT_REQUESTED));
	SaveList = new BMenuItem("Save Playlist", new BMessage(SAVE_LIST), 'S');
	Quit = new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q');
	OpenList = new BMenuItem("Open Playlist", new BMessage(OPEN_PLAYLIST), 'O');
	AddMidis = new BMenuItem("Add Midis", new BMessage(OPEN_PLAYLIST), 'A');
//create all the menu items now for the midi menu
	Shuffle = new BMenuItem("Shuffle Midis", new BMessage(SHUFFLE_LIST),'/');
	ResetTempo = new BMenuItem("Reset Tempo", new BMessage(RESET_TEMPO));
	RemoveItem = new BMenuItem("Remove Item", new BMessage(REMOVE_FROM_LIST),'D');
	RemoveItem->SetEnabled(false);
	Pause = new BMenuItem("Pause/Resume", new BMessage(PAUSE_MIDI), 'P');
	Pause->SetEnabled(false);
	PlayStop = new BMenuItem("Play", new BMessage(PLAY_STOP_MIDI), '.');
	PlayStop->SetEnabled(false);
//create the menu items for for prefs menu
	loop = new BMenu("Looping");
	loop->SetRadioMode(true);
	PlayOnStart = new BMenuItem("Play List On Open", new BMessage(PLAY_ON_START));
	AutoSave = new BMenuItem("Auto Save Playlist", new BMessage(AUTO_SAVE));
	LoopAll = new BMenuItem("Loop All", new BMessage(LOOP_ALL));
	LoopCurrent = new BMenuItem("Loop Current", new BMessage(LOOP_CURRENT));
	loop->AddItem(LoopAll);
	loop->AddItem(LoopCurrent);
//Make menus in the menu bar
	file_m = new BMenu("File");
	midi_m=new BMenu("Midi");
	prefs=new BMenu("Preferences");
	//add items to menu
	//file menu
	file_m->AddItem(AddMidis);
	file_m->AddItem(OpenList); file_m->AddItem(SaveList); 
	file_m->AddSeparatorItem();
	file_m->AddItem(about);
	file_m->AddSeparatorItem();
	file_m->AddItem(Quit);
	//midi menu
	midi_m->AddItem(PlayStop); midi_m->AddItem(Pause);
	midi_m->AddSeparatorItem();
	midi_m->AddItem(ResetTempo); midi_m->AddItem(RemoveItem);
	midi_m->AddSeparatorItem();
	midi_m->AddItem(reverb_menu); midi_m->AddItem(transpose_menu);
	midi_m->AddSeparatorItem();
	midi_m->AddItem(Shuffle);
	//prefs menu
	prefs->AddItem(loop);
	prefs->AddSeparatorItem();
	prefs->AddItem(PlayOnStart);
	prefs->AddItem(AutoSave);
	prefs->SetRadioMode(false);
	//add the menus to the menu bar
	main_menu_bar->AddItem(file_m); main_menu_bar->AddItem(midi_m); main_menu_bar->AddItem(prefs);
//This is the Slider that changes the tempo
	BRect box(190, 23, 373, 73);
	Tempo = new BSlider(box, "Tempo", "Tempo 100%", new BMessage(TEMPO_CHANGED), 10, 300, B_TRIANGLE_THUMB);
	Tempo->SetModificationMessage(new BMessage(TEMPO_CHANGED));
	Tempo->SetHashMarks(B_HASH_MARKS_BOTTOM);
	Tempo->SetHashMarkCount(10);
	Tempo->SetLimitLabels("10%", "300%");
	Tempo->SetValue(100);//100 percent
	Tempo->SetBarColor(LightMetallicBlue);
	Tempo->UseFillColor(true, &Purple);
//Create the volume slider
	box.OffsetBy(0, 50);
	volume_control=new BSlider(box, "Volume", "Volume", new BMessage(CHANGE_VOLUME), 1, 10);
	volume_control->SetModificationMessage(new BMessage(CHANGE_VOLUME));
	volume_control->SetHashMarks(B_HASH_MARKS_BOTTOM);
	volume_control->SetHashMarkCount(5);
	volume_control->SetValue(2);
	be_synth->SetSynthVolume(1.0);
	be_synth->SetSynthVolume((double)(2/2.8));
	volume_control->SetLimitLabels("Min", "Max");
	volume_control->SetBarColor(LightMetallicBlue);
	volume_control->UseFillColor(true, &Purple);
//Add the position slider
	box.OffsetBy(0,50);
	position = new BSlider(box, "Position", "Position", new BMessage(POSITION_CHANGED), 1, 100);
	position->SetModificationMessage(new BMessage(POSITION_CHANGED));
	position->SetHashMarks(B_HASH_MARKS_NONE);
	position->SetLimitLabels("Beginning", "End");
	position->SetBarColor(LightMetallicBlue);
	position->UseFillColor(true, &Purple);
//attach them all to the view now
	AddChild(main_menu_bar);
	AddChild(midi_scroll);
	AddChild(Tempo);
	AddChild(volume_control);
	AddChild(position);
}

MidiView::~MidiView()
{
	//close the midi if any is playing and then save the preferences
	CurrentPlayingMidi.Fade();
	SavePreferences();
}

void MidiView::AttachedToWindow(void)
{
//Set targets of stuff
	MidiList->SetTarget(this);
	Tempo->SetTarget(this);
	volume_control->SetTarget(this);
	position->SetTarget(this);
	main_menu_bar->SetTargetForItems(this);
	file_m->SetTargetForItems(this);
	midi_m->SetTargetForItems(this);
	transpose_menu->SetTargetForItems(this);
	reverb_menu->SetTargetForItems(this);
	prefs->SetTargetForItems(this);
	Quit->SetTarget(be_app);
}

void MidiView::AllAttached(void)
{
//Load the preferences
//Create thread here to open default+prefs
	thread_id da_thread;
	da_thread = spawn_thread(load_prefs, "prefs_load", B_NORMAL_PRIORITY, this);
	resume_thread(da_thread);
}

//this where all our messages are received pretty much
void MidiView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case B_SIMPLE_DATA:
		case B_REFS_RECEIVED:
		{
			add_da_refs(midiRef, message);
			break;
		}
		case PREFS_DONE_LOADING:
		{
			if(!MidiList->IsEmpty())
			{
				MidiList->Select(0);
				if(play_on_start==true)
					load_midi(0);
			}
			break;
		}
		case PLAY_MIDI: //turns the midi on or off
		{
		//check to see if the list is empty or the current selection is nothing
			if(MidiList->IsEmpty() == true || MidiList->CurrentSelection() < 0)
				return;
			//if it is playing they want to play the one they double-clicked
			//so lets stop it and unload and set some other stuff
			if(is_playing == true)
			{
				stop_midi();
				status_t nothing;
				wait_for_thread(find_thread("stop_midi_thread"), &nothing);
			}
			load_midi();
				
			break;
		}
		case TEMPO_CHANGED:
		{
			//IsFinished returns false if it is still playing
			//or if there is no file loaded
			if(CurrentPlayingMidi.IsFinished() == false)
			{
				if(MidiList->IsEmpty() == true)
				{ 
					Tempo->SetValue(100);
					Tempo->SetLabel("Tempo 100%");
				}
				else
				{
				//find the item then set the tempo
				MidiItem *daItem = current;
				//do this so we know its at the starting point before we
				//use a percentage to scale it
				CurrentPlayingMidi.SetTempo(daItem->tempo);
				CurrentPlayingMidi.ScaleTempoBy((double)(Tempo->Value()/100.00));
				}
			}
			char buf[20];
			sprintf(buf, "Tempo %d%%", Tempo->Value());
			Tempo->SetLabel(buf);
			break;
		}
		case PAUSE_MIDI:
		{
			if(MidiList->IsEmpty()==true)
				return;
			MidiItem *daItem = current;
			if(daItem->paused == false)
			{
				CurrentPlayingMidi.Pause();
				daItem->paused = true;
			}
			else
			{
				CurrentPlayingMidi.Resume();
				daItem->paused = false;
			}
			break;
		}
		case REMOVE_FROM_LIST:
		{
			if(MidiList->IsEmpty()==true || MidiList->CurrentSelection() < 0)
				return;
			long daItem = MidiList->CurrentSelection();
			//what this doing?
			//well it checks to see if there is an item below then above
			//to select that before the item gets removed
			MidiItem *daMidItem = (MidiItem *)MidiList->ItemAt(daItem);
			if(CurrentPlayingMidi.IsFinished() == false && daMidItem->is_playing == true)
			{
				//CurrentPlayingMidi.Fade();
				//CurrentPlayingMidi.UnloadFile();
				stop_midi();
				PlayStop->SetLabel("Play");
				current=NULL;
			}
			if(MidiList->CountItems() > 1)
			{
				if(MidiList->ItemAt(daItem+1) != NULL)
					MidiList->Select(daItem+1);
				else if(MidiList->ItemAt(daItem-1) != NULL)
					MidiList->Select(daItem-1);
			}
			MidiList->RemoveItem(daItem);
			break;
		}
		case RESET_TEMPO:
		{
			if(MidiList->IsEmpty()==false)
			{
				//Resets the tempo to 100%
				MidiItem *daItem = current;
				CurrentPlayingMidi.SetTempo(daItem->tempo);
				CurrentPlayingMidi.ScaleTempoBy(1.0);
				Tempo->SetValue(100);
				Tempo->SetLabel("Tempo 100%");
			}
			break;
		}
		case SHUFFLE_LIST:
		{
			if(MidiList->IsEmpty() == false) //lets shuffle the list!
			{
				srand((unsigned)system_time());
				for(int i=0; i < MidiList->CountItems()-1; i++)
					MidiList->MoveItem(i, rand() % MidiList->CountItems()-1);
				SaveDefault();
			}
			else
				(new BAlert("", "Sorry there is nothing to shuffle!", "Ok!"))->Go();

			break;
		}
		case SAVE_LIST:
		{
			//Save the list to a file
			//this should send the message B_SAVE_REQUESTED to this view
			//which will be handled after this message
			if(saveFile==NULL)
			{
				saveFile=new BFilePanel(B_SAVE_PANEL,new BMessenger(this));
				app_info daInfo;
				be_app->GetAppInfo(&daInfo);
				BEntry daEntry(&daInfo.ref);
				daEntry.GetParent(&daEntry);
				saveFile->SetSaveText("UntitledPlayList");
				saveFile->SetPanelDirectory(&daEntry);
			}
			saveFile->Show();
			break;
		}
		case B_SAVE_REQUESTED: //save the playlist
		{
			entry_ref da_ref;			
			BPath pPath; //for pathname
			BEntry entry; //used to make path
			
			message->FindRef("directory",&da_ref);
			const char *name=message->FindString("name");//for the filename they chose
			entry.SetTo(&da_ref);
			entry.GetPath(&pPath); //create pathname for directory
			pPath.Append(name); //tack on the file name
			SavePlayList(pPath);
			//now that it's saved we will save it again when they quit..if set in preferences
			playlist_path.SetTo(pPath.Path());

			break;
		}
		case PLAY_STOP_MIDI: //this message is from the Play Button
		//this is almost exactly like PLAY_MIDI with a few differences though
		//the only difference is they can click stop and it will stop
		{
		//check to see if the list is empty or the current selection is nothing
			if(MidiList->IsEmpty() == true)
				return;
			//hmm how to get the midi to play it? this is how =)
			//find the current selection
			if(is_playing == true) //Let's stop it
				stop_midi();
			else //is_playing == false
				load_midi();
		}
		//Here are all the reverb messages -- real easy to handle
		//check to see if reverb is enabled and then set what kind
		case REVERB_NONE:
		{
			be_synth->EnableReverb(false);
			break;
		}
		case REVERB_CLOSET:
		{
			if(be_synth->IsReverbEnabled()==false)
				be_synth->EnableReverb(true);
			be_synth->SetReverb(B_REVERB_CLOSET);
			break;
		}
		case REVERB_GARAGE:
		{
			if(be_synth->IsReverbEnabled()==false)
				be_synth->EnableReverb(true);
			be_synth->SetReverb(B_REVERB_GARAGE);			
			break;
		}
		case REVERB_BALLROOM:
		{
			if(be_synth->IsReverbEnabled()==false)
				be_synth->EnableReverb(true);
			be_synth->SetReverb(B_REVERB_BALLROOM);
			break;
		}
		case REVERB_CAVERN:
		{
			if(be_synth->IsReverbEnabled()==false)
				be_synth->EnableReverb(true);
			be_synth->SetReverb(B_REVERB_CAVERN);
			break;
		}
		case REVERB_DUNGEON:
		{
			if(be_synth->IsReverbEnabled()==false)
				be_synth->EnableReverb(true);
			be_synth->SetReverb(B_REVERB_DUNGEON);
			break;
		}
		//another easy message to handle, first we set the volume to 1.0
		//which is the default then we scale the volume by the value
		//of the volume slider
		case CHANGE_VOLUME:
		{
			be_synth->SetSynthVolume(1.0);
			be_synth->SetSynthVolume((double)(volume_control->Value()/2.8));
			break;
		}
		//The next three messages deal with transposing the midi
		//These are easy to deal with also because you don't have
		//to have a midi loaded or playing to do this and it won't harm
		//anything
		case TRANSPOSE_UP:
		//This one adds 1 half-step to the current midi if it isn't greater than 12 half-steps
		//I just put this because well who needs more than 1 octave?
		{
		//change this back later if too much
			if(CurrentPlayingMidi.Transposition() >= 12)
				return;
			CurrentPlayingMidi.SetTransposition(CurrentPlayingMidi.Transposition()+1);
			break;
		}
		//This one subtracts 1 half-step from the current midi if it isn't greater than 12 half-steps
		case TRANSPOSE_DOWN:
		{
			if(CurrentPlayingMidi.Transposition() <= -12)
				return;
			CurrentPlayingMidi.SetTransposition(CurrentPlayingMidi.Transposition()-1);
			break;
		}
		//This changes it back to its default
		case TRANSPOSE_DEFAULT:
		{
			CurrentPlayingMidi.SetTransposition(0);
			break;
		}
		//This is used when they change the position slider
		case POSITION_CHANGED:
		{
			if(MidiList->IsEmpty() == true || MidiList->CurrentSelection() < 0 || is_playing==false)
				position->SetValue(0);

			if(CurrentPlayingMidi.IsFinished() == false)
			{
				if(MidiList->IsEmpty() == false)
					CurrentPlayingMidi.Position((CurrentPlayingMidi.Duration() * (position->Value())) / 100.0);
			}	
			break;
		}
		//Here's the easiest to deal with
		//This is when they click on the about box
		case B_ABOUT_REQUESTED:
		{
			char bla[600] = "MidiWorld! 1.7.3 Release\n"
						"ISLE Software Copyright 1999\n\n"
						"This program allows you to play midis, tranpose\n"
						"them, play a list of them, loop them, change\ntheir tempo,"
						"and other cool stuff!\n\n"
						"This application was created by Brendan Allen\n"
						"Read the readme for help on this program.\n"
						"If you like this program you can give the creator\n"
						"some money if you wish ($3-5) to\n"
						"Brendan Allen\n2750 E. Midland Rd.\nIndianapolis, IN 46227\n"
						"Also if you find any bugs report them to darkmoon96@tcon.net\n"
						"and I will try to fix them.  If you have any feature\n"
						"requests send them to the same email address above also.\n";
			(new BAlert("About Box", bla, "Ok Cool!"))->Go();
			break;
		}
		case MIDI_SELECTION_CHANGED:
		{
			if(MidiList->IsEmpty() == true)
			{
				PlayStop->SetEnabled(false);
				Pause->SetEnabled(false);
				RemoveItem->SetEnabled(false);
				return;
			}
			if(MidiList->CurrentSelection() < 0) //if there is nothing selected
			{
				if(is_playing == false)
				{
					PlayStop->SetEnabled(false);
					Pause->SetEnabled(false);
				}
				RemoveItem->SetEnabled(false);
			}
			else
			{
				PlayStop->SetEnabled(true);
				RemoveItem->SetEnabled(true);
				if(is_playing == true)
					Pause->SetEnabled(true);
			}
			break;
		}
		case LOOP_ALL:
			looping=true;
			break;
		case LOOP_CURRENT:
			looping=false;
			break;
		case PLAY_ON_START:
			play_on_start=!play_on_start;
			PlayOnStart->SetMarked(play_on_start);
			break;
		case AUTO_SAVE:
			auto_save=!auto_save;
			AutoSave->SetMarked(auto_save);
			break;
		case OPEN_PLAYLIST:
		{
			if(openFile==NULL)
				openFile=new BFilePanel(B_OPEN_PANEL,new BMessenger(this));
			openFile->Show();
			break;
		}
		default:
   			BView::MessageReceived(message);
   		break;
   }
}

void MidiView::SavePlayList(BPath pPath)
{
	// Now lets save the file
	//This BMessage will be used to store the entry_refs of where the midi's
	//are located
	BFile pFile; //used to save it
	status_t err; //for return code
	BMessage midi_loc;
	//now we set the file to the path		
	pFile.SetTo(pPath.Path(), B_READ_WRITE | B_CREATE_FILE);
	//We add all the ref's into a BMessage which will index them since
	//if a message name is already there it creates an array of them
	for(int i=0; i < MidiList->CountItems(); i++)
	{	//Find the Item
		MidiItem *bla = (MidiItem *)MidiList->ItemAt(i);
		//add the ref to the BMessage
		midi_loc.AddRef("midis", &bla->midi_path);		
	}
	//Now what this does is basically store the message inside of pFile
	//which is alot easier to read the info back later
	midi_loc.Flatten(&pFile);
	//now lets make its type to the playlist type
	BNode node(pPath.Path()); //this creates a node for the Default file
	BNodeInfo mime_stuff(&node); //now we want to be able to set the mimetype
	mime_stuff.SetType("text/x-midiworld-playlist");
}

void MidiView::Pulse(void)
{
	if(MidiList->IsEmpty() == true || current == NULL)
		return;
	//Before we see if it is finished lets check to see if the current song is supposed
	//to loop
	CurrentPlayingMidi.EnableLooping(!looping);
	//Um this is where we check to see if it is finished playing.
	if(CurrentPlayingMidi.IsFinished() == true)
	{
		stop_midi();
		status_t nothing;
		wait_for_thread(find_thread("stop_midi_thread"), &nothing);
		//play next in list if there is one otherwise start at the beginning
		int midi_index=MidiList->IndexOf(current)+1;
		if(!(MidiList->IndexOf(current)+1 < MidiList->CountItems()))
			midi_index=0;
		//select the new current midi & scroll to the new midi so we always see the playing midi
		MidiList->Select(midi_index);
		MidiList->ScrollToSelection();
		//call load midi
		load_midi(midi_index);
	}
	//Move the position slider if it is playing 
	MidiItem *daItem = current;
	if(is_playing == true && daItem->paused == false)
		position->SetValue(((float)CurrentPlayingMidi.Seek() / (float)CurrentPlayingMidi.Duration())*100.00);
}

void MidiView::SavePreferences()
{
	status_t err;
	//Lets find the path to our preferences and set it	
	BPath pPath;
	BFile *pFile = new BFile();
	BDirectory this_dir;

	find_directory(B_USER_SETTINGS_DIRECTORY, &pPath);
	pPath.Append("MidiWorld");
	BDirectory().CreateDirectory(pPath.Path(), &this_dir);
	pPath.Append("preferences");
	err = pFile->SetTo(pPath.Path(), B_READ_WRITE | B_CREATE_FILE );

	//Lets store our attributes	
	long *val = new long;
	pFile->WriteAttr("looping", B_BOOL_TYPE, 0, &looping, sizeof(bool));
	pFile->WriteAttr("auto_save", B_BOOL_TYPE, 0, &auto_save, sizeof(bool));
	pFile->WriteAttr("play_on_start", B_BOOL_TYPE, 0, &play_on_start, sizeof(bool));
	BMenuItem *bla = reverb_menu->FindMarked();
	*val = reverb_menu->IndexOf(bla);
	pFile->WriteAttr("reverb_type", B_INT32_TYPE, 0, val, sizeof(long));
	//Add one for volume
	*val = volume_control->Value();
	pFile->WriteAttr("volume", B_INT32_TYPE, 0, val, sizeof(long));
	
	//Get windows Left and Top corner values and save those too
	float *left = new float;
	float *top = new float;
	*left = w_left; *top = w_top;
	pFile->WriteAttr("left_corner", B_FLOAT_TYPE, 0, left, sizeof(float));
	pFile->WriteAttr("top_corner", B_FLOAT_TYPE, 0, top, sizeof(float));
	
	SaveDefault();
	//delete the stuff we made
	delete pFile;
	delete val; 
}

void MidiView::SaveDefault()
{
	//have a Default PlayList which has the last ones before they quit
	//and when they add the replicant to deskbar or when they shuffle the list
	BPath pPath;
	find_directory(B_USER_SETTINGS_DIRECTORY, &pPath);
	pPath.Append("MidiWorld/Default");
	SavePlayList(pPath);
	if(auto_save) //if they have autosave on then save where they last saved
		SavePlayList(playlist_path);
}

void MidiView::OpenPreferences()
{
	status_t err;

	BPath pPath;
	BFile *pFile = new BFile();
	long *val = new long;

	find_directory(B_USER_SETTINGS_DIRECTORY, &pPath);
	pPath.Append("MidiWorld/preferences");
	err = pFile->SetTo(pPath.Path(), B_READ_ONLY);

	if(err==B_OK)
	{
	//Retrieve our attributes from our preferences file
		pFile->ReadAttr("looping", B_BOOL_TYPE, 0, &looping, sizeof(bool));
		if(looping==true)
			loop->ItemAt(0)->SetMarked(true);
		else
			loop->ItemAt(1)->SetMarked(true);

		pFile->ReadAttr("auto_save", B_BOOL_TYPE, 0, &auto_save, sizeof(bool));
		AutoSave->SetMarked(auto_save);
		pFile->ReadAttr("play_on_start", B_BOOL_TYPE, 0, &play_on_start, sizeof(bool));
		PlayOnStart->SetMarked(play_on_start);
		pFile->ReadAttr("reverb_type", B_INT32_TYPE, 0, val, sizeof(long));
		BMenuItem *bla=reverb_menu->ItemAt(*val);
		bla->SetMarked(true);
		pFile->ReadAttr("volume", B_INT32_TYPE, 0, val, sizeof(long));
		volume_control->SetValue(*val);
		be_synth->SetSynthVolume(1.0);
		double scale_by = (double)(*val/2.8);
		be_synth->SetSynthVolume(scale_by);
		//Set windows Left and Top corner values -- maybe not if they have replicant this might crash
		//but I guess we aren't having replicants for desktop
		float *left = new float;
		float *top = new float;
		pFile->ReadAttr("left_corner", B_FLOAT_TYPE, 0, left, sizeof(float));
		pFile->ReadAttr("top_corner", B_FLOAT_TYPE, 0, top, sizeof(float));
		w_top = *top;
		w_left = *left;
		Window()->MoveTo(w_left, w_top);
	}
	//Delete the stuff we used
	delete pFile, val;
}

void MidiView::OpenDefault()
{
	//Seperate thread
	//Open Default Playlist
	//This is similar to saving the playlist -- read comments above
	status_t err;
	BPath pPath;
	BFile *pFile = new BFile();

	find_directory(B_USER_SETTINGS_DIRECTORY, &pPath);
	pPath.Append("MidiWorld/Default");
	err = pFile->SetTo(pPath.Path(), B_READ_ONLY);
	//Now we check to see if it is there, if it isn't then we don't want to 
	//do anything else, or if the list is not empty that means that
	//someone opened another playlist we don't want both lists
	if(err == B_ENTRY_NOT_FOUND || MidiList->IsEmpty() == false)
		return;
	//Now the entry_ref will be used to get each one in the Flattened
	//BMessage which we did above
	entry_ref midiRef;
	BMessage message;
	//Now when we unflatten the file we now have the
	//BMessage we stored above in SavePreferences()
	message.Unflatten(pFile);
	//Now lets get all the midi's and add them to our list!
	int32 i = 0;
	while (message.FindRef("midis", i++, &midiRef) == B_OK) 
	{
		if(BEntry(&midiRef).Exists())
		{
			MidiItem *bla = new MidiItem(midiRef.name, midiRef);
			MidiList->AddItem(bla);
		}
	}
}

void MidiView::load_midi(int num_to_load)
{
	//this is the default
	if(num_to_load==-1)
		num_to_load=MidiList->CurrentSelection();
		
	current = (MidiItem *)MidiList->ItemAt(num_to_load);
	MidiItem *daItem=current;
	if(CurrentPlayingMidi.LoadFile(&daItem->midi_path) == B_OK)
	{
		//store the midi's tempo
		daItem->tempo = CurrentPlayingMidi.Tempo();
		position->SetValue(0);
		is_playing = daItem->is_playing = true;
		CurrentPlayingMidi.Start();
		CurrentPlayingMidi.ScaleTempoBy((double)(Tempo->Value()/100.00));
		PlayStop->SetLabel("Stop");
		Pause->SetEnabled(true);
		MidiList->InvalidateItem(MidiList->IndexOf(current));
	}
}

void MidiView::stop_midi()
{
	thread_id da_thread;
	da_thread = spawn_thread(stop_midi_thread, "stop_midi_thread", B_NORMAL_PRIORITY, this);
	resume_thread(da_thread);
	MidiItem *daItem2 = current;
	is_playing=daItem2->is_playing=daItem2->paused=false;
	MidiList->InvalidateItem(MidiList->IndexOf(current));
	PlayStop->SetLabel("Play");
	Pause->SetEnabled(false);
	if(MidiList->CurrentSelection() < 0)
		PlayStop->SetEnabled(false);
	
}

void MidiView::add_da_refs(entry_ref midiRef, BMessage *message)
{// Look for refs in the message
	int32 i = 0;
	while (message->FindRef("refs", i++, &midiRef) == B_OK) 
       {
  		char type[B_MIME_TYPE_LENGTH];
  		(new BNodeInfo(new BNode(&midiRef)))->GetType(type);
  		BMimeType mime(type);
  		if(mime == "audio/x-midi" || mime == "audio/midi")
			MidiList->AddItem(new MidiItem(midiRef.name, midiRef));
		else if(mime == "text/x-midiworld-playlist")
		{
			if(MidiList->IsEmpty() == false)
			{
				MidiList->RemoveItems(0,MidiList->CountItems()-1);
				MidiList->MakeEmpty();
			}
			BFile *pFile = new BFile(&midiRef, B_READ_WRITE);
			BMessage playlist;
			playlist.Unflatten(pFile);
			//if(message->WasDropped())
			CurrentPlayingMidi.Fade();
			BEntry(&midiRef).GetPath(&playlist_path);
			entry_ref midi_ref;
			int32 i=0;
			while(playlist.FindRef("midis", i++, &midi_ref) == B_OK)
			{
			if(BEntry(&midi_ref).Exists())
			  {
				MidiItem *bla = new MidiItem(midi_ref.name, midi_ref);
				MidiList->AddItem(bla);
			  }
			}
			MidiList->Select(0);
			if(play_on_start==true)
				load_midi(0);
		}
		else
			(new BAlert("", "That is not a midi file/playlist or it is not the right mimetype.", "OK"))->Go();
	}//end while
}

/*
void MidiView::ShowInstruments()
{
	int16 insts[128];
	int16 count;
	
	CurrentPlayingMidi.GetPatches(insts, &count);
	printf("\nShowing instruments\n");
	int n=0;
	
	while(n < count)
	{
	  switch(insts[n])
	  {
		  // Pianos
		  case B_ACOUSTIC_GRAND:
				printf("\nB_ACOUSTIC_GRAND\n");
		       break;
		  case B_BRIGHT_GRAND:
				printf("\nB_BRIGHT_GRAND\n");
		       break;
		  case B_ELECTRIC_GRAND:
				printf("\nB_ELECTRIC_GRAND\n");
		       break;
		  case B_HONKY_TONK:
				printf("\nB_HONKY_TONK\n");
		       break;
		  case B_ELECTRIC_PIANO_1:
				printf("\nB_ELECTRIC_PIANO_1\n");
		       break;
		  case B_ELECTRIC_PIANO_2:
				printf("\nB_ELECTRIC_PIANO_2\n");
		       break;
		  case B_HARPSICHORD:
				printf("\n B_HARPSICHORD\n");
		       break;
		  case B_CLAVICHORD:
				printf("\nB_CLAVICHORD\n");
		       break;
		
		  // Tuned Idiophones
		  case B_CELESTA:
				printf("\nB_CELESTA\n");
		       break;
		  case B_GLOCKENSPIEL:
				printf("\nB_GLOCKENSPIEL\n");
		       break;
		  case B_MUSIC_BOX:
				printf("\nB_MUSIC_BOX\n");
		       break;
		  case B_VIBRAPHONE:
				printf("\nB_VIBRAPHONE\n");
		       break;
		  case B_MARIMBA:
				printf("\nB_MARIMBA\n");
		       break;
		  case B_XYLOPHONE:
				printf("\nB_XYLOPHONE\n");
		       break;
		  case B_TUBULAR_BELLS:
				printf("\nB_TUBULAR_BELLS\n");
		       break;
		  case B_DULCIMER:
				printf("\nB_DULCIMER\n");
		       break;
		
		  // Organs
		  case B_DRAWBAR_ORGAN:
				printf("\nB_DRAWBAR_ORGAN\n");
		       break;
		  case B_PERCUSSIVE_ORGAN:
				printf("\nB_PERCUSSIVE_ORGAN\n");
		       break;
		  case B_ROCK_ORGAN:
				printf("\nB_ROCK_ORGAN\n");
		       break;
		  case B_CHURCH_ORGAN:
				printf("\nB_CHURCH_ORGAN\n");
		       break;
		  case B_REED_ORGAN:
				printf("\nB_REED_ORGAN\n");
		       break;
		  case B_ACCORDION:
				printf("\nB_ACCORDION\n");
		       break;
		  case B_HARMONICA:
				printf("\nB_HARMONICA\n");
		       break;
		  case B_TANGO_ACCORDION:
				printf("\nB_TANGO_ACCORDION\n");
		       break;
		
		  // Guitars 
		  case B_ACOUSTIC_GUITAR_NYLON:
				printf("\nB_ACOUSTIC_GUITAR_NYLON\n");
		       break;
		  case B_ACOUSTIC_GUITAR_STEEL:
				printf("\nB_ACOUSTIC_GUITAR_STEEL\n");
		       break;
		  case B_ELECTRIC_GUITAR_JAZZ:
				printf("\nB_ELECTRIC_GUITAR_JAZZ\n");
		       break;
		  case B_ELECTRIC_GUITAR_CLEAN:
				printf("\n B_ELECTRIC_GUITAR_CLEAN\n");
		       break;
		  case B_ELECTRIC_GUITAR_MUTED:
				printf("\nB_ELECTRIC_GUITAR_MUTED\n");
		       break;
		  case B_OVERDRIVEN_GUITAR:
				printf("\nB_OVERDRIVEN_GUITAR\n");
		       break;
		  case B_DISTORTION_GUITAR:
				printf("\nB_DISTORTION_GUITAR\n");
		       break;
		  case B_GUITAR_HARMONICS:
				printf("\nB_GUITAR_HARMONICS\n");
		       break;
		  
		  // Basses
		  case B_ACOUSTIC_BASS:
				printf("\nB_ACOUSTIC_BASS\n");
		       break;
		  case B_ELECTRIC_BASS_FINGER:
				printf("\nB_ELECTRIC_BASS_FINGER\n");
		       break;
		  case B_ELECTRIC_BASS_PICK:
				printf("\nB_ELECTRIC_BASS_PICK\n");
		       break;
		  case B_FRETLESS_BASS:
				printf("\nB_FRETLESS_BASS\n");
		       break;
		  case B_SLAP_BASS_1:
				printf("\nB_SLAP_BASS_1\n");
		       break;
		  case B_SLAP_BASS_2:
				printf("\nB_SLAP_BASS_2\n");
		       break;
		  case B_SYNTH_BASS_1:
				printf("\nB_SYNTH_BASS_1\n");
		       break;
		  case B_SYNTH_BASS_2:
				printf("\nB_SYNTH_BASS_2\n");
		       break;
		
		  // Strings 
		  case B_VIOLIN:
				printf("\nB_VIOLIN\n");
		       break;
		  case B_VIOLA:
				printf("\nB_VIOLA\n");
		       break;
		  case B_CELLO:
				printf("\nB_CELLO\n");
		       break;
		  case B_CONTRABASS:
				printf("\nB_CONTRABASS\n");
		       break;
		  case B_TREMOLO_STRINGS:
				printf("\nB_TREMOLO_STRINGS\n");
		       break;
		  case B_PIZZICATO_STRINGS:
				printf("\nB_PIZZICATO_STRINGS\n");
		       break;
		  case B_ORCHESTRAL_STRINGS:
				printf("\nB_ORCHESTRAL_STRINGS\n");
		       break;
		  case B_TIMPANI:
				printf("\nB_TIMPANI\n");
		       break;
		
		  // Ensemble strings and voices 
		  case B_STRING_ENSEMBLE_1:
				printf("\nB_STRING_ENSEMBLE_1\n");
		       break;
		  case B_STRING_ENSEMBLE_2:
				printf("\nB_STRING_ENSEMBLE_2\n");
		       break;
		  case B_SYNTH_STRINGS_1:
				printf("\nB_SYNTH_STRINGS_1\n");
		       break;
		  case B_SYNTH_STRINGS_2:
				printf("\nB_SYNTH_STRINGS_2\n");
		       break;
		  case B_VOICE_AAH:
				printf("\nB_VOICE_AAH\n");
		       break;
		  case B_VOICE_OOH:
				printf("\nB_VOICE_OOH\n");
		       break;
		  case B_SYNTH_VOICE:
				printf("\nB_SYNTH_VOICE\n");
		       break;
		  case B_ORCHESTRA_HIT:
				printf("\nB_ORCHESTRA_HIT\n");
		       break;
		
		  // Brass 
		  case B_TRUMPET:
				printf("\nB_TRUMPET\n");
		       break;
		  case B_TROMBONE:
				printf("\nB_TROMBONE\n");
		       break;
		  case B_TUBA:
				printf("\nB_TUBA\n");
		       break;
		  case B_MUTED_TRUMPET:
				printf("\nB_MUTED_TRUMPET\n");
		       break;
		  case B_FRENCH_HORN:
				printf("\nB_FRENCH_HORN\n");
		       break;
		  case B_BRASS_SECTION:
				printf("\nB_BRASS_SECTION\n");
		       break;
		  case B_SYNTH_BRASS_1:
				printf("\nB_SYNTH_BRASS_1\n");
		       break;
		  case B_SYNTH_BRASS_2:
				printf("\nB_SYNTH_BRASS_2\n");
		       break;
		
		  // Reeds 
		  case B_SOPRANO_SAX:
				printf("\nB_SOPRANO_SAX\n");
		       break;
		  case B_ALTO_SAX:
				printf("\nB_ALTO_SAX\n");
		       break;
		  case B_TENOR_SAX:
				printf("\nB_TENOR_SAX\n");
		       break;
		  case B_BARITONE_SAX:
				printf("\nB_BARITONE_SAX\n");
		       break;
		  case B_OBOE:
				printf("\nB_OBOE\n");
		       break;
		  case B_ENGLISH_HORN:
				printf("\nB_ENGLISH_HORN\n");
		       break;
		  case B_BASSOON:
				printf("\nB_BASSOON\n");
		       break;
		  case B_CLARINET:
				printf("\nB_CLARINET\n");
		       break;
		
		  // Pipes 
		  case B_PICCOLO:
				printf("\nB_PICCOLO\n");
		       break;
		  case B_FLUTE:
				printf("\nB_FLUTE\n");
		       break;
		  case B_RECORDER:
				printf("\nB_RECORDER\n");
		       break;
		  case B_PAN_FLUTE:
				printf("\nB_PAN_FLUTE\n");
		       break;
		  case B_BLOWN_BOTTLE:
				printf("\nB_BLOWN_BOTTLE\n");
		       break;
		  case B_SHAKUHACHI:
				printf("\nB_SHAKUHACH\n");
		       break;
		  case B_WHISTLE:
				printf("\nB_WHISTLE\n");
		       break;
		  case B_OCARINA:
				printf("\nB_OCARINA\n");
		       break;
		
		  // Synth Leads
		  case B_SQUARE_WAVE:
				printf("\nB_SQUARE_WAVE\n");
		       break;
		  case B_SAWTOOTH_WAVE:
				printf("\nB_SAWTOOTH_WAVE\n");
		       break;
		  case B_CALLIOPE:
				printf("\nB_CALLIOPE\n");
		       break;
		  case B_CHIFF:
				printf("\nB_CHIFF\n");
		       break;
		  case B_CHARANG:
				printf("\nB_CHARANG\n");
		       break;
		  case B_VOICE:
				printf("\nB_VOICE\n");
		       break;
		  case B_FIFTHS:
				printf("\nB_FIFTHS\n");
		       break;
		  case B_BASS_LEAD:
				printf("\nB_BASS_LEAD:\n");
		       break;
		  
		  // Synth Pads 
		  case B_NEW_AGE:
				printf("\nB_NEW_AGE\n");
		       break;
		  case B_WARM:
				printf("\nB_WARM\n");
		       break;
		  case B_POLYSYNTH:
				printf("\nB_POLYSYNTH\n");
		       break;
		  case B_CHOIR:
				printf("\nB_CHOIR\n");
		       break;
		  case B_BOWED:
				printf("\nB_BOWED\n");
		       break;
		  case B_METALLIC:
				printf("\nB_METALLIC\n");
		       break;
		  case B_HALO:
				printf("\nB_HALO\n");
		       break;
		  case B_SWEEP:
				printf("\nB_SWEEP\n");
		       break;
		
		  // Effects 
		  case B_FX_1:
				printf("\nB_FX_1\n");
		       break;
		  case B_FX_2:
				printf("\nB_FX_2\n");
		       break;
		  case B_FX_3:
				printf("\nB_FX_3\n");
		       break;
		  case B_FX_4:
				printf("\nB_FX_4\n");
		       break;
		  case B_FX_5:
				printf("\nB_FX_5\n");
		       break;
		  case B_FX_6:
				printf("\nB_FX_6\n");
		       break;
		  case B_FX_7:
				printf("\nB_FX_7\n");
		       break;
		  case B_FX_8:
				printf("\nB_FX_8\n");
		       break;
		
		  // Ethnic 
		  case B_SITAR:
				printf("\nB_SITAR\n");
		       break;
		  case B_BANJO:
				printf("\nB_BANJO\n");
		       break;
		  case B_SHAMISEN:
				printf("\nB_SHAMISEN\n");
		       break;
		  case B_KOTO:
				printf("\nB_KOTO\n");
		       break;
		  case B_KALIMBA:
				printf("\nB_KALIMBA\n");
		       break;
		  case B_BAGPIPE:
				printf("\nB_BAGPIPE\n");
		       break;
		  case B_FIDDLE:
				printf("\nB_FIDDLE\n");
		       break;
		  case B_SHANAI:
				printf("\nB_SHANA\n");
		       break;
		
		  // Percussion 
		  case B_TINKLE_BELL:
				printf("\nB_TINKLE_BELL\n");
		       break;
		  case B_AGOGO:
				printf("\nB_AGOGO\n");
		       break;
		  case B_STEEL_DRUMS:
				printf("\nB_STEEL_DRUMS\n");
		       break;
		  case B_WOODBLOCK:
				printf("\nB_WOODBLOCK\n");
		       break;
		  case B_TAIKO_DRUMS:
				printf("\nB_TAIKO_DRUMS\n");
		       break;
		  case B_MELODIC_TOM:
				printf("\nB_MELODIC_TOM\n");
		       break;
		  case B_SYNTH_DRUM:
				printf("\nB_SYNTH_DRUM\n");
		       break;
		  case B_REVERSE_CYMBAL:
				printf("\nB_REVERSE_CYMBAL\n");
		       break;
		
		  // Sound Effects 
		  case B_FRET_NOISE:
				printf("\nB_FRET_NOISE\n");
		       break;
		  case B_BREATH_NOISE:
				printf("\nB_BREATH_NOISE\n");
		       break;
		  case B_SEASHORE:
				printf("\nB_SEASHORE\n");
		       break;
		  case B_BIRD_TWEET:
				printf("\nB_BIRD_TWEET\n");
		       break;
		  case B_TELEPHONE:
				printf("\nB_TELEPHONE\n");
		       break;
		  case B_HELICOPTER:
				printf("\nB_HELICOPTER\n");
		       break;
		  case B_APPLAUSE:
				printf("\nB_APPLAUSE\n");
		       break;
		  case B_GUNSHOT:
				printf("\nB_GUNSHOT\n");
				break;
		  default:
				printf("\nUnknown Instrument #%i", insts[n]);
				break;
		}
		n++;
  	}
}
*/