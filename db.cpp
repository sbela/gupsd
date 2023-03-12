/*
 * Copyright 2023. 02. 18. sbela
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

#include "db.h"
#include "asfx.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

LogDB::LogDB()
{
    PRINT("LOGDB");
    QSettings s(QStringLiteral("gups.ini"), QSettings::IniFormat);
    QString server = s.value(QStringLiteral("SERVER"), QStringLiteral("192.168.100.70")).toString();
    QString dbName = s.value(QStringLiteral("DB"), QStringLiteral("ups")).toString();
    QString un     = s.value(QStringLiteral("UN"), QStringLiteral("postgres")).toString();
    QString pw     = s.value(QStringLiteral("PW"), QStringLiteral("sysauto")).toString();
    int port       = s.value(QStringLiteral("PORT"), 5432).toInt();

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QPSQL"),
                                     QString("ASLOG_%1").arg(QDateTime::currentDateTime().toString("yyyyMMhhmmsszzz")));
    m_db.setHostName(server);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(un);
    m_db.setPassword(pw);
    m_db.setPort(port);
    PRINT("LOGDB INIT DONE");
}

LogDB::~LogDB()
{
    if (m_db.isOpen())
        m_db.close();
    PRINT("~LOGDB");
}

void LogDB::addToDB(const QByteArray& msg)
{
    if (initDB())
    {
        if (m_db.transaction())
        {
            QStringList items { QString("%1").arg(msg.constData()).split('\n', Qt::SkipEmptyParts) };

            QString battery_charge, battery_voltage, input_frequency_nominal, input_voltage, input_voltage_fault,
                input_voltage_nominal, output_voltage;

            constexpr auto setItem = [](const QString& item, const QString& elem, QString& value) {
                if (item.contains(elem))
                {
                    auto items { item.split(':', Qt::SkipEmptyParts) };
                    if (items.size() > 1)
                        value = items.at(1).trimmed();
                }
            };

            for (int i = 0; i < items.size(); ++i)
            {
                auto item { items.at(i) };
                setItem(item, QStringLiteral("battery.charge"), battery_charge);
                setItem(item, QStringLiteral("battery.voltage"), battery_voltage);
                setItem(item, QStringLiteral("input.frequency.nominal"), input_frequency_nominal);
                setItem(item, QStringLiteral("input.voltage.fault"), input_voltage_fault);
                setItem(item, QStringLiteral("input.voltage.nominal"), input_voltage_nominal);
                setItem(item, QStringLiteral("input.voltage"), input_voltage);
                setItem(item, QStringLiteral("output.voltage"), output_voltage);
            }

            QSqlQuery qry(m_db);
            if (qry.prepare(QStringLiteral(
                    "insert into ups_data(datum, battery_charge, battery_voltage, input_frequency_nominal,"
                    "input_voltage, input_voltage_fault, input_voltage_nominal, output_voltage) "
                    "values(CURRENT_TIMESTAMP, :battery_charge, :battery_voltage, :input_frequency_nominal,"
                    ":input_voltage, :input_voltage_fault, :input_voltage_nominal, :output_voltage)")))
            {
                qry.bindValue(":battery_charge", battery_charge);
                qry.bindValue(":battery_voltage", battery_voltage);
                qry.bindValue(":input_frequency_nominal", input_frequency_nominal);
                qry.bindValue(":input_voltage", input_voltage);
                qry.bindValue(":input_voltage_fault", input_voltage_fault);
                qry.bindValue(":input_voltage_nominal", input_voltage_nominal);
                qry.bindValue(":output_voltage", output_voltage);

                if (qry.exec())
                {
                    m_db.commit();
                }
                else
                {
                    PRINT("EXEC FAILED: DBERR:{} QRYERR:{} SQL:[{}]", m_db.lastError().text(), qry.lastError().text(),
                          qry.lastQuery());
                    m_db.rollback();
                }
            }
            else
                PRINT("PREPARE FAILED: [{}] [{}]", m_db.lastError().text(), qry.lastError().text());
        }
        else
            PRINT("CAN NOT INIT TRANSACTION: {}", m_db.lastError().text());
    }
    else
        PRINT("CAN NOT INIT DB: {}", m_db.lastError().text());
}

bool LogDB::isConnected()
{
    return m_db.isOpen();
}

bool LogDB::initDB()
{
    if (m_db.isOpen())
        return true;
    return m_db.open();
}
