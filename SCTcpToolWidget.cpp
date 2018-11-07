#include "SCTcpToolWidget.h"
#include "ui_SCTcpToolWidget.h"
#include <QDateTime>
#include <QFileDialog>
#include <QHostInfo>

SCTcpToolWidget::SCTcpToolWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SCTcpToolWidget)
{
    ui->setupUi(this);

    //db类
    initDb();

    //自动滚动
    connect(ui->textEdit_info,SIGNAL(textChanged()),this,SLOT(slotAutomaticallyScroll()));
    //tcp
    _pSCStatusTcp = new SCStatusTcp(this);
    connect(_pSCStatusTcp,SIGNAL(sigPrintInfo(QString)),this,SLOT(slotPrintInfo(QString)));
    connect(_pSCStatusTcp,SIGNAL(sigChangedText(bool,int,QByteArray,QByteArray,int,int)),
            this,SLOT(slotChangedText(bool,int,QByteArray,QByteArray,int,int)));
    //ip正则
    QRegExp regExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    QRegExpValidator *ev = new QRegExpValidator(regExp);
    ui->lineEdit_ip->setValidator(ev);
    //0-65535
    QIntValidator *intV = new QIntValidator(0,65535);
    ui->lineEdit_number->setValidator(intV);
    ui->lineEdit_sendCommand->setValidator(intV);

    on_checkBox_timeOut_clicked(true);
    //-------------------------
    _pProtobufWidget = new ProtobufWidget(ui->tabWidget);
    ui->tabWidget->addTab(_pProtobufWidget,QStringLiteral("proto二进制/Json转换"));
}

SCTcpToolWidget::~SCTcpToolWidget()
{
    //类不用手动释放，qt会自动释放SCTcpToolWidget的子类.
    delete ui;
}

void SCTcpToolWidget::initDb()
{
    _pSqliteClass = new SqliteClass(this);
    if(!_pSqliteClass->createConnection(QFileInfo("./Roboshop.db").absoluteFilePath())){
        SCWarning<<_pSqliteClass->errorString();
        return;
    }
    if( !_pSqliteClass->getProtocol()){
        SCWarning<<_pSqliteClass->errorString();
        return;
    }
    QMap<int ,QString> reqDescriptionMap = _pSqliteClass->getProtocol()->ReqValueReqDescriptionMap;
    for(int i=0; i<reqDescriptionMap.keys().size(); ++i){
        auto key = reqDescriptionMap.keys().at(i);
        if(_pSqliteClass->getProtocol()->getPort(key) == 10000) continue;
        QString desc = QString("%1 : %2")
                .arg(reqDescriptionMap.value(key))
                .arg(_pSqliteClass->getProtocol()->getReq(key));
        ui->comboBox_allReq->addItem(desc,key);
    }
    connect(ui->comboBox_allReq,SIGNAL(currentIndexChanged(int)),this,SLOT(slotAllReqCurrentChanged(int)));
}

void SCTcpToolWidget::slotAllReqCurrentChanged(int index)
{
    ui->lineEdit_sendCommand->setText(QString("%1").arg(ui->comboBox_allReq->itemData(index).toInt()));
}

/** socket连接/断开
 * @brief SCTcpToolWidget::on_pushButton_connect_clicked
 * @param checked
 */
void SCTcpToolWidget::on_pushButton_connect_clicked(bool checked)
{
    switch (_pSCStatusTcp->connectHost(ui->lineEdit_ip->text(),ui->comboBox_port->currentText().toInt())) {
    case 1:
        ui->pushButton_connect->setText(QStringLiteral("开始连接"));
        break;

    default:
        break;
    }
}

//TODO---------------tcp----------------------
/** tcp槽实时监测tcp状态
 * @brief SCTcpToolWidget::stateChanged
 * @param state
 */
void SCTcpToolWidget::stateChanged(QAbstractSocket::SocketState state)
{
    QString info;
    switch (state) {
    case QAbstractSocket::UnconnectedState:
        info = "QAbstractSocket::UnconnectedState";
        ui->comboBox_port->setEnabled(true);
        ui->pushButton_connect->setText(QStringLiteral("开始连接"));
        break;
    case QAbstractSocket::HostLookupState:
        info = "QAbstractSocket::HostLookupState";
        break;

    case QAbstractSocket::ConnectingState:
        info = "QAbstractSocket::ConnectingState";
        ui->pushButton_connect->setText(QStringLiteral("正在连接..."));
        ui->comboBox_port->setEnabled(false);
        break;
    case QAbstractSocket::ConnectedState:
    {
        info = "QAbstractSocket::ConnectedState \n";
        ui->pushButton_connect->setText(QStringLiteral("断开连接"));
    }
        break;
    case QAbstractSocket::BoundState:
        info = "QAbstractSocket::BoundState";
        break;
    case QAbstractSocket::ListeningState:
        info = "QAbstractSocket::ListeningState";
        break;
    case QAbstractSocket::ClosingState:
        info = "QAbstractSocket::ClosingState";
        ui->comboBox_port->setEnabled(true);
        ui->pushButton_connect->setText(QStringLiteral("开始连接"));
        break;
    }
    ui->textEdit_info->append(QString("%1 IP:%2:%3 %4")
                              .arg(_pSCStatusTcp->getCurrentDateTime())
                              .arg(ui->lineEdit_ip->text())
                              .arg(ui->comboBox_port->currentText())
                              .arg(info));
}
/** tcp槽 返回tcp错误
 * @brief SCTcpToolWidget::receiveTcpError
 * @param error
 */
void SCTcpToolWidget::receiveTcpError(QAbstractSocket::SocketError error)
{
    ui->textEdit_info->append(QString("%1  connect error[%2]: IP:%3:%4")
                              .arg(_pSCStatusTcp->getCurrentDateTime())
                              .arg(error)
                              .arg(ui->lineEdit_ip->text())
                              .arg(ui->comboBox_port->currentText()));
    ui->comboBox_port->setEnabled(true);
    ui->pushButton_connect->setText(QStringLiteral("开始连接"));

}

/** 发送
 * @brief SCTcpToolWidget::on_pushButton_send_clicked
 */
void SCTcpToolWidget::on_pushButton_send_clicked()
{
    if (_pSCStatusTcp->tcpSocket() && _pSCStatusTcp->tcpSocket()->state()==QAbstractSocket::ConnectedState)
    {
        //报头数据类型
        uint16_t sendCommand = ui->lineEdit_sendCommand->text().toInt();

        //数据区数据
        QString sendDataStr = ui->textEdit_sendData->toPlainText();
        QByteArray sendData = sendDataStr.toLatin1();

        //发送数据size
        quint64 sendDataSize = sendData.size();

        //如果数据区有.zip表示是文件直接打开读取发送
        if(sendDataStr.contains(".zip"))
        {
            QFile file(sendDataStr);

            if(file.open(QIODevice::ReadOnly))
            {
                sendData = file.readAll();
                sendDataSize = sendData.size();
                qDebug()<<"sendData(zip file): size"<<sendDataSize;
            }
            file.close();
        }
        //序号
        uint16_t number = ui->lineEdit_number->text().toInt();

        //清理接收数据区域
        ui->textEdit_revData->clear();

        //发送数据
        if(!_pSCStatusTcp->writeTcpData(sendCommand,sendData,number))
        {
            slotPrintInfo(QString(QStringLiteral("<font color=\"red\">"
                                                 "%1--------- 发送错误----------\n"
                                                 "发送的报文类型:%2  \n"
                                                 "错误: %3"
                                                 "</font>"))
                          .arg(_pSCStatusTcp->getCurrentDateTime())
                          .arg(sendCommand)
                          .arg(_pSCStatusTcp->lastError()));
        }
    }
    else
    {
        ui->textEdit_info->append(QString("UnconnectedState"));
    }
}
/** 发送后，响应
 * @brief SCTcpToolWidget::slotChangedText
 * @param isOk 是否正常返回
 * @param revCommand 返回的数据类型
 * @param revData 返回的数据
 * @param revHex 返回hex
 * @param number 序号
 * @param msTime 发送->返回时间 单位：ms
 */
void SCTcpToolWidget::slotChangedText(bool isOk,int revCommand,
                                      QByteArray revData,QByteArray revHex,
                                      int number,int msTime)
{
    if(isOk){

        int dataSize = 0;

        if(ui->checkBox_revHex->isChecked()){//16进制显示
            dataSize = revHex.size();
            ui->textEdit_revData->setText(_pSCStatusTcp->hexToQString(revHex));
        }else{//文本显示
            dataSize = revData.size();
            ui->textEdit_revData->setText(QString(revData));
        }
        ui->label_revText->setText(QString(QStringLiteral("响应的报文类型: %1 (0x%2) \t\n\n"
                                                          "序号: %4 (0x%5)\t\n\n"
                                                          "响应时间: %6 ms \t\n\n"
                                                          "响应数据区字节数: %7"))
                                   .arg(revCommand)
                                   .arg(QString::number(revCommand,16))
                                   .arg(number)
                                   .arg(QString::number(number,16))
                                   .arg(msTime)
                                   .arg(dataSize));
        //保存到SeerReceive.temp文件
        if(ui->checkBox_saveFile->isChecked()){
            QFile file("SeerReceive.temp");
            if(file.open(QIODevice::WriteOnly)){
                file.write(revData);
            }else{
                qWarning()<<QStringLiteral("打开SeerReceive.temp文件失败");
            }
            file.close();
        }

    }else{

        slotPrintInfo(QString(QStringLiteral("<font color=\"red\">"
                                             "%1--------- 返回错误----------\n"
                                             "报文类型:%2  \n"
                                             "错误: %3"
                                             "</font>"))
                      .arg(_pSCStatusTcp->getCurrentDateTime())
                      .arg(revCommand)
                      .arg(_pSCStatusTcp->lastError()));

        ui->textEdit_revData->setText(QString(revData));
        ui->label_revText->setText(QString(QStringLiteral("响应的错误: %1 \t\n"))
                                   .arg(_pSCStatusTcp->lastError()));
    }


}
/** 打印信息
 * @brief SCTcpToolWidget::slotPrintInfo
 * @param info
 */
void SCTcpToolWidget::slotPrintInfo(QString info)
{
    ui->textEdit_info->append(info);
}
/** 清空textEdit_info数据
 * @brief SCTcpToolWidget::on_pushButton_clearInfo_clicked
 */
void SCTcpToolWidget::on_pushButton_clearInfo_clicked()
{
    ui->textEdit_info->clear();
}
/** 自动滚动
 * @brief SCTcpToolWidget::slotAutomaticallyScroll
 */
void SCTcpToolWidget::slotAutomaticallyScroll()
{
    if(ui->checkBox_automatically->isChecked())
    {
        QTextEdit *textedit = (QTextEdit*)sender();
        if(textedit)
        {
            QTextCursor cursor = textedit->textCursor();
            cursor.movePosition(QTextCursor::End);
            textedit->setTextCursor(cursor);
        }
    }
}

void SCTcpToolWidget::on_pushButton_zipFile_clicked()
{

    QString filePath = QFileDialog::getOpenFileName(this, QString("rewrite"), ".", QString("zip File(*.zip"));

    if (filePath.isEmpty()) {
        return;
    } else{
        ui->textEdit_sendData->setText(filePath);
    }
}
//是否开启超时
void SCTcpToolWidget::on_checkBox_timeOut_clicked(bool checked)
{
    if(checked){
        _pSCStatusTcp->setTimeOut(ui->spinBox_timeOut->value());
    }else{
        _pSCStatusTcp->setTimeOut(0);
    }
}

void SCTcpToolWidget::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == _queryTimeID){
        if(ui->pushButton_send->isEnabled()){
            on_pushButton_send_clicked();
        }
    }
}
void SCTcpToolWidget::on_checkBox_queryTime_clicked(bool checked)
{
    if(checked){
        _queryTimeID = this->startTimer(ui->spinBox_queryTime->value());
    }else{
        killTimer(_queryTimeID);
    }
}
