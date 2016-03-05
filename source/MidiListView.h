// ***********************************************
// Brendan Allen  -- darkmoon96@tcon.net
// Copyright 1999
// You can use this code freely in your program
// as long as you give credit to the author (ie
// me) somewhere in the documentation or the about
// box of the application AND leave these comments
// in all of the files you use.
// ***********************************************

//MidiListView.h
#ifndef _MIDI_L_V_H_
#define _MIDI_L_V_H_

#include <ListView.h>

class MidiListView : public BListView
{
	public:
		MidiListView(BRect frame);
		~MidiListView();
		virtual void SelectionChanged(void);
//		virtual void MouseDown(BPoint point);
		void AttachedToWindow ();
		virtual void MouseMoved ( BPoint, uint32, const BMessage * message);
		virtual bool InitiateDrag(BPoint point, int32 index, bool wasSelected);
//		virtual void MessageReceived(BMessage *);

//		BPopUpMenu *addons_menu;
//		bool (*func_do_stuff)(bool test);
};

#endif