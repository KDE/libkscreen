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
#include <QtCore/QObject>
#include <QtCore/QMetaType>
#include <QtCore/QPointer>
#include <QtCore/QStringList>

namespace KScreen {

class Edid;

class KSCREEN_EXPORT Output : public QObject
{
    Q_OBJECT

    public:
        Q_ENUMS(Rotation)
        Q_ENUMS(Type)
        Q_PROPERTY(int id READ id NOTIFY outputChanged)
        Q_PROPERTY(QString name READ name WRITE setName NOTIFY outputChanged)
        Q_PROPERTY(Type type READ type WRITE setType NOTIFY outputChanged)
        Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY outputChanged)
        Q_PROPERTY(ModeList modes READ modes CONSTANT)
        Q_PROPERTY(QPoint pos READ pos WRITE setPos NOTIFY posChanged)
        Q_PROPERTY(Rotation rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
        Q_PROPERTY(QString currentModeId READ currentModeId WRITE setCurrentModeId NOTIFY currentModeIdChanged)
        Q_PROPERTY(QString preferredModeId READ preferredModeId CONSTANT)
        Q_PROPERTY(bool connected READ isConnected WRITE setConnected NOTIFY isConnectedChanged)
        Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY isEnabledChanged)
        Q_PROPERTY(bool primary READ isPrimary WRITE setPrimary NOTIFY isPrimaryChanged)
        Q_PROPERTY(QList<int> clones READ clones WRITE setClones NOTIFY clonesChanged)
        Q_PROPERTY(KScreen::Edid* edid READ edid CONSTANT)

        enum Type {
            Unknown,
            VGA,
            DVI,
            DVII,
            DVIA,
            DVID,
            HDMI,
            Panel,
            TV,
            TVComposite,
            TVSVideo,
            TVComponent,
            TVSCART,
            TVC4,
            DisplayPort
        };

        enum Rotation {
            None = 1,
            Left = 2,
            Inverted = 4,
            Right = 8
        };

        explicit Output(QObject *parent = 0);
        virtual ~Output();

        int id() const;
        void setId(int id);

        QString name() const;
        void setName(const QString& name);

        Type type() const;
        void setType(Type type);

        QString icon() const;
        void setIcon(const QString& icon);

        Q_INVOKABLE Mode* mode(const QString &id) const;
        QHash<QString, Mode*> modes() const;
        void setModes(ModeList modes);

        QString currentModeId() const;
        void setCurrentModeId(const QString& mode);
        Q_INVOKABLE Mode* currentMode() const;

        void setPreferredModes(const QStringList &modes);
        QStringList preferredModes() const;
        /**
         * Returns the preferred mode with higer resolution and refresh
         */
        Q_INVOKABLE QString preferredModeId() const;
        /**
         * Returns KScreen::Mode associated with preferredModeId()
         */
        Q_INVOKABLE Mode* preferredMode() const;

        QPoint pos() const;
        void setPos(const QPoint& pos);

        Rotation rotation() const;
        void setRotation(Rotation rotation);
        /**
         * A comfortable function that returns true when output is not rotated
         * or is rotated upside down.
         */
        Q_INVOKABLE inline bool isHorizontal() const {
            return ((rotation() == Output::None) || (rotation() == Output::Inverted));
        }

        bool isConnected() const;
        void setConnected(bool connected);

        bool isEnabled() const;
        void setEnabled(bool enabled);

        bool isPrimary() const;
        void setPrimary(bool primary);

        QList<int> clones() const;
        void setClones(QList<int> outputlist);

        Edid* edid() const;

    Q_SIGNALS:
        void outputChanged();
        void posChanged();
        void currentModeIdChanged();
        void rotationChanged();
        void isConnectedChanged();
        void isEnabledChanged();
        void isPrimaryChanged();
        void clonesChanged();

    private:
        Q_DISABLE_COPY(Output)

        class Private;
        Private * const d;

};

typedef QHash<int, Output*> OutputList;

} //KScreen namespace

Q_DECLARE_METATYPE(KScreen::OutputList)
Q_DECLARE_METATYPE(KScreen::Output::Rotation)
Q_DECLARE_METATYPE(KScreen::Output::Type)

#endif //OUTPUT_H
