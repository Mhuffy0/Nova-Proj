#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsOpacityEffect>
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QScrollArea>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QTableWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QHeaderView>
#include <QProgressBar>

#include "ViewDatabaseWidget.hpp"
#include "../Nova_Backend/include/Controller.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setUserName(const QString &name) { userName = name; }
    QString getUserName() const { return userName; }

private slots:
    void showLoginPage();
    void showChatSelection();
    void showNormalChat();
    void showTeachingChat();
    void showDatabaseViewer();
    void sendMessage();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;


private:
    ChatBotController* m_chatController;
    QString userName;
    QLabel *userLabel;
    QStackedWidget *stackedWidget;
    void addUserMessage(const QString &message);
    void addBotMessage(const QString &message);
    QVBoxLayout *chatLayout = nullptr;

    // Login Page
    QWidget *loginPage;
    QLineEdit *nameInput;

    // Chat Selection Page
    QWidget *chatSelectionPage;
    QListWidget *chatList;

    // Normal Chat Page
    QWidget *normalChatPage;
    QScrollArea *scrollArea;
    QWidget *scrollContent;
    QLineEdit *normalMessageInput;
    QPushButton *normalSendButton;

    // Teaching Chat Page
    QWidget *teachingChatPage;
    QScrollArea *teachingScrollArea;
    QWidget *teachingScrollContent;
    QVBoxLayout *teachingChatLayout;
    QLineEdit *teachingMessageInput;
    QPushButton *teachingSendButton;

    enum TeachingState {
        WaitingForQuestion,
        WaitingForAnswer
    };

    TeachingState teachingState = WaitingForQuestion;
    QString currentTeachingInput;
    QString currentTeachingResponse;

    void addTeachingMessage(const QString &message, bool isUser);
    ViewDatabaseWidget *databaseViewer;

    // Database Viewer Page
    QVBoxLayout *databaseLayout;
    QTableWidget *tableWidget;

    // Setup functions
    void fadeToWidget(QWidget* newWidget);
    void setupLoginPage();
    void setupChatSelectionPage();
    void setupNormalChatPage();
    void setupTeachingChatPage();
    void setupDatabaseViewerPage();
    void applyStyling();


    QWidget* createMessageWidget(const QString &message, bool isUser, const QString &originalInput, bool isLatest);
    QWidget* buildChatModeWidget(const QString& titleText, const QString& iconPath, QLabel*& msgLabelRef);
    void animateMessageAppearance(QWidget* widget, bool isUser);
    QLabel *normalLastMsgLabel = nullptr;
    QLabel *teachingLastMsgLabel = nullptr;
    void scrollToBottom(QScrollArea* area);
    void addBotMessage(const QString &message, const QString &originalInput);

    QLabel* lastBotBubble = nullptr;
    QProgressBar* lastConfidenceBar = nullptr;
    QWidget* latestBotMessageWidget = nullptr;

};

#endif // MAINWINDOW_H
