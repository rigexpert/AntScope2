#include "selectdevicedialog.h"
#include "ui_selectdevicedialog.h"
#include "settings.h"
#include "nanovna_analyzer.h"
#include "ble_analyzer.h"



// static
SelectionParameters SelectionParameters::selected;


SelectDeviceDialog::SelectDeviceDialog(bool silent, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectDeviceDialog)
{
    ui->setupUi(this);

    QString style = "QPushButton:disabled{"
            "background-color: rgb(59, 59, 59);"
            "color: rgb(119, 119, 119);}"
            "QPushButton:checked{"
            "background-color: rgb(1, 178, 255);}";
    ui->pushButtonConnect->setStyleSheet(style);
    ui->pushButtonScan->setStyleSheet(style);

    QString path = Settings::setIniFile();
    QSettings set(path, QSettings::IniFormat);
    set.beginGroup("Connection");
    bool same = set.value("same", false).toBool();
    ReDeviceInfo::InterfaceType _type = (ReDeviceInfo::InterfaceType)set.value("type", ReDeviceInfo::HID).toInt();
    set.endGroup();

    ui->radioButtonCOM->setChecked(false);
    ui->radioButtonUSB->setChecked(false);
    ui->radioButtonBLE->setChecked(false);

    ui->checkBox->setChecked(same);
    switch(_type) {
    case ReDeviceInfo::Serial:
        ui->radioButtonCOM->setChecked(true);
        break;
    case ReDeviceInfo::BLE:
        ui->radioButtonBLE->setChecked(true);
        break;
    default:
        ui->radioButtonUSB->setChecked(true);
        break;
    }

    connect(ui->radioButtonUSB, &QRadioButton::toggled, this, [=](bool checked) {
        if (checked)
            onScan(ReDeviceInfo::HID);
    });
    connect(ui->radioButtonCOM, &QRadioButton::toggled, this, [=](bool checked) {
        if (checked)
            onScan(ReDeviceInfo::Serial);
    });
    connect(ui->radioButtonBLE, &QRadioButton::toggled, this, [=](bool checked) {
        if (checked)
            onScan(ReDeviceInfo::BLE);
    });
    connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, this, [=](QTableWidgetItem* _item) {
        if (_item == nullptr)
            return;
        QTableWidgetItem* name = ui->tableWidget->item(_item->row(), 0);
        QTableWidgetItem* column = ui->tableWidget->item(_item->row(), 1);
        ReDeviceInfo::InterfaceType _type = (ReDeviceInfo::InterfaceType)(name->data(Qt::UserRole+1).toInt());
        onApply(_type,
                name->data(Qt::DisplayRole).toString(),
                column->data(Qt::DisplayRole).toString());
    });
    connect(ui->pushButtonConnect, &QPushButton::clicked, this, [=]{
        QTableWidgetItem* item = ui->tableWidget->currentItem();
        if (item == nullptr)
            return;
        QTableWidgetItem* name = ui->tableWidget->item(item->row(), 0);
        QTableWidgetItem* id = ui->tableWidget->item(item->row(), 1);
        ReDeviceInfo::InterfaceType _type = (ReDeviceInfo::InterfaceType)(name->data(Qt::UserRole+1).toInt());
        onApply(_type,
                name->data(Qt::DisplayRole).toString(),
                id->data(Qt::DisplayRole).toString());
    });
    connect(ui->checkBox, &QCheckBox::toggled, this, [=](bool checked){
        QString path = Settings::setIniFile();
        QSettings set(path, QSettings::IniFormat);
        set.beginGroup("Connection");
        set.setValue("same", checked);
        set.endGroup();
    });
    connect(ui->pushButtonScan, &QPushButton::clicked, this, [=](){
        onScan(type());
    });
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, [=]() {
        this->reject();
    });
    if (!silent)
        onScan(type());

    int support = BleAnalyzer::supported();
    if (support == BLE_SUPPORT_NONE) {
        ui->labelSupported->hide();
        ui->radioButtonBLE->hide();
    } else if (support == BLE_SUPPORT_PARTIAL) {
        QString style = "color: white; background: red";
        ui->labelSupported->setStyleSheet(style);
        QString txt = tr("This version of the operating system does not guarantee the correct operation of the BLE.");
        ui->labelSupported->setText(txt);
    } else {
        ui->labelSupported->hide();
    }
}

SelectDeviceDialog::~SelectDeviceDialog()
{
    delete ui;
}

void SelectDeviceDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SelectDeviceDialog::onApply(ReDeviceInfo::InterfaceType type,
                                 QString name, QString port_or_serial)
{
    name.replace("\r", "");
    int _type = (int)type;
    QString serial;
    qDebug() << "SelectDeviceDialog::onApply" << (int)type << name << port_or_serial;
    AnalyzerParameters* param=nullptr;
    if (type == (int)ReDeviceInfo::HID) {
        int prefix = AnalyzerParameters::prefixFromSerial(port_or_serial);
        param = AnalyzerParameters::byPrefix(prefix);
        if (prefix == 0 || param == nullptr) {
            QMessageBox::warning(this, tr("Select device"), tr("Serial number does not match the type of device"));
            return;
        }
    } else if (type == (int)ReDeviceInfo::NANO) {
        param = AnalyzerParameters::byName(name);
    } else if (type == (int)ReDeviceInfo::Serial) {
        //name = "COMPORT";
        param = AnalyzerParameters::byName(name);
    } else if (type == (int)ReDeviceInfo::BLE) {
        param = AnalyzerParameters::byName(name);
        QStringList args = name.split(' ');
        if (args.size() > 1) {
            serial = args.at(args.size()-1);
        }
        if (serial.length() < 9 && param != nullptr) {
            serial = QString("%1%2").arg(param->prefix(), 4, 10, QChar('0')).arg(serial);
        }
    }
    if (param == nullptr)
        return;
    SelectionParameters::selected.name = param->name();
    SelectionParameters::selected.type = type;
    SelectionParameters::selected.id = port_or_serial;
    SelectionParameters::selected.modelIndex = param->index();
    SelectionParameters::selected.serial = serial;

    AnalyzerParameters::setCurrent(param);

    QString path = Settings::setIniFile();
    QSettings set(path, QSettings::IniFormat);
    set.beginGroup("Connection");
    set.setValue("same", ui->checkBox->isChecked());
    set.setValue("type", SelectionParameters::selected.type);
    set.setValue("name", SelectionParameters::selected.name);
    set.setValue("id", SelectionParameters::selected.id);
    set.endGroup();

    QDialog::accept();
}

extern void showPortInfo(const QSerialPortInfo& info);

void SelectDeviceDialog::onScan(ReDeviceInfo::InterfaceType type)
{
    reset();
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(2);;
    ui->tableWidget->setHorizontalHeaderItem(0,  new QTableWidgetItem(tr("Device name")));
    ui->tableWidget->setHorizontalHeaderItem(1,  new QTableWidgetItem(tr("Serial number")));
    switch(type) {
    case ReDeviceInfo::Serial:
    case ReDeviceInfo::NANO:
    {
        ui->tableWidget->horizontalHeaderItem(1)->setText(tr("Port name"));
        QList<ReDeviceInfo> list = ReDeviceInfo::availableDevices(ReDeviceInfo::Serial);
        NanovnaAnalyzer::detectPorts();
        int bluetooth_rows = 6;
        int rows = list.size() + NanovnaAnalyzer::portsCount() + bluetooth_rows;
        ui->tableWidget->setRowCount(rows);
        int row = 0;
        foreach (const ReDeviceInfo &info, list)
        {
            QString name = info.deviceName(info).replace("Analyzer", "", Qt::CaseInsensitive).trimmed();

            QTableWidgetItem* item = new QTableWidgetItem(name);
            item->setData(Qt::UserRole+1, (int)ReDeviceInfo::Serial);
            ui->tableWidget->setItem(row, 0, item);

            item = new QTableWidgetItem(info.portName().trimmed());
            ui->tableWidget->setItem(row, 1, item);
            row++;
        }
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        {
            showPortInfo(info);
            if (info.description().contains("Bluetooth", Qt::CaseInsensitive)) {
                QTableWidgetItem* item = new QTableWidgetItem("AA-55 ZOOM");
                item->setData(Qt::UserRole+1, (int)ReDeviceInfo::Serial);
                ui->tableWidget->setItem(row, 0, item);

                item = new QTableWidgetItem(info.portName().trimmed());
                ui->tableWidget->setItem(row, 1, item);
                row++;
            }
        }
        foreach (const QSerialPortInfo &info, NanovnaAnalyzer::availablePorts())
        {
            showPortInfo(info);
            QString name("NanoVNA");
            QTableWidgetItem* item = new QTableWidgetItem(name);
            item->setData(Qt::UserRole+1, (int)ReDeviceInfo::NANO);
            ui->tableWidget->setItem(row, 0, item);

            item = new QTableWidgetItem(info.portName().trimmed());
            ui->tableWidget->setItem(row, 1, item);
            row++;
        }
        ui->pushButtonConnect->setEnabled(row != 0);
        ui->pushButtonScan->setEnabled(true);
    }
        break;
    case ReDeviceInfo::BLE:    
        ui->pushButtonConnect->setEnabled(false);
        ui->pushButtonScan->setEnabled(false);
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_analyzer = new BleAnalyzer();
        ((BleAnalyzer*)m_analyzer)->searchAnalyzer();
        ui->tableWidget->horizontalHeaderItem(0)->setText(tr("Name"));
        ui->tableWidget->horizontalHeaderItem(1)->setText(tr("Address"));
        ui->tableWidget->setRowCount(10);
        connect((BleAnalyzer*)m_analyzer, &BleAnalyzer::devicesChanged, this, [=](BleDeviceInfo* info){
            BleAnalyzer* analyzer = (BleAnalyzer*)m_analyzer;
            auto dev = analyzer->devices();
            int row = dev.size()-1;
            QString name = info->getName();
            QTableWidgetItem* item = new QTableWidgetItem(name);
            item->setData(Qt::UserRole+1, (int)ReDeviceInfo::BLE);
            ui->tableWidget->setItem(row, 0, item);
            QString addr = info->getAddress().trimmed();
            item = new QTableWidgetItem(addr);
            ui->tableWidget->setItem(row, 1, item);
            //qDebug() << row << name << addr;
        });
        connect((BleAnalyzer*)m_analyzer, &BleAnalyzer::scanningChanged, this, [=](int state) {
            if (state == 0) {
                if (m_analyzer != nullptr) {
                    BleAnalyzer* analyzer = (BleAnalyzer*)m_analyzer;
                    auto dev = analyzer->devices();
                    if (dev.size() != 0) {
                        ui->pushButtonConnect->setEnabled(true);
                    }
                }
                ui->pushButtonScan->setEnabled(true);
            }
        });
        break;
    default:
    {
        ui->tableWidget->horizontalHeaderItem(1)->setText(tr("Serial number"));
        QList<ReDeviceInfo> list = ReDeviceInfo::availableDevices(ReDeviceInfo::HID);
        ui->tableWidget->setRowCount(list.size());
        int row = 0;
        foreach (const ReDeviceInfo &info, list)
        {
            QString name = info.systemName().replace("Analyzer", "", Qt::CaseInsensitive).trimmed();

            QTableWidgetItem* item = new QTableWidgetItem(name);
            item->setData(Qt::UserRole+1, (int)ReDeviceInfo::HID);
            ui->tableWidget->setItem(row, 0, item);

            item = new QTableWidgetItem(info.serial().trimmed());
            ui->tableWidget->setItem(row, 1, item);
            row++;
        }
        ui->pushButtonConnect->setEnabled(row != 0);
        ui->pushButtonScan->setEnabled(true);
    }
        break;
    }
    ui->tableWidget->setCurrentIndex(ui->tableWidget->model()->index(0, 0));
}

QString SelectDeviceDialog::scanSilent(QString& device_name)
{
    QString name = device_name;
    QString address;
    if (m_analyzer == nullptr)
        m_analyzer = new BleAnalyzer();
    m_foundBle = false;
    ((BleAnalyzer*)m_analyzer)->searchAnalyzer();
    connect((BleAnalyzer*)m_analyzer, &BleAnalyzer::devicesChanged, this, [=](BleDeviceInfo* info) {
        if (info != nullptr)
            qDebug() << "BleAnalyzer::devicesChanged" << info->getName() << info->getAddress();
        else
            qDebug() << "BleAnalyzer::devicesChanged NULL";
    });
    connect((BleAnalyzer*)m_analyzer, &BleAnalyzer::scanningChanged, this, [=, &address](int state) {
        if (state == 0) {
            BleAnalyzer* analyzer = (BleAnalyzer*)m_analyzer;
            auto devices = analyzer->devices();
            for (auto dev : devices) {
                if (dev->getName() == name) {
                    m_foundBle = true;
                    address = dev->getAddress();
                    qDebug() << "scanSilent 0" << address << analyzer->devices().size();
                    break;
                }
            }
        }
        qDebug() << "scanSilent 1" << address << ((BleAnalyzer*)m_analyzer)->devices().size();
        return address;
    });
    qDebug() << "scanSilent 0" << address << ((BleAnalyzer*)m_analyzer)->devices().size();
    return address;
}

ReDeviceInfo::InterfaceType SelectDeviceDialog::type()
{
    ReDeviceInfo::InterfaceType type = ReDeviceInfo::HID;
    if (ui->radioButtonCOM->isChecked())
        type = ReDeviceInfo::Serial;
    else if (ui->radioButtonBLE->isChecked())
        type = ReDeviceInfo::BLE;
    return type;
}

QString SelectDeviceDialog::name()
{
    QTableWidgetItem* item = ui->tableWidget->currentItem();
    return (item == nullptr ? "" : item->data(Qt::UserRole).toString());
}

bool SelectDeviceDialog::connectSilent(int _type, QString _device_name)
{
    QString port_or_serial;
    AnalyzerParameters* analyzer = AnalyzerParameters::byName(_device_name);
    if (analyzer == nullptr)
        return false;

    switch(_type) {
    case ReDeviceInfo::Serial:
    {
        QList<ReDeviceInfo> list = ReDeviceInfo::availableDevices(ReDeviceInfo::Serial);
        foreach (ReDeviceInfo info, list) {
            if (info.deviceName(info) == analyzer->name()) {
                port_or_serial = info.portName();
                break;
            }
        }
    }
        break;
    case ReDeviceInfo::NANO:
    {
        NanovnaAnalyzer::detectPorts();
        foreach (const QSerialPortInfo &info, NanovnaAnalyzer::availablePorts())
        {
            port_or_serial = info.portName().trimmed();
            break;
        }
    }
        break;
    case ReDeviceInfo::BLE: {
        QString address;
        int attempt = 3;
        while (!m_foundBle) {
            address = scanSilent(_device_name);
            if (--attempt == 0)
                break;
            QThread::msleep(500);
        }
        port_or_serial = address;
    }
        break;
    case ReDeviceInfo::HID:
    {
        QList<ReDeviceInfo> list = ReDeviceInfo::availableDevices(ReDeviceInfo::HID);
        foreach (const ReDeviceInfo &info, list)
        {
            if (info.systemName().replace("Analyzer", "", Qt::CaseInsensitive).trimmed() == _device_name) {
                port_or_serial = info.serial().trimmed();
                break;
            }
        }
    }
        break;
    }
    if (!port_or_serial.isEmpty()) {
        SelectionParameters::selected.name = _device_name;
        SelectionParameters::selected.type = (ReDeviceInfo::InterfaceType)_type;
        SelectionParameters::selected.id = port_or_serial;
        SelectionParameters::selected.modelIndex = analyzer->index();

        AnalyzerParameters::setCurrent(analyzer);
        return true;
    }
    return false;
}

void SelectDeviceDialog::reset()
{
    if (m_analyzer != nullptr) {
      BaseAnalyzer* tmp = m_analyzer;
      m_analyzer = nullptr;
      tmp->deleteLater();
    }
}


