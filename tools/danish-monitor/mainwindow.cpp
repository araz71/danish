#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QTableWidgetItem>

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
}

void MainWindow::listiner_function() {
    while (1) {
        serial_port_locker.lock();
        if (serial_port->waitForReadyRead(100)) {
            QByteArray data = serial_port->readAll();
            danish_st result;
            if (danish_ach(reinterpret_cast<uint8_t*>(const_cast<char*>(data.toStdString().c_str())),
                       data.length(), &result) == 1)
            {
                add_row("IN", result);
            }
        }
        serial_port_locker.unlock();

        this_thread::sleep_for(100ms);
    }
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
    ui->tblPackets->setItem(selected_row, COL_DIR, new MyItem(dir));
    ui->tblPackets->setItem(selected_row, COL_ADDRESS, new MyItem(QString::number(inf.address)));
    ui->tblPackets->setItem(selected_row, COL_FUNCTION, new MyItem(functions[inf.function]));
    ui->tblPackets->setItem(selected_row, COL_LEN, new MyItem(QString::number(inf.len)));
    ui->tblPackets->setItem(selected_row, COL_REGISTER_ID, new MyItem(QString::number(inf.regID)));
    ui->tblPackets->setItem(selected_row, COL_DATA,
                            new MyItem(QByteArray(reinterpret_cast<char*>(inf.data), inf.len).toHex()));

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnOpen_clicked()
{
    serial_port.reset(new QSerialPort);

    serial_port->setPortName(ui->cbPorts->currentText());
    serial_port->setBaudRate(QSerialPort::Baud115200);

    if (serial_port->open(QSerialPort::ReadWrite)) {
        listiner_thread = new std::thread(&MainWindow::listiner_function, this);
        ui->frmOpen->setEnabled(false);
        ui->frmRead->setEnabled(true);

    } else {
        QMessageBox::critical(this, "Serial Port", "Can not open selected serial port.");
        serial_port.release();
    }
}


void MainWindow::on_btnread_clicked()
{
    serial_port_locker.lock();

    danish_st inf {
        static_cast<uint8_t>(ui->leAddress->text().toInt()), FUNC_READ,
                static_cast<uint16_t>(ui->leRegID->text().toInt()), 0, NULL
    };

    add_row("Out", inf);

    uint8_t buffer[256];
    uint8_t len = danish_make(ui->leAddress->text().toInt(), FUNC_READ,
                ui->leRegID->text().toInt(), 0, NULL, buffer);

    serial_port->write(reinterpret_cast<char*>(buffer), len);
    serial_port->flush();

    serial_port_locker.unlock();
}

