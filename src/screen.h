/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
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

#ifndef SCREEN_CONFIG_H
#define SCREEN_CONFIG_H

#include "types.h"
#include "kscreen_export.h"

#include <QSize>
#include <QObject>

namespace KScreen {

class KSCREEN_EXPORT Screen : public QObject
{
    Q_OBJECT

    public:
        Q_PROPERTY(int id READ id CONSTANT)
        Q_PROPERTY(QSize currentSize READ currentSize WRITE setCurrentSize NOTIFY currentSizeChanged)
        Q_PROPERTY(QSize minSize READ minSize CONSTANT)
        Q_PROPERTY(QSize maxSize READ maxSize CONSTANT)
        Q_PROPERTY(int maxActiveOutputsCount READ maxActiveOutputsCount CONSTANT)

        explicit Screen();
        virtual ~Screen();

        ScreenPtr clone() const;

        /**
         * The id of this screen.
         * @return Minimum screen size in pixels
         */
        int id() const;
        /**
         * The identifier of this screen.
         * @param id id of the screen
         */
        void setId(int id);

        /**
         * The current screen size in pixels.
         * @return Screen size in pixels
         */
        QSize currentSize() const;
        /**
         * Set the current screen size in pixels.
         * @param currentSize Screen size in pixels
         */
        void setCurrentSize(const QSize& currentSize);

        /**
         * The minimum screen size in pixels.
         * @return Minimum screen size in pixels
         */
        QSize minSize() const;
        /**
         * Set the minimum screen size in pixels.
         * @param minSize Minimum screen size in pixels
         */
        void setMinSize(const QSize& minSize);

        /**
         * The maximum screen size in pixels.
         * @return Maximum screen size in pixels
         */
        QSize maxSize() const;
        /**
         * Set the maximum screen size in pixels.
         * @param maxSize Maximum screen size in pixels
         */
        void setMaxSize(const QSize& maxSize);

        int maxActiveOutputsCount() const;
        void setMaxActiveOutputsCount(int maxActiveOutputsCount);

        void apply(const ScreenPtr &other);

    Q_SIGNALS:
        void currentSizeChanged();

    private:
        Q_DISABLE_COPY(Screen)

        class Private;
        Private * const d;

        Screen(Private *dd);
};

} //KScreen namespace
#endif //SCREEN_H
