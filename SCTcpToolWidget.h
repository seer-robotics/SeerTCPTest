#ifndef SCTCPTOOLWIDGET_H
#define SCTCPTOOLWIDGET_H

#include <QWidget>
#include "SCStatusTcp.h"
#include "Core/SqliteClass.h"

#define TEST 1 // 【此部分代码为测试代码】  0：非测试， 1：测试中

#if TEST

#include <QListView>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QTime>

#endif

namespace Ui {
class SCTcpToolWidget;
}

class SCTcpToolWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SCTcpToolWidget(QWidget *parent = 0);
    ~SCTcpToolWidget();

    /**
     * @brief timerEvent - 重写 timerEvent(QTimerEvent *event)
     * @param event
     */
    void timerEvent(QTimerEvent *event);

    /**
     * @brief myKillTimer - 停止定时器，并重置 _queryTimeID 为-1
     * @param id - 定时器的id
     */
    void myKillTimer(int id);

    /**
     * @brief initDb - 获取数据库数据，并显示部分信息到界面上
     */
    void initDb();

    /**
     * @brief myConnection - 建立/断开连接
     */
    void myConnect();

    /**
     * @brief sendCommand - 发送指令
     */
    void sendCommand();

    /**
     * @brief resetDateTime - 重置定时发送时，建立连接的超时时间
     */
    void resetDateTime();


public slots:

    /**
     * @brief SCTcpToolWidget::stateChanged - tcp槽，实时监测tcp状态
     * @param state - socket的当前状态
     */
    void stateChanged(QAbstractSocket::SocketState state);

    /**
     * @brief SCTcpToolWidget::receiveTcpError - tcp槽 返回tcp错误
     * @param error - socket的错误描述
     */
    void receiveTcpError(QAbstractSocket::SocketError error);

    /**
     * @brief SCTcpToolWidget::slotPrintInfo - 打印信息
     * @param info - SCStatusTcp发送的信息
     */
    void slotPrintInfo(QString info);

    /**
     * @brief SCTcpToolWidget::slotChangedText - 发送后，响应
     * @param isOk - 是否正常返回
     * @param revCommand - 返回的数据类型
     * @param revData - 返回的数据
     * @param revHex - 返回hex
     * @param number - 序号
     * @param msTime - 发送->返回时间 单位：ms
     */
    void slotChangedText(bool isOk, int revCommand, QByteArray revData, QByteArray revHex, int number, int msTime);

    /**
     * @brief SCTcpToolWidget::slotAutomaticallyScroll - 自动滚动
     */
    void slotAutomaticallyScroll();

    /**
     * @brief slotPortChanged - 将被选中的端口的编号显示在界面上，同步显示对应的指令
     * @param index - 被选中的端口号的中文描述
     */
    void slotPortChanged(QString portDescription);

    /**
     * @brief slotSendCommandChanged - 将选中的指令的编号显示在界面上
     * @param index - 被选中的指令的编号
     */
    void slotSendCommandChanged(int index);

private slots:
    /**
     * @brief on_pushButton_connectAndSend_clicked - 创建连接并发送指令
     * @param checked
     */
    void on_pushButton_connectAndSend_clicked(bool checked);

    /**
     * @brief SCTcpToolWidget::on_pushButton_clearInfo_clicked - 清空textEdit_info数据
     */
    void on_pushButton_clearInfo_clicked();

    /**
     * @brief on_pushButton_zipFile_clicked - 发送ZIP文件
     */
    void on_pushButton_zipFile_clicked();

    /**
     * @brief on_checkBox_timeOut_clicked - 设置超时
     * @param checked - true：启用超时；false：不启用
     */
    void on_checkBox_timeOut_clicked(bool checked);

    /**
     * @brief on_checkBox_queryTime_clicked - 设置定时发送指令
     * @param checked - true:启用；false：不启用
     */
    void on_checkBox_queryTime_clicked(bool checked);



private:
    Ui::SCTcpToolWidget *ui;
    SCStatusTcp *_pSCStatusTcp = Q_NULLPTR;
    SqliteClass *_pSqliteClass = Q_NULLPTR;
    int _queryTimeID = -1;

    bool _reqFinished = false; // false：还未发送请求或正在发送请求       true：发送请求完成并成功接收响应

    QTime _dateTime = QTime(0, 0, 0, 0);

    //QMap<reqValue, reqDescription>
    QMap<int, QString> _reqDescriptionMap;
    // QMap<reqPort, reqPortDescription>
    QMap<int, QString> _reqPortMap;

};

#endif // SCTCPTOOLWIDGET_H
