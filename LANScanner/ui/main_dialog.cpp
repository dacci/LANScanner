// Copyright (c) 2014 dacci.org

#include "ui/main_dialog.h"

#include <atlstr.h>

#include "app/lan_scanner.h"

MainDialog::MainDialog() : scan_(), cancel_() {
}

BOOL MainDialog::PreTranslateMessage(MSG* message) {
  if (IsDialogMessage(message))
    return TRUE;

  return FALSE;
}

bool MainDialog::StartScan() {
  if (scan_ != NULL)
    return false;

  int index = network_combo_.GetCurSel();
  if (index < 0)
    return false;

  cancel_ = false;

  CString text;
  text.LoadString(IDCANCEL);
  scan_button_.SetWindowText(text);

  neighbour_list_.DeleteAllItems();

  scan_ = net_util::ScanNetwork(networks_[index], ScanCallback, this);
  if (scan_ == NULL) {
    EndScan();
    return false;
  }

  return true;
}

bool MainDialog::StopScan() {
  if (scan_ == NULL)
    return false;

  cancel_ = true;
  scan_button_.EnableWindow(FALSE);

  return true;
}

void MainDialog::EndScan() {
  if (scan_ != NULL) {
    CloseHandle(scan_);
    scan_ = NULL;
  }

  CString text;
  text.LoadString(IDOK);
  scan_button_.SetWindowText(text);
  scan_button_.EnableWindow();
}

bool MainDialog::ScanCallback(DWORD result, sockaddr_in* address,
                              BYTE* mac_address, void* param) {
  return static_cast<MainDialog*>(param)->
      ScanCallback(result, address, mac_address);
}

bool MainDialog::ScanCallback(DWORD result, sockaddr_in* address,
                              BYTE* mac_address) {
  CString text;

  if (result == ERROR_NO_MORE_ITEMS) {
    EndScan();
    return false;
  }

  text.Format(L"%d.%d.%d.%d",
              address->sin_addr.S_un.S_un_b.s_b1,
              address->sin_addr.S_un.S_un_b.s_b2,
              address->sin_addr.S_un.S_un_b.s_b3,
              address->sin_addr.S_un.S_un_b.s_b4);

  int index = neighbour_list_.GetItemCount();
  index = neighbour_list_.AddItem(index, 0, text);

  text.Empty();

  if (result == ERROR_SUCCESS) {
    for (int i = 0; i < 6; ++i) {
      if (i > 0)
        text.AppendChar(L':');
      text.AppendFormat(L"%02X", mac_address[i]);
    }
  } else {
    text.Format(L"Error: %lu", result);
  }

  neighbour_list_.AddItem(index, 1, text);

  return !cancel_;
}

BOOL MainDialog::OnInitDialog(CWindow focus, LPARAM init_param) {
  SetIcon(AtlLoadIcon(IDD_MAIN));
  DoDataExchange(DDX_LOAD);
  DlgResize_Init();

  CString text;
  net_util::GetConnectedNetworks(&networks_);
  for (auto network : networks_) {
    text.Format(L"%d.%d.%d.%d/%d",
                network.address.sin_addr.S_un.S_un_b.s_b1,
                network.address.sin_addr.S_un.S_un_b.s_b2,
                network.address.sin_addr.S_un.S_un_b.s_b3,
                network.address.sin_addr.S_un.S_un_b.s_b4,
                network.prefix);
    network_combo_.AddString(text);
  }

  neighbour_list_.SetExtendedListViewStyle(
      LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS);

  text.LoadString(IDS_IP_ADDRESS);
  neighbour_list_.InsertColumn(0, text, LVCFMT_LEFT, 120);

  text.LoadString(IDS_MAC_ADDRESS);
  neighbour_list_.InsertColumn(1, text, LVCFMT_LEFT, 120);

  _Module.GetMessageLoop()->AddMessageFilter(this);

  return TRUE;
}

void MainDialog::OnDestroy() {
  _Module.GetMessageLoop()->RemoveMessageFilter(this);

  PostQuitMessage(0);
}

void MainDialog::OnOK(UINT notify_code, int id, CWindow control) {
  if (scan_ != NULL)
    StopScan();
  else
    StartScan();
}

void MainDialog::OnCancel(UINT notify_code, int id, CWindow control) {
  if (scan_ != NULL)
    StopScan();
  else
    DestroyWindow();
}

void MainDialog::OnEditCopy(UINT notify_code, int id, CWindow control) {
  CString text, item;

  for (int index = -1;;) {
    index = neighbour_list_.GetNextItem(index, LVNI_SELECTED);
    if (index == -1)
      break;

    int length = neighbour_list_.GetItemText(index, 0, item.GetBuffer(16), 16);
    item.ReleaseBuffer(length);
    text += item;

    text.AppendChar(L'\t');

    length = neighbour_list_.GetItemText(index, 1, item.GetBuffer(18), 18);
    item.ReleaseBuffer(length);
    text += item;

    text.Append(L"\x0D\x0A");
  }

  if (!OpenClipboard())
    return;

  if (EmptyClipboard()) {
    int size = (text.GetLength() + 1) * sizeof(CString::XCHAR);
    HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, size);
    if (handle != NULL) {
      void* memory = GlobalLock(handle);
      if (memory != nullptr) {
        memmove_s(memory, size, text.GetString(), size);
        SetClipboardData(CF_UNICODETEXT, memory);
        GlobalUnlock(memory);
      } else {
        GlobalFree(handle);
      }
    }
  }

  CloseClipboard();
}

LRESULT MainDialog::OnItemRClick(NMHDR* header) {
  NMITEMACTIVATE* notify = reinterpret_cast<NMITEMACTIVATE*>(header);
  if (notify->iItem < 0)
    return 0;

  CMenu menu;
  if (!menu.LoadMenu(IDD_MAIN))
    return 0;

  CMenuHandle context_menu;
  context_menu = menu.GetSubMenu(0);
  if (!context_menu)
    return 0;

  POINT point = notify->ptAction;
  neighbour_list_.ClientToScreen(&point);

  context_menu.TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, m_hWnd);

  return 0;
}
