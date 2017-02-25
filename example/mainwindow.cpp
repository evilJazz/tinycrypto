/*
 ---------------------------------------------------------------------------
 Copyright (c) 2013, Igor Saric. All rights reserved.
 Copyright (c) 2014-2017, Andre Beckedorf, Oldenburg, Germany. All rights reserved.

 LICENSE TERMS

 The redistribution and use of this software (with or without changes)
 is allowed without the payment of fees or royalties provided that:

  1. source code distributions include the above copyright notice, this
     list of conditions and the following disclaimer;

  2. binary distributions include the above copyright notice, this list
     of conditions and the following disclaimer in their documentation;

  3. the name of the copyright holder is not used to endorse products
     built using this software without specific written permission.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "tinyaes.h"
#include "tinyhmac.h"
#include "tinypbkdf2.h"

#include <QBuffer>
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->PlainText->setText("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");

    QByteArray inputPassword = QByteArray("Hello world!");
    QByteArray inputSalt = QByteArray("My secret salt");

    QByteArray derivedKey = TinyPBKDF2::derive(inputPassword, inputSalt, QCryptographicHash::Sha1, 256 / 8, 10000);

    ui->KeyText->setText(derivedKey.toHex());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_ExecuteButton_clicked()
{
    QByteArray key = TinyAES::hexStringToByte(ui->KeyText->text());
    QByteArray data = ui->PlainText->toPlainText().toUtf8();

    QByteArray dataDigest = QCryptographicHash::hash(data, QCryptographicHash::Sha1);

    QByteArray encrypted = TinyAES::encrypt(data, key);
    QByteArray decrypted = TinyAES::decrypt(encrypted, key);

    QByteArray decryptedDigest = QCryptographicHash::hash(decrypted, QCryptographicHash::Sha1);

    if (dataDigest == decryptedDigest)
        ui->statusLabel->setText("MATCH");
    else
        ui->statusLabel->setText("CORRUPTED DATA!!!");

    ui->EncryptedText->setText(QString(encrypted.toHex()));
    ui->DecryptedTextRaw->setText(QString(decrypted.toHex()));
    ui->DecryptedText->setText(QString::fromUtf8(decrypted));
}

void MainWindow::on_ExecuteButton_2_clicked()
{
    QByteArray key = TinyAES::hexStringToByte(ui->KeyText->text());
    QByteArray data = ui->PlainText->toPlainText().toUtf8();

    QByteArray dataDigest = QCryptographicHash::hash(data, QCryptographicHash::Sha1);

    QBuffer dataBuffer; dataBuffer.setData(data); dataBuffer.open(QIODevice::ReadWrite);

    QByteArray encrypted;
    QBuffer encryptedBuffer; encryptedBuffer.setBuffer(&encrypted); encryptedBuffer.open(QIODevice::ReadWrite);

    QByteArray decrypted;
    QBuffer decryptedBuffer; decryptedBuffer.setBuffer(&decrypted); decryptedBuffer.open(QIODevice::ReadWrite);

    TinyAES::encrypt(&dataBuffer, &encryptedBuffer, key);
    dataBuffer.seek(0);

    encryptedBuffer.seek(0);
    TinyAES::decrypt(&encryptedBuffer, &decryptedBuffer, key);

    decryptedBuffer.seek(0);
    QByteArray decryptedDigest = QCryptographicHash::hash(decryptedBuffer.data(), QCryptographicHash::Sha1);

    if (dataDigest == decryptedDigest)
        ui->statusLabel->setText("MATCH");
    else
        ui->statusLabel->setText("CORRUPTED DATA!!!");

    ui->EncryptedText->setText(QString(encryptedBuffer.data().toHex()));
    ui->DecryptedTextRaw->setText(QString(decryptedBuffer.data().toHex()));
    ui->DecryptedText->setText(QString::fromUtf8(decryptedBuffer.data()));
}
