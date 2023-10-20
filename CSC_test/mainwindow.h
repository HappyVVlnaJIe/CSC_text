#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <map>
#include <vector>
#include "server.h"
#include "client.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    DefaultServer* server = nullptr;
    DefaultClient* client = nullptr;
private slots:
    void on_userServerRunButton_clicked();

    void on_connectToServerButton_clicked();

    void runServer();

    void runClient();

    void updateChat();

    void on_chatList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_saveToFileButton_clicked();

    void on_chatPushButton_clicked();

    void on_findTextButton_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *serverTimer;
    QTimer *clientTimer;
    QTimer *chatTimer;
    std::unordered_map<std::string, QListWidgetItem> chats;
    std::map<QListWidgetItem, std::vector<std::string>> chats_contents;
    std::map<QListWidgetItem, std::string> server_chats;
    std::map<QListWidgetItem, DefaultClient*> clients_chats;
    std::string client_appeal = "Вы: ";
};
#endif // MAINWINDOW_H
