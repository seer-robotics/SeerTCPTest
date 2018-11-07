#include "SqliteClass.h"
#include <QFileInfo>

SqliteClass::SqliteClass(QObject *parent) : QObject(parent)
{
}
SqliteClass::~SqliteClass()
{
    if(_pProtocolClass){
        delete _pProtocolClass;
    }
    //之所以这么写，是为了消除警告
    //QSqlDatabasePrivate::removeDatabase: connection 'XXX' is still in use, all queries will cease to work

    QSqlDatabase *db = & QSqlDatabase::database(_connectionName);
    if(db->isValid() ){
        if(db->isOpen())
            db->close();
        QSqlDatabase::removeDatabase(_connectionName);
    }
}

QString SqliteClass::errorString() const
{
    return _errorString;
}

void SqliteClass::setErrorString(const QString &errorString)
{
    _errorString = errorString;
}

//建立一个数据库连接
bool SqliteClass::createConnection(const QString & filePath)
{
    QFileInfo fileInfo(filePath);
    if( ! fileInfo.exists() ){
        setErrorString(tr("Not found file: %1").arg(filePath));
        return false;
    }
    _connectionName = fileInfo.completeBaseName();
    //fileInfo.completeBaseName() 数据库的文件名，来作为连接名(如果多个db名字一致，则修改此处)
    QSqlDatabase db = QSqlDatabase::database(_connectionName);

    if(!db.isValid()){ //如果db不可用，则添加
        db = QSqlDatabase::addDatabase("QSQLITE", _connectionName);
        db.setDatabaseName(filePath);
    }
    if( !db.open()){ //判断db是否已经打开
        setErrorString(tr("Sqlite not open").arg(filePath));
        return false;
    }
    return true;
}

QSqlDatabase & SqliteClass::dbConnection()
{
    QSqlDatabase db = QSqlDatabase::database(_connectionName);

    if( !db.open()){ //判断db是否已经打开
        setErrorString(tr("Sqlite not open").arg(db.connectionName()));
    }
    return db;
}

/** 获取指令数据类
 * @brief ProtocolClass::getProtocol
 * @return
 */
ProtocolClass * SqliteClass::getProtocol()
{
    if(!_pProtocolClass){
        return queryProtocol();
    }
    return _pProtocolClass;
}

/** 查询指令库
 * @brief SqliteClass::queryProtocol
 * @return
 */
ProtocolClass * SqliteClass::queryProtocol()
{
    QSqlQuery query(dbConnection());
    QString mysql;

    mysql= QString("select b.reqValue,a.port,b.req,b.reqDescription "
                   "from RobotProtocol a,SCProtocol b where a.type = b.type");
    if(!query.exec(mysql)){
        setErrorString(QString("%1:%2").arg(mysql).arg(query.lastError().text()));
        return Q_NULLPTR;
    }
    if(_pProtocolClass){
        delete _pProtocolClass;
    }
    _pProtocolClass = new ProtocolClass();
    while(query.next()){
        _pProtocolClass->addData(
                    query.value("reqValue").toInt(),
                    query.value("port").toInt(),
                    query.value("req").toString(),
                    query.value("reqDescription").toString());
    }
    query.clear();

    return _pProtocolClass;
}
