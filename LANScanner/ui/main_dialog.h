// Copyright (c) 2014 dacci.org

#ifndef LANSCANNER_UI_MAIN_DIALOG_H_
#define LANSCANNER_UI_MAIN_DIALOG_H_

#include <atlbase.h>

#include <atlapp.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlddx.h>
#include <atldlgs.h>
#include <atlframe.h>

#include "misc/net_util.h"
#include "res/resource.h"

class MainDialog
    : public CDialogImpl<MainDialog>,
      public CWinDataExchange<MainDialog>,
      public CDialogResize<MainDialog>,
      public CMessageFilter {
 public:
  static const UINT IDD = IDD_MAIN;

  MainDialog();

  BEGIN_MSG_MAP(MainDialog)
    MSG_WM_INITDIALOG(OnInitDialog)
    MSG_WM_DESTROY(OnDestroy)

    COMMAND_ID_HANDLER_EX(IDOK, OnOK)
    COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
    COMMAND_ID_HANDLER_EX(ID_EDIT_COPY, OnEditCopy)

    NOTIFY_HANDLER_EX(IDC_NEIGHBOURS, NM_RCLICK, OnItemRClick)

    CHAIN_MSG_MAP(CDialogResize)
  END_MSG_MAP()

  BEGIN_DDX_MAP(MainDialog)
    DDX_CONTROL_HANDLE(IDC_NETWORK, network_combo_)
    DDX_CONTROL_HANDLE(IDOK, scan_button_)
    DDX_CONTROL_HANDLE(IDC_NEIGHBOURS, neighbour_list_)
  END_DDX_MAP()

  BEGIN_DLGRESIZE_MAP(MainDialog)
    DLGRESIZE_CONTROL(IDC_NETWORK, DLSZ_SIZE_X)
    DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(IDC_NEIGHBOURS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
  END_DLGRESIZE_MAP()

  BOOL PreTranslateMessage(MSG* message) override;

 private:
  bool StartScan();
  bool StopScan();
  void EndScan();

  static bool ScanCallback(DWORD result, sockaddr_in* ip_address,
                           BYTE* mac_address, void* param);
  bool ScanCallback(DWORD result, sockaddr_in* address, BYTE* mac_address);

  BOOL OnInitDialog(CWindow focus, LPARAM init_param);
  void OnDestroy();

  void OnOK(UINT notify_code, int id, CWindow control);
  void OnCancel(UINT notify_code, int id, CWindow control);
  void OnEditCopy(UINT notify_code, int id, CWindow control);

  LRESULT OnItemRClick(NMHDR* header);

  std::vector<net_util::PrefixedAddress> networks_;
  HANDLE scan_;
  bool cancel_;

  CComboBox network_combo_;
  CButton scan_button_;
  CListViewCtrl neighbour_list_;
};

#endif  // LANSCANNER_UI_MAIN_DIALOG_H_
