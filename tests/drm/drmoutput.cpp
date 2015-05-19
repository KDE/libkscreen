/*************************************************************************************
 *  Copyright 2015 Sebastian Kügler <sebas@kde.org>                                  *
 *  Copyright 2015 Martin Gräßlin <mgraesslin@kde.org>                               *
 *                                                                                   *
 *  This library is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU Lesser General Public                       *
 *  License as published by the Free Software Foundation; either                     *
 *  version 2.1 of the License, or (at your option) any later version.               *
 *                                                                                   *
 *  This library is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                *
 *  Lesser General Public License for more details.                                  *
 *                                                                                   *
 *  You should have received a copy of the GNU Lesser General Public                 *
 *  License along with this library; if not, write to the Free Software              *
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA       *
 *************************************************************************************/

#include "drmoutput.h"

#include "drmbackend.h"

#include <QDebug>
#include <QLoggingCategory>

#include <QSocketNotifier>

// drm
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm_mode.h>


using namespace KScreen;

DrmOutput::DrmOutput(DrmBackend *backend) :
    m_backend(backend),
    m_kscreenEdid(0)

{
    qCDebug(KSCREEN_WAYLAND) << "new DrmOutput";
}

DrmOutput::~DrmOutput()
{
//     hideCursor();
//     cleanupBlackBuffer();
    delete m_kscreenEdid;
}

KScreen::Edid* DrmOutput::edid()
{
    if (!m_kscreenEdid) {
    }
    return m_kscreenEdid;

}


void DrmOutput::hideCursor()
{
    //drmModeSetCursor(m_backend->fd(), m_crtcId, 0, 0, 0);
}

void DrmOutput::restoreSaved()
{
    if (!m_savedCrtc.isNull()) {
//         drmModeSetCrtc(m_backend->fd(), m_savedCrtc->crtc_id, m_savedCrtc->buffer_id,
//                        m_savedCrtc->x, m_savedCrtc->y, &m_connector, 1, &m_savedCrtc->mode);
    }
}

// void DrmOutput::showCursor(DrmBuffer *c)
// {
//     const QSize &s = c->size();
//     drmModeSetCursor(m_backend->fd(), m_crtcId, c->handle(), s.width(), s.height());
// }

void DrmOutput::moveCursor(const QPoint &globalPos)
{
//     const QPoint p = globalPos - m_globalPos;
//     drmModeMoveCursor(m_backend->fd(), m_crtcId, p.x(), p.y());
}

QSize DrmOutput::size() const
{
    return QSize(m_mode.hdisplay, m_mode.vdisplay);
}

QRect DrmOutput::geometry() const
{
    return QRect(m_globalPos, size());
}

// bool DrmOutput::present(DrmBuffer *buffer)
// {
//     if (!buffer || buffer->bufferId() == 0) {
//         return false;
//     }
//     if (!VirtualTerminal::self()->isActive()) {
//         m_currentBuffer = buffer;
//         return false;
//     }
//     if (m_currentBuffer) {
//         return false;
//     }
//     if (m_lastStride != buffer->stride()) {
//         // need to set a new mode first
//         if (!setMode(buffer)) {
//             return false;
//         }
//     }
//     const bool ok = drmModePageFlip(m_backend->fd(), m_crtcId, buffer->bufferId(), DRM_MODE_PAGE_FLIP_EVENT, this) == 0;
//     if (ok) {
// //         m_currentBuffer = buffer;
//     } else {
//         qWarning(KWIN_DRM) << "Page flip failed";
//         buffer->releaseGbm();
//     }
//     return ok;
// }

void DrmOutput::pageFlipped()
{
//     if (!m_currentBuffer) {
//         return;
//     }
//     m_currentBuffer->releaseGbm();
//     m_currentBuffer = nullptr;
//     cleanupBlackBuffer();
}

// void DrmOutput::cleanupBlackBuffer()
// {
//     if (m_blackBuffer) {
//         delete m_blackBuffer;
//         m_blackBuffer = nullptr;
//     }
// }

void DrmOutput::init(drmModeConnector *connector)
{
    initEdid(connector);
//     m_savedCrtc.reset(drmModeGetCrtc(m_backend->fd(), m_crtcId));
//     blank();
    //m_waylandOutput.reset(waylandServer()->display()->createOutput());
    QString manufacturer;
    if (!m_edid.eisaId.isEmpty()) {
        manufacturer = QString::fromLatin1(m_edid.eisaId);
    } else {
        manufacturer = QStringLiteral("unknown");
    }
    QString model;
    if (!m_edid.monitorName.isEmpty()) {
        model = QString::fromLatin1(m_edid.monitorName);
        if (!m_edid.serialNumber.isEmpty()) {
            model.append('/');
            model.append(QString::fromLatin1(m_edid.serialNumber));
        }
//         m_waylandOutput->setModel(model);
    } else if (!m_edid.serialNumber.isEmpty()) {
        model = QString::fromLatin1(m_edid.serialNumber);
    } else {
        model = "unknown";
    }
    QSize physicalSize = !m_edid.physicalSize.isEmpty() ? m_edid.physicalSize : QSize(connector->mmWidth, connector->mmHeight);
    // the size might be completely borked. E.g. Samsung SyncMaster 2494HS reports 160x90 while in truth it's 520x292
    // as this information is used to calculate DPI info, it's going to result in everything being huge
    // This can only be reliably done if we know the output's model
//     if (m_waylandOutput->model() != i18n("unknown")) {
//         KSharedConfig::Ptr config = KSharedConfig::openConfig(KWIN_CONFIG);
//         KConfigGroup group = config->group("EdidOverwrite").group(m_edid.eisaId).group(m_edid.monitorName).group(m_edid.serialNumber);
//         if (group.hasKey("PhysicalSize")) {
//             const QSize overwriteSize = group.readEntry("PhysicalSize", physicalSize);
//             qCWarning(KWIN_DRM) << "Overwriting monitor physical size for" << m_edid.eisaId << "/" << m_edid.monitorName << "/" << m_edid.serialNumber << " from " << physicalSize << "to " << overwriteSize;
//             physicalSize = overwriteSize;
//         }
//     }
//     m_waylandOutput->setPhysicalSize(physicalSize);

    qDebug() << "   model: " << manufacturer << model << physicalSize << m_edid.monitorName << m_edid.serialNumber;
    // read in mode information
    for (int i = 0; i < connector->count_modes; ++i) {
        auto *m = &connector->modes[i];
//         KWayland::Server::OutputInterface::ModeFlags flags;
//         if (isCurrentMode(m)) {
//             flags |= KWayland::Server::OutputInterface::ModeFlag::Current;
//         }
//         if (m->type & DRM_MODE_TYPE_PREFERRED) {
//             flags |= KWayland::Server::OutputInterface::ModeFlag::Preferred;
//         }

        // Calculate higher precision (mHz) refresh rate
        // logic based on Weston, see compositor-drm.c
        quint64 refreshRate = (m->clock * 1000000LL / m->htotal + m->vtotal / 2) / m->vtotal;
        if (m->flags & DRM_MODE_FLAG_INTERLACE) {
            refreshRate *= 2;
        }
        if (m->flags & DRM_MODE_FLAG_DBLSCAN) {
            refreshRate /= 2;
        }
        if (m->vscan > 1) {
            refreshRate /= m->vscan;
        }
        qDebug() << "       Mode: " << QSize(m->hdisplay, m->vdisplay) << refreshRate;
        //         m_waylandOutput->addMode(QSize(m->hdisplay, m->vdisplay), flags, refreshRate);
    }
//     m_waylandOutput->create();
}

bool DrmOutput::isCurrentMode(const drmModeModeInfo *mode) const
{
    return mode->clock   == m_mode.clock
    && mode->hdisplay    == m_mode.hdisplay
    && mode->hsync_start == m_mode.hsync_start
    && mode->hsync_end   == m_mode.hsync_end
    && mode->htotal      == m_mode.htotal
    && mode->hskew       == m_mode.hskew
    && mode->vdisplay    == m_mode.vdisplay
    && mode->vsync_start == m_mode.vsync_start
    && mode->vsync_end   == m_mode.vsync_end
    && mode->vtotal      == m_mode.vtotal
    && mode->vscan       == m_mode.vscan
    && mode->vrefresh    == m_mode.vrefresh
    && mode->flags       == m_mode.flags
    && mode->type        == m_mode.type
    && qstrcmp(mode->name, m_mode.name) == 0;
}

void DrmOutput::blank()
{
//     if (!m_blackBuffer) {
//         m_blackBuffer = m_backend->createBuffer(size());
//         m_blackBuffer->map();
//         m_blackBuffer->image()->fill(Qt::black);
//     }
//     setMode(m_blackBuffer);
}

// bool DrmOutput::setMode(DrmBuffer *buffer)
// {
//     if (drmModeSetCrtc(m_backend->fd(), m_crtcId, buffer->bufferId(), 0, 0, &m_connector, 1, &m_mode) == 0) {
//         m_lastStride = buffer->stride();
//         return true;
//     } else {
//         qCWarning(KWIN_DRM) << "Mode setting failed";
//         return false;
//     }
// }

static bool verifyEdidHeader(drmModePropertyBlobPtr edid)
{
    const uint8_t *data = reinterpret_cast<uint8_t*>(edid->data);
    if (data[0] != 0x00) {
        return false;
    }
    for (int i = 1; i < 7; ++i) {
        if (data[i] != 0xFF) {
            return false;
        }
    }
    if (data[7] != 0x00) {
        return false;
    }
    return true;
}

static QByteArray extractEisaId(drmModePropertyBlobPtr edid)
{
    /*
     * From EDID standard section 3.4:
     * The ID Manufacturer Name field, shown in Table 3.5, contains a 2-byte representation of the monitor's
     * manufacturer. This is the same as the EISA ID. It is based on compressed ASCII, “0001=A” ... “11010=Z”.
     *
     * The table:
     * | Byte |        Bit                    |
     * |      | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
     * ----------------------------------------
     * |  1   | 0)| (4| 3 | 2 | 1 | 0)| (4| 3 |
     * |      | * |    Character 1    | Char 2|
     * ----------------------------------------
     * |  2   | 2 | 1 | 0)| (4| 3 | 2 | 1 | 0)|
     * |      | Character2|      Character 3  |
     * ----------------------------------------
     **/
    const uint8_t *data = reinterpret_cast<uint8_t*>(edid->data);
    static const uint offset = 0x8;
    char id[4];
    if (data[offset] >> 7) {
        // bit at position 7 is not a 0
        return QByteArray();
    }
    // shift two bits to right, and with 7 right most bits
    id[0] = 'A' + ((data[offset] >> 2) & 0x1f) -1;
    // for first byte: take last two bits and shift them 3 to left (000xx000)
    // for second byte: shift 5 bits to right and take 3 right most bits (00000xxx)
    // or both together
    id[1] = 'A' + (((data[offset] & 0x3) << 3) | ((data[offset + 1] >> 5) & 0x7)) - 1;
    // take five right most bits
    id[2] = 'A' + (data[offset + 1] & 0x1f) - 1;
    id[3] = '\0';
    return QByteArray(id);
}

static void extractMonitorDescriptorDescription(drmModePropertyBlobPtr blob, DrmOutput::Edid &edid)
{
    // see section 3.10.3
    const uint8_t *data = reinterpret_cast<uint8_t*>(blob->data);
    static const uint offset = 0x36;
    static const uint blockLength = 18;
    for (int i = 0; i < 5; ++i) {
        const uint co = offset + i * blockLength;
        // Flag = 0000h when block used as descriptor
        if (data[co] != 0) {
            continue;
        }
        if (data[co + 1] != 0) {
            continue;
        }
        // Reserved = 00h when block used as descriptor
        if (data[co + 2] != 0) {
            continue;
        }
        /*
         * FFh: Monitor Serial Number - Stored as ASCII, code page # 437, ≤ 13 bytes.
         * FEh: ASCII String - Stored as ASCII, code page # 437, ≤ 13 bytes.
         * FDh: Monitor range limits, binary coded
         * FCh: Monitor name, stored as ASCII, code page # 437
         * FBh: Descriptor contains additional color point data
         * FAh: Descriptor contains additional Standard Timing Identifications
         * F9h - 11h: Currently undefined
         * 10h: Dummy descriptor, used to indicate that the descriptor space is unused
         * 0Fh - 00h: Descriptor defined by manufacturer.
         */
        if (data[co + 3] == 0xfc && edid.monitorName.isEmpty()) {
            edid.monitorName = QByteArray((const char *)(&data[co + 5]), 12).trimmed();
        }
        if (data[co + 3] == 0xfe) {
            const QByteArray id = QByteArray((const char *)(&data[co + 5]), 12).trimmed();
            if (!id.isEmpty()) {
                edid.eisaId = id;
            }
        }
        if (data[co + 3] == 0xff) {
            edid.serialNumber = QByteArray((const char *)(&data[co + 5]), 12).trimmed();
        }
    }
}

static QByteArray extractSerialNumber(drmModePropertyBlobPtr edid)
{
    // see section 3.4
    const uint8_t *data = reinterpret_cast<uint8_t*>(edid->data);
    static const uint offset = 0x0C;
    /*
     * The ID serial number is a 32-bit serial number used to differentiate between individual instances of the same model
     * of monitor. Its use is optional. When used, the bit order for this field follows that shown in Table 3.6. The EDID
     * structure Version 1 Revision 1 and later offer a way to represent the serial number of the monitor as an ASCII string
     * in a separate descriptor block.
     */
    uint32_t serialNumber = 0;
    serialNumber  = (uint32_t) data[offset + 0];
    serialNumber |= (uint32_t) data[offset + 1] << 8;
    serialNumber |= (uint32_t) data[offset + 2] << 16;
    serialNumber |= (uint32_t) data[offset + 3] << 24;
    if (serialNumber == 0) {
        return QByteArray();
    }
    return QByteArray::number(serialNumber);
}

static QSize extractPhysicalSize(drmModePropertyBlobPtr edid)
{
    const uint8_t *data = reinterpret_cast<uint8_t*>(edid->data);
    return QSize(data[0x15], data[0x16]) * 10;
}

void DrmOutput::initEdid(drmModeConnector *connector)
{
    ScopedDrmPointer<_drmModePropertyBlob, &drmModeFreePropertyBlob> edid;
    for (int i = 0; i < connector->count_props; ++i) {
        ScopedDrmPointer<_drmModeProperty, &drmModeFreeProperty> property(drmModeGetProperty(m_backend->fd(), connector->props[i]));
        if (!property) {
            continue;
        }
        if ((property->flags & DRM_MODE_PROP_BLOB) && qstrcmp(property->name, "EDID") == 0) {
            edid.reset(drmModeGetPropertyBlob(m_backend->fd(), connector->prop_values[i]));
        }
    }
    if (!edid) {
        return;
    }

    // for documentation see: http://read.pudn.com/downloads110/ebook/456020/E-EDID%20Standard.pdf
    if (edid->length < 128) {
        return;
    }
    if (!verifyEdidHeader(edid.data())) {
        return;
    }
    m_edid.eisaId = extractEisaId(edid.data());
    m_edid.serialNumber = extractSerialNumber(edid.data());

    // parse monitor descriptor description
    extractMonitorDescriptorDescription(edid.data(), m_edid);

    m_edid.physicalSize = extractPhysicalSize(edid.data());

    //const uint8_t *data = reinterpret_cast<uint8_t*>(edid->data);

    QByteArray edid_data;
    // Allow 7 extended edid blocks
    size_t len = 1024;
    quint8 *data = reinterpret_cast<uint8_t*>(edid->data);
    if (data) {
        edid_data = QByteArray((char *) data, len);
        qCDebug(KSCREEN_WAYLAND) << " edid data" << edid_data;
    } else {
        edid_data = QByteArray();
    }

    m_kscreenEdid = new KScreen::Edid(edid_data, m_backend);
    qCDebug(KSCREEN_WAYLAND) << " edid eisaId" << m_kscreenEdid->eisaId() << " (" << m_edid.eisaId << ")";
    qCDebug(KSCREEN_WAYLAND) << " edid name" << m_kscreenEdid->name();
    qCDebug(KSCREEN_WAYLAND) << " edid vendor" << m_kscreenEdid->vendor();
}
