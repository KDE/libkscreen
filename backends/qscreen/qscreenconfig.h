/*************************************************************************************
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
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

#ifndef QSCREEN_CONFIG_H
#define QSCREEN_CONFIG_H

#include "config.h"

#include <QScreen>

namespace KScreen
{
class Output;
class QScreenOutput;
class QScreenScreen;

class QScreenConfig : public QObject
{
    Q_OBJECT

public:
    explicit QScreenConfig(QObject *parent = 0);
    virtual ~QScreenConfig();

    KScreen::ConfigPtr toKScreenConfig() const;
    void updateKScreenConfig(KScreen::ConfigPtr &config) const;

    QMap<int, QScreenOutput *> outputMap() const;
    int outputId(const QScreen *qscreen);

private Q_SLOTS:
    void screenAdded(const QScreen *qscreen);
    void screenRemoved(QScreen *qscreen);

Q_SIGNALS:
    void configChanged(const KScreen::ConfigPtr &config);

private:
    QMap<int, QScreenOutput *> m_outputMap;
    QScreenScreen *m_screen;
    int m_lastOutputId = -1;
    bool m_blockSignals;
};

} // namespace

#endif // QSCREEN_CONFIG_H
