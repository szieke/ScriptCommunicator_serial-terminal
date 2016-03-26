#include <QCoreApplication>
#include <QDir>
#include <QThread>

int main(int argc, char *argv[])
{
    QThread::msleep(10000);
    for(int i = 1; i < argc; i++)
    {
        QString currentArg = QString::fromLocal8Bit(argv[i]);
        QDir(currentArg).removeRecursively();
    }
    return 0;
}

