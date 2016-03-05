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
// MidiView.h 
// ----------------------------------------------------------------------- 
#ifndef _MIDI_V_H_
#define _MIDI_V_H_

#include <View.h>
#include <Path.h>
#include <Slider.h>
#include <MidiSynthFile.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <ScrollView.h>
#include "MidiListView.h"
#include "MidiItem.h"

class MidiView : public BView 
{ 
	  
	public:
		MidiView(BRect frame);
		~MidiView();

		virtual void AttachedToWindow(void);
		virtual void AllAttached(void);
		virtual void MessageReceived(BMessage *msg);
		virtual void Pulse(void);
        void SavePreferences();
        void OpenPreferences();
		void OpenDefault();
		void SaveDefault(); //saves the default list
		void SavePlayList(BPath);
		void load_midi(int num_to_load=-1);
		void stop_midi();
		void add_da_refs(entry_ref midiRef, BMessage *message);
		//show instruments -- maybe not
		void ShowInstruments();

		float w_top, w_left; //this is to save the window's position
		
		bool is_playing; //this is used to see if *any* midi is playing
						//not just the currently selected item
		bool looping; //true = loop all, false = loop current
		bool play_on_start; //true means to start playing when midi world is launched
		bool auto_save; //automatically saves the playlist once you have saved it
		
		MidiItem *current;
		BPath playlist_path;
		//File Openers
		BFilePanel *openFile, *saveFile;
		MidiListView *MidiList;
		BScrollView *midi_scroll;
		//this is the midi file itself
		BMidiSynthFile CurrentPlayingMidi;
		//to get where each midi is located
		entry_ref midiRef;
		BMenuBar *main_menu_bar;
		//File menu (or mini-icon menu)
		BMenu *file_m;		
		BMenuItem *SaveList, *about, *Quit, *OpenList, *AddMidis;
		//Midi menu
		BMenu *midi_m;
		BMenuItem *Pause, *PlayStop, *ResetTempo, *RemoveItem, *Shuffle;
		BMenu *reverb_menu;
		BMenuItem *reverb[6]; //for the 6 different types of reverb
		BMenu *transpose_menu;
		BMenuItem *transpose[3]; //for 3 different items - up, down, reset
		//Preferences Menu
		BMenu *prefs;
		BMenu *loop;
		BMenuItem *LoopCurrent, *LoopAll, *PlayOnStart, *AutoSave;
		//have a slider to change the tempo
		BSlider *Tempo;
		//show the length?
		BSlider *position;
		//Have a volume control
		BSlider *volume_control;
		//BList *addons; //the list of addons id
};

#endif