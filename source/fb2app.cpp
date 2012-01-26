#include <QApplication>
#include <QErrorMessage>

#include "fb2main.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(fb2edit);

    QApplication app(argc, argv);
    app.setApplicationName("fb2edit");
    app.setOrganizationName("LinTest");

    QErrorMessage::qtHandler();

    MainWindow * mainWin = new MainWindow;
    mainWin->show();

    return app.exec();
}
