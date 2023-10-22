#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QTimer>
#include <QFileDialog>
#include <QDebug>
#include  <QMessageBox>
#include "message.h"

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
        this->server = new DefaultServer(port.c_str(),tech_message + separator + signature);
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
    if (server->Status() != 0)
    {
        ui->connectsLogs->setText(ui->connectsLogs->toPlainText() + "Возникла ошибка у сервера " + QString::number(server->Status()) + "\n");
        serverTimer->stop();
        delete server;
        server = nullptr;
        return;
    }
    server->CheckClient();
    while(!server->messages.empty())
    {
        std::string text = server->messages.front();
        server->messages.pop();
        Message message = ParseMessage(text, separator);
        if (message.sender == "") continue;
        if (chats.find(message.sender) == chats.end())
        {
            ui->chatList->addItem(QString::fromStdString(message.sender));
            QListWidgetItem* item = ui->chatList->item(ui->chatList->count()-1);
            chats[message.sender] = item;
            chats_contents[*item] = std::vector<std::string>();
            server_chats[*item] = message.sender;
            ui->chatList->repaint();
        }
        chats_contents[*chats[message.sender]].push_back(message.text);
        updateChat();
    }
}

void MainWindow::on_connectToServerButton_clicked()
{
    if (this->client)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "", "Вы хотите переподключиться?", QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No)
        {
            return;
        }
        delete this->client;
        std::string key;
        for(auto& elem : chats)
        {
            if (elem.second == client_chat)
            {
                key = elem.first;
                break;
            }
        }
        chats.erase(key);
        clients_chats.erase(*client_chat);
        chats_contents.erase(*client_chat);
        delete ui->chatList->takeItem(ui->chatList->row(client_chat));
        ui->chatList->repaint();
        ui->chatText->setText("");
    }
    bool ok;
    std::string server_address = ui->serverAdressLineEdit->text().toStdString();
    std::string server_port = ui->serverPortLineEdit->text().toStdString();

    ui->serverPortLineEdit->text().toInt(&ok);

    if (ok && server_port != "")
    {
        this->client = new DefaultClient(server_address.c_str(), server_port.c_str(), 1);
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
    if (client->Status() != 0)
    {
        ui->connectsLogs->setText(ui->connectsLogs->toPlainText() + "Возникла ошибка у клиента " + QString::number(client->Status()) + "\n");
        clientTimer->stop();
        delete client;
        client = nullptr;
        return;
    }
    Message message = ParseMessage(client->ReadFromServer(), separator);

    if (message.sender == "") return;
    if (chats.find(message.sender) == chats.end())
    {
        ui->chatList->addItem(QString::fromStdString(message.sender));
        QListWidgetItem* item = ui->chatList->item(ui->chatList->count()-1);
        chats[message.sender] = item;
        chats_contents[*item] = std::vector<std::string>();
        clients_chats[*item] = client;
        ui->chatList->repaint();
        client_chat = item;
    }
    chats_contents[*chats[message.sender]].push_back(message.text);
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
    std::string text = ui->chatInput->text().toStdString() + separator + ui->signatureLineEdit->text().toStdString();
    if (clients_chats.find(*ui->chatList->currentItem()) != clients_chats.end())
    {
        new_message = clients_chats[*ui->chatList->currentItem()]->Send(text);
    }
    qDebug(std::to_string(server_chats.size()).c_str());
    if (server_chats.find(*ui->chatList->currentItem()) != server_chats.end())
    {
        new_message = server->Send(server_chats[*ui->chatList->currentItem()], text );
    }

    if (new_message)
    {
        chats_contents[*ui->chatList->currentItem()].push_back(client_appeal + ui->chatInput->text().toStdString());
        updateChat();
    }
    else
    {
        QMessageBox::about(this, "Ошибка отправки сообщения", "Чат с " + ui->chatList->currentItem()->text() + " был закрыт из-за ошибки");
        delete ui->chatList->takeItem(ui->chatList->row(ui->chatList->currentItem()));
        ui->chatList->repaint();
        ui->chatText->setText("");
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

void MainWindow::on_registrationButton_clicked()
{
    const QWidget* pWidget= ui->tabs->currentWidget();
    std::string login = pWidget->findChild<QLineEdit*>("loginLineEdit")->text().toStdString();
    std::string pass = pWidget->findChild<QLineEdit*>("passLineEdit")->text().toStdString();
    if ((login.find(" ") != std::string::npos) || (pass.find(" ") != std::string::npos))
    {
        QMessageBox::about(this, "Ошибка", "нельзя использовать пробелы");
        return;
    }
    std::unique_ptr<DefaultClient> auth_client = std::make_unique<DefaultClient>(auth_server_address, auth_server_port, 0);
    auth_client->Send(registration_command + " " + login + " " + pass + " ");
    std::string response =auth_client->ReadFromServer();
    const char* indx = response.c_str();
    while(*indx != '\0') indx++;
    response = response.substr(0, indx - response.c_str());
    if (response == positive_response)
    {
        ui->tabs->setTabEnabled(1, true);
        ui->tabs->setCurrentIndex(1);
        ui->tabs->currentWidget()->findChild<QLineEdit*>("signatureLineEdit")->setText(QString::fromStdString( login));
        ui->tabs->currentWidget()->findChild<QListWidget*>("chatList")->setEnabled(true);
        ui->tabs->currentWidget()->findChild<QLineEdit*>("chatInput")->setEnabled(true);
        ui->tabs->currentWidget()->findChild<QPushButton*>("chatPushButton")->setEnabled(true);
    }
    else
    {
        QMessageBox::about(this, "Ошибка", "Ошибка при регистрации");
    }
}


void MainWindow::on_authorizationButton_clicked()
{
    const QWidget* pWidget= ui->tabs->currentWidget();
    std::string login = pWidget->findChild<QLineEdit*>("loginLineEdit")->text().toStdString();
    std::string pass = pWidget->findChild<QLineEdit*>("passLineEdit")->text().toStdString();
    if ((login.find(" ") != std::string::npos) || (pass.find(" ") != std::string::npos))
    {
        QMessageBox::about(this, "Ошибка", "нельзя использовать пробелы");
        return;
    }
    std::unique_ptr<DefaultClient> auth_client = std::make_unique<DefaultClient>(auth_server_address, auth_server_port, 0);
    auth_client->Send(authorization_command + " " + login + " " + pass + " ");
    std::string response =auth_client->ReadFromServer();
    const char* indx = response.c_str();
    while(*indx != '\0') indx++;
    response = response.substr(0, indx - response.c_str());
    if (response == positive_response)
    {
        ui->tabs->setTabEnabled(1, true);
        ui->tabs->setCurrentIndex(1);
        ui->tabs->currentWidget()->findChild<QLineEdit*>("signatureLineEdit")->setText(QString::fromStdString( login));
        ui->tabs->currentWidget()->findChild<QListWidget*>("chatList")->setEnabled(true);
        ui->tabs->currentWidget()->findChild<QLineEdit*>("chatInput")->setEnabled(true);
        ui->tabs->currentWidget()->findChild<QPushButton*>("chatPushButton")->setEnabled(true);


    }
    else
    {
        QMessageBox::about(this, "Ошибка", "Ошибка при регистрации");
    }
}

