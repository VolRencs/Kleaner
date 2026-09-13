#include "apt_source_manager_page.h"
#include "Managers/tool_manager.h"
#include "Utils/command_util.h"
#include "ui_apt_source_manager_page.h"
#include "utilities.h"

APTSourceManagerPage::~APTSourceManagerPage()
{
    delete ui;
}

APTSourcePtr APTSourceManagerPage::selectedAptSource = nullptr;

APTSourceManagerPage::APTSourceManagerPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::APTSourceManagerPage)
{
    ui->setupUi(this);

    init();
}

void APTSourceManagerPage::init()
{
    if (CommandUtil::isAptRpm()) {
        // APT-RPM only supports the line format for repository definition
        ui->txtAptSource->setPlaceholderText(tr("example %1")
                                                 .arg("'rpm [p10] http://mirror.yandex.ru/altlinux/ p10/branch/x86_64-i586 classic'"));
    } else {
        // APT still supports one line repository format but it's deprecated (i.e. signed-by option is not supported anymore)
        ui->txtAptSource->setPlaceholderText(tr("example %1")
                                                 .arg("'ppa:deadsnakes/ppa'"));
    }

    loadAptSources();

    on_btnCancel_clicked();

    QList<QWidget *> widgets = {
        ui->btnAddAPTSourceRepository, ui->btnCancel, ui->btnDeleteAptSource,
        ui->btnEditAptSource, ui->txtSearchAptSource, ui->txtSearchAptSource
    };

    Utilities::addDropShadow(widgets, 40);
}

void APTSourceManagerPage::loadAptSources()
{
    ui->listWidgetAptSources->clear();

    selectedAptSource.clear(); // Clear selection after reload

    QList<APTSourcePtr> aptSourceList = ToolManager::ins()->getSourceList();

    for (APTSourcePtr &aptSource : aptSourceList) {
        QListWidgetItem *listItem = new QListWidgetItem(ui->listWidgetAptSources);
        listItem->setData(5, aptSource->source); // for search

        APTSourceRepositoryItem *aptSourceItem = new APTSourceRepositoryItem(aptSource, ui->listWidgetAptSources);

        // Adjust size hint to fit the list widget's width
        QSize itemSize = aptSourceItem->sizeHint();
        int listWidth = ui->listWidgetAptSources->viewport()->width();
        itemSize.setWidth(listWidth);
        listItem->setSizeHint(itemSize + QSize(-10, 1));

        ui->listWidgetAptSources->setItemWidget(listItem, aptSourceItem);
    }

    ui->notFoundWidget->setVisible(aptSourceList.isEmpty());

    ui->lblAptSourceTitle->setText(tr("APT Repositories (%1)").arg(aptSourceList.count()));
}

void APTSourceManagerPage::on_btnAddAPTSourceRepository_clicked(bool checked)
{
    if (checked) {
        ui->btnAddAPTSourceRepository->setText(tr("Save"));
        changeElementsVisible(checked);
    } else {
        QString aptSourceRepository = ui->txtAptSource->text().trimmed();

        if (!aptSourceRepository.isEmpty()) {
            ui->btnAddAPTSourceRepository->setText(tr("Adding..."));
            ui->btnAddAPTSourceRepository->setEnabled(false);
            QApplication::processEvents(); // force UI update

            ToolManager::ins()->addAPTRepository(aptSourceRepository, ui->checkEnableSource->isChecked());

            ui->btnAddAPTSourceRepository->setEnabled(true);
            ui->txtAptSource->clear();
            ui->checkEnableSource->setChecked(false);
            on_btnCancel_clicked();
            loadAptSources();
            on_txtSearchAptSource_textChanged(ui->txtSearchAptSource->text());
        }
    }
}

void APTSourceManagerPage::on_btnCancel_clicked()
{
    ui->btnAddAPTSourceRepository->setChecked(false);
    changeElementsVisible(false);
    ui->btnAddAPTSourceRepository->setText(tr("Add Repository"));
}

void APTSourceManagerPage::changeElementsVisible(const bool checked)
{
    ui->txtAptSource->setVisible(checked);
    ui->checkEnableSource->setVisible(checked);
    ui->btnCancel->setVisible(checked);
    ui->btnEditAptSource->setVisible(!checked);
    ui->btnDeleteAptSource->setVisible(!checked);
    ui->bottomSectionHorizontalSpacer->changeSize(0, 0, checked ? QSizePolicy::Minimum : QSizePolicy::Expanding);
}

void APTSourceManagerPage::on_listWidgetAptSources_itemClicked(QListWidgetItem *item)
{
    QWidget *widget = ui->listWidgetAptSources->itemWidget(item);
    if (widget) {
        APTSourceRepositoryItem *aptSourceItem = dynamic_cast<APTSourceRepositoryItem *>(widget);
        if (aptSourceItem) {
            selectedAptSource = aptSourceItem->aptSource();
        }
    } else {
        selectedAptSource.clear();
    }
}

void APTSourceManagerPage::on_listWidgetAptSources_itemDoubleClicked(QListWidgetItem *item)
{
    on_listWidgetAptSources_itemClicked(item);
    on_btnEditAptSource_clicked();
}

void APTSourceManagerPage::on_btnDeleteAptSource_clicked()
{
    if (!selectedAptSource.isNull()) {
        ToolManager::ins()->removeAPTSource(selectedAptSource);
        selectedAptSource.clear(); // Clear selection after delete
        loadAptSources();
        on_txtSearchAptSource_textChanged(ui->txtSearchAptSource->text());
    }
}

void APTSourceManagerPage::on_txtSearchAptSource_textChanged(const QString &val)
{
    for (int i = 0; i < ui->listWidgetAptSources->count(); ++i) {
        QListWidgetItem *item = ui->listWidgetAptSources->item(i);
        if (item) {
            bool isContain = item->data(5).toString().contains(val, Qt::CaseInsensitive);
            item->setHidden(!isContain);
        }
    }
}

void APTSourceManagerPage::on_btnEditAptSource_clicked()
{
    if (!selectedAptSource.isNull()) {
        if (mAptSourceEditDialog.isNull()) {
            mAptSourceEditDialog = QSharedPointer<APTSourceEdit>(new APTSourceEdit(this));
            connect(mAptSourceEditDialog.data(), &APTSourceEdit::saved, this, &APTSourceManagerPage::loadAptSources);
            connect(mAptSourceEditDialog.data(), &APTSourceEdit::saved, [this]() { selectedAptSource.clear(); });
        }
        APTSourceEdit::selectedAptSource = selectedAptSource;
        mAptSourceEditDialog->show();
    }
}
