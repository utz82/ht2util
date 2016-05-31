//ht2util-gui - ht2 savestate manager utility by utz 2015-16
//version 0.0.2


#include <wx/wxprec.h>		//use precompiled wx headers unless compiler does not support precompilation
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/file.h>
#include <wx/listctrl.h>
#include <wx/artprov.h>
#include <wx/imaglist.h>

#include "ht2util.h"



enum
{
    ID_ExtractState = 5,
    ID_InsertState = 6,
    ID_DeleteState = 7,
    ID_ExportAsm = 8,
    ID_Retune = 10,
    ID_ChangeSamplePointers = 11,
    ID_ReplaceKick = 12,
    ID_DirList = 13,
    ID_StateList = 14

};

wxBEGIN_EVENT_TABLE(mainFrame, wxFrame)
    EVT_MENU(wxID_OPEN,		mainFrame::OnOpenHT)
    EVT_MENU(wxID_SAVE,		mainFrame::OnSaveHT)
    EVT_MENU(wxID_SAVEAS,	mainFrame::OnSaveAsHT)
    EVT_MENU(wxID_CLOSE,	mainFrame::OnCloseHT)
    EVT_MENU(ID_ExtractState,	mainFrame::OnExtractState)
    EVT_MENU(wxID_ADD,		mainFrame::OnInsertState)
    EVT_MENU(wxID_REMOVE,	mainFrame::OnDeleteState)
    EVT_MENU(ID_ExportAsm,	mainFrame::OnExportAsm)
    EVT_MENU(wxID_EXIT,		mainFrame::OnExit)
    
    EVT_MENU(ID_Retune,		mainFrame::OnRetune)
    EVT_MENU(ID_ChangeSamplePointers, mainFrame::OnChangeSamplePointers)
    EVT_MENU(ID_ReplaceKick,	mainFrame::OnReplaceKick)
    
    EVT_MENU(wxID_ABOUT,	mainFrame::OnAbout)
    
    EVT_LIST_ITEM_ACTIVATED(ID_DirList, mainFrame::OnListItemActivated)
    EVT_LIST_BEGIN_DRAG(ID_DirList, mainFrame::OnDirListDrag)
    
    EVT_LIST_BEGIN_DRAG(ID_StateList, mainFrame::OnStateListDrag)
    
    EVT_CLOSE(mainFrame::XExit)
    
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(ht2UtilGUI);


bool ht2UtilGUI::OnInit() {
    mainFrame *frame = new mainFrame( "ht2util", wxPoint(50, 50), wxSize(640, 480) );
    frame->Show( true );
    return true;
    
}


mainFrame::mainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
	htdata = NULL;
	stateData = NULL;
	dirList = NULL;
	fileList = NULL;
	fileSizeList = NULL;
	unsavedChanges = false;

	wxPanel *mainPanel = new wxPanel(this, -1);

	wxBoxSizer *all = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *infoBox = new wxBoxSizer(wxVERTICAL);
	wxGridSizer *mainBox = new wxGridSizer(2,5,5);		//was (1,2,5,5)
	wxBoxSizer *leftSide = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *rightSide = new wxBoxSizer(wxVERTICAL);

	//main menu
	menuFile = new wxMenu;
	#ifndef __WINDOWS__
		menuFile->Append(wxID_OPEN, "&Open HT2 file...", "Open HT2 executable to be modified");
		menuFile->Append(wxID_SAVE, "&Save HT2 file", "Save the current HT2 executable");
	#else
		menuFile->Append(wxID_OPEN, "&Open HT2 file...\tCtrl-O", "Open HT2 executable to be modified");
		menuFile->Append(wxID_SAVE, "&Save HT2 file\tCtrl-S", "Save the current HT2 executable");	
	#endif
	menuFile->Append(wxID_SAVEAS, "&Save HT2 file as...", "Save HT2 executable with a new name");
	menuFile->Append(wxID_CLOSE, "&Close HT2 file", "Close the current HT2 executable");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_ADD, "&Insert savestate...\tIns", "Insert a savestate into the current HT2 executable");
	menuFile->Append(wxID_REMOVE, "&Delete savestate\tDel", "Delete a savestate from the current HT2 executable");
	menuFile->Append(ID_ExtractState, "&Extract savestate...\tCtrl-E", "Extract a savestate and save to file");
	menuFile->Append(ID_ExportAsm, "&Export .asm...", "Decompress and disassemble a savestate, and export as .asm");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);
	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);
	wxMenu *menuTools = new wxMenu;
	menuTools->Append(ID_Retune, "&Retune...", "Modify the gloabl tuning table");
	menuTools->Append(ID_ChangeSamplePointers, "&Change Sample Pointers...", "Change the standard sample pointers");
	menuTools->Append(ID_ReplaceKick, "&Replace Kick...", "Replace the standard kick sample");
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append( menuFile, "&File" );
//	menuBar->Append( menuTools, "&Tools" );		//TODO disabled for now as functionality is not yet implemented
	menuBar->Append( menuHelp, "&Help" );
	SetMenuBar( menuBar ); 
	disableMenuItems();
    
	//main window
	htFileInfo = new wxStaticText(mainPanel, -1, wxT("model:\nHT2 version:\nsavestate version:"), wxPoint(-1, -1));
	htSizeInfo = new wxStaticText(mainPanel, -1, wxT("mem free:"), wxPoint(-1, -1));
	savestateList = new wxListCtrl(mainPanel, ID_StateList, wxPoint(-1,-1), wxSize(-1,-1), wxLC_REPORT);
	directoryList = new wxListCtrl(mainPanel, ID_DirList, wxPoint(-1,-1), wxSize(-1,-1), wxLC_REPORT);


	//construct savestate list view	
	wxListItem itemCol;
	itemCol.SetText(wxT("slot"));
	savestateList->InsertColumn(0, itemCol);
	savestateList->SetColumnWidth(0, wxLIST_AUTOSIZE );

	itemCol.SetText(wxT("begin"));
	savestateList->InsertColumn(1, itemCol);
	savestateList->SetColumnWidth(1, wxLIST_AUTOSIZE );

	itemCol.SetText(wxT("end"));
	savestateList->InsertColumn(2, itemCol);
	savestateList->SetColumnWidth(2, wxLIST_AUTOSIZE );

	itemCol.SetText(wxT("length"));
	itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
	savestateList->InsertColumn(3, itemCol);
	savestateList->SetColumnWidth(3, wxLIST_AUTOSIZE );
	
	stateDropTarget *mdt = new stateDropTarget(savestateList);
	savestateList->SetDropTarget(mdt);
	
	populateEmptySList();	
	
	//construct image list
	const wxSize iconSize = wxSize(24,24);
	const wxIcon folderIcon = wxArtProvider::GetIcon(wxART_FOLDER, wxART_OTHER, iconSize);
	const wxIcon fileIcon = wxArtProvider::GetIcon(wxART_NORMAL_FILE, wxART_OTHER, iconSize);
	fbIcons = new wxImageList(24, 24, false, 0);
	fbIcons->Add(folderIcon);
	fbIcons->Add(fileIcon);
	
	//construct directory listing
	wxListItem dirListCol;	
	directoryList->AssignImageList(fbIcons, wxIMAGE_LIST_SMALL);
	
	dirListCol.SetText(wxT(""));
	itemCol.SetImage(-1);
	directoryList->InsertColumn(0, dirListCol);
	directoryList->SetColumnWidth(0, wxLIST_AUTOSIZE );
	
	dirListCol.SetText(wxT("name"));
	directoryList->InsertColumn(1, dirListCol);
	directoryList->SetColumnWidth(1, wxLIST_AUTOSIZE );
	
	dirListCol.SetText(wxT("size"));
	dirListCol.SetAlign(wxLIST_FORMAT_RIGHT);
	directoryList->InsertColumn(2, dirListCol);
	directoryList->SetColumnWidth(2, wxLIST_AUTOSIZE );
	
	currentFBDir = wxGetCwd();
	populateDirList(currentFBDir);
	
	exportDropTarget *mdtx = new exportDropTarget(directoryList);
	directoryList->SetDropTarget(mdtx);
	
	
	//construct main layout
	all->Add(new wxPanel(mainPanel, -1));
	
	all->Add(infoBox, 0, wxALIGN_LEFT | wxALL, 10);
		infoBox->Add(htFileInfo);
		infoBox->Add(htSizeInfo);
		
	all->Add(mainBox, 1, wxEXPAND | wxALL, 10);
	
	mainBox->Add(leftSide, 1, wxEXPAND);
		leftSide->Add(savestateList,1,wxEXPAND);
			
	mainBox->Add(rightSide, 1, wxEXPAND);
		rightSide->Add(directoryList,1,wxEXPAND);
	
	mainPanel->SetSizer(all);

	CreateStatusBar();
	SetStatusText( "" );
	
}



void mainFrame::XExit(wxCloseEvent& event) {

	if (unsavedChanges) {
	
		wxMessageDialog *unsavedChgMsg = new wxMessageDialog(NULL, wxT("Save changes?"), wxT("Question"), 
			wxCANCEL | wxYES_NO | wxCANCEL_DEFAULT | wxICON_QUESTION);

		wxInt16 response = unsavedChgMsg->ShowModal();
		if (response == wxID_CANCEL) return;
		else if (response == wxID_YES) saveHTFile();
		
	}
	
	event.Skip();

}



//recalculate checksum and write it to file
void mainFrame::writeChecksum() {

	long checksum = 0;
	for (int i=55; i < (htsize-2); i++) {
	
		checksum += htdata[i];
		
	}
	
	checksum = checksum & 0xffff;
	
	htdata[htsize-2] = static_cast<unsigned char>(checksum & 0xff);
	htdata[htsize-1] = static_cast<unsigned char>(long(checksum/256) & 0xff);
	
	return;
	
}



void mainFrame::saveHTFile() {
	
	wxFile htfile;
	if (!htfile.Open(CurrentDocPath, wxFile::write)) {
	
		wxMessageDialog error1(NULL, wxT("Error: Could not save file."), wxT("Error"), wxOK_DEFAULT|wxICON_ERROR);
		error1.ShowModal();
		
		return;
		
	}
	
	htfile.Write(htdata, (size_t) htsize);
	htfile.Close();
	unsavedChanges = false;
	wxTopLevelWindow::SetTitle("ht2util - " + CurrentDocPath);
		
	return;
}



void mainFrame::disableMenuItems() {

	menuFile->Enable(wxID_SAVE, false);
	menuFile->Enable(wxID_SAVEAS, false);
	menuFile->Enable(wxID_CLOSE, false);
	menuFile->Enable(wxID_ADD, false);
	menuFile->Enable(wxID_REMOVE, false);
	menuFile->Enable(ID_ExtractState, false);
	menuFile->Enable(ID_ExportAsm, false);

}

void mainFrame::enableMenuItems() {

	menuFile->Enable(wxID_SAVE, true);
	menuFile->Enable(wxID_SAVEAS, true);
	menuFile->Enable(wxID_CLOSE, true);
	menuFile->Enable(wxID_ADD, true);
	menuFile->Enable(wxID_REMOVE, true);
	menuFile->Enable(ID_ExtractState, true);
	menuFile->Enable(ID_ExportAsm, true);

}
