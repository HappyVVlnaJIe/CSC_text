#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QTimer>
#include <QFileDialog>
#include <QDebug>
#include  <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    const int Period = 1000/25;   // ms

    clientTimer = new QTimer(this);
    clientTimer->setInterval(Period);
    connect(clientTimer, SIGNAL(timeout()), this, SLOT(runClient()));
    clientTimer->stop();

    serverTimer = new QTimer(this);
    serverTimer->setInterval(Period);
    connect(serverTimer, SIGNAL(timeout()), this, SLOT(runServer()));
    serverTimer->stop();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateChat()
{
    if (ui->chatList->count() > 0)
    {
        QListWidgetItem* cur_item = ui->chatList->currentItem();
        if (cur_item)
        {
            QString contents;
            for (int i=0;i<chats_contents[*cur_item].size();i++)
            {
                contents.append(QString::fromStdString(chats_contents[*cur_item][i]) + "\n");
            }
            ui->chatText->setText(contents);
            ui->chatText->repaint();
        }
    }
}

void MainWindow::on_userServerRunButton_clicked()
{
    if (this->server)
    {
        ui->connectsLogs->setText(ui->connectsLogs->toPlainText() + "Сервер уже запущен" + "\n");
        return;
    }
    bool ok;
    std::string signature = ui->signatureLineEdit->text().toStdString();
    std::string port = ui->userServerPortLineEdit->text().toStdString();
    ui->userServerPortLineEdit->text().toInt(&ok);

    if (ok && signature != "" && port != "")
    {
        this->server = new DefaultServer(port.c_str(), signature);
        if (server->Status() != 0)
        {
            ui->connectsLogs->setText(ui->connectsLogs->toPlainText() + "Ошибка при создании сервера " + QString::number(server->Status()) + "\n");
            return;
        }
        serverTimer->start();
    }
    else
    {
        ui->connectsLogs->setText(ui->connectsLogs->toPlainText() + "Заполните поля подпись и порт для входящих" + "\n");
    }
}

void MainWindow::runServer()
{
    server->CheckClient();
    while(!server->messages.empty())
    {
        Message message = server->messages.front();
        server->messages.pop();
        if (message.sender == "") continue;
        if (chats.find(message.sender) == chats.end())
        {
            ui->chatList->addItem(QString::fromStdString(message.sender));
            QListWidgetItem* item = ui->chatList->item(ui->chatList->count()-1);
            chats[message.sender] = *item;
            chats_contents[*item] = std::vector<std::string>();
            server_chats[*item] = message.sender;
            ui->chatList->repaint();
        }
        chats_contents[chats[message.sender]].push_back(message.text);
        updateChat();
    }
}

void MainWindow::on_connectToServerButton_clicked()
{
    if (this->client)
    {
        ui->connectsLogs->setText(ui->connectsLogs->toPlainText() + "Клиент уже подключился" + "\n");
        return;
    }
    bool ok;
    std::string signature = ui->signatureLineEdit->text().toStdString();
    std::string server_address = ui->serverAdressLineEdit->text().toStdString();
    std::string server_port = ui->serverPortLineEdit->text().toStdString();

    ui->serverPortLineEdit->text().toInt(&ok);

    if (ok && signature != "" && server_port != "")
    {
        this->client = new DefaultClient(server_address.c_str(), server_port.c_str(), signature);
        if (client->Status() != 0)
        {
            ui->connectsLogs->setText(ui->connectsLogs->toPlainText() + "Ошибка при создании клиента " + QString::number(server->Status()) + "\n");
            return;
        }
        clientTimer->start();
    }
}

void MainWindow::runClient()
{
    Message* message = client->ReadFromServer();
    if (message == nullptr || message->sender =="") return;
    if (chats.find(message->sender) == chats.end())
    {
        ui->chatList->addItem(QString::fromStdString(message->sender));
        QListWidgetItem* item = ui->chatList->item(ui->chatList->count()-1);
        chats[message->sender] = *item;
        chats_contents[*item] = std::vector<std::string>();
        clients_chats[*item] = client;
        ui->chatList->repaint();
    }
    chats_contents[chats[message->sender]].push_back(message->text);
    updateChat();
}

void MainWindow::on_chatList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    updateChat();
}


void MainWindow::on_saveToFileButton_clicked()
{
    QString file_path = QFileDialog::getOpenFileName(this, "Open a file", QDir::currentPath(),"*.txt");
    if (!file_path.isEmpty())
    {
        QFile file(file_path);
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file);
            stream << ui->chatText->toPlainText();
        }
        file.close();
    }
}


void MainWindow::on_chatPushButton_clicked()
{
    if (ui->chatInput->text() == "") return;
    bool new_message = false;
    if (clients_chats.find(*ui->chatList->currentItem()) != clients_chats.end())
    {
        new_message = clients_chats[*ui->chatList->currentItem()]->Send(ui->chatInput->text().toStdString());
    }
    qDebug(std::to_string(server_chats.size()).c_str());
    if (server_chats.find(*ui->chatList->currentItem()) != server_chats.end())
    {
        new_message = server->Send(server_chats[*ui->chatList->currentItem()], ui->chatInput->text().toStdString());
    }

    if (new_message)
    {
        chats_contents[*ui->chatList->currentItem()].push_back(client_appeal + ui->chatInput->text().toStdString());
        updateChat();
    }
}


void MainWindow::on_findTextButton_clicked()
{

    std::string sub_text = ui->findTextLineEdit->text().toStdString();
    std::string result = "";

    if (ui->chatList->count() > 0)
    {
        QListWidgetItem* cur_item = ui->chatList->currentItem();
        if (cur_item)
        {
            QString contents;
            for (int i=0;i<chats_contents[*cur_item].size();i++)
            {
                if (chats_contents[*cur_item][i].find(sub_text) != std::string::npos)
                {
                    contents.append(QString::fromStdString(chats_contents[*cur_item][i]) + "\n");
                }
            }
            QMessageBox::about(this, "Результаты поиска", contents);
        }
    }
}
