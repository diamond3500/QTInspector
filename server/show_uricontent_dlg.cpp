#include "show_uricontent_dlg.h"
#include "ui_show_uricontent_dlg.h"
#include <QPixmap>
#include <QCloseEvent>
#include <QFileDialog>
#include "util/qobject_helper.h"

ShowUriContentDlg::ShowUriContentDlg(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShowUriContentDlg)
{
  ui->setupUi(this);
  show_content_ = new ShowUriContent(this);
  ui->horizontalLayout_2->addWidget(show_content_);
}

ShowUriContentDlg::~ShowUriContentDlg()
{
    delete ui;
}

void ShowUriContentDlg::SetContent(const pb::GetUriContentRsp& response) {
    response_ = response;
    show_content_->SetContent(response);
}

void ShowUriContentDlg::closeEvent(QCloseEvent* e) {
    e->ignore();
    hide();
}

void ShowUriContentDlg::on_saveas_clicked() {
  auto& content = response_.content();
  if (content.empty()) {
    return;
  }
  const QString file_path = QFileDialog::getSaveFileName(this);
  if (file_path.isEmpty()) {
    return;
  }
  QObjectHelper::SaveFile(file_path, content.data(), (int)content.length());
}

