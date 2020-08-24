#include "SCTcpToolWidget.h"
#include "ui_SCTcpToolWidget.h"
#include <QDateTime>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHostInfo>
#include <QUrl>

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
}

SCTcpToolWidget::~SCTcpToolWidget()
{
    //类不用手动释放，qt会自动释放SCTcpToolWidget的子类.
    delete ui;
}

bool SCTcpToolWidget::copyQrcFile(const QString & from,const QString & to)
{
    QFile fileFrom(from);
    QFile fileTo(to);
    if(!fileFrom.open(QIODevice::ReadOnly)){
        SCWarning<<"read failed: from:"<<from;
        return false;
    }

    if(!fileTo.open(QIODevice::WriteOnly)){
        SCWarning<<"write failed: to:"<<to;
        return false;
    }
    fileTo.write(fileFrom.readAll());
    return true;
}

void SCTcpToolWidget::initDb()
{
    _pSqliteClass = new SqliteClass(this);
    copyQrcFile(":/resource/Roboshop.db","./Roboshop.db");
    if(!_pSqliteClass->createConnection("Roboshop.db")){
        SCWarning<<_pSqliteClass->errorString();
        return;
    }
    if( !_pSqliteClass->getProtocol()){
        SCWarning<<_pSqliteClass->errorString();
        return;
    }

    connect(ui->comboBox_port, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotPortChanged(QString)));
    connect(ui->comboBox_sendCommand, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSendCommandChanged(int)));
    connect(ui->comboBox_commandName, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCommandNameChanged(int)));

    //---------------------------------------
    // 把端口添加到界面上
    QList<int> ilistPorts;
    QMap<int, int> portMap = _pSqliteClass->getProtocol()->ReqValuePortMap;

    for (int j = 0; j < portMap.keys().count(); j ++) {
        auto key = portMap.keys().at(j);
        // 过滤掉端口号为10000和19208的相关指令
        if(_pSqliteClass->getProtocol()->getPort(key) == 10000 &&
                _pSqliteClass->getProtocol()->getPort(key == 19208)) continue;

        int iPortTmp = portMap.value(key);
        if (j == 0) {// 将第一个端口号添加到空列表中
            ilistPorts.append(iPortTmp);
        } else { // 对比从第二个开始的端口号，并将不重复的端口号添加到
            bool blRepeated = false;
            foreach (int iPort, ilistPorts) {
                if (iPortTmp == iPort) {
                    blRepeated = true;
                    break;
                }
            }

            // 把没有重复的端口号添加到列表中
            if (!blRepeated) {
                // 忽略端口10000和端口19208
                if (iPortTmp == 10000 || iPortTmp == 19208) continue;
                ilistPorts.append(iPortTmp);
            }
        }
    }

    foreach (int iPort, ilistPorts) {
        QString strPortDescription;
        switch (iPort) {
        case 19204:
            strPortDescription = QString(QStringLiteral("机器人状态API"));
            break;
        case 19205:
            strPortDescription = QString(QStringLiteral("机器人控制API"));
            break;
        case 19206:
            strPortDescription = QString(QStringLiteral("机器人导航API"));
            break;
        case 19207:
            strPortDescription = QString(QStringLiteral("机器人配置API"));
            break;
        case 19210:
            strPortDescription = QString(QStringLiteral("其他API"));
            break;
        default:
            break;
        }

        // 记录当前的端口号及其对应的中文描述
        _reqPortMap.insert(iPort, strPortDescription);

        // 将端口号的中文描述显示在界面上
        ui->comboBox_port->addItem(strPortDescription);

    }

    // 获取数据库中所有指令的中文描述
    _reqDescriptionMap = _pSqliteClass->getProtocol()->ReqValueReqDescriptionMap;

    // 显示被选中的端口对应的指令
    slotPortChanged(ui->comboBox_port->currentText());

}

void SCTcpToolWidget::myConnect()
{
    qDebug() << "try connecting";
    QString strInfo;
    switch (_pSCStatusTcp->connectHost(ui->lineEdit_ip->text(),ui->lineEdit_port->text().toInt())) {
    case 1:
        strInfo = QString(QStringLiteral("连接失败"));
        qDebug() << strInfo;
        ui->textEdit_info->append(strInfo);
        break;
    default:
        break;
    }
}

void SCTcpToolWidget::sendCommand()
{
    if (_pSCStatusTcp->tcpSocket() && _pSCStatusTcp->tcpSocket()->state()==QAbstractSocket::ConnectedState)
    {
        _reqFinished = false; // 重置标志

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
        if(!_pSCStatusTcp->writeTcpData(sendCommand,sendData,number)) {
            slotPrintInfo(QString(QStringLiteral("<font color=\"red\">"
                                                 "%1--------- 发送错误---------- <br />" // 用html的语言的“<br />”实现换行，“\n”在此处不能实现换行
                                                 "发送的报文类型:%2  <br />"
                                                 "错误: %3 "
                                                 "</font>"))
                          .arg(_pSCStatusTcp->getCurrentDateTime())
                          .arg(sendCommand)
                          .arg(_pSCStatusTcp->lastError()));

            ui->textEdit_info->append("<font color = \"black\"> &nbsp; </font>");
        }
    }
    else
    {
        qDebug() << "not connected, send failed";
        ui->textEdit_info->append(QString("UnconnectedState"));
    }
}


void SCTcpToolWidget::slotPortChanged(QString portDescription)
{
    // 当前端口的端口号
    int iPort = _reqPortMap.key(portDescription);

    // 获取当前的端口号并显示在界面上
    ui->lineEdit_port->setText(QString::number(iPort));

    // 清空之前显示的API
    ui->comboBox_sendCommand->clear();
    // 显示与此端口相关的所有指令
    for (int k = 0; k < _reqDescriptionMap.keys().count(); k ++) {
        auto key = _reqDescriptionMap.keys().at(k);
        if (_pSqliteClass->getProtocol()->getPort(key) == iPort) {
            ui->comboBox_sendCommand->addItem(_reqDescriptionMap.value(key));
        }
    }

    // 清空之前显示的API名称
    ui->comboBox_commandName->clear();
    // 显示与此端口相关的所有API名称
    for (int l = 0; l < _pSqliteClass->getProtocol()->ReqValueReqMap.count(); l ++) {
        auto key1 = _pSqliteClass->getProtocol()->ReqValueReqMap.keys().at(l);
        if (_pSqliteClass->getProtocol()->getPort(key1) == iPort) {
            ui->comboBox_commandName->addItem(
                        _pSqliteClass->getProtocol()->ReqValueReqMap.value(key1));
        }
    }
}

void SCTcpToolWidget::slotSendCommandChanged(int index)
{
    qDebug() << "command ......................." << index;
    // 过滤索引值为负数时导致读取错误信息的情况
    if (index < 0)
        return;

    // 当前指令的编号
    int iAPIID = _reqDescriptionMap.key(ui->comboBox_sendCommand->itemText(index));
    qDebug() << iAPIID;

    if (iAPIID < 10000)
    {
        // 显示当前指令的编号
        ui->lineEdit_sendCommand->setText(QString::number(iAPIID));

        // 显示当前指令的名称
        ui->comboBox_commandName->blockSignals(true);
        ui->comboBox_commandName->setCurrentText(_pSqliteClass->getProtocol()->ReqValueReqMap.value(iAPIID));
        ui->comboBox_commandName->blockSignals(false);
    }
}

void SCTcpToolWidget::slotCommandNameChanged(int index)
{
    qDebug() << "name ......................." << index;
    // 过滤索引值为负数时导致读取错误信息的情况
    if (index < 0)
        return;

    // 当前指令的编号

    int iAPIID = _pSqliteClass->getProtocol()->ReqValueReqMap.key(
                ui->comboBox_commandName->itemText(index));
    qDebug() << iAPIID;

    if (iAPIID < 10000)
    {
        // 显示当前指令的编号
        ui->lineEdit_sendCommand->setText(QString::number(iAPIID));

        // 显示当前指令的名称
        ui->comboBox_sendCommand->blockSignals(true);
        ui->comboBox_sendCommand->setCurrentText(_reqDescriptionMap.value(iAPIID));
        ui->comboBox_sendCommand->blockSignals(false);
    }
}

void SCTcpToolWidget::stateChanged(QAbstractSocket::SocketState state)
{
    QString info;
    switch (state) {
    case QAbstractSocket::UnconnectedState:
        info = "QAbstractSocket::UnconnectedState";
//        ui->checkBox_queryTime->setEnabled(true);
//        ui->pushButton_connectAndSend->setEnabled(true);
        //        qDebug() << QStringLiteral("开始连接");
        ui->textEdit_info->append(QString(QStringLiteral("连接已断开！！！")));

        break;
    case QAbstractSocket::HostLookupState:
        info = "QAbstractSocket::HostLookupState";
//        ui->pushButton_connectAndSend->setEnabled(false);
        break;

    case QAbstractSocket::ConnectingState:
        info = "QAbstractSocket::ConnectingState";
        //        qDebug() << QStringLiteral("正在连接...");
//        ui->pushButton_connectAndSend->setEnabled(false);
        break;
    case QAbstractSocket::ConnectedState:
        info = "QAbstractSocket::ConnectedState \n";
        //        qDebug() << QStringLiteral("断开连接");
//        ui->pushButton_connectAndSend->setEnabled(false);
        break;
    case QAbstractSocket::BoundState:
        info = "QAbstractSocket::BoundState";
//        ui->pushButton_connectAndSend->setEnabled(false);
        break;
    case QAbstractSocket::ListeningState:
        info = "QAbstractSocket::ListeningState";
//        ui->pushButton_connectAndSend->setEnabled(false);
        break;
    case QAbstractSocket::ClosingState:
        info = "QAbstractSocket::ClosingState";
//        ui->pushButton_connectAndSend->setEnabled(false);
        //        qDebug() << QStringLiteral("开始连接");
        break;
    }

    ui->textEdit_info->append(QString("%1 IP:%2:%3 %4")
                              .arg(_pSCStatusTcp->getCurrentDateTime())
                              .arg(ui->lineEdit_ip->text())
                              .arg(ui->lineEdit_port->text())
                              .arg(info));

    // 连接成功之后，发送请求
    if (_pSCStatusTcp->tcpSocket()->state() == QAbstractSocket::ConnectedState) {
        sendCommand();
    }

    if (_reqFinished && !ui->checkBox_queryTime->isChecked()) { // 断开当前的连接
        _reqFinished = false;
        ui->textEdit_info->append(QString(QStringLiteral("指令发送完成，断开连接")));
        _pSCStatusTcp->releaseTcpSocket();
//        ui->checkBox_queryTime->setEnabled(true);

    } else {
        // do nothing
    }
}

void SCTcpToolWidget::receiveTcpError(QAbstractSocket::SocketError error)
{
    ui->textEdit_info->append(QString("<font color = \"red\">"
                                      "%1  connect error[%2]: IP:%3:%4"
                                      "</font>")
                              .arg(_pSCStatusTcp->getCurrentDateTime())
                              .arg(error)
                              .arg(ui->lineEdit_ip->text())
                              .arg(ui->lineEdit_port->text())); // 用红色字体显示错误信息
    ui->textEdit_info->append(QString("<font color = \"black\"> &nbsp; </font>")); // 用黑色字体显示正确信息
//    ui->pushButton_connectAndSend->setEnabled(true);
    qDebug() << QStringLiteral("开始连接");

}

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
                                             "%1--------- 返回错误---------- <br />"
                                             "报文类型:%2  <br />"
                                             "错误: %3"
                                             "</font>"))
                      .arg(_pSCStatusTcp->getCurrentDateTime())
                      .arg(revCommand)
                      .arg(_pSCStatusTcp->lastError()));

        ui->textEdit_info->append("<font color = \"black\"> &nbsp; </font>");

        ui->textEdit_revData->setText(QString(revData));
        ui->label_revText->setText(QString(QStringLiteral("响应的错误: %1 \t\n"))
                                   .arg(_pSCStatusTcp->lastError()));

    }

    // 发送指令成功并接收到返回值后，可以断开连接
    _reqFinished = true;

}

void SCTcpToolWidget::slotPrintInfo(QString info)
{
    ui->textEdit_info->append(info);
}

void SCTcpToolWidget::on_pushButton_clearInfo_clicked()
{
    ui->textEdit_info->clear();
}

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
    // 先建立连接，然后再开始定时发送指令
    if (_pSCStatusTcp->tcpSocket() && _pSCStatusTcp->tcpSocket()->state()==QAbstractSocket::ConnectedState) {
        qDebug() << "connected";
        if(event->timerId() == _queryTimeID){
            sendCommand();
        }
    } else {
        if (_dateTime == QTime(0, 0, 0, 0)) {
            _dateTime = QTime::currentTime().addMSecs(ui->spinBox_timeOut->value());
            qDebug() << _dateTime;
        }

        if (_dateTime <= QTime::currentTime()) {
            myKillTimer(_queryTimeID);
            resetDateTime();
            ui->checkBox_queryTime->setChecked(false);
            qDebug() << "unconnected";
        }
    }
}

void SCTcpToolWidget::myKillTimer(int id)
{
    killTimer(id);
    id = -1;
}

void SCTcpToolWidget::on_checkBox_queryTime_clicked(bool checked)
{
    qDebug() << "query time" << checked;
    if(checked){
        myConnect(); // 先建立连接
        _queryTimeID = this->startTimer(ui->spinBox_queryTime->value());
    }else {
        if (_queryTimeID > 0) {
            myKillTimer(_queryTimeID);
            resetDateTime();
        }
        if (_reqFinished) { // 在发送完指令，并接受到响应之后再断开连接
            _reqFinished = false; // 提前重置标志，防止界面上输出多有的信息
            _pSCStatusTcp->releaseTcpSocket();
        }
    }

}

void SCTcpToolWidget::on_pushButton_connectAndSend_clicked(bool checked)
{
    Q_UNUSED(checked);
    if (ui->lineEdit_ip->text().isEmpty() ||
            ui->lineEdit_sendCommand->text().isEmpty() ||
            ui->lineEdit_port->text().isEmpty()) {
        QMessageBox::warning(
                    this,
                    "warnning",
                    QStringLiteral("请输入完整的信息！！！"),
                    QMessageBox::Ok);
    } else {
        // 建立连接
//        ui->checkBox_queryTime->setEnabled(false);
        myConnect();
    }
}

void SCTcpToolWidget::resetDateTime()
{
    _dateTime = QTime(0, 0, 0, 0);
}

void SCTcpToolWidget::on_checkBox_queryTime_stateChanged(int arg1)
{
    bool enable;
    qDebug() << "current state: " << arg1;
    if (arg1 == 0) {// 未勾选状态
        // 激活comboBox和pushButton
        enable = true;
        ui->comboBox_port->setEnabled(enable);
        ui->comboBox_sendCommand->setEnabled(enable);
//        ui->pushButton_connectAndSend->setEnabled(enable);

        // 放弃连接
        _pSCStatusTcp->releaseTcpSocket();

    } else if (arg1 == 2) { // 勾选状态
        // 抑制comboBox和pushButton
        bool enable = false;
        ui->comboBox_port->setEnabled(enable);
        ui->comboBox_sendCommand->setEnabled(enable);
//        ui->pushButton_connectAndSend->setEnabled(enable);
    }
}

