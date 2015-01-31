/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"

#include <QCursor>
#include <QMessageBox>
#include <QtSerialPort/QSerialPort>

//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    currentStart = 0;
    currentEnd = 0;
    accelX = 0;
    accelY = 0;
    accelZ = 0;
    gyroX = 0;
    gyroY = 0;
    gyroZ = 0;
    flexThumb = 0;
    flexIndex = 0;
    flexMiddle = 0;
    flexRing = 0;
    flexPinky = 0;

    accelXConverted = 0.0;
    accelYConverted = 0.0;
    accelZConverted = 0.0;
    gyroXConverted = 0.0;
    gyroYConverted = 0.0;
    gyroZConverted = 0.0;
    flexThumbConverted = 0.0;
    flexIndexConverted = 0.0;
    flexMiddleConverted = 0.0;
    flexRingConverted = 0.0;
    flexPinkyConverted = 0.0;

    xCoord = 500;
    yCoord = 250;

    numErrors = 0;

    //myCursor.setPos(test1,test2);
//! [0]
    ui->setupUi(this);
    console = new Console;
    console->setEnabled(false);
    //setCentralWidget(console);
//! [1]
    serial = new QSerialPort(this);
//! [1]
    settings = new SettingsDialog;

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);

    initActionsConnections();

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError)));

//! [2]
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
//! [2]
    connect(console, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray)));
//! [3]
}
//! [3]

MainWindow::~MainWindow()
{
    delete settings;
    delete ui;
}

//! [4]
void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    if (serial->open(QIODevice::ReadWrite)) {
            console->setEnabled(true);
            console->setLocalEchoEnabled(p.localEchoEnabled);
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);
            ui->actionConfigure->setEnabled(false);
            ui->statusBar->showMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                       .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                       .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        ui->statusBar->showMessage(tr("Open error"));
    }
}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{
    serial->close();
    console->setEnabled(false);
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    ui->statusBar->showMessage(tr("Disconnected"));
}
//! [5]

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Terminal"),
                       tr("The <b>Simple Terminal</b> example demonstrates how to "
                          "use the Qt Serial Port module in modern GUI applications "
                          "using Qt, with a menu bar, toolbars, and a status bar."));
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
    serial->write(data);
}
//! [6]

//! [7]
void MainWindow::readData()
{
    QByteArray data = serial->readAll();
    unsigned char* dataArray = (unsigned char*)data.data();
    QString tempString = "";
    tempString.sprintf("%d", data.size());

    for(int i = 0; i < data.size(); i++)
    {
        currentInput[currentEnd] = dataArray[i];
        currentEnd++;
        if(dataArray[i]!='S'&& dataArray[i]!='T'&& dataArray[i]!='E'&& dataArray[i]!='N')
            tempString.sprintf("%u ", dataArray[i]);
        else
            tempString.sprintf("%c ", dataArray[i]);

        output.append(tempString);
        //ui->textEdit->setText(output);

        if(currentInput[0] != 'S')
        {
            for(int i = 0; i < 99; i++)
            {
               currentInput[i] = currentInput[i+1];
            }
            currentEnd--;
        }

        if(currentInput[0] == 'S' && currentInput[1] == 'T' && currentInput[24] == 'E' && currentInput[25] == 'N')
        {
            accelX =(signed short) (currentInput[2] + (currentInput[3]<<8));
            accelY = currentInput[4] + (currentInput[5]<<8);
            accelZ = currentInput[6] + (currentInput[7]<<8);
            gyroX = currentInput[8] + (currentInput[9]<<8);
            gyroY = currentInput[10] + (currentInput[11]<<8);
            gyroZ = currentInput[12] + (currentInput[13]<<8);
            flexThumb = currentInput[14] + (currentInput[15]<<8);
            flexIndex = currentInput[16] + (currentInput[17]<<8);
            flexMiddle = currentInput[18] + (currentInput[19]<<8);
            flexRing = currentInput[20] + (currentInput[21]<<8);
            flexPinky = currentInput[22] + (currentInput[23]<<8);

            if(accelX > 200 && xCoord < 1500)
                xCoord+=20;
            if(accelX < -200 && xCoord > 20)
                xCoord-=20;
            if(accelY > 200 && yCoord < 1500)
                yCoord+=20;
            if(accelY < -200 && yCoord > 20)
                yCoord-=20;


            ui->lineEdit->setText(tempString.sprintf("%d",accelX));
            ui->lineEdit_2->setText(tempString.sprintf("%d",accelY));
            ui->lineEdit_3->setText(tempString.sprintf("%d",accelZ));
            ui->lineEdit_4->setText(tempString.sprintf("%d",gyroX));
            ui->lineEdit_5->setText(tempString.sprintf("%d",gyroY));
            ui->lineEdit_6->setText(tempString.sprintf("%d",gyroZ));

            ui->lineEdit_9->setText(tempString.sprintf("%u",flexThumb));
            ui->lineEdit_10->setText(tempString.sprintf("%u",flexIndex));
            ui->lineEdit_11->setText(tempString.sprintf("%u",flexMiddle));
            ui->lineEdit_12->setText(tempString.sprintf("%u",flexRing));
            ui->lineEdit_13->setText(tempString.sprintf("%u",flexPinky));

            currentInput[24] = 0;
            currentInput[25] = 0;
            currentEnd = 0;
        }
    }
    myCursor.setPos(xCoord,yCoord);
   //console->putData(data);
}
//! [7]

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}
//! [8]

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionConfigure, SIGNAL(triggered()), settings, SLOT(show()));
    connect(ui->actionClear, SIGNAL(triggered()), console, SLOT(clear()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}
