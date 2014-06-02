// SimDlg.h : header file
//

#pragma once

#include <ctime>
#include <iostream>
#include <vector>

#include "Context.h"
#include "WagonEnviroment.h"

// CSimDlg dialog
class CSimDlg : public CDialog
{

	DrawingContext *dContext;
	Context* context;
	WagonEnviroment* env;

	std::vector<Passenger*> passengers;

	CClientDC* devc;

	CButton* boardButton;
	CButton* stopButton;
	CButton* disButton;
	CButton* startButton;
	CButton* dbButton;

// Construction
public:
	CSimDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SIM_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBoarding();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedDis();

	CSimDlg::~CSimDlg();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedBd();
};
