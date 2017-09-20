#include "rbf_to_c.h"
#include "ui_rbf_to_c.h"

#include <QDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>

#include <QDateTime>

#include <QDebug>

#define SYMBOL_OK                            "<font color=green>●</font>"
#define SYMBOL_FAIL                          "<font color=red>●</font>"

#define MIN_FILE_SIZE_FOR_RPOGRESSBAR        50000
#define STEP_PROGRESSBAR                     1000

QString hFileName;

RBF_To_C::RBF_To_C(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::RBF_To_C)
{
  ui->setupUi(this);
  ui->progressBar->setValue(0);
  ui->progressBar->setVisible(false);
}

RBF_To_C::~RBF_To_C()
{
  delete ui;
}

void RBF_To_C::on_OpenRBF_Button_clicked()
{
  QString pathToFile = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Raw Binary Format Files (*.rbf)"));

  if (!pathToFile.isEmpty())
    ui->Path_To_RBF->setText(pathToFile);

  QFile file(ui->Path_To_RBF->text());

  if (file.open(QIODevice::ReadOnly))
  {
    QFileInfo fileInfo(file);

    ui->status_label->setText(tr("<font color=green<b>Select ") + fileInfo.fileName() + tr(" file.</b></font>"));
    ui->StatusPathRBF->setText(SYMBOL_OK);
    file.close();
  }
  else
  {
    ui->status_label->setText(tr("<font color=red<b>Path to RBF file is not correct.</b></font>"));
    ui->StatusPathRBF->setText(SYMBOL_FAIL);
  }
}

void RBF_To_C::on_OpenH_Button_clicked()
{
  QString pathToFile = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("C header file (*.h)"));

  if (!pathToFile.isEmpty())
    ui->Path_To_H->setText(pathToFile);

  QFile file(ui->Path_To_H->text());

  if (file.open(QIODevice::WriteOnly))
  {
    QFileInfo fileInfo(file);
    hFileName = fileInfo.fileName();

    ui->status_label->setText(tr("<font color=green<b>Select or create ") + fileInfo.fileName() + tr(" file.</b></font>"));
    ui->StatusPathH->setText(SYMBOL_OK);
    file.close();
  }
  else
  {
    ui->status_label->setText(tr("<font color=red<b>Path to header file is not correct.</b></font>"));
    ui->StatusPathH->setText(SYMBOL_FAIL);
  }
}

void RBF_To_C::on_Convert_Button_clicked()
{
  QFile rbfFile(ui->Path_To_RBF->text());
  QFile hFile(ui->Path_To_H->text());

  quint8 buffer;

  bool isDefaultArrayName = false;

  if ((rbfFile.open(QIODevice::ReadOnly)) && (hFile.open(QIODevice::WriteOnly)))
  {
    QDataStream rbfStream(&rbfFile);
    QTextStream hStream(&hFile);

    QFileInfo rbfFileInfo(rbfFile);
    QFileInfo hFileInfo(hFile);

    qint32 column_count = 0;

    ui->status_label->setText(tr("<font color=orange<b>Size file ") + rbfFileInfo.fileName() + " " + QString::number(rbfFileInfo.size()/1024.0) + tr(" KiB.</b></font>"));

    hFileName = hFileInfo.fileName().replace('.', '_');

    hStream << "#ifndef " << hFileName.toUpper() << "_\n";
    hStream << "#define " << hFileName.toUpper() << "_\n\n";

    hStream << "#include <stdint.h>\n";

    if (ui->IsAddDateTime->checkState() == Qt::Checked)
    {
      hStream << "\n/** Generated " << QDateTime::currentDateTime().toString("dd.MM.yy")
              << " at " << QDateTime::currentDateTime().toString("hh:mm:ss") << " */\n\r";
    }

    QStringList linesComment = ui->TextComment->toPlainText().split('\n');
    QString firstLine = linesComment.at(0);

    if (firstLine.length() > 0)
    {
      for (qint32 i = 0; i < linesComment.length(); i++)
      {
        if (i == 0)
          hStream << "\n/** Comment to file:\n";

        hStream << " *  " << linesComment.at(i) << "\n";

        if (i == linesComment.length() - 1)
          hStream << " */\n";
      }
    }

    if (linesComment.length() != 0)
      hStream << "\n";

    if (ui->ArrayName->text().length() > 0)
    {
      hStream.setIntegerBase(10);
      hStream << "const uint32_t " << ui->ArrayName->text() << "_Size = " << rbfFileInfo.size() << "; /** in bytes */\n\n";

    if (ui->IsFirstLSB->isChecked())
      hStream << "/** first LBS bit */\n";
    else
      hStream << "/** first MBS bit */\n";

      hStream << "const uint8_t " << ui->ArrayName->text() << "[] = \n{\n";
      isDefaultArrayName = false;
    }
    else
    {
      hStream.setIntegerBase(10);
      hStream << "const uint32_t FPGA_Firmware_Size = " << rbfFileInfo.size() << "; /** in bytes */\n\n";

      if (ui->IsFirstLSB->isChecked())
        hStream << "/** first LBS bit */\n";
      else
        hStream << "/** first MBS bit */\n";

      hStream << "const uint8_t FPGA_Firmware[] = \n{\n";
      isDefaultArrayName = true;
    }

    hStream.setIntegerBase(16);

    hStream << "  ";

    if (rbfFileInfo.size() > MIN_FILE_SIZE_FOR_RPOGRESSBAR)
      ui->progressBar->setVisible(true);
    else
      ui->progressBar->setVisible(false);

    ui->progressBar->setMaximum(rbfFileInfo.size()/STEP_PROGRESSBAR);
    ui->progressBar->setValue(0);

    for(qint64 i = 0; i < rbfFileInfo.size(); i++)
    {
      rbfStream >> buffer;

      column_count++;

    if (ui->IsFirstLSB->isChecked())
    {
      if (i == rbfFileInfo.size() - 1)
        hStream << QString("%0%1 ").arg("0x").arg(buffer, 2, 16, QChar('0'));
      else
        hStream << QString("%0%1, ").arg("0x").arg(buffer, 2, 16, QChar('0'));
    }
    else
    {
      quint8 result = buffer & 1;
      qint32 shift = sizeof(buffer) * CHAR_BIT - 1;

      for (buffer >>= 1; buffer; buffer >>= 1)
      {
        result <<= 1;
        result |= buffer & 1;
        shift--;
      }
      result <<= shift;

      if (i == rbfFileInfo.size() - 1)
        hStream << QString("%0%1 ").arg("0x").arg(result, 2, 16, QChar('0'));
      else
        hStream << QString("%0%1, ").arg("0x").arg(result, 2, 16, QChar('0'));
    }

    if (column_count == ui->ColumnArray->value() && i < rbfFileInfo.size() - 1)
    {
      hStream << "\n  ";
      column_count = 0;
    }
    else
      if (i == rbfFileInfo.size() - 1)
        hStream << "\n";

      ui->progressBar->setValue(i/STEP_PROGRESSBAR);
    }
    ui->progressBar->setVisible(false);

    hStream << "};\n\n#endif\n\n/** end file " << hFileInfo.fileName() << " */\n";

    if (isDefaultArrayName)
      ui->status_label->setText(tr("<font color=orange<b>Converted at ") + QDateTime::currentDateTime().toString("hh:mm:ss") + tr(". Use default array name.</b></font>"));
    else
      ui->status_label->setText(tr("<font color=green<b>Converted at ") + QDateTime::currentDateTime().toString("hh:mm:ss") + tr(".</b></font>"));
    }
  else
  {
    ui->status_label->setText(tr("<font color=red<b>Converted failed.</b></font>"));
  }
  rbfFile.close();
  hFile.close();
}
