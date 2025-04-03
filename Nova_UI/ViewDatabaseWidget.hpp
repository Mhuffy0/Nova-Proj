#ifndef VIEWDATABASEWIDGET_HPP
#define VIEWDATABASEWIDGET_HPP

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QHeaderView>

class MainWindow;
class ViewDatabaseWidget : public QWidget {
    Q_OBJECT
public:
    explicit ViewDatabaseWidget(const QString &dbPath, QWidget *parent = nullptr);
    void refreshData();
    void showError(const QString& title, const QString& message);

signals:
    void backRequested(); ;

private:
    QString dbPath;
    QTableWidget *table;
    MainWindow* m_mainWindow;
    QPushButton *backButton;
    QVBoxLayout *mainLayout;
    QString databasePath;
};

#endif // VIEWDATABASEWIDGET_HPP
