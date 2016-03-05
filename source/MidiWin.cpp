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
// MidiWin.cpp 
// ----------------------------------------------------------------------- 

#include <stdio.h>
#include <string.h>
#include "MidiWin.h" 
#include "MidiItem.h"
#include "Colors.h"
#include "constants.h"
   
#include <Application.h> //for be_app
#include <Alert.h>

MidiWindow::MidiWindow(BRect frame)     
            :BWindow(frame, "Midi World!", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS) 
{
	//Set the pulse rate of how often we check to see if the midi is playing
	//or ready for the next one
	//set it to every second
	SetPulseRate(1000000);
	//Add the MidiView
    BRect viewRect = Bounds();
    daView = new MidiView(viewRect);
    daView->SetViewColor(BeBackgroundGrey);
    AddChild(daView); 
	//get the coordinates of the window
	daView->w_top = Frame().top;
	daView->w_left = Frame().left;
	//show the window
    Show(); 
}

//this is so we can store the screen coordinates
void MidiWindow::FrameMoved(BPoint screenPoint)
{
	daView->w_left = screenPoint.x;
	daView->w_top  = screenPoint.y;
}

bool MidiWindow::QuitRequested() 
{ 
     be_app->PostMessage(B_QUIT_REQUESTED); 
     return true; 
}

//basically all the messages go to the view now
//so nothing goes here really
void MidiWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		default:
			BWindow::MessageReceived(message);
			break;
	}
}