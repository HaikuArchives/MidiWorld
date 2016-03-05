// ***********************************************
// Brendan Allen  -- darkmoon96@tcon.net
// Copyright 1999
// You can use this code freely in your program
// as long as you give credit to the author (ie
// me) somewhere in the documentation or the about
// box of the application AND leave these comments
// in all of the files you use.
// ***********************************************

//MidiItem.h
#ifndef _MIDI_ITEM_H_
#define _MIDI_ITEM_H_

#include <ListItem.h>
#include <Entry.h>

class _EXPORT MidiItem : public BListItem
{
	public:
		MidiItem(char *s, entry_ref path);
		MidiItem(BMessage *);
		~MidiItem();
		virtual void DrawItem(BView *owner, BRect frame, bool complete = false);
		static MidiItem *Instantiate(BMessage *archive);
		virtual status_t Archive(BMessage *archive, bool deep = true) const;

		entry_ref midi_path;
		char *name;
		int32 tempo;
		bool is_playing;
		bool paused;//to see if it is paused or not
};

#endif
