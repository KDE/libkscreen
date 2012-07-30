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

#ifndef OUTPUT_H
#define OUTPUT_H

#include "kscreen_export.h"
#include "mode.h"

#include <QtCore/QObject>
#include <QtCore/QSize>
#include <QtCore/QPoint>
#include <QtCore/QHash>
#include <QtCore/QMetaType>

class Mode;
class KSCREEN_EXPORT Output : public QObject
{
    Q_OBJECT

    public:
        Q_PROPERTY(QString name READ name WRITE setName)
        Q_PROPERTY(QString type READ type WRITE setType)
        Q_PROPERTY(QString icon READ icon WRITE setIcon)
        Q_PROPERTY(ModeList modes READ modes)
        Q_PROPERTY(QPoint pos READ pos WRITE setPos)
        Q_PROPERTY(QSize size READ size WRITE setSize)
        Q_PROPERTY(Rotation rotation READ rotation WRITE setRotation)
        Q_PROPERTY(int id READ id WRITE setId)
        Q_PROPERTY(bool connected READ isConnected WRITE setConnected)
        Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
        Q_PROPERTY(bool primary READ isPrimary WRITE setPrimary)

        Q_ENUMS(Rotation)

        enum Rotation {
            None = 0x0,
            Inverted = 0x1,
            Left = 0x2,
            Right = 0x3
        };

        explicit Output(int id, QObject* parent = 0);
        virtual ~Output();

        QString name() const;
        void setName(const QString& name);

        QString type() const;
        void setType(const QString& type);

        QString icon() const;
        void setIcon(const QString& icon);

        QHash<int, Mode*> modes() const;
        void setModes(ModeList modes);

        Mode* currentMode() const;
        void setCurrentMode(Mode* mode);

        QPoint pos() const;
        void setPos(const QPoint& pos);

        QSize size() const;
        void setSize(const QSize& size);

        Rotation rotation() const;
        void setRotation(Rotation rotation);

        int id();
        void setId(int id);

        bool isConnected() const;
        void setConnected(bool connected);

        bool isEnabled() const;
        void setEnabled(bool enabled);

        bool isPrimary() const;
        void setPrimary(bool primary);

        QHash<int, Output*> clones();
        void setClones(QHash<int, Output*> outputlist);

    private:
        int m_id;
        QString m_name;
        QString m_type;
        QString m_icon;
        ModeList m_modeList;
        QHash<int, Output*> m_clones;
        Mode* m_currentMode;
        QSize m_size;
        QPoint m_pos;
        Rotation m_rotation;
        bool m_connected;
        bool m_enabled;
        bool m_primary;
};

typedef QHash<int, Output*> OutputList;
Q_DECLARE_METATYPE(OutputList);

#endif //OUTPUT_H
