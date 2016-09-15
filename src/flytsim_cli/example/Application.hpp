#ifndef EXAMPLE_APPLICATION_HPP
#define EXAMPLE_APPLICATION_HPP

#include "Config.hpp"

namespace cli { namespace example {

class MainWindow;

class Application
  : public QApplication
{
public:
  Application(int &argc, char **argv);
  ~Application();

  static Application* instance();

  MainWindow* getMainWindow();


private:
  MainWindow *m_pMainWindow;
};

} //namespace example
} //namespace cli



#endif //EXAMPLE_APPLICATION_HPP