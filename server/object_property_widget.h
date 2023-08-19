#ifndef OBJECT_PROPERTY_WIDGET_H
#define OBJECT_PROPERTY_WIDGET_H

#include <QWidget>
#include "qt_object_node.h"
#include "network/tcp_client_impl.h"
#include "pb/app_window.pb.h"

namespace Ui {
class ObjectPropertyWidget;
}

class ObjectPropertyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectPropertyWidget(QWidget *parent = nullptr);
    ~ObjectPropertyWidget();

Q_SIGNALS:
    void sendPacket(pb::PacketType type,
                    const std::string& body,
                    TcpClientImpl::SendPacketDelegate delegate);

public slots:
    void OnShowProperty(const MetaObjectDetail& detail, QtObjectNode* current_select_node);

private slots:
    void OnPropertyValueEdit(int row, int column);
    void OnClickChangeColor();
    void OnClickChangeFont();
    void OnClickBrowser();
    void OnProperyValueStateChange(int state);

private:
    void InnerSetProperty(const QString& property_name,
                          const QVariant& value,
                          TcpClientImpl::SendPacketDelegate delegate = nullptr);

    void ShowPropertyValue(const QString& property_name, int row, const QVariant& value);


    void HandleBoolProperty(const QString& property_name,
                            int row,
                            const QVariant& value);
    void HandleColorProperty(const QString& property_name,
                             int row,
                             const QVariant& value);

    void HandleFontProperty(const QString& property_name,
                            int row,
                            const QVariant& value);


    void HandleUrlProperty(const QString& property_name,
                           int row,
                           const QVariant& value);


    void HandleDefaultProperty(const QString& property_name,
                               int row,
                               const QVariant& value);
private:
    Ui::ObjectPropertyWidget *ui;
    QtObjectNode* current_select_node_ = nullptr;
};

#endif // OBJECT_PROPERTY_WIDGET_H
