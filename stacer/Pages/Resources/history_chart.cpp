#include "history_chart.h"
#include "ui_history_chart.h"

HistoryChart::~HistoryChart()
{
    delete ui;
}

HistoryChart::HistoryChart(const QString &title, const int &seriesCount, QCategoryAxis *categoryAxisY, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HistoryChart),
    mTitle(title),
    mSeriesCount(seriesCount),
    mChartView(new QChartView(this)),
    mChart(mChartView->chart())
{
    ui->setupUi(this);

    init();

    if (categoryAxisY) {
        mAxisY = categoryAxisY;
        mAxisY->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
        // remove default axis
        mChart->removeAxis(mChart->axes(Qt::Vertical).at(0));
        mChart->addAxis(mAxisY, Qt::AlignLeft);
        for (int i = 0; i < seriesCount; ++i) {
            mSeriesList.at(i)->attachAxis(mAxisY);
        }
    }
}

void HistoryChart::init()
{
    ui->lblHistoryTitle->setText(mTitle);

    // add series to chart
    for (int i = 0; i < mSeriesCount; i++) {
        mSeriesList.append(new QSplineSeries);
        mChart->addSeries(mSeriesList.at(i));
    }

    mChartView->setRenderHint(QPainter::Antialiasing);

    for (int i = 0; i < mSeriesList.count(); ++i) {
        dynamic_cast<QSplineSeries *>(mChart->series().at(i))->setColor(QColor::fromHsvF((qreal)i / mSeriesList.count(), 0.8, 0.8));
    }

    // Chart Settings
    mChart->createDefaultAxes();

    mChart->axes(Qt::Horizontal).back()->setRange(0, 60);
    mChart->axes(Qt::Horizontal).back()->setReverse(true);

    mChart->setContentsMargins(-11, -11, -11, -11);
    mChart->setMargins(QMargins(20, 0, 10, 10));
    ui->layoutHistoryChart->addWidget(mChartView, 1, 0, 1, 3);

    // theme changed
    connect(SignalMapper::ins(), &SignalMapper::sigChangedAppTheme, [this] {
        QString chartLabelColor = AppManager::ins()->getStyleValues()->value("@chartLabel").toString();
        QString chartGridColor = AppManager::ins()->getStyleValues()->value("@chartGrid").toString();
        QString historyChartBackgroundColor = AppManager::ins()->getStyleValues()->value("@historyChartBackground").toString();

        mChart->axes(Qt::Horizontal).back()->setLabelsColor(chartLabelColor);
        mChart->axes(Qt::Horizontal).back()->setGridLineColor(chartGridColor);

        mChart->axes(Qt::Vertical).back()->setLabelsColor(chartLabelColor);
        mChart->axes(Qt::Vertical).back()->setGridLineColor(chartGridColor);

        mChart->setBackgroundBrush(QColor(historyChartBackgroundColor));
        mChart->legend()->setLabelColor(chartLabelColor);
    });
}

void HistoryChart::setYMax(const int &value)
{
    mChart->axes(Qt::Vertical).back()->setRange(0, value);
}

QCategoryAxis *HistoryChart::getAxisY()
{
    return mAxisY;
}

void HistoryChart::setCategoryAxisYLabels()
{
    if (mAxisY) {
        for (const QString &label : mAxisY->categoriesLabels()) {
            mAxisY->remove(label);
        }

        for (int i = 1; i < 5; ++i) {
            mAxisY->append(FormatUtil::formatBytes((mAxisY->max() / 4) * i), (mAxisY->max() / 4) * i);
        }
    }
}

QVector<QSplineSeries *> HistoryChart::getSeriesList() const
{
    return mSeriesList;
}

void HistoryChart::setSeriesList(const QVector<QSplineSeries *> &seriesList)
{
    for (int i = 0; i < seriesList.count(); ++i) {
        mChart->series().replace(0, seriesList.at(i));
    }

    mChartView->repaint();
}

void HistoryChart::on_checkHistoryTitle_clicked(bool checked)
{
    QLayout *charts = topLevelWidget()->findChild<QWidget *>("charts")->layout();

    for (int i = 0; i < charts->count(); ++i) {
        charts->itemAt(i)->widget()->setVisible(!checked);
    }

    show();
}
