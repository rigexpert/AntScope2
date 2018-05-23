#ifndef FQSETTINGS_H
#define FQSETTINGS_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class FqSettings;
}

class FqSettings : public QDialog
{
    Q_OBJECT

public:
    explicit FqSettings(QWidget *parent = 0);
    ~FqSettings();

    void setDotsNumber(int dotsNumber);

signals:
    void dotsNumber(int);

private slots:
    void on_dotsSlider_valueChanged(int value);

    void on_OKBtn_clicked();

    void setTitle();

private:
    Ui::FqSettings *ui;
};

#endif // FQSETTINGS_H
