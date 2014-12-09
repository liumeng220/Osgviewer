//////////////////////////////////////////////////////////// CXTreeCtrl
#include "stdafx.h"
//#include "DragTree.h"
#include "CXTreeCtrl.h"
#include "MainFrm.h"


#define DRAG_DELAY 60
CXTreeCtrl::CXTreeCtrl()
{
	m_bDragging = false;
} 
CXTreeCtrl::~CXTreeCtrl()
{}

BEGIN_MESSAGE_MAP(CXTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CXTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CXTreeCtrl::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;

	m_hItemDragS = pNMTreeView->itemNew.hItem;
	//�õ������϶�ʱ��ʾ��ͼ���б�
	
	m_bDragging = true;

	SetCapture();
	::SetCursor((HCURSOR)::LoadCursor(NULL, IDC_HAND)); // �������Ϊ��״��ʾ�����ƶ�����
}

void CXTreeCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	//HTREEITEM hItem;
	//UINT flags;

	m_HoverPoint = point;
	if( m_bDragging )
	{
		CPoint pt = point;
		::SetCursor((HCURSOR)::LoadCursor(NULL, IDC_HAND)); // �������Ϊ��״��ʾ�����ƶ�����
	}
	CTreeCtrl::OnMouseMove(nFlags, point);
}

void CXTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CTreeCtrl::OnLButtonUp(nFlags, point);
	if( m_bDragging )
	{
		m_bDragging = FALSE;
		ReleaseCapture();
		SelectDropTarget( NULL );
		::SetCursor((HCURSOR)::LoadCursor(NULL, IDC_ARROW));

		CPoint tmp = point;
		ClientToScreen(&tmp);
		CMainFrame *pMain = (CMainFrame *)AfxGetApp()->m_pMainWnd;
		RECT lineRect;
		pMain->getOutPutWnd().GetWindowRect(&lineRect);
		if(PtInRect(&lineRect,tmp))
		{
			HTREEITEM trItem = GetSelectedItem();
			/*UINT uFlag = 0;
			trItem = HitTest( point,&uFlag );*/
			CString dataAttribute = GetItemText(trItem);
			CString dataName = GetItemText(GetParentItem(trItem));
			pMain->getOutPutWnd().updateDataName(tmp,dataName,dataAttribute);
		}
	}
}


void CXTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point) //����������ҷ
{
	m_dwDragStart = GetTickCount();

	HTREEITEM trItem = 0;
	UINT uFlag = 0;
	trItem = HitTest( point,&uFlag );
	if( trItem )
	{
		SelectItem( trItem );
		//Expand( trItem,TVE_EXPAND );
	}
	CTreeCtrl::OnLButtonDown(nFlags, point);
}
void CXTreeCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	HTREEITEM trItem = 0;
	UINT uFlag = 0;
	trItem = HitTest( point,&uFlag );
	CString dataAttribute = GetItemText(trItem);
	CString dataName = GetItemText(GetParentItem(trItem));
	CMainFrame *pMain = (CMainFrame *)AfxGetApp()->m_pMainWnd;
	pMain->getOutPutWnd().getChar1()->updateDataName(dataName,dataAttribute);
}