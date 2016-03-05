// ***********************************************
// Brendan Allen  -- darkmoon96@tcon.net
// Copyright 1999
// You can use this code freely in your program
// as long as you give credit to the author (ie
// me) somewhere in the documentation or the about
// box of the application AND leave these comments
// in all of the files you use.
// ***********************************************

//MidiWin.h
//This program is going to let you drag midi files on to a BListView
//and it will play the midi's in that order
//also the list view will be able to become a replicant
#ifndef _MIDI_WIN_H_
#define _MIDI_WIN_H_

#include <Window.h>
#include "MidiView.h"

class MidiWindow : public BWindow
{
	public:
		MidiWindow(BRect frame);
		virtual bool QuitRequested();
		virtual void MessageReceived(BMessage *message);
		virtual void FrameMoved(BPoint screenPoint);
		
		MidiView *daView;
};

#endif