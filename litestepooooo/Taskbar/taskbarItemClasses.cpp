#include "taskbarItemClasses.h"
#include "CTaskbar.h"

#include "../Utils.h"
#include "drawingApi.h"
#include "../Services/TrayService.h"
#pragma warning(disable : 4996)

#include <algorithm>
#include "../Logging/FLogger.h"

//void drawwww(barItem* r, HDC hDC, COLORREF color)
//{
//	HGDIOBJ hOldBrush, hOldPen = 0;
//
//	HPEN m_hBorderPen = CreatePen(PS_SOLID, 1, color);
//	hOldBrush = SelectObject(hDC, r->mainbar->m_hBackBrush);
//	hOldPen = SelectObject(hDC, m_hBorderPen);
//
//	// draws border and the back of it
//	Rectangle(hDC, r->itemRect.left, r->itemRect.top, r->itemRect.right, r->itemRect.bottom);
//
//	SelectObject(hDC, hOldBrush);
//	SelectObject(hDC, hOldPen);
//
//
//	DeleteObject(m_hBorderPen);
//}

//HBRUSH taskEntryBtn::Selected_taskBrush = CreateSolidBrush(m_Style.task_FocusColor);
//HBRUSH taskEntryBtn::normal_taskBrush = CreateSolidBrush(m_Style.windowBackgroundColor);

taskEntryBtn::taskEntryBtn() : barItem(M_TASK)
{
}

taskEntryBtn::~taskEntryBtn()
{
}



void taskEntryBtn::draw(HWND hWnd, HDC hDC)
{
    bool    bSelected = (m_dwFlags == taskbarItemFlags::M_TASKBUTTONACTIVE_FLAG);
    HGDIOBJ hOldPen, hOldBrush, hOldFont;
    COLORREF color;

    RECT drawingRect;

    drawingRect.left = itemRect.left;
    drawingRect.top = itemRect.top + m_Style.task_buttonTopSpacing;
    drawingRect.right = itemRect.right;
    drawingRect.bottom = itemRect.bottom - (itemRect.top + m_Style.task_buttonTopSpacing);



    HBRUSH taskBrush = CreateSolidBrush(bSelected ? m_Style.task_FocusColor : m_Style.windowBackgroundColor);
    HPEN taskPen = CreatePen(PS_SOLID, 1, m_Style.task_BevelStyle == 1 ? m_Style.borderColor : m_Style.windowBackgroundColor);

    // draw/clear the selection rectangle
    hOldBrush = SelectObject(hDC, taskBrush);
    hOldPen = SelectObject(hDC, taskPen); // allows the pen to make a bevel for us so its easier when its rounded for example.

    if (m_Style.rectRoundedEdge_TaskButtons != true)

        Rectangle(hDC, drawingRect.left, drawingRect.top, drawingRect.right, drawingRect.bottom);
    else
        RoundRect(hDC, drawingRect.left, drawingRect.top, drawingRect.right, drawingRect.bottom, m_Style.rectRoundedEdge_TaskButtons_Width, m_Style.rectRoundedEdge_TaskButtons_Height);

    //m_Style.taskBevelStyle != 1 CUS it creates a flat looking bevel using the pen.
    if (m_Style.task_BevelStyle != 0 && m_Style.task_BevelStyle != 1)
        DrawingApi::drawBevel(hDC, m_Style.borderColor, &drawingRect, m_Style.task_BevelStyle, BF_RECT);

    // is there an icon?
    if (m_icon != NULL) {

        int iconSize = m_Style.task_iconSize;;

        if (iconSize > (drawingRect.right - drawingRect.left)) {
            iconSize = drawingRect.bottom - drawingRect.top; //make sure the icon is not bigger than the box. the icon size does exceed the top tho which i like tbf
        }

        DrawIconEx(
            hDC,
            drawingRect.left,
            ((drawingRect.bottom + drawingRect.top) / 2) - (iconSize / 2),
            m_icon,
            iconSize,
            iconSize,
            0, NULL, DI_NORMAL);

    }




    if (!m_Style.tasks_IconOnly) {
        // add spacing for text beside the icon
        if (m_icon != NULL)
            drawingRect.left += (drawingRect.bottom - drawingRect.top); // add the width of the icon for the basically the x cord for the text.

        hOldFont = SelectObject(hDC, mainbar->m_hFont);

        SetBkMode(hDC, TRANSPARENT);

        if (bSelected)
            color = m_Style.focusedTextColor;
        else
            color = m_Style.unfocusedTextColor;

        SetTextColor(hDC, color);
        DrawText(hDC, m_strName.c_str(), -1, &drawingRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX);
        SelectObject(hDC, hOldFont);
    }
    SelectObject(hDC, hOldPen);
    SelectObject(hDC, hOldBrush);
    SelectObject(hDC, GetStockObject(NULL_BRUSH));

    DeleteObject(taskPen);
    DeleteObject(taskBrush);
}



void taskEntryBtn::mouse_event(int mx, int my, int message, unsigned flags)
{
    HWND taskAppHwnd = (HWND)m_data;


    if (message == WM_LBUTTONDOWN) {
        SetForegroundWindow(taskAppHwnd);
        // sends WM_SYSCOMMAND the parm SC_RESTORE which is kinda like right clicking the taskbutton on the normal windows shell and clicking open or something
        SendMessageTimeout(taskAppHwnd, WM_SYSCOMMAND, SC_RESTORE, 0, 2, 500, NULL);
        mainbar->taskService.updateActiveTask(taskAppHwnd);
    }
    else if (message == WM_RBUTTONDOWN) {
        SendMessageTimeout(taskAppHwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0, 2, 500, NULL);
        mainbar->taskService.updateActiveTask(NULL, true);
    }
    else if (message == WM_MBUTTONDOWN) {
        //graceful close of a window
        SendMessageTimeout(taskAppHwnd, WM_SYSCOMMAND, SC_CLOSE, 0, 2, 500, NULL);
    }
}

/// <summary>
/// TRAY ENTRY BTN
/// </summary>
/// <param name="bar"></param>

void TrayEntryBtn::trayMouseDown(int message)
{

    TrayItem* ni = (TrayItem*)m_data;
    DWORD procId = 0;
    GetWindowThreadProcessId(ni->hWnd, &procId);
    AllowSetForegroundWindow(procId);
    if (message == WM_RBUTTONDOWN)
        SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, 0, WM_RBUTTONDOWN | (ni->uID << 16));
    else if (message == WM_LBUTTONDOWN)
        SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, 0, WM_LBUTTONDOWN | (ni->uID << 16));
}

// only when the mouse is up is when the menu is actually opened. 
void TrayEntryBtn::trayMouseUp(int message)
{
    TrayItem* ni = (TrayItem*)m_data;

    POINT ps;
    GetCursorPos(&ps);
    LPARAM lparam = MAKELPARAM(ps.x, ps.y);

    DWORD procId = 0;
    GetWindowThreadProcessId(ni->hWnd, &procId);
    AllowSetForegroundWindow(procId);
    if (message == WM_LBUTTONUP)
        //SendNotifyMessage(ni->hWnd , ni->uCallbackMessage , 0 , NIN_SELECT | (ni->uID << 16));
        SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, 0, NIN_SELECT | (ni->uID << 16));
    else if (message == WM_RBUTTONUP) {
        SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, 0, WM_RBUTTONUP);
        SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, lparam, WM_CONTEXTMENU | (ni->uID << 16));
    }
}

void TrayEntryBtn::trayMouseDOUBLECLICKDown(int message)
{
    TrayItem* ni = (TrayItem*)m_data;

    DWORD procId = 0;
    GetWindowThreadProcessId(ni->hWnd, &procId);
    if (message == WM_LBUTTONDBLCLK)
        SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, 0, WM_LBUTTONDBLCLK | (ni->uID << 16));
    else if (message == WM_RBUTTONDBLCLK)
        SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, 0, WM_RBUTTONDBLCLK | (ni->uID << 16));
}

TrayEntryBtn::TrayEntryBtn() : barItem(M_TRAY)
{
}

TrayEntryBtn::~TrayEntryBtn()
{
    

    delete (TrayItem*)m_data; // not the best way to handle this TODO
    DestroyIcon(m_icon);


}

void TrayEntryBtn::draw(HWND hWnd, HDC hDC)
{
    int iconSize = m_Style.tray_iconSize;

    int      h{ 0 };
    RECT drawingRect;
    int y = m_Style.border_Width;
    drawingRect.left = itemRect.left;
    drawingRect.top = y;
    drawingRect.right = itemRect.right;
    drawingRect.bottom = itemRect.bottom - (y);


    if (LB_Api::isIconValid(m_icon) == false)
        FLogger::error("ICON NOT VALID? TRAY STR : %s", m_strName);

    if (iconSize > (drawingRect.bottom - drawingRect.top)){ // make sure the icon is not bigger than the box.
        iconSize = (drawingRect.bottom - drawingRect.top);
    }

    DrawIconEx(
        hDC,
        drawingRect.left,
        ((drawingRect.bottom + drawingRect.top) / 2) - (iconSize / 2), // centers the icon
        m_icon,
        iconSize,
        iconSize,
        0, NULL, DI_NORMAL);

}

void TrayEntryBtn::mouse_event(int mx, int my, int message, unsigned flags)
{
    switch (message) {
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        trayMouseUp(message);
        break;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        trayMouseDown(message);
    }
    break;
    //case WM_RBUTTONDBLCLK:
    //case WM_LBUTTONDBLCLK:
    //    trayMouseDOUBLECLICKDown(message);
    //    break;
    }
}

// BARITEMLIST
int baritemlist::getItem(short x, short y, const RECT& rect)
{

    int newitem;
    std::vector<barItem*>::size_type i;

    if (x <= 2 || y <= 2 || x > rect.right || y > rect.bottom)
        newitem = -1;
    else {
        for (i = 0; i < m_Items.size(); i++) {
            if (x > 2 && x < m_Items.at(i)->itemRect.right)
                break;
        }

        newitem = i == m_Items.size() ? -1 : i;
        if (newitem != -1 && (m_Items.at(newitem)->m_dwFlags & taskbarItemFlags::itemNotSelectable) != 0)
            newitem = -1;
    }

    return newitem;

}
bool baritemlist::calc_itemsSizes()
{
    return FALSE;
}

baritemlist::baritemlist(int type) : barItem(type)
{

}

baritemlist::~baritemlist()
{
    for (int i = 0; i < m_Items.size(); i++) {
        delete m_Items.at(i);
    }
    m_Items.clear();
}

void baritemlist::add(barItem* entry)
{
    m_Items.push_back(entry);
}

void baritemlist::remove(barItem* entry)
{

}

void baritemlist::draw(HWND hWnd, HDC hDC)
{
    for (int i = 0; i < m_Items.size(); i++) {
        m_Items.at(i)->draw(hWnd, hDC);
    }
}

void baritemlist::mouse_event(int mx, int my, int message, unsigned flags)
{
    RECT            rect;
    POINT           p;

    // get a DC and the window rectangle
    GetClientRect(mainbar->m_hWnd, &rect);

    // shift tracking if we're over another menu

    p.x = mx;
    p.y = my;
    ClientToScreen(mainbar->m_hWnd, &p);


    // get new selected item, if any
    int newitem = getItem(mx, my, rect);

    if (newitem != -1)
        m_Items.at(newitem)->mouse_event(mx, my, message, 0);
}

bool baritemlist::calc_size(int* px, int y, int w, int h)
{
    int x = *px;
    bool f = false;

    if (itemRect.left != x)
        itemRect.left = x, f = true;
    x += w;
    if (itemRect.right != x)
        itemRect.right = x, f = true;
    *px = x;

    itemRect.top = y;
    itemRect.bottom = h;

    calc_itemsSizes();

    return true;
}

// this function gives me a headache.
bool taskItemList::calc_itemsSizes()
{
    int amountOfTasks = m_Items.size();

    if (0 == amountOfTasks)
        return false;

    int b = 1 + m_Style.task_buttonSpacing;
    int max_width = itemRect.right - itemRect.left; // the max width of the task item list.
    int h = itemRect.bottom - itemRect.top;
    int xpos = itemRect.left + b;
    int y = itemRect.top + m_Style.border_Width;

    int is = h + b;
    int min_width = is / 2;

    int r = max(max_width / 100, is);
    if (r / amountOfTasks >= max_width || m_Style.tasks_IconOnly)
        max_width = amountOfTasks * r;


    int n = 0;
    for (int i = 0; i < m_Items.size(); i++) {
        int left, right;
        {
            left = xpos + max_width * n / amountOfTasks;
            right = xpos + max_width * (n + 1) / amountOfTasks - b;
            if (right - left < min_width)
                right = left + min_width;
        }

        if (right > itemRect.right - b)
            right -= b;


        m_Items.at(i)->calc_size(&left, y, right - left, h);
        ++n;
    }
    return TRUE;
}



taskItemList::taskItemList() : baritemlist(M_TASKLIST)
{
}


bool compareString(barItem* a, barItem* b)
{
    TCHAR           abuf[100];
    GetClassName((HWND)a->m_data, abuf, 100);
    TCHAR           bbuf[100];
    GetClassName((HWND)b->m_data, bbuf, 100);

    return std::wcscmp(abuf, bbuf) < 0;
}
 
void taskItemList::add(barItem* entry)
{

    // i believe worse case its o(n ^ 2) when factoring in the insert vector function which is better than before where it was o(n log n)
    for(int i =0 ; i < m_Items.size(); i++){
        
       if(compareString(m_Items.at(i), entry)){ // if new entry has same icon then normally its the same app as unlike trayicons they dont often change when same applicaiton
            m_Items.insert(m_Items.begin() +i, entry);
            return ;
        }

    }
    m_Items.push_back(entry);

}

void taskItemList::remove(barItem* entry)
{

    for (int i = 0; i < m_Items.size(); i++) {
        barItem* item = m_Items.at(i);

        if ((HWND)item->m_data == (HWND)entry->m_data) {

            m_Items.erase(m_Items.begin() + i); // o(n)
            invalidate(true);

        }
    }
}



bool trayItemList::calc_itemsSizes()
{
    int amountOfTasks = m_Items.size();

    if (0 == amountOfTasks)
        return false;

    int max_width = itemRect.right - itemRect.left; // the max width of the task item list.
    int h = itemRect.bottom - itemRect.top;
    int xpos = itemRect.left;
    int y = itemRect.top;


    int n = 0;
    for (int i = 0; i < m_Items.size(); i++) {
        int left, right;
        {
            left = xpos + max_width * n / amountOfTasks;
            right = xpos + max_width * (n + 1) / amountOfTasks - 5;

        }




        m_Items.at(i)->calc_size(&left, y, right - left, h);
        ++n;
    }
    return TRUE;
}

void trayItemList::invalidate(int flag)
{
    InvalidateRect(mainbar->m_hWnd, NULL, flag); // tray item list may need to readjust the tasklist and the traylist. so might as welll refresh the whole bar.
}

trayItemList::trayItemList() : baritemlist(M_TRAYLIST)
{
}

std::wstring clockBtn::convert24hourTo12HourTime(int time24Hour, int time24minute)
{
    wchar_t  buff[20];
    switch (time24Hour) {
    case 12:
        swprintf(buff, 20, L"%02d:%02d PM", time24Hour, time24minute);
        break;
    case 00:
        swprintf(buff, 20, L"%02d:%02d AM", time24Hour + 12, time24minute);
        break;
    default:
    {
        if (time24Hour > 12) {
            swprintf(buff, 20, L"%02d:%02d PM", time24Hour - 12, time24minute);

        }
        else {
            swprintf(buff, 20, L"%02d:%02d AM", time24Hour, time24minute);

        }
        break;
    }
    break;
    }
    std::wstring ret = buff;
    return ret;
}

void clockBtn::updateClock(clockFormat format)
{
    std::wstring formatTime;

    // gets the date and time 
    SYSTEMTIME dateAndTime;
    GetLocalTime(&dateAndTime);

    if (format == clockFormat::format_12hour) {
        formatTime = convert24hourTo12HourTime(dateAndTime.wHour, dateAndTime.wMinute);
        if (formatTime != m_strName) {// so it doesnt set time when its not necessary
            m_strName = formatTime;
            invalidate(1);
        }
    }
    else {
        wchar_t buff[20];
        swprintf(buff, 20, L"%02d:%02d", dateAndTime.wHour, dateAndTime.wMinute);

        formatTime = buff;
        if (formatTime != m_strName) {// so it doesnt set time when its not necessary
            m_strName = formatTime;
            invalidate(1);
        }
    }
}

LRESULT clockBtn::timerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_TIMER:
    {
        ((clockBtn*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->updateClock(static_cast<clockFormat>(m_Style.clock_TimeFormat));

        SYSTEMTIME time;
        GetLocalTime(&time);
        SetTimer(hwnd, CLOCK_TIMER, 1100 - time.wMilliseconds, 0);
    }
    break;
    case WM_DESTROY:
        KillTimer(hwnd, CLOCK_TIMER);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool clockBtn::classRegistered = false;

clockBtn::clockBtn() : barItem(M_CLOCK)
{
    createClockTimer();
}

clockBtn::~clockBtn()
{
    DestroyWindow(messageWindow);
}

void clockBtn::createClockTimer()
{
    if (!classRegistered) {
        WNDCLASS wc = { };

        wc.lpfnWndProc = timerProc;
        wc.hInstance = LB_Api::main_hinstance;
        wc.lpszClassName = L"clockproo";

        if (!RegisterClass(&wc)) {
            return;
        }
        classRegistered = true;
    }

    // Create the window.

    messageWindow = CreateWindow(L"clockproo", 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);
    SetWindowLongPtr(messageWindow, GWLP_USERDATA, (LONG_PTR)this); // sets the GWLP_USERDATA to the object so we can get it easily via the HWND 

    SYSTEMTIME lt;
    GetLocalTime(&lt);
    SetTimer(messageWindow, CLOCK_TIMER, 0, 0);
}

void clockBtn::draw(HWND hWnd, HDC hDC)
{
    COLORREF color;

    RECT drawingRect;
    int y = m_Style.border_Width;
    drawingRect.left = itemRect.left;
    drawingRect.top = y > 0 ? y : 1;
    drawingRect.right = itemRect.right;
    drawingRect.bottom = itemRect.bottom - (y);

    // draw/clear the selection rectangle
    HBRUSH background = CreateSolidBrush(m_Style.clock_Color != CLR_INVALID ? m_Style.clock_Color : m_Style.windowBackgroundColor);
    HPEN borderPen = CreatePen(PS_SOLID, 1, m_Style.borderColor);

    HGDIOBJ hOldBrush = SelectObject(hDC, background);
    HGDIOBJ hOldPen = SelectObject(hDC, m_Style.clock_BevelStyle == 1 ? borderPen : GetStockObject(NULL_PEN)); // allows the pen to make a bevel for us so its easier when its rounded for example.

    if (m_Style.rectRoundedEdge_Clock != true)
        Rectangle(hDC, drawingRect.left, drawingRect.top, drawingRect.right, drawingRect.bottom);
    else
        RoundRect(hDC, drawingRect.left, drawingRect.top, drawingRect.right, drawingRect.bottom, m_Style.rectRoundedEdge_Clock_Width, m_Style.rectRoundedEdge_Clock_Height);

    //.m_Style.taskBevelStyle != 1 CUS it creates a flat looking bevel using the pen.
    if (m_Style.clock_BevelStyle != 0 && m_Style.clock_BevelStyle != 1)
        DrawingApi::drawBevel(hDC, m_Style.borderColor, &drawingRect, m_Style.clock_BevelStyle, BF_RECT);




    HGDIOBJ hOldFont = SelectObject(hDC, mainbar->m_hClockFont);

    SetBkMode(hDC, TRANSPARENT);

    color = m_Style.unfocusedTextColor;



    SetTextColor(hDC, color);
    DrawText(hDC, m_strName.c_str(), -1, &drawingRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(hDC, hOldFont);
    SelectObject(hDC, hOldPen);
    SelectObject(hDC, hOldBrush);
    SelectObject(hDC, GetStockObject(NULL_BRUSH));

    DeleteObject(background);
    DeleteObject(borderPen);
}

void clockBtn::mouse_event(int mx, int my, int message, unsigned flags)
{

    if (message == WM_RBUTTONUP) {
        ShellExecuteA(NULL, "open", "sndvol.exe", NULL, NULL, SW_SHOW);
    }

}
