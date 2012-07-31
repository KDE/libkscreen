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

#ifndef KSCREEN_H
#define KSCREEN_H

#include "kscreen_export.h"

#include <QtCore/QString>

class Config;
class AbstractBackend;
class KSCREEN_EXPORT KScreen
{
    public:
        enum Error {
            None = 0x0,
            NoCompatibleBackend = 0x1
        };

        /**
         * Returns an instance of KScreen as a pointer. The library is in charge of the memory
         * you should not delete this pointer.
         * @return KScreen* instance or 0 if in error
         */
        static KScreen* self();

        static Error error();
        static const QString errorString();

        /**
         * Under some circumstances the object won't be valid or will be invalidated. A possible
         * reason is if no valid backend is found
         * @return Wherever the instance is valid or not
         */
        bool isValid();

        const QString backend();

        Config* config();
    private:
        KScreen();

    private:
        static Error s_error;
        static KScreen* s_instance;

        AbstractBackend *m_backend;
};

#endif //KSCREEN_H