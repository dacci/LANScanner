// Copyright (c) 2014 dacci.org

#include "app/lan_scanner.h"

#include "ui/main_dialog.h"

CAppModule _Module;

int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
                       wchar_t* /*command_line*/, int /*show_mode*/) {
  HRESULT result = S_OK;

  result = _Module.Init(nullptr, hInstance);
  ATLASSERT(SUCCEEDED(result));
  if (FAILED(result))
    return __LINE__;

  {
    CMessageLoop message_loop;
    _Module.AddMessageLoop(&message_loop);

    MainDialog dialog;
    if (dialog.Create(NULL)) {
      dialog.ShowWindow(SW_SHOWNORMAL);
      dialog.UpdateWindow();

      message_loop.Run();
    }

    _Module.RemoveMessageLoop();
  }

  _Module.Term();

  return 0;
}
