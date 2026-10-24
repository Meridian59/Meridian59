/*
 * windowsx.h:  Wine's windowsx.h, with the item-data setters passing pointers
 *   whole.  Wine's cast the data through DWORD, which truncates the pointers the
 *   client stores as list and combo box item data; Microsoft's cast to LPARAM.
 */

#include_next <windowsx.h>

#undef ComboBox_SetItemData
#define ComboBox_SetItemData(hwndCtl, index, data) \
	((int)SendMessage((hwndCtl), CB_SETITEMDATA, (WPARAM)(int)(index), (LPARAM)(data)))
#undef ListBox_SetItemData
#define ListBox_SetItemData(hwndCtl, index, data) \
	((int)SendMessage((hwndCtl), LB_SETITEMDATA, (WPARAM)(int)(index), (LPARAM)(data)))
