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
// MidiApp.cpp 
// ----------------------------------------------------------------------- 
     
#include "MidiApp.h" 
#include "MidiWin.h"
#include "MidiView.h"
#include "MidiItem.h"
#include "DeskbarView.h"
#include "constants.h"
#include <Mime.h>
#include <File.h>
#include <Alert.h>
#include <Directory.h>
#include <Node.h>
#include <NodeInfo.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) 
{	
	if ( argv[1] && strcmp(argv[1], "-deskbar") != 0) 
	{
		// print a simple usage string
		printf("# %s (c) 1999 ISLE BUG. \n#(command line feature still under development)\n"
			//	"# Usage: %s -f midifile1 midifile2 ...\n"
			//	"# Usage: %s -f playlist\n"
				"# Usage: %s -deskbar\n", argv[0], argv[0]);//, argv[0], argv[0]);
		return B_OK;
	}
	
	bool install = argv[1] && (strcmp(argv[1], "-deskbar") == 0);
	(new MidiApp(install))->Run();
 
     return B_OK;    
} 

MidiApp::MidiApp(bool install) : BApplication("application/x-vnd.ISLE-BUG.MidiWorld") 
{

	if(install){
		install_view1();
		PostMessage(B_QUIT_REQUESTED);
		return;
	}
     //add the playlist mimetype
     BMimeType mime("text/x-midiworld-playlist");
     if (!mime.IsInstalled())
     {
     	mime.Install();
     	mime.SetPreferredApp("application/x-vnd.ISLE-BUG.MidiWorld");
     }
     BRect frameRect(100,100,487,310); 
     MidiWindow *midi_win = new MidiWindow(frameRect);
}

MidiApp::~MidiApp()
{
}

void MidiApp::ArgvReceived(int32 argc, char **argv) 
{
	if ( strcmp(argv[1], "-deskbar") == 0 )
		return;
//Commented out until figure out command line stuff
//	entry_ref my_midi_ref;
//	MidiWindow *midi_win = (MidiWindow*)WindowAt(0);
//	midi_win->LockLooper();
//
//	if(midi_win->daView->MidiList->IsEmpty() == false)
//	{
//		int32 last_item = midi_win->daView->MidiList->CountItems();
//		midi_win->daView->MidiList->RemoveItems(0,last_item);
//	}
//
//  for (int32 i = 2; i < argc; i++) 
//  { 
//	BEntry entry(argv[i], false);
//	if(entry.Exists())
//	{
//		entry.GetRef(&my_midi_ref);	
//    	BNode mynode(&my_midi_ref);
//		char type[B_MIME_TYPE_LENGTH];
//		BNodeInfo bla(&mynode);
//		bla.GetType(type);
//		BMimeType mime(type);
//		if(mime == "audio/x-midi" || mime == "audio/midi")
//		{
//			MidiItem *daItem = new MidiItem(my_midi_ref.name, my_midi_ref);
//			midi_win->daView->MidiList->AddItem(daItem);
//		}
//		else if(mime == "text/x-midiworld-playlist")
//		{
//			midi_win->daView->MidiList->MakeEmpty();
//			BFile *pFile = new BFile(&my_midi_ref, B_READ_WRITE);
//			BMessage playlist;
//			playlist.Unflatten(pFile);
//			entry_ref midi_ref;
//			int32 i=0;
//			while(playlist.FindRef("midis", i++, &midi_ref) == B_OK)
//			{
//  			if(BEntry(&midi_ref).Exists())
//			  {
//				MidiItem *bla = new MidiItem(midi_ref.name, midi_ref);
//				midi_win->daView->MidiList->AddItem(bla);
//			  }
//			}
//		}
//	}
//  }
//	midi_win->UnlockLooper();
} 
        
void MidiApp::RefsReceived(BMessage *msg) 
{
	MidiWindow *midi_win = (MidiWindow*)WindowAt(0);
	midi_win->LockLooper();

	if(midi_win->daView->MidiList->IsEmpty() == false)
	{
		int32 last_item = midi_win->daView->MidiList->CountItems();
		midi_win->daView->MidiList->RemoveItems(0,last_item);
	}
	//add_da_refs(midi_win->daView->MidiList, midi_win->daView->midiRef, msg);
	midi_win->daView->add_da_refs(midi_win->daView->midiRef, msg);
	midi_win->UnlockLooper();
}