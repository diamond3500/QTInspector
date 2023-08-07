#ifndef SHOW_URI_CONTENT_H
#define SHOW_URI_CONTENT_H

#include <QWidget>
#include "pb/app_window.pb.h"

namespace Ui {
class ShowUriContent;
}

class ShowUriContent : public QWidget
{
    Q_OBJECT

public:
    explicit ShowUriContent(QWidget *parent = nullptr);
    ~ShowUriContent();
    void SetContent(const pb::GetUriContentRsp& rsp);

private:
    void HideAll();
    void ShowImage(const std::string& buffer);
    void ShowText(const std::string& buffer);

private:
    Ui::ShowUriContent *ui;
};

#endif // SHOW_URI_CONTENT_H
