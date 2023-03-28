#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>

#include <thread>
#include <memory>
#include <queue>
#include <mutex>

extern "C" {
    #include "danish_link.h"
    #include "danish.h"
}
using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnOpen_clicked();

    void on_btnread_clicked();

    void on_cbType_currentTextChanged(const QString &arg1);

    void on_btnWrite_clicked();

    void on_btnClear_clicked();

private:

    enum {
        COL_DIR,
        COL_SOURCE_ADDRESS,
        COL_DESTINATION_ADDRESS,
        COL_REGISTER_ID,
        COL_FUNCTION,
        COL_DATA,
        COL_LEN,
        COL_MAX,
    } column_enu;

    Ui::MainWindow *ui;

    std::thread* listiner_thread;
    queue<danish_st> packets;
    unique_ptr<QSerialPort> serial_port;
    std::mutex serial_port_locker;

    void listiner_function();

    void add_row(QString dir, danish_st inf);
    void serial_write(uint8_t* data, uint8_t len);
};
#endif // MAINWINDOW_H
