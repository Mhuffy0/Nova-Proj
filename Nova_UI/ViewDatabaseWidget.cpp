#include "ViewDatabaseWidget.hpp"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <sqlite3.h>
#include <QSqlRecord>

ViewDatabaseWidget::ViewDatabaseWidget(const QString& dbPath, QWidget *parent)
    : QWidget(parent), dbPath(dbPath)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);
    setStyleSheet("background-color: #f5f7fa;");

    // Back Button
    backButton = new QPushButton("← Back to Chat");
    backButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #6a7b9a;"
        "   color: white;"
        "   border-radius: 6px;"
        "   padding: 8px 16px;"
        "   font-size: 14px;"
        "   min-width: 120px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #7b8ca9;"
        "}");
    connect(backButton, &QPushButton::clicked, this, &ViewDatabaseWidget::backRequested);
    mainLayout->addWidget(backButton, 0, Qt::AlignLeft);

    // Table Widget
    table = new QTableWidget();
    table->setStyleSheet(
        "QTableWidget {"
        "   background-color: white;"
        "   border-radius: 8px;"
        "   gridline-color: #e0e0e0;"
        "   font-size: 13px;"
        "}"
        "QHeaderView::section {"
        "   background-color: #a7bed3;"
        "   color: white;"
        "   padding: 8px;"
        "   border: none;"
        "   font-weight: bold;"
        "}"
        "QTableWidget::item {"
        "   color : black;"
        "   padding: 6px;"
        "   border-bottom: 1px solid #f0f0f0;"
        "}");

    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    mainLayout->addWidget(table);

    refreshData();
}

void ViewDatabaseWidget::refreshData()
{
    // Clear existing data
    table->clear();
    table->setRowCount(0);
    table->setColumnCount(0);

    // 1. Check and initialize database connection
    QSqlDatabase db;
    if (QSqlDatabase::contains("viewerConnection")) {
        db = QSqlDatabase::database("viewerConnection");
        if (!db.isOpen()) {
            if (!db.open()) {
                showError("Connection Error", db.lastError().text());
                return;
            }
        }
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "viewerConnection");
        db.setDatabaseName(dbPath);
        if (!db.open()) {
            showError("Open Error", db.lastError().text());
            return;
        }
    }

    // 2. Execute query with error handling
    QSqlQuery query(db);
    if (!query.exec("SELECT * FROM responses")) {
        showError("Query Error", query.lastError().text());
        return;
    }

    // 3. Set up table columns
    QSqlRecord record = query.record();
    int columnCount = record.count();
    table->setColumnCount(columnCount);

    QStringList headers;
    for (int i = 0; i < columnCount; ++i) {
        headers << record.fieldName(i);
    }
    table->setHorizontalHeaderLabels(headers);

    // 4. Populate data
    while (query.next()) {
        int row = table->rowCount();
        table->insertRow(row);
        for (int col = 0; col < columnCount; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(query.value(col).toString());
            table->setItem(row, col, item);
        }
    }

    // 5. Adjust columns
    table->resizeColumnsToContents();
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
}


void ViewDatabaseWidget::showError(const QString& title, const QString& message)
{
    table->setRowCount(1);
    table->setColumnCount(1);
    table->setHorizontalHeaderLabels({title});

    QTableWidgetItem *item = new QTableWidgetItem(message);
    item->setForeground(Qt::red);
    table->setItem(0, 0, item);
}
