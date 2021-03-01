/*
 *  SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef QSCREEN_SCREEN_H
#define QSCREEN_SCREEN_H

#include "config.h"
#include "screen.h"

#include <QLoggingCategory>
#include <QScreen>
#include <QSize>

namespace KScreen
{
class Output;

class QScreenScreen : public QObject
{
    Q_OBJECT

public:
    explicit QScreenScreen(QScreenConfig *config);
    ~QScreenScreen() override;

    KScreen::ScreenPtr toKScreenScreen() const;
    void updateKScreenScreen(KScreen::ScreenPtr &screen) const;
};

} // namespace

#endif // QSCREEN_SCREEN_H
