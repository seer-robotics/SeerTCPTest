#include "ProtobufThread.h"
#include <ostream>
using namespace std;
ProtobufThread::ProtobufThread(QObject *parent) : BaseThread(parent)
{

}
ProtobufThread::~ProtobufThread()
{
    stopThread();
}
bool ProtobufThread::initProtobufThread(const QString &protoFilePath,
                                        const QString &packageName,
                                        const QString &messageName,
                                        const QString &binaryMessageName,
                                        int threadType,
                                        const QString &jsonStr,
                                        bool prettyJSON)
{
    _msgByteArray.clear();
    setLastError("");
    _jsonData.clear();
    _protoFileInfo.setFile(protoFilePath);

    if(!_protoFileInfo.isFile() || _protoFileInfo.suffix() !="proto"){
        setLastError(tr("未选择有效的.proto文件"));
    }else if(packageName.isEmpty()){
        setLastError(tr("packageName 不能为空"));
    }else if(messageName.isEmpty()){
        setLastError(tr("messageName 不能为空"));
    }
    if(1==threadType){
        QFileInfo jsonFIle(binaryMessageName);
        if(!jsonFIle.isFile()){
            _jsonIsFile = false;
            if(jsonStr.isEmpty()){
                setLastError(tr("json数据不能为空"));
            }
            setJsonData(jsonStr);
        }else{
            _jsonIsFile = true;
        }
    }
    if(!lastError().isEmpty()) return false;
    _prettyJSON = prettyJSON;
    _packageName = packageName;
    _messageName = messageName;
    _binaryMessageName = binaryMessageName;
    setThreadType(threadType);
    return true;
}

QByteArray ProtobufThread::msgByteArray() const
{
    return _msgByteArray;
}

void ProtobufThread::setMsgByteArray(const QByteArray &msgByteArray)
{
    _msgByteArray = msgByteArray;
}

QString ProtobufThread::outBinaryFilePath() const
{
    return _outBinaryFilePath;
}

void ProtobufThread::setOutBinaryFilePath(const QString &outBinaryFilePath)
{
    _outBinaryFilePath = outBinaryFilePath;
}

QString ProtobufThread::jsonData() const
{
    return _jsonData;
}

void ProtobufThread::setJsonData(const QString &jsonData)
{
    _jsonData = jsonData;
}
void ProtobufThread::run()
{
    this->setResult(0);
    //---------
    std::string rootPath = _protoFileInfo.absolutePath().toStdString();
    std::string protoFileName =_protoFileInfo.fileName().toStdString();
    std::string packageName = _packageName.toStdString();
    std::string messageName = _messageName.toStdString();
    std::string binaryMessageName = _binaryMessageName.toStdString();

    google::protobuf::compiler::DiskSourceTree sourceTree;
    sourceTree.MapPath("", rootPath);
    google::protobuf::compiler::Importer importer(&sourceTree, NULL);

    //---------
    switch (this->threadType()) {
    case 0://proto::message->json
    {
        const google::protobuf::FileDescriptor* filedescriptor = importer.Import(protoFileName);
        if (filedescriptor) {
            const google::protobuf::Descriptor *descriptor = importer.pool()->FindMessageTypeByName(packageName + "." + messageName);
            if (descriptor) {
                google::protobuf::DynamicMessageFactory factory;
                const google::protobuf::Message *message = factory.GetPrototype(descriptor);
                google::protobuf::Message *msg = message->New();

                google::protobuf::ifstream in;
                in.open(binaryMessageName, std::ios::in | std::ios::binary);
                if (in.is_open()) {
                    msg->ParseFromIstream(&in);
                    in.close();

                    google::protobuf::util::JsonOptions json_options;
                    json_options.add_whitespace = _prettyJSON;
                    json_options.always_print_primitive_fields = true;
                    std::string json_buffer;
                    google::protobuf::util::Status convertStatus = google::protobuf::util::MessageToJsonString(*msg, &json_buffer, json_options);
                    if (convertStatus.ok()) {
                        setJsonData(QString::fromStdString(json_buffer));
                    }
                    else {
                        setLastError("MessageToJsonString error: "+ QString::fromStdString(convertStatus.ToString()));
                    }
                }
                delete msg;
            }
            else {
                setLastError("FindMessageTypeByName error: " +_packageName+ _messageName);
            }
        }
        else {
            setLastError("import file descriptor error: "+_protoFileInfo.filePath());
        }
    }
        break;
    case 1://json->proto::message
    {
        QString jsonStr;
        if(_jsonIsFile){
            QFile file(_binaryMessageName);
            if(file.open(QIODevice::ReadOnly)){
                jsonStr = QString( file.readAll());
            }else{
                setLastError(QString("打开【%1】文件失败").arg(_binaryMessageName));
            }
        }else {
            jsonStr = jsonData();
        }
        const google::protobuf::FileDescriptor* filedescriptor = importer.Import(protoFileName);
        if (filedescriptor) {
            const google::protobuf::Descriptor *descriptor = importer.pool()->FindMessageTypeByName(packageName + "." + messageName);
            if (descriptor) {

                google::protobuf::DynamicMessageFactory factory;
                const google::protobuf::Message *message = factory.GetPrototype(descriptor);
                google::protobuf::Message *msg =  message->New();
                google::protobuf::util::Status convertStatus = google::protobuf::util::JsonStringToMessage(jsonStr.toStdString(),msg);
                if (convertStatus.ok()) {
                    qDebug()<<"convertStatus.ok()";
                    _outBinaryFilePath = "./outStream.message";
                    google::protobuf::ofstream ofs(_outBinaryFilePath.toStdString(),ios::out |ios::binary);
                    //std::ofstream ostram(_outBinaryFilePath.toStdString(),std::ios::binary);
                    if(!msg->SerializeToOstream(&ofs)){
                           setLastError("导出地图错误: "+QString::fromStdString(msg->DebugString()));
                    }
//                  setMsgByteArray(QByteArray::fromStdString(msg->SerializeToOstream()));
                }else {
                    setLastError("JsonStringToMessage error: "+ QString::fromStdString(convertStatus.ToString()));
                }
                delete msg;
            } else {
                setLastError("FindMessageTypeByName error: " +_packageName+ _messageName);
            }
        } else {
            setLastError("import file descriptor error: "+_protoFileInfo.filePath());
        }
    }
        break;
    default:
        break;
    }
}

