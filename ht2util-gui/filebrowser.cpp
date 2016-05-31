#include <wx/wxprec.h>		//use precompiled wx headers unless compiler does not support precompilation
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/file.h>
#include <wx/dir.h>
#include <wx/listctrl.h>
#include <algorithm>

#include "ht2util.h"



void mainFrame::OnListItemActivated(wxListEvent& event) {

	long itemnr = event.GetIndex();
	
	if (dotdot && (!itemnr)) {

		if (currentFBDir.BeforeLast(SEPERATOR_CHAR) != "") {
		
			currentFBDir = currentFBDir.BeforeLast(SEPERATOR_CHAR);
			directoryList->DeleteAllItems();
			populateDirList(currentFBDir);
			
		}
		
		return;
		
	}


	if ((itemnr >= dotdot) && (itemnr < noDirs + dotdot)) {
	
		currentFBDir = currentFBDir + SEPERATOR + dirList[itemnr - dotdot];
		directoryList->DeleteAllItems();
		populateDirList(currentFBDir);
		return;
	}

	return;	
}




//get current directory listing and display it
void mainFrame::populateDirList(wxString currentDir) {

	wxDir dir(currentDir);

	if (!dir.IsOpened()) {

		wxMessageDialog error(NULL, wxT("Error: Cannot load directory list."), wxT("Error"), wxOK_DEFAULT|wxICON_ERROR);
		error.ShowModal();
		return;
	}
	
	
	wxString filename;
	//wxInt16 i = 0;
	bool cont;

	
	//get # of directories
	noDirs = 0;
	cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_DIRS);
	while (cont) {
		noDirs++;
		cont = dir.GetNext(&filename);
	}
	
	//get # of files
	noFiles = 0;
	cont = dir.GetFirst(&filename, "*.ht2s", wxDIR_FILES);
	while (cont) {
		noFiles++;
		cont = dir.GetNext(&filename);
	}
	
	//get double dot
	dotdot = false;
	if (currentFBDir.BeforeLast(SEPERATOR_CHAR) != "") dotdot = true;
	
	delete[] dirList;
	delete[] fileList;
	delete[] fileSizeList;
	dirList = new wxString[noDirs];
	fileList = new wxString[noFiles];
	fileSizeList = new wxString[noFiles];
	

	noDirs = 0;
	cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_DIRS);
	while (cont) {
		dirList[noDirs] = filename;
		noDirs++;
		cont = dir.GetNext(&filename);
	}
	if (noDirs) std::sort(dirList, dirList + noDirs);
	
	noFiles = 0;
	wxString fPath;
	cont = dir.GetFirst(&filename, "*.ht2s", wxDIR_FILES);
	while (cont) {
		fileList[noFiles] = filename;
		fPath = currentDir + SEPERATOR + filename;
		
		wxFile sFile(fPath);
		wxInt16 fSize = sFile.Length();
		if (sFile.IsOpened()) {
			fileSizeList[noFiles] = wxString::Format("%i", (fSize - 9));
			sFile.Close();
		}
		else fileSizeList[noFiles] = "broken";
				
		noFiles++;
		cont = dir.GetNext(&filename);
	}
	if (noFiles) std::sort(fileList, fileList + noFiles);
	
	wxInt16 noAllItems = noDirs + noFiles;
	if (dotdot) noAllItems++;
	
	wxInt16 j = 0;
	wxInt16 dd = 0;

	if (dotdot) {
		directoryList->InsertItem(0, "  ", -1);
		directoryList->SetItemData(0, 0);
		//directoryList->SetItem(0, 0, " ");
		directoryList->SetItem(0, 1, "..");
		directoryList->SetItem(j, 2, " ");
		j++;
		dd++;
	}

	for (; j < noAllItems-noFiles; j++) {
		directoryList->InsertItem(j, "", 0);
		directoryList->SetItemData(j, j);
		//directoryList->SetItem(j, 0, "");
		directoryList->SetItem(j, 1, dirList[j-dd]);
		directoryList->SetItem(j, 2, " ");	
	}
	
	for (; j < noAllItems; j++) {
		directoryList->InsertItem(j, "", 1);
		directoryList->SetItemData(j, j);
		//directoryList->SetItem(j, 0, "");
		directoryList->SetItem(j, 1, fileList[j-dd-noDirs]);
		directoryList->SetItem(j, 2, fileSizeList[j-dd-noDirs]);	
	}
	
	

	directoryList->SetColumnWidth(0,-1);
	directoryList->SetColumnWidth(1,-1);
	directoryList->SetColumnWidth(2,-1);
	
	return;
}



//drag'n'drop handling

exportDropTarget::exportDropTarget(wxListCtrl *owner) {

	m_owner = owner;
}

bool exportDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data) {

	return true;
}



void mainFrame::OnDirListDrag(wxListEvent& event) {

	if (htdata) {			//ignore dnd event if no htfile opened
		long itemnr = event.GetIndex();
	
		if (itemnr >= dotdot + noDirs) {

			wxString text = "blabla";
  
			wxTextDataObject tdo(text);
			wxDropSource tds(tdo, directoryList);
			if (tds.DoDragDrop(wxDrag_CopyOnly)) {
				
				for (long i = itemnr; i < (dotdot + noDirs + noFiles); i++) {

					if (directoryList->GetItemState(i,wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED) {
				
						if (!isEmptyStateAvailable()) return;
						
						currentStateDoc = currentFBDir + SEPERATOR + fileList[itemnr - dotdot - noDirs];
						
						if (!insertState(currentStateDoc)) return;
						
					}	
				}
			}	
		}
	}
	return;
}