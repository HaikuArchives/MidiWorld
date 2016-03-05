// ***********************************************
// Brendan Allen  -- darkmoon96@tcon.net
// Copyright 1999
// You can use this code freely in your program
// as long as you give credit to the author (ie
// me) somewhere in the documentation or the about
// box of the application AND leave these comments
// in all of the files you use.
// ***********************************************

//MidiItem.cpp
#include "MidiItem.h"
#include "Colors.h"
#include "MidiWin.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <Resources.h>
#include <File.h>
#include <Application.h>

MidiItem::MidiItem(char *s, entry_ref path) : BListItem()
{
	//allocate memory for name
	name = (char *)malloc(strlen(s)+1);
	//copy the s into name
	strcpy(name, s);
	//sets the midi_path to what was passed
	midi_path = path;
	//it's not paused
	paused = false;
	//it's not playing
	is_playing = false;
}

MidiItem::MidiItem(BMessage *data) : BListItem()
{
	const char *s=data->FindString("name");
	name = (char *)malloc(strlen(s)+1);
	strcpy(name, s);
	data->FindRef("midi_path", &midi_path);
	data->FindBool("is_paused", &paused);
	data->FindBool("is_playing", &is_playing);
}

status_t MidiItem::Archive(BMessage *data, bool deep) const
{
	BListItem::Archive(data, deep);

	data->AddString("class", "MidiItem");
	data->AddString("name", name);
	data->AddRef("midi_path", &midi_path);
	data->AddBool("is_paused", paused);
	data->AddBool("is_playing", is_playing);

	return B_NO_ERROR;
}

MidiItem *MidiItem::Instantiate(BMessage *data)
{
	if(!validate_instantiation(data, "MidiItem"))
		return NULL;
	return new MidiItem(data);
}

MidiItem::~MidiItem()
{
//free the memory for the name
	free(name);
}

//this lets us draw an item
void MidiItem::DrawItem(BView *owner, BRect frame, bool complete)
{
	if (IsSelected() || complete)
	{ 
		rgb_color color; 
		if (IsSelected())
	    	color = LightMetallicBlue;
		else
	    	color = owner->ViewColor(); 
		owner->SetHighColor(color);
		owner->FillRect(frame);
	}
	owner->MovePenTo(frame.left+4, frame.bottom-2); 
    if (IsEnabled() && !IsSelected()  && !is_playing)
    	owner->SetHighColor(Blue);
    else if(this->is_playing==true)
		owner->SetHighColor(Black);
	else if(IsEnabled())
    	owner->SetHighColor(Black);
    owner->DrawString(name);
	owner->SetLowColor(owner->ViewColor());
	owner->SetHighColor(owner->ViewColor());
}