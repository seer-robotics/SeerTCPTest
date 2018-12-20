#ifndef SCStatusTcp_H
#define SCStatusTcp_H


#include <QTcpSocket>
#include <QObject>
#include <QTime>

class SCStatusTcp : public QObject
{
    Q_OBJECT
public:
    explicit SCStatusTcp(QObject *parent = 0);

    ~SCStatusTcp();

    /**
     * @brief releaseTcpSocket - 释放tcpSocket
     */
    void releaseTcpSocket();

    /**
     * @brief lastError - 获取错误信息
     * @return - 错误信息的字符串描述
     */
    QString lastError() const;

    /**
     * @brief setLastError - 设置错误信息
     * @param lastError - 错误信息的字符串描述
     */
    void setLastError(const QString &lastError);

    /**
     * @brief writeTcpData - 发送TCP请求
     * @param sendCommand - 报文类型
     * @param sendData - 数据区数据
     * @param number - 序号
     * @return
     */
    bool writeTcpData(uint16_t sendCommand,
                      const QByteArray &sendData,
                      uint16_t &number);

    /**
     * @brief tcpSocket - 获取tcpSocket
     * @return
     */
    QTcpSocket *tcpSocket() const;

    /**
     * @brief time - 记录从发送TCP请求到接收到回复过程中的时间开销
     * @return
     */
    QTime time() const;

    /** 连接
     * @brief connectHost - 与主机建立连接
     * @param ip - IP地址
     * @param port - API对应的端口号
     * @return
     */
    int connectHost(const QString &ip, quint16 port);

    /**
     * @brief SCStatusTcp::getCurrentDateTime - 获取当前时间
     * @return
     */
    QString getCurrentDateTime() const;

    /**
     * @brief SCStatusTcp::hexToQString - 16进制转字符串，16进制全部显示大写
     * @param b
     * @return
     */
    QString hexToQString(const QByteArray &b);

    /**
     * @brief getTimeOut - 获取超时时间
     * @return
     */
    int getTimeOut() const;

    /**
     * @brief setTimeOut - 设置超时时间
     * @param timeOut - 超时时间，单位：ms
     */
    void setTimeOut(int timeOut);

public slots:
    /**
     * @brief receiveTcpReadyRead - 读取所有接受到的数据
     */
    void receiveTcpReadyRead();

signals:

    /**
     * @brief sigPrintInfo - 以信号的方式输出要打印的信息
     */
    void sigPrintInfo(QString);

    /**
     * @brief sigChangedText - 以信号的方式输出返回信息
     */
    void sigChangedText(bool ,int ,QByteArray ,QByteArray ,int ,int );


private:

    QTcpSocket *_tcpSocket = Q_NULLPTR;
    QString _lastError;
    QString _ip; //ip
    quint16 _port; //端口号
    uint16_t _number;//序号

    QByteArray _lastMessage;//存放所有读取的数据
    QTime _time;//用来监测时间

    int _oldSendCommand = -1;
    int _oldNumber = -1;
    int _timeOut = -1;


};

#endif // SCStatusTcp_H
