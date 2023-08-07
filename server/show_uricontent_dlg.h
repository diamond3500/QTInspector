#ifndef SHOW_URICONTENT_DLG_H
#define SHOW_URICONTENT_DLG_H

#include <QWidget>
#include "show_uri_content.h"
#include "pb/app_window.pb.h"

namespace Ui {
class ShowUriContentDlg;
}

class ShowUriContentDlg : public QWidget
{
    Q_OBJECT

public:
    explicit ShowUriContentDlg(QWidget *parent = nullptr);
    ~ShowUriContentDlg();

    void SetContent(const pb::GetUriContentRsp& response);

private slots:
    void on_saveas_clicked();

private:
   void closeEvent(QCloseEvent* e) override;

private:
    Ui::ShowUriContentDlg *ui;
 pb::GetUriContentRsp response_;
    ShowUriContent* show_content_ = nullptr;
};

#endif // SHOW_URICONTENT_DLG_H
