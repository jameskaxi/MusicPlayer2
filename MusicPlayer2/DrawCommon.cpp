#include "stdafx.h"
#include "DrawCommon.h"


CDrawCommon::CDrawCommon()
{
}

CDrawCommon::~CDrawCommon()
{
}

void CDrawCommon::Create(CDC * pDC, CWnd * pMainWnd)
{
	m_pDC = pDC;
	m_pMainWnd = pMainWnd;
	m_pfont = m_pMainWnd->GetFont();
}

void CDrawCommon::SetBackColor(COLORREF back_color)
{
	m_backColor = back_color;
}

void CDrawCommon::SetFont(CFont * pfont)
{
	m_pfont = pfont;
}

void CDrawCommon::SetDC(CDC * pDC)
{
	m_pDC = pDC;
}

void CDrawCommon::DrawWindowText(CRect rect, LPCTSTR lpszString, COLORREF color, bool center, bool no_clip_area)
{
	m_pDC->SetTextColor(color);
	//m_pDC->SetBkMode(TRANSPARENT);
	m_pDC->SelectObject(m_pfont);
	//设置绘图的剪辑区域
	if (!no_clip_area)
	{
		SetDrawArea(m_pDC, rect);
	}
	CSize text_size = m_pDC->GetTextExtent(lpszString);
	//用背景色填充矩形区域
	m_pDC->FillSolidRect(rect, m_backColor);
	if (text_size.cx > rect.Width())		//如果文本宽度超过了矩形区域的宽度，则总是左对齐
		m_pDC->DrawText(lpszString, rect, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	else
		m_pDC->DrawText(lpszString, rect, (center ? DT_CENTER : 0) | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

void CDrawCommon::DrawWindowText(CRect rect, LPCTSTR lpszString, COLORREF color1, COLORREF color2, int split, bool center, bool no_clip_area)
{
	if (split < 0) split = 0;
	if (split > 1000) split = 1000;
	m_pDC->SetBkMode(TRANSPARENT);
	m_pDC->SelectObject(m_pfont);
	CSize text_size;	//文本的大小
	int text_top, text_left;		//输出文本的top和left位置
	//设置绘图的剪辑区域，防止文字输出超出控件区域
	if (!no_clip_area)
	{
		SetDrawArea(m_pDC, rect);
	}
	//获取文字的宽度和高度
	text_size = m_pDC->GetTextExtent(lpszString);
	//计算文字的起始坐标
	text_top = rect.top + (rect.Height() - text_size.cy) / 2;
	if (center)
		text_left = rect.left + (rect.Width() - text_size.cx) / 2;
	else
		text_left = rect.left;
	//计算背景文字和覆盖文字的矩形区域
	CRect text_rect{ CPoint{ text_left, text_top }, text_size };		//背景文字的区域
	CRect text_f_rect{ CPoint{ text_left, text_top }, CSize{ text_size.cx * split / 1000, text_size.cy } };		//覆盖文字的区域
																												//如果文本宽度大于控件宽度，就要根据分割的位置滚动文本
	if (text_size.cx > rect.Width())
	{
		//如果分割的位置（歌词进度）剩下的宽度已经小于控件宽度的一半，此时使文本右侧和控件右侧对齐
		if (text_rect.Width() - text_f_rect.Width() < rect.Width() / 2)
		{
			text_rect.MoveToX(rect.left - (text_rect.Width() - rect.Width()));
			text_f_rect.MoveToX(text_rect.left);
		}
		//分割位置剩下的宽度还没有到小于控件宽度的一半，但是分割位置的宽度已经大于控件宽度的一半时，需要移动文本使分割位置正好在控件的中间
		else if (text_f_rect.Width() > rect.Width() / 2)
		{
			text_rect.MoveToX(rect.left - (text_f_rect.Width() - rect.Width() / 2));
			text_f_rect.MoveToX(text_rect.left);
		}
		//分割位置还不到控件宽度的一半时，使文本左侧和控件左侧对齐
		else
		{
			text_rect.MoveToX(rect.left);
			text_f_rect.MoveToX(rect.left);
		}
	}

	//用背景色填充矩形区域
	m_pDC->FillSolidRect(rect, m_backColor);
	//输出文本
	m_pDC->SetTextColor(color2);
	m_pDC->DrawText(lpszString, text_rect, DT_SINGLELINE | DT_NOPREFIX);		//绘制背景文字
	m_pDC->SetTextColor(color1);
	m_pDC->DrawText(lpszString, text_f_rect, DT_SINGLELINE | DT_NOPREFIX);		//绘制覆盖文字
}

void CDrawCommon::DrawScrollText(CRect rect, LPCTSTR lpszString, COLORREF color, int pixel, bool center, ScrollInfo& scroll_info, bool reset)
{
	//static int shift_cnt;		//移动的次数
	//static bool shift_dir;		//移动的方向，右移为false，左移为true
	//static int freez;			//当该变量大于0时，文本不滚动，直到小于等于0为止
	//static bool dir_changed{ false };	//如果方向发生了变化，则为true
	if (reset)
	{
		scroll_info.shift_cnt = 0;
		scroll_info.shift_dir = false;
		scroll_info.freez = 20;
		scroll_info.dir_changed = false;
	}
	m_pDC->SetTextColor(color);
	m_pDC->SetBkMode(TRANSPARENT);
	m_pDC->SelectObject(m_pfont);
	CSize text_size;	//文本的大小
	int text_top, text_left;		//输出文本的top和left位置
	//设置绘图的剪辑区域，防止文字输出超出控件区域
	SetDrawArea(m_pDC, rect);
	//获取文字的宽度和高度
	text_size = m_pDC->GetTextExtent(lpszString);
	//计算文字的起始坐标
	text_top = rect.top + (rect.Height() - text_size.cy) / 2;
	if (center)
		text_left = rect.left + (rect.Width() - text_size.cx) / 2;
	else
		text_left = rect.left;
	//计算文字的矩形区域
	CRect text_rect{ CPoint{ text_left, text_top }, text_size };
	//如果文本宽度大于控件宽度，就滚动文本
	if (text_size.cx > rect.Width())
	{
		text_rect.MoveToX(rect.left - scroll_info.shift_cnt*pixel);
		if ((text_rect.right < rect.right || text_rect.left > rect.left))		//移动到边界时换方向
		{
			if (!scroll_info.dir_changed)
			{
				scroll_info.shift_dir = !scroll_info.shift_dir;
				scroll_info.freez = 20;		//变换方向时稍微暂停滚动一段时间
			}
			scroll_info.dir_changed = true;
		}
		else
		{
			scroll_info.dir_changed = false;
		}
	}
	//用背景色填充矩形区域
	m_pDC->FillSolidRect(rect, m_backColor);
	//输出文本
	m_pDC->DrawText(lpszString, text_rect, DT_SINGLELINE | DT_NOPREFIX);
	if (scroll_info.freez <= 0)		//当freez为0的时候才滚动
	{
		if (scroll_info.shift_dir)
			scroll_info.shift_cnt--;
		else
			scroll_info.shift_cnt++;
	}
	else
	{
		scroll_info.freez--;
	}
}

void CDrawCommon::DrawScrollText2(CRect rect, LPCTSTR lpszString, COLORREF color, int pixel, bool center, ScrollInfo & scroll_info, bool reset)
{
	if (reset)
	{
		scroll_info.shift_cnt = 0;
		scroll_info.freez = 20;
	}
	m_pDC->SetTextColor(color);
	m_pDC->SetBkMode(TRANSPARENT);
	m_pDC->SelectObject(m_pfont);
	CSize text_size;	//文本的大小
	int text_top, text_left;		//输出文本的top和left位置
	//设置绘图的剪辑区域，防止文字输出超出控件区域
	SetDrawArea(m_pDC, rect);
	//获取文字的宽度和高度
	text_size = m_pDC->GetTextExtent(lpszString);
	//计算文字的起始坐标
	text_top = rect.top + (rect.Height() - text_size.cy) / 2;
	if (center)
		text_left = rect.left + (rect.Width() - text_size.cx) / 2;
	else
		text_left = rect.left;
	//计算文字的矩形区域
	CRect text_rect{ CPoint{ text_left, text_top }, text_size };
	//如果文本宽度大于控件宽度，就滚动文本
	if (text_size.cx > rect.Width())
	{
		text_rect.MoveToX(rect.left - scroll_info.shift_cnt*pixel);
		if ((text_rect.right < rect.right || text_rect.left > rect.left))		//移动超出边界时暂停滚动，freez从20开始递减
		{
			scroll_info.shift_cnt--;	//让文本往回移动一次，防止反复判断为超出边界
			text_rect.MoveToX(rect.left - scroll_info.shift_cnt*pixel);
			scroll_info.freez = 20;		//变换方向时稍微暂停滚动一段时间
		}
	}
	//用背景色填充矩形区域
	m_pDC->FillSolidRect(rect, m_backColor);
	//输出文本
	m_pDC->DrawText(lpszString, text_rect, DT_SINGLELINE | DT_NOPREFIX);
	if (scroll_info.freez <= 0)		//当freez为0的时候才滚动
	{
		scroll_info.shift_cnt++;
	}
	else
	{
		scroll_info.freez--;
		if (scroll_info.freez == 10)		//当freez递减到一半时将文本复位
			scroll_info.shift_cnt = 0;
	}
}

//void CDrawCommon::FillRect(CDC * pDC, CRect rect, COLORREF color)
//{
//	CBrush BGBrush, *pOldBrush;
//	BGBrush.CreateSolidBrush(color);
//	pOldBrush = pDC->SelectObject(&BGBrush);
//	pDC->FillRect(&rect, &BGBrush);
//	pDC->SelectObject(pOldBrush);
//	BGBrush.DeleteObject();
//}

void CDrawCommon::SetDrawArea(CDC * pDC, CRect rect)
{
	CRgn rgn;
	rgn.CreateRectRgnIndirect(rect);
	pDC->SelectClipRgn(&rgn);
}
