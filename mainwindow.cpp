#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QSerialPortInfo>

void MainWindow::configApp()
{

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    setWindowIcon(QIcon("PorterIcon.ico"));
    setFixedSize(QSize(550, 450));



    QtConcurrent::run(this, &MainWindow::configApp);

    core = new Core();
    core->moveToThread(&coreThread);
    QObject::connect(&coreThread, &QThread::started, core, &Core::loop);
    QObject::connect(core, &Core::appendReceivedData, this, &MainWindow::displayData);
    coreThread.start();

    updateSerialPortsFuture = QtConcurrent::run(this, &MainWindow::updateSerialPortsNames);
}

MainWindow::~MainWindow()
{
    core->disableLoop();
    coreThread.quit();
    coreThread.wait();
    delete core;

    continueUpdatedSerialPorts = false;
    updateSerialPortsFuture.waitForFinished();

    delete ui;
}

void MainWindow::on_serialButton_clicked()
{
    if (ui->serialButton->text() == "Connect") {
        core->getSerial()->setName(ui->serialName_ComboBox->currentText());
        core->getSerial()->setSpeed(ui->serialSpeed_ComboBox->currentText().toInt());
        core->getSerial()->setStartByte(ui->spinBox->value());
        core->getSerial()->setDataLength(ui->spinBox_2->value());
        core->getSerial()->setReceiveBufferSize(ui->spinBox_3->value());

        if (core->getSerial()->open()) {
            ui->serialButton->setText("Disconnect");
            ui->statusBar->showMessage("Connected to " + ui->serialName_ComboBox->currentText() + " with speed " + ui->serialSpeed_ComboBox->currentText());
            ui->serialName_ComboBox->setEnabled(false);
            ui->serialSpeed_ComboBox->setEnabled(false);

            ui->spinBox->setEnabled(false);
            ui->spinBox_2->setEnabled(false);
            ui->spinBox_3->setEnabled(false);
        }
    } else {
        core->getSerial()->close();
        ui->serialButton->setText("Connect");
        ui->serialName_ComboBox->setEnabled(true);
        ui->serialSpeed_ComboBox->setEnabled(true);
        ui->spinBox->setEnabled(true);
        ui->spinBox_2->setEnabled(true);
        ui->spinBox_3->setEnabled(true);
    }
}

void MainWindow::updateSerialPortsNames()
{
    while(continueUpdatedSerialPorts) {
        static clock_t time = clock();
        if ((clock() - time) >= 100) {
            time = clock();
            auto info = QSerialPortInfo::availablePorts();
            QString portNamesString = "Available ports: ";
            QList<QString> currentPortsNames;
            for (auto &el: info) {
                portNamesString += el.portName() + " ";
                currentPortsNames.push_back(el.portName());
            }

            if (!core->getSerial()->isOpen()) {
                if (info.size()) {
                    ui->statusBar->showMessage(portNamesString);
                } else {
                    ui->statusBar->showMessage("Not available ports");
                }
            }

            if (currentPortsNames != portsNames) {
                portsNames = currentPortsNames;
                ui->serialName_ComboBox->setCurrentIndex(-1);
                ui->serialName_ComboBox->clear();
                if (currentPortsNames.size() > 0) {
                    ui->serialName_ComboBox->addItems(currentPortsNames);
                    ui->serialName_ComboBox->setCurrentIndex(0);
                }
            }

        }
        QThread().currentThread()->msleep(1);
    }
}

void MainWindow::on_pushButton_clicked()
{
    QByteArray testData;
    testData.push_back(12);
    testData.push_back(65);
    testData.push_back(98);
    core->getSerial()->write(testData);
}


void MainWindow::on_pushButton_2_clicked()
{
    QByteArray testData;
    testData.push_back(32);
    testData.push_back(12);
    testData.push_back(24);
    core->getSerial()->write(testData);
}


void MainWindow::on_clearButton_clicked()
{
    ui->transmitHexLine->setText("");
}

void MainWindow::on_pushButton_4_clicked()
{
    QString values = ui->transmitHexLine->text();
    if (values.length()) {
        QStringList valuesList = values.split(QRegExp("\\s+"));

        QByteArray numbers;
        for(auto &el: valuesList) {
            numbers.push_back(el.toUInt());
        }

        core->getSerial()->write(numbers);
    }

}

void MainWindow::displayData(QByteArray data)
{
    ui->textEdit->append(data.toHex('.'));
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->textEdit->clear();
}
