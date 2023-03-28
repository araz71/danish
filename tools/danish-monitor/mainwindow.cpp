#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <danish_link.h>

#include <QMessageBox>
#include <QTableWidgetItem>

#define MyAddress   23

class MyItem : public QTableWidgetItem {
public:
    MyItem(QString text) {
        setText(text);
        setTextAlignment(Qt::AlignCenter);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto ports = QSerialPortInfo::availablePorts();
    for (auto port : ports)
        ui->cbPorts->addItem(port.portName());

    danish_link_init(MyAddress, NULL);

    uint8_t* buf1 = new uint8_t[10];
    reg_st reg1;
    reg1.ptr = buf1;
    reg1.regID = 1;
    reg1.size = 3;
    reg1.filled_callback = NULL;
    reg1.write_ack_callback = NULL;
    memset(buf1, 10, reg1.size);

    danish_add_register(&reg1);
}

void MainWindow::listiner_function() {
    serial_port_locker.lock();
    QByteArray data = serial_port->readAll();
    danish_st result;
    char hex_format[1024];
    char hex[10];

    memset(hex_format, 0, sizeof(hex_format));
    for (int i = 0; i < data.size(); i++) {
        danish_collect(data.at(i));

        sprintf(hex, "0x%02X", static_cast<uint8_t>(data.at(i)));
        strcat(hex_format, hex);
        strcat(hex_format, " ");
    }
    ui->textEditLog->setPlainText(ui->textEditLog->toPlainText() + "\r\nIN  : " + hex_format);

    if (danish_parse(&result) == 1) {
        add_row("IN", result);
        uint8_t buffer[256];
        uint8_t size = danish_handle(&result, buffer);

        if (size != 0) {
            danish_ach(buffer, size, &result);
            add_row("My Response", result);
            serial_write(buffer, size);
        }
    }

    serial_port_locker.unlock();
}

void MainWindow::add_row(QString dir, danish_st inf)
{
    static const char* functions[20] = {
        "Write",
        "Write Ack",
        "Read",
        "Read Ack",
    };

    ui->tblPackets->setRowCount(ui->tblPackets->rowCount() + 1);

    int selected_row = ui->tblPackets->rowCount() - 1;
    ui->tblPackets->setItem(selected_row, COL_DIR,
                            new MyItem(dir));

    ui->tblPackets->setItem(selected_row, COL_SOURCE_ADDRESS,
                            new MyItem(QString::number(inf.src)));

    ui->tblPackets->setItem(selected_row, COL_DESTINATION_ADDRESS,
                            new MyItem(QString::number(inf.dst)));

    ui->tblPackets->setItem(selected_row, COL_FUNCTION,
                            new MyItem(functions[inf.function]));

    ui->tblPackets->setItem(selected_row, COL_LEN,
                            new MyItem(QString::number(inf.len)));

    ui->tblPackets->setItem(selected_row, COL_REGISTER_ID,
                            new MyItem(QString::number(inf.regID)));

    ui->tblPackets->setItem(selected_row, COL_DATA,
                            new MyItem(QByteArray(reinterpret_cast<char*>(inf.data), inf.len).toHex()));

}

void MainWindow::serial_write(uint8_t* data, uint8_t len)
{
    serial_port->write(reinterpret_cast<char*>(data), len);
    serial_port->flush();

    char buffer[1024];
    char hex[10];

    memset(buffer, 0, sizeof(buffer));
    strcat(buffer, "Out : ");
    for (int i = 0; i < len; i++) {
        sprintf(hex, "0x%02X", data[i]);
        strcat(buffer, hex);
        strcat(buffer, " ");
    }

    ui->textEditLog->setPlainText(ui->textEditLog->toPlainText() + "\r\n" + QString(buffer));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnOpen_clicked()
{
    serial_port.reset(new QSerialPort);

    serial_port->setPortName(ui->cbPorts->currentText());
    serial_port->setBaudRate(230400);

    if (serial_port->open(QSerialPort::ReadWrite)) {
        ui->frmOpen->setEnabled(false);
        ui->frmRead->setEnabled(true);

        QObject::connect(serial_port.get(), &QSerialPort::readyRead, [this](){
            listiner_function();
        });

    } else {
        QMessageBox::critical(this, "Serial Port", "Can not open selected serial port.");
        serial_port.release();
    }
}


void MainWindow::on_btnread_clicked()
{
    serial_port_locker.lock();

    add_row("Out", {
                MyAddress, static_cast<uint8_t>(ui->leAddress->text().toInt()), FUNC_READ,
                        static_cast<uint16_t>(ui->leRegID->text().toInt()), 0, NULL
            });

    uint8_t buffer[256];
    uint8_t len = danish_make(MyAddress, ui->leAddress->text().toInt(), FUNC_READ,
                ui->leRegID->text().toInt(), 0, NULL, buffer);

    serial_write(buffer, len);

    serial_port_locker.unlock();
}


void MainWindow::on_cbType_currentTextChanged(const QString &arg1)
{
    if (arg1 == "uint8_t") {
        ui->leData->setValidator(new QIntValidator(0, 255, ui->leData));
    } else if (arg1 == "uint16_t") {
        ui->leData->setValidator(new QIntValidator(0, 65535, ui->leData));
    } else if (arg1 == "uint32_t") {
        ui->leData->setValidator(new QIntValidator(0, 0xFFFFFFFF, ui->leData));
    } else if (arg1 == "String") {
        ui->leData->setValidator(nullptr);
    }
}

void MainWindow::on_btnWrite_clicked()
{
    serial_port_locker.lock();

    int data_size = 0;
    QString data_type = ui->cbType->currentText();
    uint8_t* data;

    int value;
    data = reinterpret_cast<uint8_t*>(&value);

    if (data_type == "uint8_t") {
        data_size = 1;
        value = ui->leData->text().toInt();

    } else if (data_type == "uint16_t") {
        data_size = 2;
        value = ui->leData->text().toInt();

    } else if (data_type == "uint32_t") {
        data_size = 4;
        value = ui->leData->text().toInt();

    } else if (data_type == "String") {
        data_size = ui->leData->text().size();
        data = reinterpret_cast<uint8_t*>(const_cast<char*>(ui->leData->text().toStdString().c_str()));
    }

    add_row("Out", {
                MyAddress, static_cast<uint8_t>(ui->leAddress->text().toInt()), FUNC_WRITE,
                static_cast<uint16_t>(ui->leRegID->text().toInt()), static_cast<uint8_t>(data_size), data
            });

    uint8_t buffer[256];
    uint8_t size = danish_make(MyAddress, static_cast<uint8_t>(ui->leAddress->text().toInt()),
                               FUNC_WRITE, static_cast<uint16_t>(ui->leRegID->text().toInt()),
                               static_cast<uint8_t>(data_size), data, buffer);
    serial_write(buffer, size);

    serial_port_locker.unlock();
}

void MainWindow::on_btnClear_clicked()
{
    ui->textEditLog->setPlainText("");
    for (int i = 0; i < ui->tblPackets->rowCount(); i++) {
        for (int j = 0; j < COL_MAX; j++)
            if (ui->tblPackets->itemAt(i, j) != nullptr)
                delete ui->tblPackets->itemAt(i, j);
    }
    ui->tblPackets->setRowCount(0);
}
