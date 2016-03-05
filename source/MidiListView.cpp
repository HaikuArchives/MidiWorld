// ***********************************************
// Brendan Allen  -- darkmoon96@tcon.net
// Copyright 1999
// You can use this code freely in your program
// as long as you give credit to the author (ie
// me) somewhere in the documentation or the about
// box of the application AND leave these comments
// in all of the files you use.
// ***********************************************

//MidiListView.cpp
//#include <MenuItem.h>
//#include <PopUpMenu.h>
#include <Window.h>
#include <Application.h>
#include <string.h>
#include <stdio.h>
#include <ScrollBar.h>
#include <Bitmap.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include "MidiListView.h"
#include "MidiItem.h"
//#include "MWAddons.h"
#include "constants.h"

MidiListView::MidiListView(BRect frame) :
	BListView(frame,"MidiListView", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL,B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS)
{
	//this is for whenever you double click on an item
	//in the list this message is sent to it's target
	SetInvocationMessage(new BMessage(PLAY_MIDI));
//	addons_menu = new BPopUpMenu("Addons", false, false);
//	image_id addon_image = load_add_on("RevealTracker");
//	get_image_symbol(addon_image, "About", B_SYMBOL_TYPE_TEXT, (void **)&func_do_stuff);
//	addons_menu->AddItem(new BMenuItem("Reveal in Tracker", new BMessage(ADDON_CALLED)));
	//Make the font bold
	SetFont(be_bold_font);
	SetFontSize(11);
}

//void MidiListView::MessageReceived(BMessage *msg)
//{
//	switch(msg->what)
//	{
//		case ADDON_CALLED:
//			(*func_do_stuff)(false);
//			break;
//	}
//}

MidiListView::~MidiListView ()
{
	//delete addons_menu;
}

void MidiListView::SelectionChanged()
{
	Window()->PostMessage(new BMessage(MIDI_SELECTION_CHANGED), Parent()->Parent());
}


void MidiListView::AttachedToWindow ()
{
	//addons_menu->SetTargetForItems(this);
	BListView::AttachedToWindow();
}

bool MidiListView::InitiateDrag(BPoint point, int32 index, bool wasSelected)
{
	// don't start unless item already selected
	if ( !wasSelected )
		return false;
	return true;
}

void MidiListView::MouseMoved (BPoint where,uint32 transit, const BMessage * message )
{
	// get fresh mouse info
	uint32 buttons;
	GetMouse ( &where, &buttons, true );
	
	switch ( transit )
	{
	//if inside the view
		case B_INSIDE_VIEW:
		{
			if(buttons == B_PRIMARY_MOUSE_BUTTON && CurrentSelection() >= 0
				&& message == NULL)
			//make sure there is something selected, and they aren't
			//dragging anything such as a playlist or another midi over
			//the listview
			 {
				int32 first=CurrentSelection();
				int32 second=IndexOf(where);
				int32 h=(int32)ItemAt(first)->Height();
				if(second >= 0 && first != second)
				{
					SwapItems(first, second);
					ScrollBy(0, ((first < second) ? h : -h) );
				}
			}
		}
		break;
	}
}

//void MidiListView::MouseDown(BPoint point)
//{
//	uint32 buttons;
//	GetMouse( &point, &buttons, true);
	
//	if(buttons==B_SECONDARY_MOUSE_BUTTON && CurrentSelection()==IndexOf(point))
//	{	//Popup a menu
//		ConvertToScreen(&point);
//		point.x-=1;
//		point.y-=1;
//		addons_menu->Go(point, true);
//	}		
//}