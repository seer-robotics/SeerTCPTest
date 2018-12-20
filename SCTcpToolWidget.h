#ifndef SCTCPTOOLWIDGET_H
#define SCTCPTOOLWIDGET_H

#include <QWidget>
#include "SCStatusTcp.h"
#include "ProtoBufTool/ProtobufWidget.h"
#include "Core/SqliteClass.h"

#define TEST 1 // 【此部分代码为测试代码】  0：非测试， 1：测试中

#if TEST

#include <QListView>
#include <QListWidget>
#include <QLabel>

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

    void timerEvent(QTimerEvent *event);

    /**
     * @brief initDb - 获取数据库数据，并显示部分信息到界面上
     */
    void initDb();

#if TEST
    /**
     * @brief initTestWidget - 初始化测试界面
     */
    void initTestWidget();
#endif

public slots:

    /**
     * @brief stateChanged
     * @param state
     */
    void stateChanged(QAbstractSocket::SocketState state);

    void receiveTcpError(QAbstractSocket::SocketError error);
    void slotPrintInfo(QString info);

    void slotChangedText(bool isOk, int revCommand, QByteArray revData, QByteArray revHex, int number, int msTime);
    void slotAutomaticallyScroll();
    void slotAllReqCurrentChanged(int index);

#if TEST
    /**
     * @brief slotUpdateAPINamesByAPIType - 根据API类型显示API名称
     * @param APIType - API 类型
     */
    void slotUpdateAPINamesByAPIType(QString APIType);

#endif

private slots:
    void on_pushButton_connect_clicked(bool checked);
    void on_pushButton_send_clicked();

    void on_pushButton_clearInfo_clicked();

    void on_pushButton_zipFile_clicked();

    void on_checkBox_timeOut_clicked(bool checked);


    void on_checkBox_queryTime_clicked(bool checked);

private:
    Ui::SCTcpToolWidget *ui;
    SCStatusTcp *_pSCStatusTcp = Q_NULLPTR;
    ProtobufWidget *_pProtobufWidget = Q_NULLPTR;
    SqliteClass *_pSqliteClass = Q_NULLPTR;
    int _queryTimeID = -1;

#if TEST

    QWidget *_testWidget = Q_NULLPTR;
    QLabel *_APITypeLabel = Q_NULLPTR,
                *_APINameLabel = Q_NULLPTR,
                *_destRobotIPLabel = Q_NULLPTR,
                *_portLabel = Q_NULLPTR,
                *_APIIDLabel = Q_NULLPTR;

    QListWidget *_testAPIType = Q_NULLPTR, *_testAPIName = Q_NULLPTR;
    //QMap<reqValue, reqDescription>
    QMap<int, QString> _reqDescriptionMap;

#endif
};

#endif // SCTCPTOOLWIDGET_H
