#ifndef CHATBUBBLE_H
#define CHATBUBBLE_H

#include <QLabel>

class ChatBubble : public QLabel
{
    Q_OBJECT

public:
    enum BubbleType {
        User,
        Bot    };

    explicit ChatBubble(const QString &text, BubbleType type, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    BubbleType m_type;
    QColor m_userColor = QColor(231, 241, 220);  // #E7F1DC
    QColor m_botColor = QColor(180, 205, 201);   // #B4CDC9
    QColor m_textColor = QColor(85, 85, 85);     // #555555
};

#endif // CHATBUBBLE_H
