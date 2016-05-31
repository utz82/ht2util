#include <wx/wxprec.h>		//use precompiled wx headers unless compiler does not support precompilation
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/file.h>
#include <wx/dir.h>
#include <wx/listctrl.h>

#include "ht2util.h"



void mainFrame::populateEmptySList() {

	wxString temp;
	for (int i=0; i<8; i++) {
		
		temp.Printf(wxT("%d"), i);
		savestateList->InsertItem(i, temp, 0);
		savestateList->SetItemData(i, i);
		savestateList->SetItem(i, 0, temp);
		temp.Printf(wxT("-----"));
		savestateList->SetItem(i, 1, temp);
		savestateList->SetItem(i, 2, temp);
		savestateList->SetItem(i, 3, temp);

	}
	
	for (int j=0; j<4; j++) {
	
		savestateList->SetColumnWidth(j,wxLIST_AUTOSIZE);
		
	}

	return;
}


//clear savestate list
void mainFrame::clearSList() {

	wxString temp;
	savestateList->DeleteAllItems();
	for (int i=0; i<8; i++) {
		
		temp.Printf(wxT("-----"));
		savestateList->SetItem(i, 1, temp);
		savestateList->SetItem(i, 2, temp);
		savestateList->SetItem(i, 3, temp);
		
	}
	
	for (int j=0; j<4; j++) {
	
		savestateList->SetColumnWidth(j,wxLIST_AUTOSIZE);
		
	}
	
	return;
}



//drag'n'drop handling
stateDropTarget::stateDropTarget(wxListCtrl *owner) {

	m_owner = owner;
}

bool stateDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data) {

	return true;
}



void mainFrame::OnStateListDrag(wxListEvent& event) {

	if (htdata) {			//ignore dnd event if no htfile opened
	
		wxString text = "blabla";
		wxString now;
		wxDateTime dt;
		
		wxTextDataObject tdo(text);
		wxDropSource tds(tdo, savestateList);
		if (tds.DoDragDrop(wxDrag_CopyOnly)) {
		
			
			for (long i=0; i<8; i++) {

				if (savestateList->GetItemState(i,wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED) {
		
					if (statelen[i] != 0) {
					
						//now = wxNow();
						dt = wxDateTime::Now();
						now = dt.Format(wxT("-%y-%b-%d_%H-%M-%S"));
		
						currentStateDoc = currentFBDir + SEPERATOR + CurrentFileName + "-slot" + wxString::Format("%d",static_cast<wxInt16>(i)) + "-" + now + ".ht2s";
						
						if (!exportState(currentStateDoc, i)) return;
						
						directoryList->DeleteAllItems();
						populateDirList(currentFBDir);
										
					}
				}
			}
		}
	}
	
	return;

}


