#ifndef BINARYINSERTER_H
#define BINARYINSERTER_H

#include <QDialog>

namespace Ui {
class BinaryInserter;
}

class BinaryInserter : public QDialog
{
    Q_OBJECT

public:
    explicit BinaryInserter(QWidget *parent = nullptr);
    ~BinaryInserter();

    QByteArray binary_data;
private slots:
    void on_btnOk_clicked();

private:
    Ui::BinaryInserter *ui;
};

#endif // BINARYINSERTER_H
