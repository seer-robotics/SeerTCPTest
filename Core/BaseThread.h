#ifndef BaseThread_H
#define BaseThread_H

#include <QThread>
#include <QtDebug>
#include <QMutex>

class BaseThread : public QThread
{
    Q_OBJECT
public:
    explicit BaseThread(QObject *parent = 0);
    virtual ~BaseThread();
    //错误返回
    static QString lastError();
    static void setLastError(const QString &lastError);
    //定义线程需要执行任务类型
    int threadType() const;
    void setThreadType(int threadType);
    //返回结果可以自己定义返回值
    int result() const;
    void setResult(int result);
    //是否需要中途跳出线程
    void setNeedStop(bool isNeedStop);
    bool isNeedStop() const;
    //停止线程
    void stopThread();
signals:
    //线程输出
    void sigThreadOut(int threadType);
public slots:
private:
    //线程类型
    int _threadType = 0;
    //线程返回0为正常返回
    int _result = 0;
    //是否停止线程，该变量用于跳出while(!_isNeedStop)循环
    bool _isNeedStop = false;
    //最后错误信息
    static QString _lastError;
    //线程锁，注意return函数中记得解锁
    QMutex _mutex;
};

#endif // BaseThread_H
