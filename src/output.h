/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#ifndef OUTPUT_CONFIG_H
#define OUTPUT_CONFIG_H

#include "mode.h"
#include "kscreen_export.h"

#include <QtCore/QSize>
#include <QtCore/QPoint>
#include <QtCore/QPoint>
#include <QtCore/QObject>
#include <QtCore/QMetaType>
#include <QPointer>

namespace KScreen {

class Edid;

class KSCREEN_EXPORT Output : public QObject
{
    Q_OBJECT

    public:
        Q_PROPERTY(int id READ id NOTIFY outputChanged)
        Q_PROPERTY(QString name READ name WRITE setName NOTIFY outputChanged)
        Q_PROPERTY(QString type READ type WRITE setType NOTIFY outputChanged)
        Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY outputChanged)
        Q_PROPERTY(ModeList modes READ modes CONSTANT)
        Q_PROPERTY(QPoint pos READ pos WRITE setPos NOTIFY posChanged)
        Q_PROPERTY(Rotation rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
        Q_PROPERTY(int currentMode READ currentMode WRITE setCurrentMode NOTIFY currentModeChanged)
        Q_PROPERTY(int preferredMode READ preferredMode CONSTANT)
        Q_PROPERTY(bool connected READ isConnected WRITE setConnected NOTIFY isConnectedChanged)
        Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY isEnabledChanged)
        Q_PROPERTY(bool primary READ isPrimary WRITE setPrimary NOTIFY isPrimaryChanged)
        Q_PROPERTY(QList<int> clones READ clones WRITE setClones NOTIFY clonesChanged)
        Q_PROPERTY(KScreen::Edid* edid READ edid CONSTANT)

        Q_ENUMS(Rotation)

        enum Rotation {
            None = 1,
            Right = 2,
            Inverted = 4,
            Left = 8
        };

        explicit Output(QObject *parent = 0);
        virtual ~Output();

        int id() const;
        void setId(int id);

        QString name() const;
        void setName(const QString& name);

        QString type() const;
        void setType(const QString& type);

        QString icon() const;
        void setIcon(const QString& icon);

        Q_INVOKABLE Mode* mode(int id) const;
        QHash<int, Mode*> modes() const;
        void setModes(ModeList modes);

        int currentMode() const;
        void setCurrentMode(int mode);

        void setPreferredMode(int preferredMode);
        int preferredMode() const;

        QPoint pos() const;
        void setPos(const QPoint& pos);

        Rotation rotation() const;
        void setRotation(Rotation rotation);

        bool isConnected() const;
        void setConnected(bool connected);

        bool isEnabled() const;
        void setEnabled(bool enabled);

        bool isPrimary() const;
        void setPrimary(bool primary);

        QList<int> clones();
        void setClones(QList<int> outputlist);

        Edid* edid() const;

    Q_SIGNALS:
        void outputChanged();
        void posChanged();
        void currentModeChanged();
        void rotationChanged();
        void isConnectedChanged();
        void isEnabledChanged();
        void isPrimaryChanged();
        void clonesChanged();

    private:
        int m_id;
        QString m_name;
        QString m_type;
        QString m_icon;
        ModeList m_modeList;
        QList<int> m_clones;
        int m_currentMode;
        int m_preferredMode;
        QSize m_size;
        QPoint m_pos;
        Rotation m_rotation;
        bool m_connected;
        bool m_enabled;
        bool m_primary;
        mutable QPointer<Edid> m_edid;
};

typedef QHash<int, Output*> OutputList;

} //KScreen namespace

Q_DECLARE_METATYPE(KScreen::OutputList)

#endif //OUTPUT_H
