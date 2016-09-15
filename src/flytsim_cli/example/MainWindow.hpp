#ifndef EXAMPLE_MAIN_WINDOW_HPP
#define EXAMPLE_MAIN_WINDOW_HPP

#include "Config.hpp"
#include "ui_MainWindow.h"

namespace cli { namespace example {
  class MainWindow
    : public QMainWindow
    , protected Ui::MainWindow
  {
    Q_OBJECT
  protected:


  public:
    MainWindow();
    ~MainWindow();
  };

} //namespace example
} //namespace cli

#endif //EXAMPLE_MAIN_WINDOW_HPP