// XTreeCtrl.h : header file
#ifndef AFX_XTREECTRL_H
#define AFX_XTREECTRL_H

class CXTreeCtrl : public CTreeCtrl
{
	// Construction
public:
	 CXTreeCtrl();
	  // Attributes
public:
	 // Operations
public:
	 // Overrides
	 // ClassWizard generated virtual function overrides
	 //{{AFX_VIRTUAL(CXTreeCtrl)
	 //}}AFX_VIRTUAL
	 // Implementation
public:
	virtual ~CXTreeCtrl();
	// Generated message map functions
protected:
	
	CPoint m_HoverPoint; //���λ��

	DWORD m_dwDragStart; //������������һ�̵�ʱ��
	BOOL m_bDragging; //��ʶ�Ƿ������϶�������
	CImageList* m_pDragImage; //�϶�ʱ��ʾ��ͼ���б�
	HTREEITEM m_hItemDragS; //���϶��ı�ǩ
	//{{AFX_MSG(CXTreeCtrl)
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
									//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};
#endif 
