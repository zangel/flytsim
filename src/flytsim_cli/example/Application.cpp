#include "Application.hpp"
#include "MainWindow.hpp"

namespace cli { namespace example {

  Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
  {
    setApplicationDisplayName("Example");
    setOrganizationName("Example");
    setOrganizationDomain("example.com");
    setApplicationName("Example");

    m_pMainWindow = new MainWindow();
    m_pMainWindow->show();
  }

  Application::~Application()
  {
 
  }

  Application* Application::instance()
  {
    return static_cast<Application*>(QCoreApplication::instance());
  }

  MainWindow* Application::getMainWindow()
  {
    return m_pMainWindow;
  }

} //namespace example
} //namespace cli