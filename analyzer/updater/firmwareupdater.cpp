#include "firmwareupdater.h"

FirmwareUpdater::FirmwareUpdater(QObject *parent):
    QObject(parent)

{

}

FirmwareUpdater::~FirmwareUpdater()
{

}

QString FirmwareUpdater::lastError() const
{
    return m_error;
}
