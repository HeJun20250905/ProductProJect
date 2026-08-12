#include "HuiRuSpaceBitH.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("windowsvista");
    HuiRuSpaceBitH window;
    window.show();
    return app.exec();
}
