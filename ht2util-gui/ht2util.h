
#ifndef __HTUTIL_HEADER__
#define __HTUTIL_HEADER__

#include <wx/dnd.h>

#define MAX_SUPPORTED_SAVESTATE_VERSION 1	//latest supported savestate version
#ifndef __WINDOWS__
	#define SEPERATOR "/"
	#define SEPERATOR_CHAR '/'
#else
	#define SEPERATOR "\\"
	#define SEPERATOR_CHAR '\\'
#endif

class ht2UtilGUI: public wxApp { 
public:
	virtual bool OnInit(); 
};

class stateDropTarget: public wxTextDropTarget
{
public:
	stateDropTarget(wxListCtrl *owner);
	virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data);
	wxListCtrl *m_owner;
};

class exportDropTarget: public wxTextDropTarget
{
public:
	exportDropTarget(wxListCtrl *owner);
	virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data);
	wxListCtrl *m_owner;
};


class mainFrame: public wxFrame
{
public:
	mainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    
	wxMenu *menuFile;

	wxStaticText *htFileInfo;
	wxStaticText *htSizeInfo;
	wxListCtrl *savestateList;
	wxListCtrl *directoryList;
    
	wxString CurrentDocPath;
	wxString CurrentFileName;
	wxString fileExt;
	wxString currentStateDoc;
	wxString currentAsmDoc;
	wxUint8 *stateData;
	wxFileOffset stateSize;
	unsigned totalSsize;			//total size of all savestates
    
	int tmodel,baseOffset,fOffset;
	unsigned htver,statever,lutOffset,baseDiff;
	bool legacyFileEnd, unsavedChanges;


	wxFileOffset htsize;
	wxUint8 *htdata;
    
	wxString *dirList;
	wxString *fileList;
	wxString *fileSizeList;
	wxInt16 noDirs;
	wxInt16 noFiles;
	bool dotdot;
	wxString currentFBDir;			//current file browser path
	wxImageList *fbIcons;

	unsigned statebeg[8], statelen[8];
    
   
private:
	void OnOpenHT(wxCommandEvent& event);
	void OnCloseHT(wxCommandEvent& event);
	void OnSaveHT(wxCommandEvent& event);
	void OnSaveAsHT(wxCommandEvent& event);
	void OnExtractState(wxCommandEvent& event);
	void OnInsertState(wxCommandEvent& event);
	void OnDeleteState(wxCommandEvent& event);
	void OnExportAsm(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void XExit(wxCloseEvent& event);
    
	void OnRetune(wxCommandEvent& event);
	void OnChangeSamplePointers(wxCommandEvent& event);
	void OnReplaceKick(wxCommandEvent& event);
    
	void OnAbout(wxCommandEvent& event);
    
	void disableMenuItems();
	void enableMenuItems();
    
	int getBaseOffset(wxUint8 *htdata);
	wxInt16 getFreeMem();
	int getLUToffset(char statev, wxFileOffset filesize);
	void readLUT(int fileoffset);
	void populateEmptySList();
	void populateDirList(wxString currentDir);
	void clearSList();
	unsigned getBaseDiff(int model, int baseOffset);
	void writeChecksum();
    
	void saveHTFile();
    
	void OnListItemActivated(wxListEvent& event);
	void OnDirListDrag(wxListEvent& event);
    
	void OnStateListDrag(wxListEvent& event);
    
	bool isEmptyStateAvailable();
	bool insertState(wxString currentStateDoc);
	bool exportState(wxString currentStateDoc, wxInt16 itemnr);
	bool deleteState(long itemnr);
	bool exportAsm(wxString currentAsmDoc, long itemnr);
    
	wxDECLARE_EVENT_TABLE();
};

#endif