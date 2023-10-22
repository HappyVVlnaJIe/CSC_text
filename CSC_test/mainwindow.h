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

    void on_registrationButton_clicked();

    void on_authorizationButton_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *serverTimer;
    QTimer *clientTimer;
    QTimer *chatTimer;
    std::unordered_map<std::string, QListWidgetItem*> chats;
    std::map<QListWidgetItem, std::vector<std::string>> chats_contents;
    std::map<QListWidgetItem, std::string> server_chats;
    std::map<QListWidgetItem, DefaultClient*> clients_chats;
    QListWidgetItem* client_chat;
    std::string client_appeal = "Вы: ";

    const char* auth_server_address ="127.0.0.1";
    const char* auth_server_port ="12345";
    std::string separator = "@";
    std::string tech_message = "*technical message* connected";
    std::string positive_response = "OK";
    std::string negative_response = "NOT";

    std::string registration_command = "registration";
    std::string authorization_command = "authorization";
};
#endif // MAINWINDOW_H
