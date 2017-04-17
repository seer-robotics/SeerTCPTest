#include "SCTcpToolWidget.h"
#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //设置exe所在文件夹为当前位置
    QDir::setCurrent(a.applicationDirPath());
    SCTcpToolWidget w;

    w.show();

    return a.exec();
}
