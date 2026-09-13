#include "apt_source_repository_item.h"
#include "Utils/command_util.h"
#include "ui_apt_source_repository_item.h"
#include "utilities.h"

#include <QRegularExpression>

APTSourceRepositoryItem::~APTSourceRepositoryItem()
{
    delete ui;
}

APTSourceRepositoryItem::APTSourceRepositoryItem(APTSourcePtr aptSource, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::APTSourceRepositoryItem),
    mAptSource(aptSource)
{
    init();
}

void APTSourceRepositoryItem::init()
{
    ui->setupUi(this);

    Utilities::addDropShadow(this, 30, 10);

    ui->checkAptSource->setChecked(mAptSource->isActive);

    QString source = mAptSource->source;

    // Remove bracket options for display
    source.remove(QRegularExpression("\\s[\\[]+.*[\\]]+"));

    if (mAptSource->isSource) {
        ui->lblAptSourceName->setText(tr("%1 (Source Code)").arg(source));
    } else {
        ui->lblAptSourceName->setText(source);
    }

    ui->lblAptSourceName->setToolTip(ui->lblAptSourceName->text());
}

APTSourcePtr APTSourceRepositoryItem::aptSource() const
{
    return mAptSource;
}

void APTSourceRepositoryItem::on_checkAptSource_clicked(bool checked)
{
    ToolManager::ins()->changeAPTStatus(mAptSource, checked);
}
