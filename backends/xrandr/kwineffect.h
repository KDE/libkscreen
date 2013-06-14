/*************************************************************************************
 *  Copyright (C) 2013 �lex Fiestas <afiestas@kde.org>                               *
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

#ifndef KWIN_EFFECT_H
#define KWIN_EFFECT_H

#include "xlibandxrandr.h"

#include <QWidget>
#include <QtCore/QTimer>

class KWinEffect : public QWidget
{
    Q_OBJECT

    public:
        enum State {
            Nonthing,
            FadeIn,
            Faded,
            FadeOut,
            Done
        };

        KWinEffect();
        virtual ~KWinEffect();

        void start();
        bool isValid() const;
    public Q_SLOTS:
        void stop();

    Q_SIGNALS:
        void stateChanged(KWinEffect::State state);

    protected:
        virtual bool x11Event(XEvent *event);

        State m_state;
        bool m_isValid;
        Atom m_kwinEffect;
        Window m_rootWindow;
};

#endif // KWIN_EFFECT_H
