#ifndef BaseThread_H
#define BaseThread_H

#include <QThread>
#include <QtDebug>


class BaseThread : public QThread
{
    Q_OBJECT
public:
    explicit BaseThread(QObject *parent = 0);
    virtual ~BaseThread();

    /**
      * @brief lastError - 获取错误值
      * @return
      */
     QString lastError();

     /**
      * @brief setLastError - 设置错误值
      * @param lastError - 错误值
      */
     void setLastError(const QString &lastError);

     /**
      * @brief threadType - 获取线程执行的任务的类型
      * @return
      */
     int threadType() const;

     /**
      * @brief setThreadType - 定义线程需要执行的任务类型
      * @param threadType - 任务类型
      */
     void setThreadType(int threadType);

     //返回结果可以自己定义返回值
     /**
      * @brief result - 获取线程的返回值
      * @return
      */
     int result() const;

     /**
      * @brief setResult - 设置线程的返回值
      * @param result - 返回值参数
      */
     void setResult(int result);

     /**
      * @brief setNeedStop - 是否需要中途跳出线程
      * @param isNeedStop - true:需要中途跳出线程； false：不需要中途跳出线程
      */
     void setNeedStop(bool isNeedStop);

     /**
      * @brief isNeedStop - 获取 _isNeedStop的值
      * @return  - true：中途跳出线程；false：中途不跳出线程
      */
     bool isNeedStop() const;

     /**
      * @brief stopThread - 停止线程
      */
     void stopThread();
signals:
     /**
      * @brief sigThreadOut - 线程输出
      * @param threadType - 线程类型
      */
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
     QString _lastError;

};

#endif // BaseThread_H
