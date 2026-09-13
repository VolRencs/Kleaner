#ifndef SEARCH_PAGE_H
#define SEARCH_PAGE_H

#include <QAction>
#include <QDateTime>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QSvgRenderer>
#include <QWidget>
#include <QtConcurrent>

#include "Managers/info_manager.h"
#include "Managers/setting_manager.h"
#include "Utils/format_util.h"
#include "utilities.h"

namespace Ui {
    class SearchPage;
}

class SearchPage : public QWidget
{
    Q_OBJECT

  public:
    explicit SearchPage(QWidget *parent = 0);
    ~SearchPage();

  private slots:
    void init();

    void on_btnBrowseSearchDir_clicked();
    void on_btnAdvancePaneToggle_clicked();
    void on_btnSearchAdvance_clicked();
    void initComboboxValues();
    void on_tableFoundResults_customContextMenuRequested(const QPoint &pos);
    void tableFoundResults_header_customContextMenuRequested(const QPoint &pos);
    void loadTableRowMenu();
    void loadHeaderMenu();
    void loadDataToTable(const QList<QString> &results);
    void searching();
    QList<QStandardItem *> createRow(const QString &filepath);

    void on_tableFoundResults_doubleClicked(const QModelIndex &index);

  private:
    void renderLoading();

    Ui::SearchPage *ui;

    QString mSelectedDirectory;

    QStringList mTableHeaders;
    QStandardItemModel *mItemModel;
    QSortFilterProxyModel *mSortFilterModel;
    QMenu mHeaderMenu;
    QMenu mTableRowMenu;
    QString mSearchResultDateFormat;
    int rowRole;
    QSvgRenderer *mLoadingRenderer;
};

#endif // SEARCH_PAGE_H
