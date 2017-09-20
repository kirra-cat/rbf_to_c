#ifndef RBF_TO_C_H
#define RBF_TO_C_H

#include <QWidget>

namespace Ui 
{
  class RBF_To_C;
}

class RBF_To_C : public QWidget
{
  Q_OBJECT

public:
  explicit RBF_To_C(QWidget *parent = 0);
  ~RBF_To_C();

private slots:
  void on_OpenRBF_Button_clicked();

  void on_OpenH_Button_clicked();

  void on_Convert_Button_clicked();

private:
  Ui::RBF_To_C *ui;
};

#endif // RBF_TO_C_H
