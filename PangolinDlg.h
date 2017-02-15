
// PangolinDlg.h : ͷ�ļ�
//

#pragma once


// CPangolinDlg �Ի���
class CPangolinDlg : public CDialogEx
{
// ����
public:
	CPangolinDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PANGOLIN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

private:
    void InitControlPosition();
    void ShowVideoParamTab(int bShow);
    void ShowAudioParamTab(int bShow);
    void ShowCaptureParamTab(int bShow);
    void ShowAboutTab(int bShow);

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnTabChange(NMHDR *pNMHDR, LRESULT *pResult);
};