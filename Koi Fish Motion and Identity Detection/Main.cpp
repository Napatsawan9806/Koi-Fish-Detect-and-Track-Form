#include "MainForm.h"

using namespace System;
using namespace System::Windows::Forms;
using namespace KoiTracker;

[STAThread]
int Main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);
    Application::Run(gcnew MainForm());
    return 0;
}