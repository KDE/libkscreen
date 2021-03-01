/*
 *  SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef QSCREEN_OUTPUT_H
#define QSCREEN_OUTPUT_H

#include "qscreenconfig.h"

#include "config.h"
#include "edid.h"
#include "output.h"

#include <QScreen>

namespace KScreen
{
class QScreenOutput : public QObject
{
    Q_OBJECT

public:
    explicit QScreenOutput(const QScreen *qscreen, QObject *parent = nullptr);
    ~QScreenOutput() override;

    KScreen::OutputPtr toKScreenOutput() const;
    void updateKScreenOutput(KScreen::OutputPtr &output) const;

    int id() const;
    void setId(const int newId);

    const QScreen *qscreen() const;

private:
    const QScreen *m_qscreen;
    int m_id;
};

} // namespace

#endif
