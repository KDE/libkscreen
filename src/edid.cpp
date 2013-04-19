/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti <dantti12@gmail.com>           *
 *             (C) 2012 by Dan Vratil <dvratil@redhat.com>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "edid.h"

#include <math.h>

#include <QCryptographicHash>
#include <QFile>
#include <QStringList>
#include <QStringBuilder>

#include <kdebug.h>

#define GCM_EDID_OFFSET_PNPID                           0x08
#define GCM_EDID_OFFSET_SERIAL                          0x0c
#define GCM_EDID_OFFSET_SIZE                            0x15
#define GCM_EDID_OFFSET_GAMMA                           0x17
#define GCM_EDID_OFFSET_DATA_BLOCKS                     0x36
#define GCM_EDID_OFFSET_LAST_BLOCK                      0x6c
#define GCM_EDID_OFFSET_EXTENSION_BLOCK_COUNT           0x7e

#define GCM_DESCRIPTOR_DISPLAY_PRODUCT_NAME             0xfc
#define GCM_DESCRIPTOR_DISPLAY_PRODUCT_SERIAL_NUMBER    0xff
#define GCM_DESCRIPTOR_COLOR_MANAGEMENT_DATA            0xf9
#define GCM_DESCRIPTOR_ALPHANUMERIC_DATA_STRING         0xfe
#define GCM_DESCRIPTOR_COLOR_POINT                      0xfb

#define PNP_IDS "/usr/share/hwdata/pnp.ids"

namespace KScreen {


class Edid::Private
{
  public:
    Private():
      valid(false),
      width(0),
      height(0),
      gamma(0)
    { }

    bool parse(const quint8 *data, size_t length);
    int edidGetBit(int in, int bit) const;
    int edidGetBits(int in, int begin, int end) const;
    double edidDecodeFraction(int high, int low) const;
    QString edidParseString(const quint8 *data) const;

    bool valid;
    QString monitorName;
    QString vendorName;
    QString serialNumber;
    QString eisaId;
    QString checksum;
    QString pnpId;
    uint width;
    uint height;
    qreal gamma;
    QQuaternion red;
    QQuaternion green;
    QQuaternion blue;
    QQuaternion white;
};


Edid::Edid()
  : QObject()
  , d(new Private())
{
}

Edid::Edid(const quint8 *data, size_t length, QObject *parent)
  : QObject(parent)
  , d(new Private())
{
    d->parse(data, length);
}

Edid::~Edid()
{
    delete d;
}

bool Edid::isValid() const
{
    return d->valid;
}

QString Edid::deviceId(const QString &fallbackName) const
{
    QString id = QLatin1String("xrandr");
    // if no info was added check if the fallbacName is provided
    if (vendor().isNull() && name().isNull() && serial().isNull()) {
        if (!fallbackName.isEmpty()) {
            id.append(QLatin1Char('-') % fallbackName);
        } else {
            // all info we have are empty strings
            id.append(QLatin1String("-unknown"));
        }
    } else if (d->valid) {
        if (!vendor().isNull()) {
            id.append(QLatin1Char('-') % vendor());
        }
        if (!name().isNull()) {
            id.append(QLatin1Char('-') % name());
        }
        if (!serial().isNull()) {
            id.append(QLatin1Char('-') % serial());
        }
    }

    return id;
}

QString Edid::name() const
{
    if (d->valid) {
        return d->monitorName;
    }
    return QString();
}

QString Edid::vendor() const
{
    if (d->valid) {
        return d->vendorName;
    }
    return QString();
}

QString Edid::serial() const
{
    if (d->valid) {
        return d->serialNumber;
    }
    return QString();
}

QString Edid::eisaId() const
{
    if (d->valid) {
        return d->eisaId;
    }
    return QString();
}

QString Edid::hash() const
{
    if (d->valid) {
        return d->checksum;
    }
    return QString();
}

QString Edid::pnpId() const
{
    if (d->valid) {
        return d->pnpId;
    }
    return QString();
}

uint Edid::width() const
{
    return d->width;
}

uint Edid::height() const
{
    return d->height;
}

qreal Edid::gamma() const
{
    return d->gamma;
}

QQuaternion Edid::red() const
{
    return d->red;
}

QQuaternion Edid::green() const
{
    return d->green;
}

QQuaternion Edid::blue() const
{
    return d->blue;
}

QQuaternion Edid::white() const
{
    return d->white;
}

bool Edid::Private::parse(const quint8 *data, size_t length)
{
    quint32 serial;

    /* check header */
    if (length < 128) {
        if (length > 0) {
            kWarning() << "Invalid EDID length (" << length << " bytes)";
        }
        valid = false;
        return valid;
    }
    if (data[0] != 0x00 || data[1] != 0xff) {
        kWarning() << "Failed to parse EDID header";
        valid = false;
        return valid;
    }

    /* decode the PNP ID from three 5 bit words packed into 2 bytes
     * /--08--\/--09--\
     * 7654321076543210
     * |\---/\---/\---/
     * R  C1   C2   C3 */
    pnpId[0] = 'A' + ((data[GCM_EDID_OFFSET_PNPID + 0] & 0x7c) / 4) - 1;
    pnpId[1] = 'A' + ((data[GCM_EDID_OFFSET_PNPID + 0] & 0x3) * 8) + ((data[GCM_EDID_OFFSET_PNPID+1] & 0xe0) / 32) - 1;
    pnpId[2] = 'A' + (data[GCM_EDID_OFFSET_PNPID + 1] & 0x1f) - 1;

    // load the PNP_IDS file and load the vendor name
    if (!pnpId.isEmpty()) {
        QFile pnpIds(PNP_IDS);
        if (pnpIds.open(QIODevice::ReadOnly)) {
            while (!pnpIds.atEnd()) {
                QString line = pnpIds.readLine();
                if (line.startsWith(pnpId)) {
                    QStringList parts = line.split(QLatin1Char('\t'));
                    if (parts.size() == 2) {
                        vendorName = line.split(QLatin1Char('\t')).at(1).simplified();
                    }
                    break;
                }
            }
        }
    }

    /* maybe there isn't a ASCII serial number descriptor, so use this instead */
    serial = static_cast<quint32>(data[GCM_EDID_OFFSET_SERIAL + 0]);
    serial += static_cast<quint32>(data[GCM_EDID_OFFSET_SERIAL + 1] * 0x100);
    serial += static_cast<quint32>(data[GCM_EDID_OFFSET_SERIAL + 2] * 0x10000);
    serial += static_cast<quint32>(data[GCM_EDID_OFFSET_SERIAL + 3] * 0x1000000);
    if (serial > 0) {
        serialNumber = QString::number(serial);
    }

    /* get the size */
    width = data[GCM_EDID_OFFSET_SIZE + 0];
    height = data[GCM_EDID_OFFSET_SIZE + 1];

    /* we don't care about aspect */
    if (width == 0 || height == 0) {
        width = 0;
        height = 0;
    }

    /* get gamma */
    if (data[GCM_EDID_OFFSET_GAMMA] == 0xff) {
        gamma = 1.0f;
    } else {
        gamma = (static_cast<float>(data[GCM_EDID_OFFSET_GAMMA] / 100) + 1);
    }

    /* get color red */
    red.setX(edidDecodeFraction(data[0x1b], edidGetBits(data[0x19], 6, 7)));
    red.setY(edidDecodeFraction(data[0x1c], edidGetBits(data[0x19], 5, 4)));

    /* get color green */
    green.setX(edidDecodeFraction(data[0x1d], edidGetBits(data[0x19], 2, 3)));
    green.setY(edidDecodeFraction(data[0x1e], edidGetBits(data[0x19], 0, 1)));

    /* get color blue */
    blue.setX(edidDecodeFraction(data[0x1f], edidGetBits(data[0x1a], 6, 7)));
    blue.setY(edidDecodeFraction(data[0x20], edidGetBits(data[0x1a], 4, 5)));

    /* get color white */
    white.setX(edidDecodeFraction(data[0x21], edidGetBits(data[0x1a], 2, 3)));
    white.setY(edidDecodeFraction(data[0x22], edidGetBits(data[0x1a], 0, 1)));

    /* parse EDID data */
    for (uint i = GCM_EDID_OFFSET_DATA_BLOCKS;
         i <= GCM_EDID_OFFSET_LAST_BLOCK;
         i += 18) {
        /* ignore pixel clock data */
        if (data[i] != 0) {
            continue;
        }
        if (data[i+2] != 0) {
            continue;
        }

        /* any useful blocks? */
        if (data[i+3] == GCM_DESCRIPTOR_DISPLAY_PRODUCT_NAME) {
            QString tmp = edidParseString(&data[i+5]);
            if (!tmp.isEmpty()) {
                monitorName = tmp;
            }
        } else if (data[i+3] == GCM_DESCRIPTOR_DISPLAY_PRODUCT_SERIAL_NUMBER) {
            QString tmp = edidParseString(&data[i+5]);
            if (!tmp.isEmpty()) {
                serialNumber = tmp;
            }
        } else if (data[i+3] == GCM_DESCRIPTOR_COLOR_MANAGEMENT_DATA) {
            kWarning() << "failing to parse color management data";
        } else if (data[i+3] == GCM_DESCRIPTOR_ALPHANUMERIC_DATA_STRING) {
            QString tmp = edidParseString(&data[i+5]);
            if (!tmp.isEmpty()) {
                eisaId = tmp;
            }
        } else if (data[i+3] == GCM_DESCRIPTOR_COLOR_POINT) {
            if (data[i+3+9] != 0xff) {
                /* extended EDID block(1) which contains
                                     * a better gamma value */
                gamma = ((float) data[i+3+9] / 100) + 1;
            }
            if (data[i+3+14] != 0xff) {
                /* extended EDID block(2) which contains
                                     * a better gamma value */
                gamma = ((float) data[i+3+9] / 100) + 1;
            }
        }
    }

    // calculate checksum
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(reinterpret_cast<const char *>(data), length);
    checksum = hash.result().toHex();

    valid = true;
    return valid;
}

int Edid::Private::edidGetBit(int in, int bit) const
{
        return (in & (1 << bit)) >> bit;
}

int Edid::Private::edidGetBits(int in, int begin, int end) const
{
        int mask = (1 << (end - begin + 1)) - 1;

        return (in >> begin) & mask;
}

double Edid::Private::edidDecodeFraction(int high, int low) const
{
        double result = 0.0;
        int i;

        high = (high << 2) | low;
        for (i = 0; i < 10; ++i) {
            result += edidGetBit(high, i) * pow(2, i - 10);
        }
        return result;
}

QString Edid::Private::edidParseString(const quint8 *data) const
{
        QString text;

        /* this is always 12 bytes, but we can't guarantee it's null
         * terminated or not junk. */
        text = QString::fromLocal8Bit((const char*) data, 12);

        // Remove newlines, extra spaces and stuff
        text = text.simplified();

        return text;
}

} /* namespace KScreen */

#include "edid.moc"
