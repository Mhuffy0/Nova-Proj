#include "MainWindow.h"
#include "ViewDatabaseWidget.hpp"
#include "../Nova_Backend/include/Controller.hpp"
#include <QScrollBar>
#include <QTimer>
#include <QPropertyAnimation>
#include <QFile>
#include <QApplication>
#include <QProgressBar>
#include <QEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Set window properties
    setWindowTitle("Nova Chatbot");
    setFixedSize(400, 600);

    // Create stacked widget for pages
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    // Setup pages
    setupLoginPage();
    setupChatSelectionPage();
    setupNormalChatPage();
    setupTeachingChatPage();
    setupDatabaseViewerPage();

    m_chatController = new ChatBotController();
    m_chatController->initialize("trained_model.txt");

    // Apply styling if needed
    applyStyling();

    // Show initial page (login page)
    showLoginPage();
    //setup

}

void MainWindow::showLoginPage()
{
    stackedWidget->setCurrentWidget(loginPage);
}

MainWindow::~MainWindow() {}

void MainWindow::setupChatSelectionPage()
{
    chatSelectionPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(chatSelectionPage);

    // === UNCHANGED STYLING ===
    chatSelectionPage->setStyleSheet(
        "background-color: #a7bed3;"
        "font-family: 'Poppins', sans-serif;"
        );

    userLabel = new QLabel(userName);
    userLabel->setAlignment(Qt::AlignCenter);
    userLabel->setStyleSheet(
        "font-size: 22px;"
        "font-weight: bold;"
        "color: white;"
        "padding: 15px;"
        "background-color: rgba(255,255,255,0.2);"
        "border-radius: 10px;"
        "margin: 15px 50px 25px 50px;"
        );

    QLabel *title = new QLabel("Select Chat Mode");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "font-size: 20px;"
        "font-weight: bold;"
        "color: white;"
        "margin-bottom: 15px;"
        );

    // === IMPROVED CHAT LIST ===
    chatList = new QListWidget();
    chatList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    chatList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    chatList->setStyleSheet(
        "QListWidget {"
        "   background-color: #95aec7;"  // Darker than outer background
        "   border-radius: 20px;"
        "   padding: 10px;"
        "   margin: 0 15px;"
        "   border: none;"
        "}"
        "QListWidget::item {"
        "   height: 130px;"  // Increased height
        "   background: transparent;"
        "   margin: 8px 0;"  // Increased spacing
        "   border: none;"
        "}"
        "QListWidget::item:selected {"
        "   background: transparent;"
        "}"
        );

    // Increased height calculation
    const int itemCount = 3;
    const int itemHeight = 130;  // Increased from 110
    const int padding = 40;      // Increased from 30
    chatList->setFixedHeight(itemCount * itemHeight + padding);

    // === CHAT MODE ITEMS ===
    struct ChatModeConfig {
        QString title;
        QString iconPath;
        QString mode;
        QString initialMessage;
        QLabel** labelPtr;
    };

    const QVector<ChatModeConfig> modes = {
        {"Normal Chat", ":/bot_icon.png", "normalChat", "Ready to chat!", &normalLastMsgLabel},
        {"Teaching Mode", ":/bot_teach_icon.png", "teachingChat", "Let's learn together!", &teachingLastMsgLabel},
        {"View Database", ":/book.png", "viewDatabase", "Check conversation history", nullptr}
    };

    for (const auto& config : modes) {
        QListWidgetItem *item = new QListWidgetItem(chatList);
        item->setData(Qt::UserRole, config.mode);
        item->setSizeHint(QSize(0, itemHeight));

        // Container with proper rounded corners
        QWidget *container = new QWidget();
        container->setMaximumWidth(360);
        container->setStyleSheet(
            "background-color: #78A2CC;"
            "border-radius: 12px;"
            "margin-left: 5px;"
            "margin-right: 5px;"
            );
        QHBoxLayout *hLayout = new QHBoxLayout(container);
        hLayout->setContentsMargins(10, 10, 10, 10);
        hLayout->setSpacing(10);

        // Icon
        QLabel *icon = new QLabel();
        icon->setPixmap(QPixmap(config.iconPath).scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        icon->setFixedSize(80, 72);
        icon->setStyleSheet(
            "border-radius: 15px;"
            "border: 3px solid white;"
            );

        // Text Content
        QVBoxLayout *textLayout = new QVBoxLayout();
        textLayout->setSpacing(8);  // Increased spacing

        QLabel *nameLabel = new QLabel(config.title);
        nameLabel->setStyleSheet(
            "font-size: 20px;"  // Slightly larger
            "font-weight: bold;"
            "color: #2d5b7a;"
            "margin-bottom: 5px;"
            );

        QLabel *msgLabel = new QLabel(config.initialMessage);
        msgLabel->setStyleSheet(
            "font-size: 15px;"
            "color: #4a6b8a;"
            "max-width: 150px;"
            "white-space: nowrap;"
            "padding-right: 5px;"
            );

        msgLabel->setWordWrap(false);
        msgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        msgLabel->setMinimumWidth(180);

        textLayout->addWidget(nameLabel);
        textLayout->addWidget(msgLabel);
        textLayout->addStretch();

        hLayout->addWidget(icon);
        hLayout->addLayout(textLayout);
        hLayout->addStretch();

        chatList->setItemWidget(item, container);

        // Store reference to message label if needed
        if (config.labelPtr) {
            *config.labelPtr = msgLabel;
        }
    }

    // === UNCHANGED CONNECTION ===
    connect(chatList, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
        QString mode = item->data(Qt::UserRole).toString();
        if (mode == "normalChat") showNormalChat();
        else if (mode == "teachingChat") showTeachingChat();
        else if (mode == "viewDatabase") showDatabaseViewer();
    });

    // === UNCHANGED LAYOUT ===
    layout->addSpacing(10);

    // Inline name with pencil
    QWidget *nameContainer = new QWidget();
    QHBoxLayout *nameLayout = new QHBoxLayout(nameContainer);
    nameLayout->setContentsMargins(20, 0, 20, 0);
    nameLayout->setSpacing(8);

    userLabel = new QLabel(userName);
    userLabel->setStyleSheet(
        "font-size: 22px;"
        "font-weight: bold;"
        "color: white;"
        "padding: 12px 20px;"
        "background-color: rgba(255,255,255,0.2);"
        "border-radius: 10px;"
        );

    QPushButton *editNameBtn = new QPushButton("✎");
    editNameBtn->setFixedSize(24, 24);
    editNameBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: white;"
        "   border: none;"
        "   font-size: 16px;"
        "}"
        "QPushButton:hover { color: #f1ffc4; }"
        );
    connect(editNameBtn, &QPushButton::clicked, this, &MainWindow::showLoginPage);

    nameLayout->addStretch();
    nameLayout->addWidget(userLabel);
    nameLayout->addWidget(editNameBtn);
    nameLayout->addStretch();

    layout->addSpacing(10);
    layout->addWidget(nameContainer, 0, Qt::AlignCenter);

    nameLayout->addStretch();
    nameLayout->addWidget(userLabel);
    nameLayout->addWidget(editNameBtn);
    nameLayout->addStretch();

    layout->addSpacing(10);
    layout->addWidget(nameContainer, 0, Qt::AlignCenter);


    layout->addWidget(title);
    layout->addWidget(chatList);
    layout->setContentsMargins(20, 10, 20, 20);
    layout->setSpacing(0);

    stackedWidget->addWidget(chatSelectionPage);
}

void MainWindow::setupNormalChatPage()
{
    normalChatPage = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(normalChatPage);

    normalChatPage->setStyleSheet("background-color: #F2DFB9;");
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QWidget *headerWidget = new QWidget();
    headerWidget->setObjectName("normalHeader");
    headerWidget->setStyleSheet(
        "QWidget#normalHeader {"
        "   background-color: #cccccc;"    // Gray color
        "   border-top-left-radius: 0px;" // If you want rounding at the top
        "   border-top-right-radius: 0px;"
        "   padding: 20px;"
        "}"
        );
    headerLayout->setContentsMargins(10, 15, 10, 15);
    headerWidget->setLayout(headerLayout);

    //backbutton
    QPushButton *backButton = new QPushButton("←");
    backButton->setFixedSize(40, 40);
    backButton->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(255,255,255,0.3);"
        "   border-radius: 13px;"
        "   font-size: 18px;"
        "   color: #555555;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(255,255,255,0.5);"
        "}"
        );
    connect(backButton, &QPushButton::clicked, this, &MainWindow::showChatSelection);

    QLabel *profilePic = new QLabel();
    profilePic->setPixmap(
        QPixmap(":/bot_icon.png").scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation)
        );
    profilePic->setFixedSize(36, 36);
    profilePic->setStyleSheet(
        "border-radius: 8px;"
        "border: 2px solid #E0E0E0;"
        );

    QLabel *chatTitle = new QLabel("Normal Nova Mode");
    chatTitle->setStyleSheet(
        "font-size: 18px;"
        "font-weight: bold;"
        "color: #555555;"
        );

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "   border-radius: 15px;"
        "   background-color: #F2DFB9;"
        "}"
        "QScrollArea > QWidget > QWidget {"
        "   background-color: rgba(255,255,255,0.2);"
        "}"
        "QScrollBar:vertical {"
        "   border: none;"
        "   background: #F2DFB9;"
        "   width: 8px;"
        "   margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: #c0c0c0;"
        "   min-height: 20px;"
        "   border-radius: 4px;"
        "}"
    );


    headerLayout->addWidget(backButton, 0, Qt::AlignVCenter);
    headerLayout->addWidget(profilePic, 0, Qt::AlignVCenter);
    headerLayout->addSpacing(8);
    headerLayout->addWidget(chatTitle, 0, Qt::AlignVCenter);
    headerLayout->addStretch();
    scrollContent = new QWidget();
    chatLayout = new QVBoxLayout(scrollContent);
    chatLayout->setSpacing(10);
    chatLayout->setContentsMargins(15, 15, 15, 15);
    chatLayout->addStretch();
    scrollArea->setWidget(scrollContent);


    QWidget *inputWidget = new QWidget();
    inputWidget->setObjectName("normalInputWidget");
    inputWidget->setStyleSheet(
        "QWidget#normalInputWidget {"
        "   background-color: #dddddd;"
        "   color: black;"
        "   border-bottom-left-radius: 10px;" // Round corners if desired
        "   border-bottom-right-radius: 10px;"
        "   padding: 10px;"
        "}"
        );

    // Input area (unchanged)
    QHBoxLayout *inputLayout = new QHBoxLayout(inputWidget);
    inputLayout->setContentsMargins(10, 10, 10, 10);
    inputLayout->setSpacing(10);

    normalMessageInput = new QLineEdit();
    normalMessageInput->setPlaceholderText("Type your message...");
    normalMessageInput->setStyleSheet(
        "QLineEdit {"
        "   background-color: white;"
        "   color: black;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 20px;"
        "   padding: 10px 15px;"
        "   font-size: 14px;"
        "}"
        );

    normalSendButton = new QPushButton("Send");
    normalSendButton->setFixedWidth(80);
    normalSendButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #B4CDC9;"
        "   border: none;"
        "   border-radius: 20px;"
        "   padding: 10px;"
        "   color: white;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #9cbab5; }"
        );
    connect(normalSendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(normalMessageInput, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);

    inputLayout->addWidget(normalMessageInput);
    inputLayout->addWidget(normalSendButton);
    inputLayout->setSpacing(10);

    // Add widgets to main layout
    mainLayout->setContentsMargins(0, 0, 0, 0); // Let header & bottom widget define corners
    mainLayout->setSpacing(0);
    mainLayout->addWidget(headerWidget); // Gray header
    mainLayout->addWidget(scrollArea, 1); // Chat area (expanding)
    mainLayout->addWidget(inputWidget);   // Gray input area

    stackedWidget->addWidget(normalChatPage);
}

void MainWindow::setupLoginPage()
{
    loginPage = new QWidget();
    QVBoxLayout *loginLayout = new QVBoxLayout(loginPage);
    loginPage->setStyleSheet("background-color: #F2DFB9;");

    // Header
    QLabel *title = new QLabel("Welcome to Nova");
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #555555;");
    title->setAlignment(Qt::AlignCenter);

    // Name input
    nameInput = new QLineEdit();
    nameInput->setPlaceholderText("Enter your name");
    nameInput->setStyleSheet(
        "QLineEdit {"
        "background-color: white;"
        "   font-size: 16px;"
        "   color: black;"
        "   padding: 12px;"
        "   border: 1px solid #ccc;"
        "   border-radius: 8px;"
        "   margin: 20px 40px;"
        "}"
        );

    // Next button
    QPushButton *nextButton = new QPushButton("Continue");
    nextButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #B4CDC9;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 12px;"
        "   font-size: 16px;"
        "   min-width: 120px;"
        "}"
        "QPushButton:hover { background-color: #9cbab5; }"
        );

    // Proper username storage connection
    connect(nextButton, &QPushButton::clicked, [this]() {
        userName = nameInput->text().trimmed();
        if (userName.isEmpty()) {
            userName = "Guest"; // Default name
        }
        qDebug() << "Username stored:" << userName;
        userLabel->setText(userName);
        showChatSelection();
    });

    // Layout
    loginLayout->addStretch();
    loginLayout->addWidget(title);
    loginLayout->addWidget(nameInput);
    loginLayout->addWidget(nextButton, 0, Qt::AlignHCenter);
    loginLayout->addStretch();
    loginLayout->setSpacing(10);

    stackedWidget->addWidget(loginPage);
}

void MainWindow::sendMessage()
{
    // Determine which input widget to use based on active mode
    bool isNormalChat = (stackedWidget->currentWidget() == normalChatPage);
    QLineEdit *currentInput = isNormalChat ? normalMessageInput : teachingMessageInput;

    QString message = currentInput->text().trimmed();
    if (message.isEmpty()) return;

    if (isNormalChat) {
        // --- NORMAL CHAT MODE (UNCHANGED) ---
        addUserMessage(message);
        normalMessageInput->clear();
        if (normalLastMsgLabel) {
            normalLastMsgLabel->setText("You: " + message);
        }

        QTimer *replyTimer = new QTimer(this);
        replyTimer->setSingleShot(true);
        connect(replyTimer, &QTimer::timeout, [this, message, replyTimer]() {
            if (!m_chatController) {
                qWarning() << "[ERROR] Chat controller is null!";
                return;
            }
            std::string inputStd = message.toStdString();
            std::string responseStd = m_chatController->getChatbotResponse(inputStd);
            QString responseText = QString::fromStdString(responseStd);
            responseText.replace("<HUMAN>", userName, Qt::CaseInsensitive);
            responseText.replace("Human", userName, Qt::CaseInsensitive);
            responseText.replace("human", userName, Qt::CaseInsensitive);

            addBotMessage(responseText, message);

            QTimer *scrollTimer = new QTimer(this);
            scrollTimer->setSingleShot(true);
            connect(scrollTimer, &QTimer::timeout, [this, scrollTimer]() {
                scrollArea->verticalScrollBar()->setValue(
                    scrollArea->verticalScrollBar()->maximum()
                    );
                scrollTimer->deleteLater();
            });
            scrollTimer->start(50);

            if (normalLastMsgLabel) {
                normalLastMsgLabel->setText("Nova: " + responseText);
            }
            replyTimer->deleteLater();
        });
        replyTimer->start(500);
    } else {
    // --- TEACHING MODE (SIMPLIFIED) ---
    addTeachingMessage(message, true); // User message
    teachingMessageInput->clear();

    if (teachingState == WaitingForQuestion) {
        // Phase 1: User asks a question
        currentTeachingInput = message;
        QTimer::singleShot(500, [this]() {
            addTeachingMessage("What should I say in response to \"" +
                                   currentTeachingInput + "\"?", false);
            teachingState = WaitingForAnswer;
        });
    }
    else if (teachingState == WaitingForAnswer) {
        // Phase 2: User provides just the response
        currentTeachingResponse = message;
        QTimer::singleShot(500, [this]() {
            // Format: "original question=response"
            std::string teachingInput = currentTeachingInput.toStdString() + "=" +
                                        currentTeachingResponse.toStdString();

            m_chatController->teachMode(teachingInput);

            addTeachingMessage("Got it! I'll respond with \"" + currentTeachingResponse +
                                   "\" when asked \"" + currentTeachingInput + "\"", false);

            teachingState = WaitingForQuestion;

            // Smooth scroll
            QTimer::singleShot(100, [this]() {
                teachingScrollArea->widget()->updateGeometry();
                teachingScrollArea->widget()->adjustSize();
                teachingScrollArea->verticalScrollBar()->setValue(
                    teachingScrollArea->verticalScrollBar()->maximum()
                    );
            });
        });
        }
    }
}

void MainWindow::fadeToWidget(QWidget* newWidget) {
    QWidget* current = stackedWidget->currentWidget();
    if (current == newWidget) return;

    QGraphicsOpacityEffect *fadeOut = new QGraphicsOpacityEffect(current);
    current->setGraphicsEffect(fadeOut);
    QPropertyAnimation *fadeOutAnim = new QPropertyAnimation(fadeOut, "opacity");
    fadeOutAnim->setDuration(300);
    fadeOutAnim->setStartValue(1.0);
    fadeOutAnim->setEndValue(0.0);

    connect(fadeOutAnim, &QPropertyAnimation::finished, [this, newWidget, fadeOut]() {
        stackedWidget->setCurrentWidget(newWidget);
        stackedWidget->setCurrentWidget(newWidget);
        newWidget->updateGeometry();
        newWidget->adjustSize();
        newWidget->repaint();
        qApp->processEvents();

            QTimer::singleShot(0, [newWidget]() {
            QGraphicsOpacityEffect *fadeIn = new QGraphicsOpacityEffect(newWidget);
            newWidget->setGraphicsEffect(fadeIn);
            QPropertyAnimation *fadeInAnim = new QPropertyAnimation(fadeIn, "opacity");
            fadeInAnim->setDuration(300);
            fadeInAnim->setStartValue(0.0);
            fadeInAnim->setEndValue(1.0);
            fadeInAnim->start(QAbstractAnimation::DeleteWhenStopped);
        });
    });
    fadeOutAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::applyStyling()
{
    // Main window base styling
    this->setStyleSheet(
        // Window background
        "QMainWindow {"
        "   background-color: #f5f5f5;"
        "   font-family: 'Poppins', sans-serif;"
        "}"

        // Scroll bars
        "QScrollBar:vertical {"
        "   border: none;"
        "   background: #f0f0f0;"
        "   width: 10px;"
        "   margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: #c0c0c0;"
        "   min-height: 20px;"
        "   border-radius: 5px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
        );

    // Chat selection page specific styling
    chatSelectionPage->setStyleSheet(
        "background-color: #a7bed3;"
        );

    chatList->setStyleSheet(
        "QListWidget {"
        "   background-color: #c6e2e9;"
        "   border: none;"
        "   border-radius: 15px;"
        "   padding: 10px;"
        "}"
        "QListWidget::item {"
        "   height: 100px;"
        "   padding: 15px;"
        "   border-bottom: 1px solid rgba(255,255,255,0.3);"
        "   color: white;"
        "   font-size: 16px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #f1ffc4;"
        "   color: #555555;"
        "   border-radius: 10px;"
        "}"
        );

    // Common chat page styling (applies to both normal and teaching pages)
    QString chatPageStyle =
        "QWidget#chatPage {"
        "   background-color: #F2DFB9;"
        "}"

        "QTextEdit#chatDisplay {"
        "   background-color: transparent;"
        "   border: none;"
        "   padding: 10px;"
        "   font-size: 14px;"
        "}"

        "QLineEdit#messageInput {"
        "   background-color: white;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 20px;"
        "   padding: 10px 15px;"
        "   font-size: 14px;"
        "}"

        "QPushButton#sendButton {"
        "   background-color: #B4CDC9;"
        "   border: none;"
        "   border-radius: 20px;"
        "   padding: 10px;"
        "   color: white;"
        "   font-weight: bold;"
        "}"
        "QPushButton#sendButton:hover {"
        "   background-color: #9cbab5;"
        "}"

        "QPushButton#backButton {"
        "   background-color: rgba(255,255,255,0.3);"
        "   border-radius: 20px;"
        "   color: #555555;"
        "   font-size: 18px;"
        "}"
        "QPushButton#backButton:hover {"
        "   background-color: rgba(255,255,255,0.5);"
        "}";

    // Apply to both chat pages
    normalChatPage->setObjectName("chatPage");
    normalChatPage->setStyleSheet(chatPageStyle);
    teachingChatPage->setObjectName("chatPage");
    teachingChatPage->setStyleSheet(chatPageStyle);

    // Additional individual page styling if needed
    // normalChatDisplay->setObjectName("chatDisplay");
    normalMessageInput->setObjectName("messageInput");
    normalSendButton->setObjectName("sendButton");

    teachingChatPage->setObjectName("chatDisplay");
    teachingMessageInput->setObjectName("messageInput");
    teachingSendButton->setObjectName("sendButton");
}

void MainWindow::addUserMessage(const QString &message)
{
    if (!normalChatPage || !chatLayout) return;

    QWidget *msg = createMessageWidget(message, true, QString(), false);

    chatLayout->insertWidget(chatLayout->count() - 1, msg);

    // Immediately update layout so widget is visible
    scrollContent->updateGeometry();
    scrollContent->adjustSize();

    // Animate after layout is finalized
    QTimer::singleShot(0, [this, msg]() {
        animateMessageAppearance(msg, true);
        scrollToBottom(scrollArea);
    });
}

void MainWindow::addBotMessage(const QString &message, const QString &originalInput)
{
    if (!normalChatPage || !chatLayout) return;

    if (latestBotMessageWidget) {
        QWidget *reactionUI = latestBotMessageWidget->findChild<QWidget*>("reactionWidget");
        if (reactionUI) reactionUI->hide();
    }

    QWidget *msg = createMessageWidget(message, false, originalInput, true);  // now passing true!
    chatLayout->insertWidget(chatLayout->count() - 1, msg);

    latestBotMessageWidget = msg;

    scrollContent->updateGeometry();
    scrollContent->adjustSize();

    QTimer::singleShot(0, [this, msg]() {
        animateMessageAppearance(msg, false);
        scrollToBottom(scrollArea);
    });
}


QWidget* MainWindow::createMessageWidget(const QString &message, bool isUser, const QString &originalInput, bool isLatest)
{
    const int SCREEN_WIDTH = 350;
    const int BUBBLE_MAX_WIDTH = SCREEN_WIDTH * 0.7;

    QWidget *widget = new QWidget();
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(8);

    if (!isUser) {
        QLabel *profilePic = new QLabel();
        profilePic->setPixmap(QPixmap(":/bot_icon.png").scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        profilePic->setFixedSize(36, 36);
        profilePic->setStyleSheet("border-radius: 18px; border: 1px solid #E0E0E0;");
        layout->addWidget(profilePic, 0, Qt::AlignTop);
    }

    QWidget *bubbleContainer = new QWidget();
    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubbleContainer);
    bubbleLayout->setContentsMargins(0, 0, 0, 0);
    bubbleLayout->setSpacing(2);

    // === Name label for bot ===
    if (!isUser) {
        QLabel *nameLabel = new QLabel("Nova");
        nameLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #4a4a4a;");
        bubbleLayout->addWidget(nameLabel, 0, Qt::AlignLeft);
    }

    // === Message bubble ===
    QLabel *bubble = new QLabel(message);
    bubble->setWordWrap(true);
    bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);
    bubble->setMaximumWidth(BUBBLE_MAX_WIDTH);
    bubble->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    bubble->setMargin(8);
    QString bubbleStyle = QString(
                              "QLabel {"
                              "   padding: 10px 15px;"
                              "   border-radius: 18px;"
                              "   font-size: 14px;"
                              "   max-width: %1px;"
                              "   margin: 2px;"
                              "   %2"
                              "}").arg(BUBBLE_MAX_WIDTH).arg(
                                  isUser ? "background-color: #E7F1DC; color: #555555; border-radius: 18px 18px 0 18px;"
                                         : "background-color: #B4CDC9; color: #555555; border-radius: 18px 18px 18px 0;"
                                  );
    bubble->setStyleSheet(bubbleStyle);
    bubbleLayout->addWidget(bubble, 0, isUser ? Qt::AlignRight : Qt::AlignLeft);

    // === Reaction logic only for latest bot msg ===
    if (!isUser && scrollArea && scrollArea->widget() && stackedWidget->currentWidget() == normalChatPage) {

        bool isLatest = true;
        QWidget *reactionWidget = new QWidget();
        QVBoxLayout *reactionLayout = new QVBoxLayout(reactionWidget);
        reactionLayout->setContentsMargins(0, 0, 0, 0);
        reactionLayout->setSpacing(2);

        QHBoxLayout *iconRow = new QHBoxLayout();
        QPushButton *thumbsUp = new QPushButton("👍");
        QPushButton *thumbsDown = new QPushButton("👎");
        thumbsUp->installEventFilter(this);
        thumbsDown->installEventFilter(this);

        thumbsUp->setFixedSize(30, 30);
        thumbsDown->setFixedSize(30, 30);
        thumbsUp->setStyleSheet("border: none; background: transparent; font-size: 18px;");
        thumbsDown->setStyleSheet("border: none; background: transparent; font-size: 18px;");
        thumbsUp->setFocusPolicy(Qt::NoFocus);
        thumbsDown->setFocusPolicy(Qt::NoFocus);

        iconRow->addWidget(thumbsUp);
        iconRow->addWidget(thumbsDown);
        iconRow->addStretch();

        QProgressBar *confidenceBar = new QProgressBar();
        confidenceBar->setMinimum(0);
        confidenceBar->setMaximum(100);
        confidenceBar->setValue(50); // default placeholder
        confidenceBar->setTextVisible(false);
        confidenceBar->setFixedHeight(6);
        confidenceBar->setStyleSheet(
            "QProgressBar { border: none; background-color: #eee; border-radius: 3px; }"
            "QProgressBar::chunk { background-color: #B4CDC9; border-radius: 3px; }"
            );
        confidenceBar->setObjectName("confBar");
        confidenceBar->hide();

        reactionLayout->addLayout(iconRow);
        reactionLayout->addWidget(confidenceBar);

        if (isLatest) {
            reactionWidget->setObjectName("reactionWidget");
            bubbleLayout->addWidget(reactionWidget);
            reactionWidget->setVisible(isLatest);

            auto lockButtons = [thumbsUp, thumbsDown]() {
                thumbsUp->setEnabled(false);
                thumbsDown->setEnabled(false);
            };

            connect(thumbsUp, &QPushButton::clicked, [this, originalInput, message, confidenceBar, lockButtons]() {
                if (m_chatController)
                    m_chatController->provideFeedback(originalInput.toStdString(), message.toStdString(), true);
                confidenceBar->show();
                lockButtons();
            });

            connect(thumbsDown, &QPushButton::clicked, [this, originalInput, message, confidenceBar, lockButtons]() {
                if (m_chatController)
                    m_chatController->provideFeedback(originalInput.toStdString(), message.toStdString(), false);
                confidenceBar->show();
                lockButtons();
            });
        }


        // === Wire feedback ===
        if (!originalInput.isEmpty()) {
            connect(thumbsUp, &QPushButton::clicked, [this, originalInput, message]() {
                if (m_chatController) {
                    m_chatController->provideFeedback(originalInput.toStdString(), message.toStdString(), true);
                }
            });
            connect(thumbsDown, &QPushButton::clicked, [this, originalInput, message]() {
                if (m_chatController) {
                    m_chatController->provideFeedback(originalInput.toStdString(), message.toStdString(), false);
                }
            });
        }
    }

    if (isUser) {
        layout->addStretch();
        layout->addWidget(bubbleContainer, 0, Qt::AlignRight);
    } else {
        layout->addWidget(bubbleContainer, 0, Qt::AlignLeft);
        layout->addStretch();
    }

    return widget;
}


void MainWindow::setupTeachingChatPage()
{
    teachingChatPage = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(teachingChatPage);
    teachingChatPage->setStyleSheet("background-color: #6B8EAD;");

    QWidget *headerWidget = new QWidget(teachingChatPage);
    headerWidget->setObjectName("teachingHeader");
    headerWidget->setStyleSheet(
        "QWidget#teachingHeader {"
        "   background-color: #6A7B9A;"
        "   padding: 20px;"
        "}"
        );

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(10, 5, 10, 5);

    // Back button
    QPushButton *backButton = new QPushButton("←", headerWidget);
    backButton->setFixedSize(40, 40);
    backButton->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(255,255,255,0.3);"
        "   border-radius: 20px;"
        "   font-size: 18px;"
        "   color: #E0E0E0;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(255,255,255,0.5);"
        "}"
        );
    connect(backButton, &QPushButton::clicked, this, &MainWindow::showChatSelection);

    // Profile picture
    QLabel *profilePic = new QLabel(headerWidget);
    profilePic->setPixmap(QPixmap(":/bot_icon.png").scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    profilePic->setFixedSize(36, 36);
    profilePic->setStyleSheet(
        "border-radius: 18px;"
        "border: 1px solid #E0E0E0;"
        );

    // Chat title
    QLabel *chatTitle = new QLabel("Teaching Nova Mode", headerWidget);
    chatTitle->setStyleSheet(
        "font-size: 18px;"
        "font-weight: bold;"
        "color: #E0E0E0;"
        );

    headerLayout->addWidget(backButton);
    headerLayout->addWidget(profilePic);
    headerLayout->addSpacing(8);
    headerLayout->addWidget(chatTitle);
    headerLayout->addStretch();

    teachingScrollContent = new QWidget();
    teachingScrollContent->setStyleSheet("background: transparent;");

    // ===== SCROLL AREA =====
    teachingScrollArea = new QScrollArea(teachingChatPage);
    teachingScrollArea->setWidgetResizable(true);
    teachingScrollArea->setStyleSheet(
        "QScrollArea {"
        "   background-color: #6B8EAD;"
        "   border: none;"
        "}"
        "QScrollBar:vertical {"
        "   border: none;"
        "   background: #6B8EAD;"
        "   width: 8px;"
        "   margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: #5A7B9A;"
        "   min-height: 20px;"
        "   border-radius: 4px;"
        "}"
        );

    teachingChatLayout = new QVBoxLayout(teachingScrollContent);
    teachingChatLayout->addStretch();
    teachingChatLayout->setSpacing(10);
    teachingChatLayout->setContentsMargins(15, 15, 15, 15);
    teachingScrollArea->setWidget(teachingScrollContent);

    // ===================== INPUT AREA =====================
    QWidget *inputWidget = new QWidget(teachingChatPage);
    inputWidget->setObjectName("teachingInputWidget");
    inputWidget->setStyleSheet(
        "QWidget#teachingInputWidget {"
        "   background-color: #5A7B9A;"
        "   border-bottom-left-radius: 10px;"
        "   border-bottom-right-radius: 10px;"
        "   padding: 10px;"
        "}"
        );

    QHBoxLayout *inputLayout = new QHBoxLayout(inputWidget);
    inputLayout->setContentsMargins(10, 10, 10, 10);

    teachingMessageInput = new QLineEdit(inputWidget);
    teachingMessageInput->setPlaceholderText("Type your message...");
    teachingMessageInput->setStyleSheet(
        "QLineEdit {"
        "   background-color: #E0E0E0;"
        "   border: 1px solid #4A6B8A;"
        "   border-radius: 20px;"
        "   padding: 10px 15px;"
        "   font-size: 14px;"
        "   color: #333333;"
        "}"
        );
    teachingSendButton = new QPushButton("Send", inputWidget);
    teachingSendButton->setFixedWidth(80);
    teachingSendButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #7FA3C2;"
        "   border: none;"
        "   border-radius: 20px;"
        "   padding: 10px;"
        "   color: white;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #8FB3D2;"
        "}"
        );
    connect(teachingSendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(teachingMessageInput, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);

    inputLayout->addWidget(teachingMessageInput);
    inputLayout->addWidget(teachingSendButton);

    // ===================== MAIN LAYOUT =====================
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(teachingScrollArea, 1);
    mainLayout->addWidget(inputWidget);

    stackedWidget->addWidget(teachingChatPage);

    teachingState = WaitingForQuestion; // Initialize state
    addTeachingMessage("Teach me how to respond! Ask me something first.", false);
}

QWidget* MainWindow::buildChatModeWidget(const QString& titleText, const QString& iconPath, QLabel*& msgLabelRef) {
    QWidget *widget = new QWidget();
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    hLayout->setContentsMargins(25, 25, 25, 25);
    hLayout->setSpacing(25);

    QLabel *icon = new QLabel();
    icon->setPixmap(QPixmap(iconPath).scaled(64, 64, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    icon->setFixedSize(64, 64);

    QVBoxLayout *textLayout = new QVBoxLayout();
    QLabel *name = new QLabel(titleText);
    name->setStyleSheet("font-size: 20px; font-weight: bold; color: #2d5b7a; margin-bottom: 5px;");
    msgLabelRef = new QLabel("Nova: Ready");
    msgLabelRef->setStyleSheet("font-size: 16px; color: #4a6b8a;");
    textLayout->addWidget(name);
    textLayout->addWidget(msgLabelRef);
    textLayout->addStretch();

    hLayout->addWidget(icon);
    hLayout->addLayout(textLayout);
    hLayout->addStretch();

    return widget;
}
void MainWindow::animateMessageAppearance(QWidget* widget, bool isUser)
{
    widget->setGraphicsEffect(nullptr); // Reset previous effects

    // Delay the animation so layout finishes first
    QTimer::singleShot(0, [widget, isUser]() {
        QPoint endPos = widget->mapToParent(QPoint(0, 0));
        QPoint startPos = endPos;
        startPos.setX(startPos.x() + (isUser ? 30 : -30));
        widget->move(startPos);

        // Slide animation
        QPropertyAnimation* slide = new QPropertyAnimation(widget, "pos");
        slide->setStartValue(startPos);
        slide->setEndValue(endPos);
        slide->setDuration(200);
        slide->setEasingCurve(QEasingCurve::OutCubic);

        // Fade animation
        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(effect);
        QPropertyAnimation* fade = new QPropertyAnimation(effect, "opacity");
        fade->setDuration(200);
        fade->setStartValue(0.0);
        fade->setEndValue(1.0);

        slide->start(QAbstractAnimation::DeleteWhenStopped);
        fade->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void MainWindow::addTeachingMessage(const QString &message, bool isUser)
{
    if (!teachingChatLayout) return;

    QWidget *msg = createMessageWidget(message, isUser, QString(), false);
    teachingChatLayout->insertWidget(teachingChatLayout->count() - 1, msg);

    teachingScrollContent->updateGeometry();
    teachingScrollContent->adjustSize();
    teachingScrollArea->ensureVisible(0, teachingScrollContent->height());

    QTimer::singleShot(0, [this, msg, isUser]() {
        animateMessageAppearance(msg, isUser);
        scrollToBottom(teachingScrollArea);
    });
}

void MainWindow::setupDatabaseViewerPage() {
    databaseViewer = new ViewDatabaseWidget("D:/Nova_Project/Nova_Backend/chatbot.db");
    connect(databaseViewer, &ViewDatabaseWidget::backRequested, this, &MainWindow::showChatSelection);
    stackedWidget->addWidget(databaseViewer);
}

void MainWindow::scrollToBottom(QScrollArea* area)
{
    QTimer::singleShot(30, [area]() {
        if (!area || !area->widget()) return;

        area->widget()->updateGeometry();
        area->widget()->adjustSize();

        QTimer::singleShot(0, [area]() {
            if (area->verticalScrollBar()) {
                area->verticalScrollBar()->setValue(
                    area->verticalScrollBar()->maximum()
                    );
            }
        });
    });
}


void MainWindow::showDatabaseViewer() {
    databaseViewer->refreshData();
    stackedWidget->setCurrentWidget(databaseViewer);
}

void MainWindow::showChatSelection()
{
    stackedWidget->setCurrentWidget(chatSelectionPage);
}

void MainWindow::showNormalChat()
{
    stackedWidget->setCurrentWidget(normalChatPage);
    normalMessageInput->setFocus();
}

void MainWindow::showTeachingChat()
{
    stackedWidget->setCurrentWidget(teachingChatPage);
    teachingMessageInput->setFocus();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // === Show/hide confidence bar ===
    if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverLeave) {
        if (QWidget* widget = qobject_cast<QWidget*>(watched)) {
            QProgressBar* bar = widget->findChild<QProgressBar*>("confBar");
            if (bar) {
                bar->setVisible(event->type() == QEvent::HoverEnter);
            }
        }
    }

    // === Emoji hover scale effect ===
    if (event->type() == QEvent::Enter || event->type() == QEvent::Leave) {
        if (QPushButton* btn = qobject_cast<QPushButton*>(watched)) {
            QPropertyAnimation* anim = new QPropertyAnimation(btn, "geometry");
            anim->setDuration(150);
            anim->setEasingCurve(QEasingCurve::OutBack);

            QRect start = btn->geometry();
            QRect end = (event->type() == QEvent::Enter)
                            ? QRect(start.x() - 2, start.y() - 2, start.width() + 4, start.height() + 4)
                            : QRect(start.x() + 2, start.y() + 2, start.width() - 4, start.height() - 4);

            anim->setStartValue(start);
            anim->setEndValue(end);
            anim->start(QAbstractAnimation::DeleteWhenStopped);

            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}



