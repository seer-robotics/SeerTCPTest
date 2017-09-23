#ifndef ProtobufThread_H
#define ProtobufThread_H

#include "Core/BaseThread.h"
#include <QFileInfo>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>
#include "google/protobuf/util/json_util.h"
#include "google/protobuf/util/type_resolver.h"
#include "google/protobuf/util/type_resolver_util.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <QDebug>

class ProtobufThread : public BaseThread
{
    Q_OBJECT
public:
    explicit ProtobufThread(QObject *parent = 0);
    virtual ~ProtobufThread();
    void run();

    QString jsonData() const;
    void setJsonData(const QString &jsonData);

    bool initProtobufThread(const QString &protoFilePath,
                            const QString &packageName,
                            const QString &messageName,
                            const QString &binaryMessageName,
                            int threadType,
                            const QString &jsonStr,
                            bool prettyJSON = true);

    QByteArray msgByteArray() const;
    void setMsgByteArray(const QByteArray &msgByteArray);

    QString outBinaryFilePath() const;
    void setOutBinaryFilePath(const QString &outBinaryFilePath);

private:
    QFileInfo _protoFileInfo;
    QString _packageName;
    QString _messageName;
    QString _binaryMessageName;
    QString _jsonData;
    QString _outBinaryFilePath;
    bool _jsonIsFile = false;
    bool _prettyJSON = true;
    QByteArray _msgByteArray;
};

#endif // ProtobufThread_H
