
#include <wx/wxprec.h>		//use precompiled wx headers unless compiler does not support precompilation
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/file.h>
#include <wx/listctrl.h>

#include "ht2util.h"



void mainFrame::OnExit(wxCommandEvent& WXUNUSED(event)) {

	Close(true);
	
}



void mainFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {

    wxMessageBox( "htutil v0.1\n\nHoustonTracker 2 savestate manager\nby utz 2015-2016",
                  "About htutil", wxOK | wxICON_INFORMATION );
		  
}



void mainFrame::OnSaveHT(wxCommandEvent& WXUNUSED(event)) {

	if (htdata) saveHTFile();
	
	return;
		
}


void mainFrame::OnRetune(wxCommandEvent& WXUNUSED(event)) {

	wxMessageDialog error(NULL, wxT("This feature is not implemented yet"), wxT("Info"), wxOK_DEFAULT|wxICON_ERROR);
	error.ShowModal();
	
}

void mainFrame::OnChangeSamplePointers(wxCommandEvent& WXUNUSED(event)) {

	wxMessageDialog error(NULL, wxT("This feature is not implemented yet"), wxT("Info"), wxOK_DEFAULT|wxICON_ERROR);
	error.ShowModal();
	
}

void mainFrame::OnReplaceKick(wxCommandEvent& WXUNUSED(event)) {

	wxMessageDialog error(NULL, wxT("This feature is not implemented yet"), wxT("Info"), wxOK_DEFAULT|wxICON_ERROR);
	error.ShowModal();
	
}



void mainFrame::OnSaveAsHT(wxCommandEvent& WXUNUSED(event)) {

	if (htdata) {
	
		wxString filetype = "HT2 executable (*" + fileExt + ")|*" + fileExt;
		wxString suggestedFileName = "[untitled]" + fileExt;
		wxFileDialog *SaveDialog = new wxFileDialog(this, _("Save file as?"), wxEmptyString, suggestedFileName,
			filetype, wxFD_SAVE|wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

	 	if (SaveDialog->ShowModal() == wxID_OK) {

	 		CurrentDocPath = SaveDialog->GetPath();
			CurrentFileName = SaveDialog->GetFilename();

			saveHTFile();
	 	}
		
		SaveDialog->Destroy();
		
	}
	
	return;
	
}



void mainFrame::OnOpenHT(wxCommandEvent& WXUNUSED(event)) {

	//check if a file is currently opened and has unsaved changes
	if (unsavedChanges) {

		wxMessageDialog *unsavedChgMsg = new wxMessageDialog(NULL, wxT("Save changes?"), wxT("Question"), 
			wxCANCEL | wxYES_NO | wxCANCEL_DEFAULT | wxICON_QUESTION);

		wxInt16 response = unsavedChgMsg->ShowModal();
		if (response == wxID_CANCEL) return;
		else if (response == wxID_YES) saveHTFile();
					
	}
	


	wxFileDialog *OpenDialog = new wxFileDialog(
		this, _("Choose a file to open"), wxEmptyString, wxEmptyString,
		_("HT2 executable (*.82p, *.83p, *.8xp)|*.82p;*.83p;*.8xp"), wxFD_OPEN|wxFD_FILE_MUST_EXIST, wxDefaultPosition);
 
	if (OpenDialog->ShowModal() == wxID_OK) {	//unless user clicked "cancel"
	
		CurrentDocPath = OpenDialog->GetPath();
		wxFile htfile(CurrentDocPath);
		
		if (!htfile.IsOpened()) {
		
			wxMessageDialog error1(NULL, wxT("Error: File could not be opened."), wxT("Error"), wxOK_DEFAULT|wxICON_ERROR);
			error1.ShowModal();
			
			return;
			
		}
		
		htsize = htfile.Length();
		
		if (htsize == wxInvalidOffset) {
		
			wxMessageDialog error2(NULL, wxT("Error: File is corrupt."), wxT("Error"), wxOK_DEFAULT|wxICON_ERROR);
			error2.ShowModal();
			
			return;
			
		}
		
		delete[] htdata;
		htdata = new wxUint8[htsize];
		
		if (htfile.Read(htdata, (size_t) htsize) != htsize) {
		
			wxMessageDialog error3(NULL, wxT("Error: File could not be read."), wxT("Error"), wxOK_DEFAULT|wxICON_ERROR);
			error3.ShowModal();
		
			delete[] htdata;
			htdata = NULL;
			
			return;
			
		}

		htfile.Close();
		
		CurrentFileName = OpenDialog->GetFilename();
		
		size_t dot = CurrentFileName.find_last_of(".");			//get file extension
		if (dot != std::string::npos) fileExt = CurrentFileName.substr(dot, CurrentFileName.size() - dot);
		
		const wxString calcversion[3] = { "TI-82", "TI-83", "TI-83 Plus/TI-84 Plus" };
		if (fileExt == ".82p" || fileExt == ".82P") tmodel = 0;
		if (fileExt == ".83p" || fileExt == ".83P") tmodel = 1;
		if (fileExt == ".8xp" || fileExt == ".8XP" || fileExt == ".8xP" || fileExt == ".8Xp") tmodel = 2;
		
		legacyFileEnd = false;
		
		//read savestate version
		if (tmodel == 0 || htdata[htsize-3] != 0) {
		
			statever = htdata[htsize-4];	//detect legacy HT2 version: if val at offset -3 is 0, it's a legacy binary
						
		} else {
		
			statever = htdata[htsize-6];
			legacyFileEnd = true;
			
		}
		
		if (statever > MAX_SUPPORTED_SAVESTATE_VERSION) {
		
			wxMessageDialog error4(NULL, wxT("Warning: File is of a newer version than supported by this version "
			  "of ht2util.\nSome functionality may not perform as expected."),
			  wxT("Warning"), wxOK_DEFAULT|wxICON_INFORMATION);
			error4.ShowModal();
			
		}
		
			
		baseOffset = getBaseOffset(htdata);
		
		if (baseOffset == 0xffff) {
			wxMessageDialog error5(NULL, wxT("Error: Not a valid HoustonTracker 2 file."), wxT("Error"),
			  wxOK_DEFAULT|wxICON_ERROR);
			error5.ShowModal();
		
			delete[] htdata;
			htdata = NULL;
			
			return;
			
		}
		
		baseDiff = getBaseDiff(tmodel, baseOffset);	
				
		char htverh = htdata[baseOffset] - 0x30;				//determine HT2 version
		char htverl = htdata[baseOffset+1] - 0x30;
		htver = htverh * 10 + htverl;
		wxString htVerStr = wxString::Format(wxT("%i"),htver);
		wxString stateVerStr = wxString::Format(wxT("%i"),statever);
		
		fOffset = getLUToffset(statever, htsize);
	
		if (fOffset == -1) {
		
			wxMessageDialog error6(NULL, wxT("Error: Savestate lookup table not found."), wxT("Error"), wxOK_DEFAULT|wxICON_ERROR);
			error6.ShowModal();
		
			delete[] htdata;
			htdata = NULL;
			
			return;
			
		}
		
		lutOffset = static_cast<unsigned>(fOffset) + 1;
		
		readLUT(lutOffset);

		wxTopLevelWindow::SetTitle("ht2util - " + CurrentDocPath);
		htFileInfo->SetLabel("model: " + calcversion[tmodel] + "\nHT2 version: 2." + htVerStr + "\nsavestate version: " + stateVerStr);
		wxString freeMem = wxString::Format("%i", getFreeMem());
		htSizeInfo->SetLabel("free mem: " + freeMem + " bytes");
		
	}
	
	unsavedChanges = false;
	enableMenuItems();
	return;

}

void mainFrame::OnCloseHT(wxCommandEvent& WXUNUSED(event)) {

	if (htdata) {
	
		if (unsavedChanges) {

			wxMessageDialog *unsavedChgMsg = new wxMessageDialog(NULL, wxT("Save changes?"), wxT("Question"), 
				wxCANCEL | wxYES_NO | wxCANCEL_DEFAULT | wxICON_QUESTION);

			wxInt16 response = unsavedChgMsg->ShowModal();
			if (response == wxID_CANCEL) return;
			else if (response == wxID_YES) saveHTFile();
			
		}

		wxTopLevelWindow::SetTitle("ht2util");
		htFileInfo->SetLabel("model:\nHT2 version:\nsavestate version:");
		htSizeInfo->SetLabel("mem free:");
		
		delete[] htdata;
		htdata = NULL;
		clearSList();
		
		unsavedChanges = false;
		disableMenuItems();
		
	}
	
	return;

}


//load a savestate from file and insert it
void mainFrame::OnInsertState(wxCommandEvent& WXUNUSED(event)) {

	//check if user has loaded a HT2 .8*p
	if (!htdata) {
	
		wxMessageDialog error(NULL, wxT("Error: No HT2 executable loaded."), wxT("Error"), wxOK_DEFAULT|wxICON_ERROR);
		error.ShowModal();
		return;
		
	}

	//check if there are empty save slots available
	if (!isEmptyStateAvailable()) return;

	//initiate file dialog
	wxFileDialog *OpenDialog = new wxFileDialog(this, _("Choose a file to open"), wxEmptyString, wxEmptyString,
		_("HT2 savestate (*.ht2s)|*.ht2s"), wxFD_OPEN, wxDefaultPosition);
 
	//unless user clicked "cancel"
	if (OpenDialog->ShowModal() == wxID_OK) {
	
		currentStateDoc = OpenDialog->GetPath();
		
		if (!insertState(currentStateDoc)) return;
		
	}
	
	return;
	
}



//extract a savestate and export to file
void mainFrame::OnExtractState(wxCommandEvent& WXUNUSED(event)) {

	//check if user has loaded a HT2 .8*p
	if (!htdata) {
		wxMessageDialog error(NULL, wxT("Error: No HT2 executable loaded."), wxT("Error"), wxOK_DEFAULT|wxICON_ERROR);
		error.ShowModal();
		return;
	}
	
	wxString suggestedFileName;
	wxDateTime dt;
	wxString now;

	for (long i=0; i<8; i++) {

		if (savestateList->GetItemState(i,wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED) {

			dt = wxDateTime::Now();
			now = dt.Format(wxT("-%y-%b-%d_%H-%M-%S"));
		
			suggestedFileName = CurrentFileName + "-slot" + wxString::Format("%d",static_cast<wxInt16>(i)) + "-" + now + ".ht2s";
			
			wxFileDialog *SaveDialog = new wxFileDialog(this, _("Save state as?"), wxEmptyString, suggestedFileName,
				_("HT2 savestate (*.ht2s)|*.ht2s"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

			if (SaveDialog->ShowModal() == wxID_OK) {
			
				if (statelen[i] != 0) {		//unless savestate is empty
			
					currentStateDoc = SaveDialog->GetPath();
				
					exportState(currentStateDoc, i);
					
					directoryList->DeleteAllItems();
					populateDirList(currentFBDir);
					
				}
			}
			
			SaveDialog->Destroy();			
		}
	}

	return;
}


void mainFrame::OnDeleteState(wxCommandEvent& WXUNUSED(event)) {
	
	//check if user has loaded a HT2 .8*p
	if (!htdata) {
		wxMessageDialog error(NULL, wxT("Error: No HT2 executable loaded."), wxT("Error"), wxOK_DEFAULT|wxICON_ERROR);
		error.ShowModal();
		return;
	}
	
	for (long i=0; i<8; i++) {

		if (savestateList->GetItemState(i,wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED) deleteState(i);
		
	}
	
	writeChecksum();
	wxString freeMem = wxString::Format("%i", getFreeMem());
	htSizeInfo->SetLabel("free mem: " + freeMem + " bytes");
	unsavedChanges = true;
	wxTopLevelWindow::SetTitle("ht2util - " + CurrentDocPath + " [modified]");
	
	return;
	
}



void mainFrame::OnExportAsm(wxCommandEvent& WXUNUSED(event)) {
	
	//check if user has loaded a HT2 .8*p
	if (!htdata) {
		wxMessageDialog error(NULL, wxT("Error: No HT2 executable loaded."), wxT("Error"), wxOK_DEFAULT|wxICON_ERROR);
		error.ShowModal();
		return;
	}
	
	wxString now,suggestedFileName;
	wxDateTime dt;
	
	
	for (long i=0; i<8; i++) {

		if (savestateList->GetItemState(i,wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED) {
		
			//now = wxNow();
			dt = wxDateTime::Now();
			now = dt.Format(wxT("-%y-%b-%d_%H-%M-%S"));
		
			suggestedFileName = CurrentFileName + "-slot" + wxString::Format("%d",static_cast<wxInt16>(i)) + "-" + now + ".asm";
			
			wxFileDialog *SaveDialog = new wxFileDialog(this, _("Save state as?"), wxEmptyString, suggestedFileName,
				_("assembler source (*.asm)|*.asm"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

			if (SaveDialog->ShowModal() == wxID_OK) {
			
				currentAsmDoc = SaveDialog->GetPath();
				exportAsm(currentAsmDoc, i);
				
				SaveDialog->Destroy();
			}
		}	
	}


	return;
}

