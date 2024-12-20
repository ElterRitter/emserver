#include <QApplication>

#include "clientwgt.h"
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char** argv)
{
    cout << "Hello world!" << endl;
    QApplication a(argc, argv);
    
    ClientWgt c;
    c.show();

    return a.exec();
}
