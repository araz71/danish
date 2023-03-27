#include "mainwindow.h"
#include "ui_mainwindow.h"

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
}

void MainWindow::listiner_function() {
    while (1) {
        serial_port_locker.lock();
        if (serial_port->waitForReadyRead(100)) {
            QByteArray data = serial_port->readAll();
            danish_st result;

            for (int i = 0; i < data.size(); i++)
                danish_collect(data.at(i));

            if (danish_parse(&result) == 1) {
                add_row("IN", result);
                uint8_t buffer[256];
                uint8_t size = danish_handle(&result, buffer);

                if (size != 0) {
                    serial_port->write(reinterpret_cast<char*>(buffer), size);
                    serial_port->flush();
                }
            }
        }
        serial_port_locker.unlock();

        this_thread::sleep_for(chrono::microseconds(100));
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

    add_row("Out", {
                MyAddress, static_cast<uint8_t>(ui->leAddress->text().toInt()), FUNC_READ,
                        static_cast<uint16_t>(ui->leRegID->text().toInt()), 0, NULL
            });

    uint8_t buffer[256];
    uint8_t len = danish_make(MyAddress, ui->leAddress->text().toInt(), FUNC_READ,
                ui->leRegID->text().toInt(), 0, NULL, buffer);

    serial_port->write(reinterpret_cast<char*>(buffer), len);
    serial_port->flush();

    serial_port_locker.unlock();
}

