#ifndef SQLITECLASS_H
#define SQLITECLASS_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTime>
#include <QSqlError>
#include <QtDebug>
#include <QSqlDriver>
#include <QSqlRecord>

#define SCDebug  qDebug() <<"[Debug]["<<__FILE__<<"]$"<<0<<"$"<<"["<<__LINE__<<"]["<<__FUNCTION__ <<"]:"
#define SCInfo  qInfo() <<"[Info]["<<__FILE__<<"]$"<<0<<"$"<<"["<<__LINE__<<"]["<<__FUNCTION__<<"]:"
#define SCWarning  qWarning()<<"[Warning][" <<__FILE__<<"]$"<<0<<"$"<<"["<<__LINE__<<"]["<<__FUNCTION__<<"]:"
#define SCritical  qCritical() <<"[Error]["<<__FILE__<<"]$"<<0<<"$"<<"["<<__LINE__<<"]["<<__FUNCTION__<<"]:"

class ProtocolClass{
public:
    ProtocolClass(){}
    ~ProtocolClass(){

        ReqValueReqMap.clear();
        ReqValueReqDescriptionMap.clear();
        ReqValuePortMap.clear();
    }
    void addData(
            int reqValue,
            int port,
            const QString & req,
            const QString & reqDescription)
    {
        ReqValueReqMap.insert(reqValue,req);
        ReqValueReqDescriptionMap.insert(reqValue,reqDescription);
        ReqValuePortMap.insert(reqValue,port);
    }
    //根据数据库中 reqValue 获取数据库中 reqDescription 字段
    QString getReqDescription(int reqValue) const {
        return ReqValueReqDescriptionMap.value(reqValue);
    }
    //根据数据库中 reqValue 获取数据库中 req 字段
    QString getReq(int reqValue) const {
        return ReqValueReqMap.value(reqValue);
    }
    //根据数据库中 reqValue 获取数据库中 port 字段
    int getPort(int reqValue) const {
        return ReqValuePortMap.value(reqValue);
    }
    //QMap<reqValue, req>
    QMap<int,QString> ReqValueReqMap;
    //QMap<reqValue, reqDescription>
    QMap<int,QString> ReqValueReqDescriptionMap;
    //QMap<reqValue, port>
    QMap<int,int>ReqValuePortMap;

};

class SqliteClass : public QObject
{
    Q_OBJECT
public:
    explicit SqliteClass(QObject *parent = nullptr);
    ~SqliteClass();
    QString errorString() const;
    void setErrorString(const QString &errorString);

    QSqlDatabase & dbConnection();
    bool createConnection(const QString &filePath);

    ProtocolClass *queryProtocol();
    ProtocolClass *getProtocol();
signals:

public slots:
private:
    QString _connectionName; //连接 db 名(用于多个db库访问时)
    QString _errorString;
    ProtocolClass * _pProtocolClass  = Q_NULLPTR;
};

#endif // SQLITECLASS_H
