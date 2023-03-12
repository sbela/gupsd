/*
 * Copyright 2023. 03. 12. sbela
 * @ AutoSys KnF Kft.
 * web: http://www.autosys.hu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include "upsdb.h"
#include "asfx.h"
#include "db.h"

#include <QProcess>
#include <QSettings>

UPSDB::UPSDB(QObject* parent)
    : QThread { parent }
{
    QSettings s(QStringLiteral("gups.ini"), QSettings::IniFormat);
    m_url = s.value(QStringLiteral("ups"), QStringLiteral("ups@192.168.100.71")).toString();
}

UPSDB::~UPSDB()
{
    {
        QMutexLocker m(&m_runMutex);
        m_bRun = false;
    }
    msleep(100);
}

void UPSDB::run()
{
    PRINT("STARTED DB ADD");
    LogDB db;

    bool bRun { m_bRun };
    int count { 60000 }; // percenkent
    while (bRun)
    {
        if (++count > 60000)
        {
            count = 0;
            QProcess pr;
            QStringList args;
            args << m_url;
            pr.start(QStringLiteral("upsc"), args);
            pr.waitForFinished();

            db.addToDB(pr.readAllStandardOutput());
        }
        QThread::msleep(10);
        {
            QMutexLocker m(&m_runMutex);
            bRun = m_bRun;
        }
    }
    PRINT("STOPPED DB ADD");
}
