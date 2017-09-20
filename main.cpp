#include "rbf_to_c.h"
#include <QApplication>

#define VERSION_MAJOR 1
#define VERSION_MINOIR 0
#define DATA_BUILD "20.09.2017"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  RBF_To_C w;

  QString windowTitle = "Convert *.rbf file to header C file [v" + QString::number(VERSION_MAJOR) + "." + QString::number(VERSION_MINOIR) + "][" + DATA_BUILD + "]";

  w.setWindowTitle(windowTitle);

  w.show();

  return a.exec();
}
