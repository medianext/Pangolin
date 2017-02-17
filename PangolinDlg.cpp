
// PangolinDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Pangolin.h"
#include "PangolinDlg.h"
#include "afxdialogex.h"

#include "Capture.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPangolinDlg �Ի���



CPangolinDlg::CPangolinDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PANGOLIN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPangolinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPangolinDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_NOTIFY(TCN_SELCHANGE, IDC_SETTING, &CPangolinDlg::OnTabChange)
END_MESSAGE_MAP()


// CPangolinDlg ��Ϣ�������

BOOL CPangolinDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
    InitControlPosition();
    CTabCtrl* hTab = (CTabCtrl*)GetDlgItem(IDC_SETTING);
    hTab->InsertItem(0, TEXT("��Ƶ����"));
    hTab->InsertItem(1, TEXT("��Ƶ����"));
    hTab->InsertItem(2, TEXT("�ɼ�����"));
    hTab->InsertItem(3, TEXT("����"));

	//��ʼ���ؼ�����
	CComboBox* hComBox = NULL;
	std::vector<WCHAR *> strList;

	hComBox = (CComboBox*)this->GetDlgItem(IDC_VIDEO_CAP);
	int vCnt = Capture::EnumVideoCature(&strList);
	for (int i = 0; i < vCnt; i++)
	{
		hComBox->AddString(strList[i]);
	}
	hComBox->SetCurSel(0);
	strList.clear();

	hComBox = (CComboBox*)this->GetDlgItem(IDC_AUDIO_CAP);
	int aCnt = Capture::EnumAudioCature(&strList);
	for (int i = 0; i < aCnt; i++)
	{
		hComBox->AddString(strList[i]);
	}
	hComBox->SetCurSel(0);

    ShowVideoParamTab(SW_SHOW);
    ShowAudioParamTab(SW_HIDE);
    ShowCaptureParamTab(SW_HIDE);
    ShowAboutTab(SW_HIDE);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CPangolinDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CPangolinDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CPangolinDlg::InitControlPosition()
{
    CRect rect;
    this->GetClientRect(&rect);
    this->SetWindowPos(NULL, 0, 0, 600, 600, FALSE);

    CWnd* hChild = NULL;

    hChild = this->GetDlgItem(IDC_RENDER);
    hChild->SetWindowPos(NULL, 4, 4, 576, 324, 0);

    hChild = this->GetDlgItem(IDC_STATIC1);
    hChild->SetWindowPos(NULL, 14, 340, 70, 30, 0);

    hChild = this->GetDlgItem(IDC_RTMPURL);
    hChild->SetWindowPos(NULL, 84, 338, 400, 24, 0);

    hChild = this->GetDlgItem(IDC_PUSH);
    hChild->SetWindowPos(NULL, 490, 336, 80, 28, 0);

    CTabCtrl* hTab = (CTabCtrl*)GetDlgItem(IDC_SETTING);
    hTab->SetWindowPos(NULL, 4, 370, 300, 180, 0);

    hChild = this->GetDlgItem(IDC_LOG);
    hChild->SetWindowPos(NULL, 314, 370, 260, 180, 0);

    //��Ƶ����
    hChild = this->GetDlgItem(IDC_STATIC2);
    hChild->SetWindowPos(NULL, 30, 414, 40, 18, 0);
    hChild = this->GetDlgItem(IDC_VIDEO_RESOLUTION);
    hChild->SetWindowPos(NULL, 90, 410, 140, 24, 0);
    hChild = this->GetDlgItem(IDC_STATIC3);
    hChild->SetWindowPos(NULL, 30, 446, 40, 18, 0);
    hChild = this->GetDlgItem(IDC_VIDEO_FPS);
    hChild->SetWindowPos(NULL, 90, 442, 140, 24, 0);
    hChild = this->GetDlgItem(IDC_STATIC4);
    hChild->SetWindowPos(NULL, 30, 478, 40, 18, 0);
    hChild = this->GetDlgItem(IDC_VIDEO_CODEC);
    hChild->SetWindowPos(NULL, 90, 474, 140, 24, 0);
    hChild = this->GetDlgItem(IDC_STATIC5);
    hChild->SetWindowPos(NULL, 30, 510, 40, 18, 0);
    hChild = this->GetDlgItem(IDC_VIDEO_BITRATE);
    hChild->SetWindowPos(NULL, 90, 506, 140, 24, 0);

    //��Ƶ����
    hChild = this->GetDlgItem(IDC_STATIC10);
    hChild->SetWindowPos(NULL, 30, 414, 40, 18, 0);
    hChild = this->GetDlgItem(IDC_AUDIO_SAMPLERATE);
    hChild->SetWindowPos(NULL, 90, 410, 140, 24, 0);
    hChild = this->GetDlgItem(IDC_STATIC11);
    hChild->SetWindowPos(NULL, 30, 446, 40, 18, 0);
    hChild = this->GetDlgItem(IDC_AUDIO_CHANNEL);
    hChild->SetWindowPos(NULL, 90, 442, 140, 24, 0);
    hChild = this->GetDlgItem(IDC_STATIC12);
    hChild->SetWindowPos(NULL, 30, 478, 40, 18, 0);
    hChild = this->GetDlgItem(IDC_AUDIO_CODEC);
    hChild->SetWindowPos(NULL, 90, 474, 140, 24, 0);
    hChild = this->GetDlgItem(IDC_STATIC13);
    hChild->SetWindowPos(NULL, 30, 510, 40, 18, 0);
    hChild = this->GetDlgItem(IDC_AUDIO_BITRATE);
    hChild->SetWindowPos(NULL, 90, 506, 140, 24, 0);

    //�ɼ�Դ
    hChild = this->GetDlgItem(IDC_STATIC20);
    hChild->SetWindowPos(NULL, 30, 414, 40, 18, 0);
    hChild = this->GetDlgItem(IDC_VIDEO_CAP);
    hChild->SetWindowPos(NULL, 90, 410, 185, 24, 0);
    hChild = this->GetDlgItem(IDC_STATIC21);
    hChild->SetWindowPos(NULL, 30, 446, 40, 18, 0);
    hChild = this->GetDlgItem(IDC_AUDIO_CAP);
    hChild->SetWindowPos(NULL, 90, 442, 185, 24, 0);

    //����
    hChild = this->GetDlgItem(IDC_STATIC30);
    hChild->SetWindowPos(NULL, 18, 414, 50, 18, 0);
    hChild = this->GetDlgItem(IDC_LINK_PROJECT);
    hChild->SetWindowPos(NULL, 80, 414, 210, 18, 0);
    hChild = this->GetDlgItem(IDC_STATIC31);
    hChild->SetWindowPos(NULL, 10, 446, 60, 18, 0);
    hChild = this->GetDlgItem(IDC_LINK_QQ);
    hChild->SetWindowPos(NULL, 80, 446, 60, 18, 0);
}


void CPangolinDlg::ShowVideoParamTab(int bShow)
{
    CWnd* hChild = NULL;
    hChild = this->GetDlgItem(IDC_STATIC2);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_VIDEO_RESOLUTION);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_STATIC3);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_VIDEO_FPS);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_STATIC4);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_VIDEO_CODEC);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_STATIC5);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_VIDEO_BITRATE);
    hChild->ShowWindow(bShow);
}


void CPangolinDlg::ShowAudioParamTab(int bShow)
{
    CWnd* hChild = NULL;
    hChild = this->GetDlgItem(IDC_STATIC10);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_AUDIO_SAMPLERATE);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_STATIC11);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_AUDIO_CHANNEL);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_STATIC12);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_AUDIO_CODEC);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_STATIC13);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_AUDIO_BITRATE);
    hChild->ShowWindow(bShow);
}


void CPangolinDlg::ShowCaptureParamTab(int bShow)
{
    CWnd* hChild = NULL;
    hChild = this->GetDlgItem(IDC_STATIC20);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_VIDEO_CAP);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_STATIC21);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_AUDIO_CAP);
    hChild->ShowWindow(bShow);
}


void CPangolinDlg::ShowAboutTab(int bShow)
{
    CWnd* hChild = NULL;
    hChild = this->GetDlgItem(IDC_STATIC30);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_LINK_PROJECT);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_STATIC31);
    hChild->ShowWindow(bShow);
    hChild = this->GetDlgItem(IDC_LINK_QQ);
    hChild->ShowWindow(bShow);
}


void CPangolinDlg::OnTabChange(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    CTabCtrl* hTab = (CTabCtrl*)GetDlgItem(IDC_SETTING);
    int sel = hTab->GetCurSel();
    switch (sel)
    {
    case 0:
    {
        ShowVideoParamTab(SW_SHOW);
        ShowAudioParamTab(SW_HIDE);
        ShowCaptureParamTab(SW_HIDE);
        ShowAboutTab(SW_HIDE);
    }
        break;
    case 1:
    {
        ShowVideoParamTab(SW_HIDE);
        ShowAudioParamTab(SW_SHOW);
        ShowCaptureParamTab(SW_HIDE);
        ShowAboutTab(SW_HIDE);
    }
    break;
    case 2:
    {
        ShowVideoParamTab(SW_HIDE);
        ShowAudioParamTab(SW_HIDE);
        ShowCaptureParamTab(SW_SHOW);
        ShowAboutTab(SW_HIDE);
    }
    break;
    case 3:
    {
        ShowVideoParamTab(SW_HIDE);
        ShowAudioParamTab(SW_HIDE);
        ShowCaptureParamTab(SW_HIDE);
        ShowAboutTab(SW_SHOW);
    }
        break;
    }

    *pResult = 0;
}
