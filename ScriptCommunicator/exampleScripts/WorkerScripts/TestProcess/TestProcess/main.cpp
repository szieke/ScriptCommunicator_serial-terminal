#include <QTextStream>
#include <QThread>
#include <QFile>

int main(int argc, char *argv[])
{
    if ( argc > 1)
    {

        QFile in;
        in.open(stdin, QIODevice::ReadOnly);
        QString line = in.readLine();
        while(line.indexOf("\n") == -1)
        {//Wait for a complete line.

            line += in.readLine();
        }



        for(int i=0; i<argc; i++)
        {
            QTextStream(stdout) << argv[i] << ";";
        }

        QTextStream(stdout) << line;
    }


    QTextStream(stderr) << "this is a simulated error" << endl; // simulate an error

    return 10;
}
