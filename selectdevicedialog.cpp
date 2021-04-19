#include "selectdevicedialog.h"
#include "ui_selectdevicedialog.h"
#include "settings.h"
#include "nanovna_analyzer.h"

// static
SelectionParameters SelectionParameters::selected;


SelectDeviceDialog::SelectDeviceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectDeviceDialog)
{
    ui->setupUi(this);

    QString path = Settings::setIniFile();
    QSettings set(path, QSettings::IniFormat);
    set.beginGroup("Connection");
    bool same = set.value("same", false).toBool();
    int _type = set.value("type", ReDeviceInfo::HID).toInt();
    QString name = set.value("name", "").toString();
    set.endGroup();

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

    connect(ui->radioButtonUSB, &QRadioButton::toggled, [this](bool checked) {
        onScan(ReDeviceInfo::HID);
    });
    connect(ui->radioButtonCOM, &QRadioButton::toggled, [&](bool checked) {
        onScan(ReDeviceInfo::Serial);
    });
    connect(ui->radioButtonBLE, &QRadioButton::toggled, [=](bool checked) {
        onScan(ReDeviceInfo::BLE);
    });
    connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, [&](QTableWidgetItem* _item) {
        if (_item == nullptr)
            return;
        QTableWidgetItem* item = ui->tableWidget->item(_item->row(), 0);
        QTableWidgetItem* column = ui->tableWidget->item(_item->row(), 1);
        onApply((ReDeviceInfo::InterfaceType)item->data(Qt::UserRole+1).toInt(),
                item->data(Qt::DisplayRole).toString(),
                column->data(Qt::DisplayRole).toString());
    });
    connect(ui->checkBox, &QCheckBox::toggled, [&](bool checked){
        QString path = Settings::setIniFile();
        QSettings set(path, QSettings::IniFormat);
        set.beginGroup("Connection");
        set.setValue("same", checked);
        set.endGroup();
    });
    connect(ui->pushButtonScan, &QPushButton::clicked, [&](){
        onScan(type());
    });
    connect(ui->pushButtonConnect, &QPushButton::clicked, [&]{
        QTableWidgetItem* item = ui->tableWidget->currentItem();
        if (item == nullptr)
            return;
        QTableWidgetItem* column = ui->tableWidget->item(item->row(), 1);
        onApply((ReDeviceInfo::InterfaceType)item->data(Qt::UserRole+1).toInt(),
                item->data(Qt::DisplayRole).toString(),
                column->data(Qt::DisplayRole).toString());
    });
    connect(ui->pushButtonCancel, &QPushButton::clicked, [&]{
        this->reject();
    });
    onScan(_type);
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
    qDebug() << "SelectDeviceDialog::onApply" << (int)type << name;
    AnalyzerParameters* param=nullptr;
    if (type == ReDeviceInfo::HID) {
        int prefix = AnalyzerParameters::prefixFromSerial(port_or_serial);
        param = AnalyzerParameters::byPrefix(prefix);
        if (prefix == 0 || param == nullptr) {
            QMessageBox::warning(this, tr("Select deice"), tr("Serial number does not match the type of device"));
            return;
        }
    } else if (type == ReDeviceInfo::Serial || type == ReDeviceInfo::NANO) {
        param = AnalyzerParameters::byName(name);
    } else if (type == ReDeviceInfo::BLE) {
        // TODO
    }
    if (param == nullptr)
        return;
    SelectionParameters::selected.name = param->name();
    SelectionParameters::selected.type = type;
    SelectionParameters::selected.port = port_or_serial;
    SelectionParameters::selected.modelIndex = param->index();

    AnalyzerParameters::setCurrent(param);

    QString path = Settings::setIniFile();
    QSettings set(path, QSettings::IniFormat);
    set.beginGroup("Connection");
    set.setValue("same", ui->checkBox->isChecked());
    set.setValue("type", SelectionParameters::selected.type);
    set.setValue("name", SelectionParameters::selected.name);
    set.endGroup();

    QDialog::accept();
}

extern void showPortInfo(const QSerialPortInfo& info);

void SelectDeviceDialog::onScan(int type)
{
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

// TODO
//        if (rows == 0) {
//            list.append(ReDeviceInfo(ReDeviceInfo::Serial, "AA-30 ZERO", "AA-30 ZERO"));
//        }
// TODO

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
            QString desc = info.description();
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
    }
        break;
    case ReDeviceInfo::BLE:
        ui->tableWidget->horizontalHeaderItem(1)->setText(tr("Address"));
        ui->tableWidget->setRowCount(0);
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
    }
        break;
    }
    ui->tableWidget->setCurrentIndex(ui->tableWidget->model()->index(0, 0));
}

int SelectDeviceDialog::type()
{
    int type = ReDeviceInfo::HID;
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
    case ReDeviceInfo::BLE:
//        ui->tableWidget->horizontalHeaderItem(1)->setText(tr("Address"));
//        ui->tableWidget->setRowCount(0);
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
        SelectionParameters::selected.port = port_or_serial;
        SelectionParameters::selected.modelIndex = analyzer->index();

        AnalyzerParameters::setCurrent(analyzer);
        return true;
    }
    return false;
}
