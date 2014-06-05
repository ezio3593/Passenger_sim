// SimDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Sim.h"
#include "SimDlg.h"
#include <string.h>

#include "Objects.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSimDlg dialog

CSimDlg::CSimDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSimDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	context = new Context();
	dContext = new DrawingContext(context->getObjCriticalSection());
	env = new WagonEnviroment();
	
	std::vector<Vector2D> vect;

	vect.push_back(Vector2D(50,250));
	vect.push_back(Vector2D(50,230));
	vect.push_back(Vector2D(150,230));
	vect.push_back(Vector2D(150,250));

	env->addSeats(vect, 4, 170, 70, 7);
	env->buildKdObstTree();
	dContext->addWagonEnv(env);
	env->buildKdObstTree();
}

void CSimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSimDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BOARDING, &CSimDlg::OnBnClickedBoarding)
	ON_BN_CLICKED(IDC_STOP, &CSimDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_DIS, &CSimDlg::OnBnClickedDis)
	ON_BN_CLICKED(IDC_START, &CSimDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_BD, &CSimDlg::OnBnClickedBd)
END_MESSAGE_MAP()


// CSimDlg message handlers

BOOL CSimDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CStatic *pc = (CStatic*)GetDlgItem(IDC_OPENGL);
	
	boardButton = (CButton*)GetDlgItem(IDC_BOARDING);
	stopButton  = (CButton*)GetDlgItem(IDC_STOP);
	disButton  = (CButton*)GetDlgItem(IDC_DIS);
	startButton = (CButton*)GetDlgItem(IDC_START);
	dbButton = (CButton*)GetDlgItem(IDC_BD);

	int res = dContext->init(pc);
	if (res) 
	{
		MessageBox(_T("Cannot init OpenGL context"), _T("Error"), MB_ICONERROR | MB_OK);
		EndDialog(res);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSimDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}

	dContext->redrawScene();
	stopButton->EnableWindow(FALSE);
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSimDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSimDlg::OnBnClickedBoarding()
{
	dContext->makeCurrent();

	stopButton->EnableWindow(FALSE);
	startButton->EnableWindow(TRUE);

	disButton->EnableWindow(TRUE);
	dbButton->EnableWindow(TRUE);
	boardButton->EnableWindow(FALSE);
	
	dContext->clearObjPool();
	dContext->redrawScene();
	
	context->clearObjPool();
	env->clearAgentPool();
	
	for (size_t i =  0; i < passengers.size(); ++i) delete passengers[i];

	passengers.erase(passengers.begin(), passengers.end());

	for (int i = 0; i < 50; ++i)
	{
		Passenger* agent = new Passenger(env);
		
		context->addObject(agent);
		dContext->addObject(agent);
		env->addAgent(agent);

		agent->setPosition(Vector2D(150 + 20 * i ,300));
		agent->setContext(context);
		
		agent->setIsBoarding(true);

		passengers.push_back(agent);
	}	

	dContext->redrawScene();
}


void CSimDlg::OnBnClickedDis()
{
	dContext->makeCurrent();

	OnBnClickedStop();

	disButton->EnableWindow(FALSE);
	boardButton->EnableWindow(TRUE);
	startButton->EnableWindow(TRUE);
	stopButton->EnableWindow(FALSE);
	dbButton->EnableWindow(TRUE);

	dContext->clearObjPool();
	dContext->redrawScene();
	
	context->clearObjPool();
	env->clearAgentPool();
	
	for (size_t i =  0; i < passengers.size(); ++i) delete passengers[i];

	passengers.erase(passengers.begin(), passengers.end());

	std::vector<Goal*> seats = env->getSeatGoals();

	for (size_t i = 0; i < seats.size(); ++i)
	{
		if (std::rand() % 2 == 1)
		{
			Passenger* agent = new Passenger(env);
			
			context->addObject(agent);
			dContext->addObject(agent);

			env->addAgent(agent);

			agent->setContext(context);

			agent->setPosition(seats[i]->getNextLevelGoal()->getSeatPoint());
			agent->setGoal(seats[i]->getNextLevelGoal(), true);
			
			agent->setIsBoarding(false);
			passengers.push_back(agent);
		}
	}

	dContext->redrawScene();
}

void CSimDlg::OnBnClickedBd()
{
	dContext->makeCurrent();

	dbButton->EnableWindow(FALSE);

	disButton->EnableWindow(TRUE);
	boardButton->EnableWindow(TRUE);
	startButton->EnableWindow(TRUE);
	stopButton->EnableWindow(FALSE);

	dContext->clearObjPool();
	dContext->redrawScene();
	
	context->clearObjPool();
	env->clearAgentPool();
	
	for (size_t i =  0; i < passengers.size(); ++i) delete passengers[i];
	passengers.erase(passengers.begin(), passengers.end());

	std::vector<Goal*> seats = env->getSeatGoals();

	for (size_t i = 0; i < seats.size(); ++i)
	{
		if (std::rand() % 3 == 1)
		{
			Passenger* agent = new Passenger(env);
			
			context->addObject(agent);
			dContext->addObject(agent);

			env->addAgent(agent);

			agent->setContext(context);

			agent->setPosition(seats[i]->getNextLevelGoal()->getSeatPoint());
			agent->setGoal(seats[i]->getNextLevelGoal(), true);
			
			agent->setIsBoarding(false);
			passengers.push_back(agent);
		}
	}
	
	for (int i = 0; i < 10; ++i)
	{
		Passenger* agent = new Passenger(env);
		
		context->addObject(agent);
		dContext->addObject(agent);
		env->addAgent(agent);

		agent->setPosition(Vector2D(150 + 20 * i ,300));
		agent->setContext(context);
		
		agent->setIsBoarding(true);

		passengers.push_back(agent);
	}

	dContext->redrawScene();
	
}

CSimDlg::~CSimDlg()
{
	delete context;
	delete dContext;
	delete env;

	for (size_t i = 0; i < passengers.size(); ++i)
	{
		delete passengers[i];
	}
}

void CSimDlg::OnBnClickedStart()
{
	stopButton->EnableWindow(TRUE);
	boardButton->EnableWindow(FALSE);
	disButton->EnableWindow(FALSE);
	startButton->EnableWindow(FALSE);
	dbButton->EnableWindow(FALSE);

	try
	{
		context->start();
		dContext->unmakeCurrent();
		dContext->startRedrawingThread();
	} catch (const std::exception& exc)
	{
		MessageBoxA(::GetActiveWindow(), exc.what(), "Error", MB_ICONERROR | MB_OK);
		EndDialog(-1);
	}

	for (int i = 0; i < passengers.size(); ++i)
	{
		passengers[i]->start();
	}
}

void CSimDlg::OnBnClickedStop()
{
	context->stop();
	dContext->stopRedrawingThread();

	stopButton->EnableWindow(FALSE);
	startButton->EnableWindow(TRUE);

	disButton->EnableWindow(TRUE);
	boardButton->EnableWindow(TRUE);
	dbButton->EnableWindow(TRUE);
}