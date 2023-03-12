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
#include "asfx.h"
#include "upsdb.h"

#include <QCoreApplication>
#include <QSettings>
#include <memory.h>
#include <signal.h>
#include <unistd.h>

static std::unique_ptr<UPSDB> g_upsdb;
ASLogger* Globals::pLog { nullptr };

void catchUnixSignals(std::initializer_list<int> quitSignals)
{
    auto handler = [](int sig) {
        PRINT("Quit the application by signal({}).\n", sig);
        g_upsdb.reset();
        QCoreApplication::quit();
    };

    sigset_t blocking_mask;
    sigemptyset(&blocking_mask);
    for (auto sig : quitSignals)
        sigaddset(&blocking_mask, sig);

    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_mask    = blocking_mask;
    sa.sa_flags   = 0;

    for (auto sig : quitSignals)
        sigaction(sig, &sa, nullptr);
}

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    g_upsdb = std::make_unique<UPSDB>();
    g_upsdb->start();

    catchUnixSignals({ SIGQUIT, SIGINT, SIGTERM, SIGHUP /*, SIGABRT*/ });
    return a.exec();
}
