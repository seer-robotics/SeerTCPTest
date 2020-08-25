/** 报文格式示意图
 *  |                                                 报文头部（16byte）                                                    |                  数据区域  (变长）      |
 *  |  报文同步头  |  协议版本  |  序号  |  数据区长度  |  报文类型(编号)  |  保留区域  |                       数据区域                 |
 *  |       1byte       |    1byte      | 2byte|       4byte       |           2byte          |     6byte     |  取决于头部中的数据区域长度  |
 *  |        0x5a       |      0x01     | 0x00 |        0x00        |          0x00            |     0x00      |         JSON序列化数据内容        |
 *
 *  详细内容可查阅手册：https://docs.seer-group.com/robokit_netprotocol/663329
 *
  */

#ifndef _SEER_PROTOCOL_V1_H_
#define _SEER_PROTOCOL_V1_H_

#include <stdint.h>
#include <QtEndian>

class SeerHeader{
public:
    SeerHeader(){
        m_header = 0x5A; // 1byte    报文同步头，用于确定报文头部的开始
        m_version = 0x01; // 1byte    协议的主版本号，对于v1.x.x版本的协议均填0X01
        m_number = 0; // 2byte    请求及响应的序号（0~65535），请求包与响应包的这个字段是相同的，API使用者自行填入序号。机器人对每个请求的响应都是使用这个序号
        m_length = 0; // 4byte    数据区长度，即JSON序列化数据的长度
        m_type = 0; // 2byte    表示报文的类型，即API的编号。
        memset(m_reserved,0, 6); // 6byte    保留区域。不必关注保留区的内容，其内容可能不为0且可能会发生变化
    }

    uint8_t m_header;
    uint8_t m_version;
    uint16_t m_number;
    uint32_t m_length;
    uint16_t m_type;
    uint8_t m_reserved[6];
};


class SeerData{
public:
    SeerData(){}
    ~SeerData(){}
    int size(){
        return sizeof(SeerHeader) + m_header.m_length;
    }

    int setData(uint16_t type, uint8_t* data = NULL, int size = 0, uint16_t number = 0x00){
        m_header.m_header = 0x5A;
        m_header.m_version = 0x01;
        qToBigEndian(type, (uint8_t*)&(m_header.m_type));
        qToBigEndian(number, (uint8_t*)&(m_header.m_number));
        memset(m_header.m_reserved, 0, 6);
        if (data != NULL) {
            memcpy(m_data, data, size);
        }
        qToBigEndian(size, (uint8_t*)&(m_header.m_length));
        return 16 + size;
    }
private:
    SeerHeader m_header;
    uint8_t m_data[1];
};


#endif
