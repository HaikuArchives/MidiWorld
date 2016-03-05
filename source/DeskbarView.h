// ***********************************************
// Brendan Allen  -- darkmoon96@tcon.net
// Copyright 1999
// You can use this code freely in your program
// as long as you give credit to the author (ie
// me) somewhere in the documentation or the about
// box of the application AND leave these comments
// in all of the files you use.
// ***********************************************

//DeskbarView.h
//this will be the view that will be put in the deskbar
//basically it will be like another application

#ifndef _D_B_VIEW_H_
#define _D_B_VIEW_H_

#include <View.h>
#include <Menu.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Archivable.h>
#include <Bitmap.h>
#include <MidiSynthFile.h>
#include <Path.h>

class _EXPORT DeskbarView;

void install_view1();
void install_view2();

class DeskbarView : public BView
{
	public:
		DeskbarView(BRect frame);
		DeskbarView(BMessage *); //get icon from app here
		~DeskbarView();
		//archiving stuff
		static DeskbarView *Instantiate(BMessage *data);
		virtual status_t Archive(BMessage *data, bool deep = true) const;

		// misc BView overrides
		virtual void AttachedToWindow();	
		virtual void MouseDown(BPoint);
		virtual void Pulse();

		virtual void Draw(BRect);

		virtual void MessageReceived(BMessage *message);
		void OpenPlaylist();
		void AddPlaylists();
		
		void OpenFile(BFile *pFile);
		void InitStuff();
		void InitMoreStuff();
		void SavePrefs();	
		void LoadPrefs();

		BBitmap *icon;
	private:
		BPopUpMenu *DBMenu;
		BPath last_playlist;
		BMenu *reverb,*transpose,*tempo_m, *play, *volume;
		BMenuItem *pause, *stop;
		BMidiSynthFile CurrentPlayingMidi;
		int32 tempo;
		bool paused;
		bool regular; //if it is the regular constructor or not
} ;

#endif