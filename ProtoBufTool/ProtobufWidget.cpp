#include "ProtobufWidget.h"
#include "ui_ProtobufWidget.h"
#include <QFileDialog>

ProtobufWidget::ProtobufWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProtobufWidget)
{
    ui->setupUi(this);
    _pProtobufThread = new ProtobufThread(this);
    connect(_pProtobufThread,&ProtobufThread::finished,this,&ProtobufWidget::slotProtobufThreadFinish);
}

ProtobufWidget::~ProtobufWidget()
{
    delete ui;
}

void ProtobufWidget::on_pushButton_Conversion_clicked()
{
    if(_pProtobufThread->isRunning()) {
        ui->label_status->setText(tr("线程正在运行中..."));
        return;
    }
    int threadType = 0;
    if(ui->radioButton->isChecked()){
        threadType = 0;
    }else if(ui->radioButton_2->isChecked()){
        threadType = 1;
    }
    if(_pProtobufThread->initProtobufThread(ui->lineEdit_protoFilePath->text(),
                                           ui->lineEdit_packageName->text(),
                                           ui->lineEdit_messageName->text(),
                                           ui->lineEdit_conversionFilePath->text(),
                                           threadType,
                                           ui->textEdit_json->toPlainText(),
                                           !ui->checkBox_indentation->isChecked()
                                           ))
    {
        ui->label_status->setText("正在转换...");
        _pProtobufThread->start();
    }else{
        ui->label_status->setText(QString("error: %1").arg(_pProtobufThread->lastError()));
    }
}
void ProtobufWidget::slotProtobufThreadFinish()
{
    if(!_pProtobufThread->lastError().isEmpty()){
        ui->label_status->setText(QString("error: %1").arg(_pProtobufThread->lastError()));
    }else{
        switch (_pProtobufThread->threadType()) {
        case 0:
        {
            ui->label_status->setText(tr("proto 二进制转 json 成功."));
            ui->textEdit_json->setText(_pProtobufThread->jsonData());
            QString path = QFileDialog::getSaveFileName(this, tr("json"), QString("./untitled.json"), tr("JSON(*.json)"));
            if (!path.isEmpty()) {
                QFile file(path);
                if(file.open(QIODevice::WriteOnly)){
                    file.write(ui->textEdit_json->toPlainText().toLatin1());
                    file.close();
                }
            }
        }
            break;
        case 1:
        {
//            QString path = QFileDialog::getSaveFileName(this, tr("message"), QString("./untitled.message"), tr("message(*.message)"));
//            if (!path.isEmpty()) {
//                QFile file(path);
//                if(file.open(QIODevice::WriteOnly)){
//                    if(!_pProtobufThread->msgByteArray().isEmpty()){

//                        file.write(_pProtobufThread->msgByteArray());

//                    }else{
//                        ui->label_status->setText(tr("message二进制文件数据为空."));
//                    }
//                    file.close();
//                }else{
//                    ui->label_status->setText(QString("打开【%1】文件失败.").arg(path));
//                }
//               ui->label_status->setText(QString("已保存：%1").arg(path));
             ui->label_status->setText(QString("已保存：%1").arg(_pProtobufThread->outBinaryFilePath()));
//            }
        }
            break;
        default:
            break;
        }
    }
}

void ProtobufWidget::on_radioButton_clicked(bool checked)
{
    if(checked){
        ui->pushButton_loadConversionFile->setText(tr("加载二进制文件"));
    }
}

void ProtobufWidget::on_radioButton_2_clicked(bool checked)
{
    if(checked){
        ui->pushButton_loadConversionFile->setText(tr("加载Json文件"));
    }
}

void ProtobufWidget::on_pushButton_loadConversionFile_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    ui->pushButton_loadConversionFile->text(),
                                                    "",
                                                    tr("文件(*.*)"));
    if (filePath.isEmpty())
    {
        return;
    }
    ui->lineEdit_conversionFilePath->setText(filePath);
}

void ProtobufWidget::on_pushButton_loadProtoFile_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    ui->pushButton_loadProtoFile->text(),
                                                    "",
                                                    tr("文件(*.proto)"));
    if (filePath.isEmpty())
    {
        return;
    }
    ui->lineEdit_protoFilePath->setText(filePath);
}
