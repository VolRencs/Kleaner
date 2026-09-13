#ifndef CPUINFO_H
#define CPUINFO_H

#include "Utils/file_util.h"
#include "stacer-core_global.h"

// Run command in English language (guarantee same behaviour across languages)
#define LSCPU_COMMAND "LC_ALL=C lscpu"
#define PROC_CPUINFO "/proc/cpuinfo"
#define PROC_LOADAVG "/proc/loadavg"
#define PROC_STAT "/proc/stat"

class STACERCORESHARED_EXPORT CpuInfo
{
  public:
    int getCpuPhysicalCoreCount() const;
    int getCpuCoreCount() const;
    QList<int> getCpuPercents() const;
    QList<double> getLoadAvgs() const;
    double getAvgClock() const;
    QList<double> getClocks() const;

  private:
    int getCpuPercent(const QList<double> &cpuTimes, const int &processor = 0) const;
};

#endif // CPUINFO_H
