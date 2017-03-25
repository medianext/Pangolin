
// PangolinDlg.h : 头文件
//

#pragma once


// CPangolinDlg 对话框
class CPangolinDlg : public CDialogEx
{
// 构造
public:
	CPangolinDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PANGOLIN_DIALOG };
#endif
private:
    Capture *videoCapture = NULL;
    Capture *audioCapture = NULL;
    Render  *render = NULL;
    Codec   *codec = NULL;
    Rtmpc   *rtmpc = NULL;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
    void InitControlPosition();
    void ShowCodecParamTab(int bShow);
    void ShowCaptureParamTab(int bShow);
    void ShowAboutTab(int bShow);
    void EnableAllControl(int bEnable);

    void GetVideoAttribute(VideoCodecAttribute *pattr);
    void GetAudioAttribute(AudioCodecAttribute *pattr);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnTabChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedPush();
	afx_msg void OnVideoChange();
	afx_msg void OnAudioChange();
	afx_msg void OnVideoCaptureChange();
	afx_msg void OnAudioCaptureChange();
};
