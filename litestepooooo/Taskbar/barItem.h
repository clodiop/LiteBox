#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include "drawingApi.h"

class CTaskbar;
class barItem
{
public:
    static CTaskbar* mainbar;
    RECT itemRect; // use this to know where it is.
    DWORD           m_dwFlags;                    // various flags
    std::wstring     m_strName;                    // text name
    HICON            m_icon;               
    int buttonType;
    int m_ItemID;
    


    LPARAM m_data; // Data which could be a HWND or something
protected:
    static int s_id;
    void Assign(const barItem& src);                // copy src to 
public:
    barItem(int type);                                      // default constructor
    barItem(const barItem& src);                      // copy constructor
    virtual ~barItem();                             // default constructor

    barItem& operator=(const barItem& src);           // assignment operator

    // check the item for mouse-over
    int mouse_over(int mx, int my);

    //-----------------------------
    virtual void mouse_event(int mx, int my, int message, unsigned flags);

    virtual void invalidate(int flag);


    //-----------------------------
    // calculate metrics, return true on changes
    virtual bool calc_size(int* px, int y, int w, int h);

    //-----------------------------
    virtual void draw(HWND hWnd, HDC hDC);
};

