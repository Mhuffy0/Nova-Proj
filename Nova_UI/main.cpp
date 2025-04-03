#include <QApplication>
#include "MainWindow.h"
#include "../Nova_Backend/include/Controller.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    try {
        // Create and initialize the backend controller.
        ChatBotController chatController;
        chatController.initialize("trained_model.txt");

        // Create the main window
        MainWindow m;
        m.show();

        return app.exec();
    } catch (const std::exception& e) {
        qCritical() << "Fatal error:" << e.what();
        return -1;
    }
}
