#include "binaryinserter.h"
#include "ui_binaryinserter.h"

#include <string>

#include <QMessageBox>

using namespace std;

BinaryInserter::BinaryInserter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BinaryInserter)
{
    ui->setupUi(this);
}

BinaryInserter::~BinaryInserter()
{
    delete ui;
}

void BinaryInserter::on_btnOk_clicked()
{
    char* buffer = new char[ui->plainData->toPlainText().size() + 1];
    memset(buffer, 0, sizeof(ui->plainData->toPlainText().size() + 1));

    strncpy(buffer,
            const_cast<char*>(ui->plainData->toPlainText().toStdString().c_str()),
            ui->plainData->toPlainText().size());

    auto string2bin = [](char* str) -> uint8_t
    {
        uint8_t temp = 0;
        auto char2int = [](char c) -> uint8_t
        {
            if (c >= '0' && c <= '9')
                return c - '0';
            else if (c >= 'a' && c <= 'f')
                return (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F')
                return (c - 'A' + 10);

            return 0;
        };

        temp = (char2int(str[0]) << 4) | (char2int(str[1]));

        return temp;
    };

    char* token = strtok(buffer, " ");
    while (token != NULL) {
        if (strlen(token) > 2) {
            qDebug("%s", token);
            QMessageBox::critical(this, "Binary Insertion", "Inputs should be in hex format");
            return;
        } else {
            uint8_t x = string2bin(token);
            binary_data.append(static_cast<char>(x));
            token = strtok(NULL, " ");
        }
    }

    delete[] buffer;

    accept();
}

