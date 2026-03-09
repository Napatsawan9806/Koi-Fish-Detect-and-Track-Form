#include "MainForm.h"

using namespace System;
using namespace System::Windows::Forms;
using namespace KoiTracker;

[STAThread]
int Main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);
    Application::SetUnhandledExceptionMode(UnhandledExceptionMode::CatchException);
    try {
        Application::Run(gcnew MainForm());
    }
    catch (System::Exception^ ex) {
        MessageBox::Show(
            ex->Message + L"\n\nStack Trace:\n" + ex->StackTrace,
            L"Startup Error",
            MessageBoxButtons::OK,
            MessageBoxIcon::Error);
    }
    catch (...) {
        MessageBox::Show(L"Unknown native exception during startup.",
            L"Startup Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
    }
    return 0;
}