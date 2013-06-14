/*************************************************************************************
 *  Copyright (C) 2013 Àlex Fiestas <afiestas@kde.org>                               *
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

#include "kwineffect.h"
#include "xlibandxrandr.h"

#include <QX11Info>

#include <KDebug>
#include <KSystemEventFilter>

KWinEffect::KWinEffect()
 : QWidget()
 , m_state(Nonthing)
 , m_isValid(false)
{
    kDebug() << "Checking if KWin effect is available";

    m_kwinEffect = XInternAtom(QX11Info::display(), "_KDE_KWIN_KSCREEN_SUPPORT", True);
    if (m_kwinEffect == None) {
        kDebug() << "KWin effect not available";
        return;
    }

    m_rootWindow = XRootWindow(QX11Info::display(), DefaultScreen(QX11Info::display()));
    XSelectInput(QX11Info::display(), m_rootWindow, PropertyChangeMask);
    KSystemEventFilter::installEventFilter(this);

    m_isValid = true;
}

void KWinEffect::start()
{
    kDebug();
    if (!m_isValid) {
        kDebug() << "kwinEffect not available";
        return;
    }

    kDebug() << "Triggering KWin effect";
    long value = 1;
    XChangeProperty(QX11Info::display(), m_rootWindow, m_kwinEffect, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)(&value), 1);
    m_state = FadeIn;
}

void KWinEffect::stop()
{
    kDebug() << "Triggering KWin effect";
    long value = 3;
    XChangeProperty(QX11Info::display(), m_rootWindow, m_kwinEffect, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)(&value), 1);
    m_state = FadeOut;
}

bool KWinEffect::isValid() const
{
    return m_isValid;
}

bool KWinEffect::x11Event(XEvent* event)
{
    if (event->type != PropertyNotify || event->xclient.window != m_rootWindow) {
        return false;
    }
    if (event->xproperty.atom != m_kwinEffect) {
        return false;
    }

    Atom retType;
    unsigned long nItemRet;
    unsigned long byteRet;
    int formatRet;
    unsigned char* propRet;
    int ret = XGetWindowProperty(QX11Info::display(), m_rootWindow, m_kwinEffect, 0, 1, false, XA_CARDINAL, &retType, &formatRet, &nItemRet, &byteRet, &propRet);
    long a = *(long*)propRet;
    kDebug() << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa:" << a;

    m_state = (KWinEffect::State) a;
    kDebug() << m_state;
    emit stateChanged(m_state);
    return false;
}

KWinEffect::~KWinEffect()
{

}