// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include <limits>

#include "Cleaner/cleaner.h"
#include "Info/cpu_info.h"
#include "Info/memory_info.h"
#include "Info/system_info.h"
#include "Utils/format.h"
#include "Utils/helpers.h"
#include "Utils/procfs.h"

class CoreTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void procfsReadsUInt64();
    void procfsReadsLines();

    void helpersReportPageSize();

    void formatBytesScalesUnits();
    void formatPercentRounds();
    void formatDurationClampsHugeValues();

    void memoryInfoReportsMemory();
    void cpuInfoReportsCores();
    void systemInfoReportsKernel();

    void cleanerComputesDirectorySize();
};

void CoreTest::procfsReadsUInt64()
{
    bool ok = false;
    const quint64 uptime = Procfs::readUInt64(QStringLiteral("/proc/uptime"), &ok);
    QVERIFY(ok);
    QVERIFY(uptime > 0);
}

void CoreTest::procfsReadsLines()
{
    const QList<QByteArray> lines = Procfs::lines(QStringLiteral("/proc/meminfo"));
    QVERIFY(!lines.isEmpty());
}

void CoreTest::helpersReportPageSize()
{
    QVERIFY(Helpers::pageSizeKiB() > 0);
    QVERIFY(Helpers::clockTicks() > 0.0);
    QVERIFY(!Helpers::userName(0).isEmpty());
}

void CoreTest::formatBytesScalesUnits()
{
    Format format;
    QVERIFY(!format.bytes(0).isEmpty());
    QVERIFY(!format.bytes(1024).isEmpty());
    QVERIFY(!format.bytes(1024ULL * 1024 * 1024).isEmpty());
}

void CoreTest::formatPercentRounds()
{
    Format format;
    QCOMPARE(format.percent(12.34, 1), QStringLiteral("12.3%"));
    QCOMPARE(format.percent(100.0, 0), QStringLiteral("100%"));
}

void CoreTest::formatDurationClampsHugeValues()
{
    Format format;
    QVERIFY(!format.duration(0).isEmpty());
    QVERIFY(!format.duration(3661).isEmpty());
    // Must not overflow into garbage for absurd inputs.
    QVERIFY(!format.duration(std::numeric_limits<qulonglong>::max()).isEmpty());
}

void CoreTest::memoryInfoReportsMemory()
{
    MemoryInfo memory;
    QSignalSpy spy(&memory, &MemoryInfo::changed);
    memory.update();
    QCOMPARE(spy.count(), 1);
    QVERIFY(memory.total() > 0);
    QVERIFY(memory.used() <= memory.total());
    QVERIFY(memory.usagePercent() >= 0.0);
    QVERIFY(memory.usagePercent() <= 100.0);
}

void CoreTest::cpuInfoReportsCores()
{
    CpuInfo cpu;
    cpu.update();
    cpu.update();
    QVERIFY(cpu.coreCount() >= 1);
    QCOMPARE(cpu.coreUsages().size(), cpu.coreCount());
}

void CoreTest::systemInfoReportsKernel()
{
    SystemInfo info;
    QVERIFY(!info.kernel().isEmpty());
    QVERIFY(!info.hostname().isEmpty());
    QVERIFY(!info.platform().isEmpty());
    info.update();
}

void CoreTest::cleanerComputesDirectorySize()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QFile small(dir.filePath(QStringLiteral("small.txt")));
    QVERIFY(small.open(QIODevice::WriteOnly));
    small.write(QByteArray(100, 'x'));
    small.close();

    QDir(dir.path()).mkdir(QStringLiteral("nested"));
    QFile nested(dir.filePath(QStringLiteral("nested/big.txt")));
    QVERIFY(nested.open(QIODevice::WriteOnly));
    nested.write(QByteArray(1000, 'y'));
    nested.close();

    QCOMPARE(Cleaner::directorySize(dir.path()), 1100ULL);
}

QTEST_GUILESS_MAIN(CoreTest)

#include "core_test.moc"
