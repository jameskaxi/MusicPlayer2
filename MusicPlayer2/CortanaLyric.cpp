#include "stdafx.h"
#include "CortanaLyric.h"


CCortanaLyric::CCortanaLyric()
{
}


CCortanaLyric::~CCortanaLyric()
{
}

void CCortanaLyric::Init()
{
	if (m_enable)
	{
		HWND hTaskBar = ::FindWindow(_T("Shell_TrayWnd"), NULL);	//任务栏的句柄
		HWND hCortanaBar = ::FindWindowEx(hTaskBar, NULL, _T("TrayDummySearchControl"), NULL);	//Cortana栏的句柄（其中包含3个子窗口）
		m_cortana_hwnd = ::FindWindowEx(hCortanaBar, NULL, _T("Button"), NULL);	//Cortana搜索框中类名为“Button”的窗口的句柄
		m_hCortanaStatic = ::FindWindowEx(hCortanaBar, NULL, _T("Static"), NULL);		//Cortana搜索框中类名为“Static”的窗口的句柄
		if (m_cortana_hwnd == NULL) return;
		wchar_t buff[32];
		::GetWindowText(m_cortana_hwnd, buff, 31);		//获取Cortana搜索框中原来的字符串，用于在程序退出时恢复
		m_cortana_default_text = buff;
		m_cortana_wnd = CWnd::FromHandle(m_cortana_hwnd);		//获取Cortana搜索框的CWnd类的指针
		if (m_cortana_wnd == nullptr) return;

		//获取左上角点的坐标
		CRect rect;
		::GetWindowRect(m_cortana_hwnd, rect);
		m_lefttop_point = rect.TopLeft();
		m_lefttop_point += CPoint{ 1, 1 };

		::GetClientRect(m_cortana_hwnd, m_cortana_rect);	//获取Cortana搜索框的矩形区域
		CRect cortana_static_rect;		//Cortana搜索框中static控件的矩形区域
		::GetClientRect(m_hCortanaStatic, cortana_static_rect);	//获取Cortana搜索框中static控件的矩形区域
		if (cortana_static_rect.Width() > 0)
		{
			m_cortana_left_space = m_cortana_rect.Width() - cortana_static_rect.Width();
			m_cortana_rect.right = m_cortana_rect.left + cortana_static_rect.Width();		//调整Cortana窗口矩形的宽度
		}
		else
		{
			m_cortana_left_space = 0;
		}

		m_cortana_pDC = m_cortana_wnd->GetDC();
		m_cortana_draw.Create(m_cortana_pDC, m_cortana_wnd);

		CheckDarkMode();
		
		//设置字体
		LOGFONT lf;
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);		//获取系统默认字体
		if (m_cortana_font.m_hObject)		//如果m_font已经关联了一个字体资源对象，则释放它
			m_cortana_font.DeleteObject();
		m_cortana_font.CreatePointFont(110, lf.lfFaceName);
		m_cortana_draw.SetFont(&m_cortana_font);		//设置绘图字体
		int a = 0;
	}
}

void CCortanaLyric::SetEnable(bool enable)
{
	m_enable = enable;
}

void CCortanaLyric::SetColors(ColorTable colors)
{
	m_colors = colors;
}

void CCortanaLyric::DrawCortanaText(LPCTSTR str, bool reset, int scroll_pixel)
{
	if (m_enable && m_cortana_hwnd != NULL && m_cortana_wnd != nullptr)
	{
		//设置缓冲的DC
		CDC MemDC;
		CBitmap MemBitmap;
		MemDC.CreateCompatibleDC(NULL);
		MemBitmap.CreateCompatibleBitmap(m_cortana_pDC, m_cortana_rect.Width(), m_cortana_rect.Height());
		CBitmap *pOldBit = MemDC.SelectObject(&MemBitmap);
		//使用m_cortana_draw绘图
		m_cortana_draw.SetDC(&MemDC);
		static CDrawCommon::ScrollInfo cortana_scroll_info;
		COLORREF color;
		color = (m_dark_mode ? m_colors.light3 : m_colors.dark2);
		m_cortana_draw.DrawScrollText(m_cortana_rect, str, color, scroll_pixel, false, cortana_scroll_info, reset);
		//将缓冲区DC中的图像拷贝到屏幕中显示
		m_cortana_pDC->BitBlt(m_cortana_left_space, (m_dark_mode ? 0 : 1), m_cortana_rect.Width(), m_cortana_rect.Height(), &MemDC, 0, 0, SRCCOPY);
		MemBitmap.DeleteObject();
		MemDC.DeleteDC();
	}
}

void CCortanaLyric::DrawCortanaText(LPCTSTR str, int progress)
{
	if (m_enable && m_cortana_hwnd != NULL && m_cortana_wnd != nullptr)
	{
		//设置缓冲的DC
		CDC MemDC;
		CBitmap MemBitmap;
		MemDC.CreateCompatibleDC(NULL);
		MemBitmap.CreateCompatibleBitmap(m_cortana_pDC, m_cortana_rect.Width(), m_cortana_rect.Height());
		CBitmap *pOldBit = MemDC.SelectObject(&MemBitmap);
		//使用m_cortana_draw绘图
		m_cortana_draw.SetDC(&MemDC);
		if (m_dark_mode)
			m_cortana_draw.DrawWindowText(m_cortana_rect, str, m_colors.light3, m_colors.light1, progress, false, true);
		else
			m_cortana_draw.DrawWindowText(m_cortana_rect, str, m_colors.dark3, m_colors.dark1, progress, false, true);
		//将缓冲区DC中的图像拷贝到屏幕中显示
		m_cortana_pDC->BitBlt(m_cortana_left_space, (m_dark_mode ? 0 : 1) , m_cortana_rect.Width(), m_cortana_rect.Height(), &MemDC, 0, 0, SRCCOPY);
		MemBitmap.DeleteObject();
		MemDC.DeleteDC();
	}
}

void CCortanaLyric::ResetCortanaText()
{
	if (m_enable && m_cortana_hwnd != NULL && m_cortana_wnd != nullptr)
	{
		COLORREF color;
		color = (m_dark_mode ? GRAY(173) : GRAY(16));
		m_cortana_draw.SetDC(m_cortana_pDC);
		CRect rect{ m_cortana_rect };
		rect.MoveToXY(rect.left + m_cortana_left_space, (m_dark_mode ? 0 : 1));
		m_cortana_draw.DrawWindowText(rect, m_cortana_default_text.c_str(), color, false);
		m_cortana_wnd->Invalidate();
	}
}

void CCortanaLyric::CheckDarkMode()
{
	if (m_enable)
	{
		HDC hDC = ::GetDC(NULL);
		COLORREF color;
		//获取Cortana左上角点的颜色
		color = ::GetPixel(hDC, m_lefttop_point.x, m_lefttop_point.y);
		m_dark_mode = (color % 256 < 220);

		//根据深浅色模式设置背景颜色
		if (m_dark_mode)
		{
			DWORD dwStyle = GetWindowLong(m_hCortanaStatic, GWL_STYLE);
			if ((dwStyle & WS_VISIBLE) != 0)		//根据Cortana搜索框中static控件是否有WS_VISIBLE属性为绘图背景设置不同的背景色
				m_cortana_draw.SetBackColor(GRAY(47));	//设置绘图的背景颜色
			else
				m_cortana_draw.SetBackColor(GRAY(10));	//设置绘图的背景颜色
		}
		else
		{
			m_cortana_draw.SetBackColor(GRAY(240));
		}
	}
}
