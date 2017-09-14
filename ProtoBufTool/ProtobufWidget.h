#ifndef PROTOBUFWIDGET_H
#define PROTOBUFWIDGET_H

#include <QWidget>
#include "ProtobufThread.h"

namespace Ui {
class ProtobufWidget;
}

class ProtobufWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProtobufWidget(QWidget *parent = 0);
    ~ProtobufWidget();

public slots:
    void slotProtobufThreadFinish();
private slots:
    void on_pushButton_Conversion_clicked();

    void on_radioButton_clicked(bool checked);

    void on_radioButton_2_clicked(bool checked);

    void on_pushButton_loadConversionFile_clicked();

    void on_pushButton_loadProtoFile_clicked();

private:
    Ui::ProtobufWidget *ui;
    ProtobufThread *pProtobufThread;
};

#endif // PROTOBUFWIDGET_H
