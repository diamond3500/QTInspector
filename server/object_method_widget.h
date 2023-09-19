#ifndef OBJECT_METHOD_WIDGET_H
#define OBJECT_METHOD_WIDGET_H

#include <QWidget>
#include "qt_object_node.h"
#include "network/tcp_client_impl.h"
#include "pb/app_window.pb.h"

namespace Ui {
class ObjectMethodWidget;
}

class ObjectMethodWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectMethodWidget(QWidget *parent = nullptr);
    ~ObjectMethodWidget();


public slots:
    void OnShowMethod(const MetaObjectDetail& detail, QtObjectNode* current_select_node);


private:
    Ui::ObjectMethodWidget *ui;
    QtObjectNode* current_select_node_ = nullptr;
};

#endif // OBJECT_METHOD_WIDGET_H
