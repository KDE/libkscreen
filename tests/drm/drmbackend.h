/*************************************************************************************
 *  Copyright 2015 Sebastian Kügler <sebas@kde.org>                                  *
 *  Copyright 2015 Martin Gräßlin <mgraesslin@kde.org>                               *
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

#ifndef KSCREEN_DRM_BACKEND_H
#define KSCREEN_DRM_BACKEND_H

#include "udev.h"
#include "drmoutput.h"

#include <QObject>
#include <QLoggingCategory>

#include <xf86drmMode.h>

Q_DECLARE_LOGGING_CATEGORY(KSCREEN_WAYLAND)

namespace KScreen
{
class DrmOutput;
class WaylandOutput;


#define KSCREEN_SINGLETON_VARIABLE(ClassName, variableName) \
public: \
    static ClassName *create(QObject *parent = nullptr);\
    static ClassName *self() { return variableName; }\
    protected: \
        explicit ClassName(QObject *parent = nullptr); \
        private: \
            static ClassName *variableName;

#define KSCREEN_SINGLETON(ClassName) KSCREEN_SINGLETON_VARIABLE(ClassName, s_self)

template <typename Pointer, void (*cleanupFunc)(Pointer*)>
struct DrmCleanup
{
    static inline void cleanup(Pointer *ptr)
    {
        cleanupFunc(ptr);
    }
};
template <typename T, void (*cleanupFunc)(T*)> using ScopedDrmPointer = QScopedPointer<T, DrmCleanup<T, cleanupFunc>>;

class DrmBackend : public QObject
{
    Q_OBJECT

public:
    explicit DrmBackend(QObject *parent = 0);
    virtual ~DrmBackend();

    int fd() const {
        return m_fd;
    }
    QVector<DrmOutput*> outputs() const {
        return m_outputs;
    }

    void start();

private:
    void openDrm();
    void queryResources();
    quint32 findCrtc(drmModeRes *res, drmModeConnector *connector, bool *ok = nullptr);
    bool crtcIsUsed(quint32 crtc);
    DrmOutput *findOutput(quint32 connector);
    QScopedPointer<Udev> m_udev;
    QScopedPointer<UdevMonitor> m_udevMonitor;
    int m_fd = -1;
    int m_drmId = 0;
    QVector<DrmOutput*> m_outputs;

};

} // namespace

#endif // KSCREEN_DRM_BACKEND_H
