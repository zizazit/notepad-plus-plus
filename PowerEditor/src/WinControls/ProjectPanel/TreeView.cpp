//this file is part of notepad++
//Copyright (C)2011 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "precompiledHeaders.h"
#include "TreeView.h"


#define CX_BITMAP         16
#define CY_BITMAP         16
#define CY_ITEMHEIGHT     18
#define NUM_BITMAPS       3


void TreeView::init(HINSTANCE hInst, HWND parent, int treeViewID)
{
	Window::init(hInst, parent);
	_hSelf = ::GetDlgItem(parent, treeViewID);
	TreeView_SetItemHeight(_hSelf, CY_ITEMHEIGHT);
}

BOOL TreeView::initImageList(int project_root_id, int open_node_id, int closed_node_id, int leaf_id) 
{
	int i;
	HBITMAP hbmp;

	// Creation of image list
	if ((_hImaLst = ImageList_Create(CX_BITMAP, CY_BITMAP, ILC_COLOR32 | ILC_MASK, NUM_BITMAPS, 0)) == NULL) 
		return FALSE;

	// Add the bmp in the list
	hbmp = LoadBitmap(_hInst, MAKEINTRESOURCE(project_root_id));
	if(hbmp == NULL)
		return FALSE;
	i =ImageList_Add(_hImaLst, hbmp, (HBITMAP)NULL);
	DeleteObject(hbmp);

	hbmp = LoadBitmap(_hInst, MAKEINTRESOURCE(open_node_id));
	if(hbmp == NULL)
		return FALSE;
	i =ImageList_Add(_hImaLst, hbmp, (HBITMAP)NULL);
	DeleteObject(hbmp);

	hbmp = LoadBitmap(_hInst, MAKEINTRESOURCE(closed_node_id));
	if(hbmp == NULL)
		return FALSE;
	i =ImageList_Add(_hImaLst, hbmp, (HBITMAP)NULL);
	DeleteObject(hbmp);

	hbmp = LoadBitmap(_hInst, MAKEINTRESOURCE(leaf_id));
	if(hbmp == NULL)
		return FALSE;
	i =ImageList_Add(_hImaLst, hbmp, (HBITMAP)NULL);
	DeleteObject(hbmp);

	if (ImageList_GetImageCount(_hImaLst) < 4)
		return FALSE;

	// Set image list to the tree view
	TreeView_SetImageList(_hSelf, _hImaLst, TVSIL_NORMAL);

	return TRUE;
}


void TreeView::destroy()
{
	::DestroyWindow(_hSelf);
	_hSelf = NULL;
} 

LRESULT TreeView::runProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	return ::CallWindowProc(_defaultProc, hwnd, Message, wParam, lParam);
}

HTREEITEM TreeView::addItem(const TCHAR *itemName, HTREEITEM hParentItem, int iImage, const TCHAR *filePath)
{
	TVITEM tvi;
	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM; 

	// Set the item label.
	tvi.pszText = (LPTSTR)itemName; 
	tvi.cchTextMax = sizeof(tvi.pszText)/sizeof(tvi.pszText[0]); 

	// Set icon
	tvi.iImage = iImage;//isNode?INDEX_CLOSED_NODE:INDEX_LEAF; 
	tvi.iSelectedImage = iImage;//isNode?INDEX_OPEN_NODE:INDEX_LEAF; 

	// Save the full path of file in the item's application-defined data area. 
	tvi.lParam = (filePath == NULL?0:(LPARAM)(new generic_string(filePath)));

	TVINSERTSTRUCT tvInsertStruct;
	tvInsertStruct.item = tvi; 
	tvInsertStruct.hInsertAfter = (HTREEITEM)TVI_LAST;
	tvInsertStruct.hParent = hParentItem;

	return (HTREEITEM)::SendMessage(_hSelf, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvInsertStruct);
}


/*
HTREEITEM TreeView::addItem(const TCHAR *itemName, int nLevel)
{ 
	TVITEM tvi; 
	TVINSERTSTRUCT tv_insert_struct;
	static HTREEITEM hPrev = (HTREEITEM)TVI_FIRST; 
	static HTREEITEM hPrevRootItem = NULL; 
	static HTREEITEM hPrevLev2Item = NULL; 
	HTREEITEM hti; 

	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM; 

	// Set the text of the item.
	tvi.pszText = (LPTSTR) itemName; 
	tvi.cchTextMax = sizeof(tvi.pszText)/sizeof(tvi.pszText[0]); 

	// Assume the item is not a parent item, so give it a 
	// document image.

	tvi.iImage = INDEX_LEAF; 
	tvi.iSelectedImage = INDEX_LEAF; 

	// Save the heading level in the item's application-defined 
	// data area. 
	tvi.lParam = (LPARAM)nLevel; 
	tv_insert_struct.item = tvi; 
	tv_insert_struct.hInsertAfter = hPrev; 

	// Set the parent item based on the specified level. 
	if (nLevel == 1) 
		tv_insert_struct.hParent = TVI_ROOT; 
	else if (nLevel == 2) 
		tv_insert_struct.hParent = hPrevRootItem; 
	else 
		tv_insert_struct.hParent = hPrevLev2Item; 

	// Add the item to the tree-view control. 
	hPrev = (HTREEITEM)SendMessage(_hSelf, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tv_insert_struct); 

	if (hPrev == NULL)
		return NULL;

	// Save the handle to the item. 
	if (nLevel == 1)
		hPrevRootItem = hPrev;
	else if (nLevel == 2)
		hPrevLev2Item = hPrev;

	// The new item is a child item. Give the parent item a 
	// closed folder bitmap to indicate it now has child items. 
	if (nLevel > 1)
	{ 
		hti = TreeView_GetParent(_hSelf, hPrev);
		tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.hItem = hti;
		tvi.iImage = INDEX_CLOSED_NODE;
		tvi.iSelectedImage = INDEX_CLOSED_NODE;
		TreeView_SetItem(_hSelf, &tvi);
	}
	return hPrev; 
}
*/
