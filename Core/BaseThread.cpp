#include "BaseThread.h"

QString BaseThread::_lastError = "";
BaseThread::BaseThread(QObject *parent) : QThread(parent)
{

}
BaseThread::~BaseThread()
{
//    qDebug()<<"~BaseThread()";

}
void BaseThread::stopThread()
{
    setNeedStop(true);
    this->quit();
    this->wait();
}
bool BaseThread::isNeedStop() const
{
    return _isNeedStop;
}

void BaseThread::setNeedStop(bool isNeedStop)
{
    _isNeedStop = isNeedStop;
}

QString BaseThread::lastError()
{
    return _lastError;
}

void BaseThread::setLastError(const QString &lastError)
{
    _lastError = lastError;
}

int BaseThread::threadType() const
{
    return _threadType;
}

void BaseThread::setThreadType(int threadType)
{
    _threadType = threadType;
}

int BaseThread::result() const
{
    return _result;
}

void BaseThread::setResult(int result)
{
    _result = result;
}
