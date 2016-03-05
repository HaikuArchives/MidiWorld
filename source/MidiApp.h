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
// MidiApp.h 
// ----------------------------------------------------------------------- 
#ifndef _MIDI_APP_H_
#define _MIDI_APP_H_

#include <Application.h>
#include "MidiListView.h"

class MidiApp : public BApplication 
{ 
     public: 
        MidiApp(bool);
        ~MidiApp();
		virtual void ArgvReceived(int32 argc, char **argv);
		virtual void RefsReceived(BMessage *msg);
};

#endif