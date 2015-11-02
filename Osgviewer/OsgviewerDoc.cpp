
// OsgviewerDoc.cpp : implementation of the COsgviewerDoc class
//

#include "stdafx.h"
#include "commonData.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Osgviewer.h"
#endif

#include "OsgviewerDoc.h"
#include "OsgviewerView.h"


#include "MFC_OSG.h"
#include "winsock2.h"
#include "MainFrm.h"
#include "OutputWnd.h"
#include "resource.h"
#include "ConnectDlg.h"
#include "ExportDlg.h"
#include "OptionDlg.h"


#include <propkey.h>

#include "bvh.h"

//#include <fstream>
//using namespace std;

extern COsgviewerView* viewSet[4];

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// COsgviewerDoc

IMPLEMENT_DYNCREATE(COsgviewerDoc, CDocument)

BEGIN_MESSAGE_MAP(COsgviewerDoc, CDocument)
	ON_COMMAND(ID_CONNECT_TCP, &COsgviewerDoc::OnTCLConnect)
	ON_COMMAND(ID_BUTTON_CONNECT, &COsgviewerDoc::OnTCLConnect)
	ON_COMMAND(ID_OPTION_PREFERENCES, &COsgviewerDoc::OnOptionPreference)
	ON_COMMAND(ID_FILE_EXPORT, &COsgviewerDoc::OnFileExport)
	ON_COMMAND(ID_FILE_RECORD, &COsgviewerDoc::OnRecord)
	ON_COMMAND(ID_FILE_STOP, &COsgviewerDoc::OnStop)
	ON_COMMAND(ID_FILE_SERVERSTART, &COsgviewerDoc::OnServerStart)
END_MESSAGE_MAP()


// COsgviewerDoc construction/destruction

COsgviewerDoc::COsgviewerDoc()
{
	// TODO: add one-time construction code here
	realWindowNum = 0;

	QString dataNameList[28] = {"hip","hip1","hip2","hip3","chest","neck","head","",
		"lCollar","lShldr","lForeArm","lHand","",
		"rCollar","rShldr","rForeArm","rHand","",
		"lThigh","lShin","lFoot","lFoot1","",
		"rThigh","rShin","rFoot","rFoot1","",
	};

	for(int i = 0; i < 28; i++)
	{
		if (dataNameList[i] != "")
		{
			dataNameMap[dataNameList[i]] = i;
		}
	}
	recordStart = -1;
	recordEnd = -1;
	curFrameIndex = 0;

	OnServerStart();
}
volatile bool isFinish = false;
COsgviewerDoc::~COsgviewerDoc()
{
	isFinish = true;
}
void COsgviewerDoc::OnRecord()
{
	vector<sDataBufferEngine>* pFrameData = getCurFrame(curFrameIndex);
	if (!pFrameData)
		return;
	vector<sDataBufferEngine>& framedata = *pFrameData;
	recordStart = framedata.size();

	CMainFrame *pMain = (CMainFrame *)AfxGetApp()->m_pMainWnd;
	pMain->getTimePane().startRecord();

	/*ifstream lxqin("lxq.bb",ios::binary);
	unsigned int sz = -1;
	lxqin.read((char*)&sz, sizeof(sz));
	framedata.clear();
	framedata.resize(sz);
	for (int i = 0; i <sz; i++)
	{
	lxqin.read((char*)&framedata.at(i), sizeof(sDataBufferEngine));
	}*/
}
void COsgviewerDoc::OnStop()
{
	vector<sDataBufferEngine>* pFrameData = getCurFrame(curFrameIndex);
	if (!pFrameData)
		return;
	vector<sDataBufferEngine>& framedata = *pFrameData;
	recordEnd = framedata.size()-1;

	CMainFrame *pMain = (CMainFrame *)AfxGetApp()->m_pMainWnd;
	pMain->getTimePane().stopRecord();

	

	/*g_cs.Lock();
	ofstream lxqout("lxq.bb",ios::binary);
	unsigned int sz = framedata.size();
	lxqout.write((char*)&sz, sizeof(sz));
	for (int i = 0; i < framedata.size(); i++)
	{
	lxqout.write((char*)&framedata.at(i), sizeof(sDataBufferEngine));
	}
	g_cs.Unlock();*/
}

void COsgviewerDoc::OnServerStart()
{
	ShellExecute( NULL,L"open", L"NewSkeletonTester.exe",NULL,L"ReleaseFast",SW_SHOWNORMAL);
}

volatile int connectState = 1;
void Render(void * b)
{	
	sDataBufferEngine sending_buffer;

	/*while(1)
	{*/
	CString strIP, strPort;
	int nPort;
	CString IpPort;
	if (connectState == 0)
	{
		CString tmpIp;
		GetPrivateProfileString(_T("Server"),_T("ServerIP"),NULL,tmpIp.GetBuffer(40),40,_T(".\\option.ini"));
		tmpIp.ReleaseBuffer();
 
		CString portstr;//�洢
		GetPrivateProfileString(_T("Server"),_T("Port"),NULL,portstr.GetBuffer(40),40,_T(".\\option.ini"));
		portstr.ReleaseBuffer();
		nPort = _ttoi(portstr);

		connectState = 1;
		strIP = tmpIp;
		strPort = portstr;
	}
	else
	{
		ConnectDlg optionD;
		optionD.DoModal();
		strIP = optionD.ip;//_T("192.168.1.2");
		//ת����Ҫ���ӵĶ˿���������
		nPort = _ttoi(optionD.port);
		strPort = optionD.port;
	}

	IpPort = strIP + strPort;

	for (int i = 0; i < mapIndexToIpPort.size(); i++)
	{
		if (mapIndexToIpPort[i] == IpPort)
		{
			AfxMessageBox(_T("��ip�Ͷ˿�������"));
			_endthread();
			return;
		}
	}

	AfxSocketInit();
	CSocket aSocket;
	//��ʼ�� CSocket ����, ��Ϊ�ͻ��˲���Ҫ���κζ˿ں͵�ַ, ������Ĭ�ϲ�������
	if(!aSocket.Create())
	{
		AfxMessageBox(_T("error!"));
		_endthread();
		return;
	}

	//����ָ���ĵ�ַ�Ͷ˿�
	if(aSocket.Connect(strIP, nPort))
	{
		char szRecValue[1024] = {0};
		g_cs.Lock();
		framedatas.resize(framedatas.size()+1);
		vector<sDataBufferEngine>& framedata = framedatas.back();
		mapIndexToIpPort.push_back(IpPort);
		g_cs.Unlock();
		CMainFrame *pMain = (CMainFrame *)AfxGetApp()->m_pMainWnd;
		pMain->getClassView().ResetPerson();

		//�������ڶ�����������ʱ����һ��ģ��
		if (framedatas.size() > 1)
		{
			cOSG::addSkeleton();
		}
		
		g_csForView.Lock();
		for (int i = 0; i < windowNum_; i++)
		{
			cOSG* osg = viewSet[i]->getOSG();
			osg->ResetSkeleton();
		}
		g_csForView.Unlock();
		//sDataBufferEngine sdb;
		while(aSocket.Receive((void *)&sending_buffer, sizeof(sending_buffer))) //���շ��������ͻ���������(�÷���������, �ڴ˵ȴ������ݽ��յ��ż�������ִ��)
		{
			//osg->skeleton->sBuffer1.setBySdb(sdb);
			g_cs.Lock();

			framedata.push_back(sending_buffer);
			g_cs.Unlock();
		}

		//ɾ�����ݺ�ģ��
		g_cs.Lock();
		vector<CString>::iterator mapi = mapIndexToIpPort.begin();
		vector<Skeleton*>::iterator modeli = cOSG::skeletonArray.begin();
		vector<osg::Node*>::iterator nodei = cOSG::skeletonGeoArray.begin(); 
		list<vector<sDataBufferEngine>>::iterator framesi = framedatas.begin();

		osg::Node* removedGeo = NULL;
		for (;mapi != mapIndexToIpPort.end(); mapi++, modeli++, nodei++, framesi++)
		{
			if (*mapi == IpPort)
			{
				mapIndexToIpPort.erase(mapi);
				if (cOSG::skeletonArray.size() > 1)
				{
					removedGeo = *nodei;
					cOSG::skeletonArray.erase(modeli);
					cOSG::skeletonGeoArray.erase(nodei);
					
				}
				framedatas.erase(framesi);
				break;
			}
		}
		g_cs.Unlock();
		g_csForView.Lock();
		for (int i = 0; i < windowNum_; i++)
		{
			cOSG* osg = viewSet[i]->getOSG();
			if (removedGeo)
			{
				removedGeo->setNodeMask(0x0);
				osg->RemoveSkeleton(removedGeo);

			}
			
			osg->ResetSkeleton();
		}
		g_csForView.Unlock();

		pMain->getClassView().ResetPerson();
	}
	else
	{
		AfxMessageBox(_T("����ʧ�ܣ������ַ"));
	}
	aSocket.Close();

	//}

	_endthread();
}

void Render2(void * b)
{	
	sDataBufferEngine sending_buffer;
	CMainFrame *pMain = (CMainFrame *)AfxGetApp()->m_pMainWnd;
	while(!isFinish)
	{
		if (connectState == 0)
		{
			Sleep(100);
		}
		if (realWindowNum == 0)
		{
			if (windowNum_ == 0)
			{
				break;
			}
			continue;
		}
		//����ģ����Ϣ
		g_cs.Lock();
		int j = 0;
		for (list<vector<sDataBufferEngine>>::iterator i = framedatas.begin(); i != framedatas.end(); i++, j++)
		{
			if (!(i->empty()))
			{
				cOSG::skeletonArray[j]->sBuffer1 = i->back();
			}
		}
		g_cs.Unlock();

		//ˢ�´���
		g_csForView.Lock();
		for (int i = 0; i < windowNum_; i++)
		{
			viewSet[i]->isConnected = true;
			cOSG* osg = viewSet[i]->getOSG();
			osg->UpdateFrame();
		}

		pMain->getOutPutWnd().updateChart();
		pMain->getTimePane().calculate();
		g_csForView.Unlock();
	}
	//_CrtDumpMemoryLeaks();

	_endthread();


}

BOOL COsgviewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	cOSG::addSkeleton();//������һ��ģ��

	m_csFileName = L"biped.3ds";

	isFinish = false;
	mThreadHandle2 = (HANDLE)_beginthread(Render2, 0, 0); 

	connectState = 0;
	//mThreadHandle = (HANDLE)_beginthread(Render, 0, 0); 
    this->SetTitle(_T("lxq"));

	return TRUE;
}


// COsgviewerDoc serialization

void COsgviewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

void COsgviewerDoc::OnTCLConnect()
{
	mThreadHandle = (HANDLE)_beginthread(Render, 0, 0); 
	
}
void COsgviewerDoc::OnOptionPreference()
{
	OptionDlg optionD;
	optionD.DoModal();
}
void COsgviewerDoc::OnFileExport()
{
	if (recordStart == -1 || recordEnd == -1)
	{
		vector<sDataBufferEngine>* pFrameData = getCurFrame(curFrameIndex);
		if (!pFrameData)
			return;
		vector<sDataBufferEngine>& framedata = *pFrameData;
		ExportDlg eDlg(0, framedata.size()-1);
		eDlg.DoModal();
	}
	else
	{
		ExportDlg eDlg(recordStart, recordEnd);
		eDlg.DoModal();
	}	
}
void COsgviewerDoc::OnInitialUpdate()
{
	
}
#ifdef SHARED_HANDLERS

// Support for thumbnails
void COsgviewerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void COsgviewerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void COsgviewerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// COsgviewerDoc diagnostics

#ifdef _DEBUG
void COsgviewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void COsgviewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// COsgviewerDoc commands
