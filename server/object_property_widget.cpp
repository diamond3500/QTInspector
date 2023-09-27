#include "object_property_widget.h"
#include "ui_objectpropertywidget.h"

#include <QColorDialog>
#include <QFontDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QDesktopServices>
#include "util/qobject_helper.h"

#define PROPERTY_NAME "custom_key"
#define PROPERTY_VALUE "custom_value"

#define PROPERTY_NAME_ROLE (Qt::UserRole + 1)
#define PROPERTY_VALUE_ROLE (Qt::UserRole + 2)

#define BEGIN_HANDLE_PROPERTY_VALUE(type_id) switch (type_id) {
#define HANDLE_PROPERTY_VALUE(t, fun) \
case t: {                           \
        fun(property_name, row, value);   \
} break;

#define HANDLE_DEFAULT_PROPERTY_VALUE(fun)\
default: \
    fun(property_name, row, value); \
    break; \

#define END_HANDLE_PROPERTY_VALUE() }

ObjectPropertyWidget::ObjectPropertyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ObjectPropertyWidget)
{
    ui->setupUi(this);
    connect(ui->object_detail_view_, &QTableWidget::cellChanged, this,
            &ObjectPropertyWidget::OnPropertyValueEdit);
}

ObjectPropertyWidget::~ObjectPropertyWidget()
{
    delete ui;
}

void ObjectPropertyWidget::OnShowProperty(const MetaObjectDetail &detail, QtObjectNode* current_select_node)
{
    current_select_node_ = current_select_node;
    QStringList keys = detail.PropertyKeys();
    int row = 0;
    ui->object_detail_view_->blockSignals(true);
    ui->object_detail_view_->clear();
    //table header
    QTableWidgetItem* property_name = new QTableWidgetItem;
    QTableWidgetItem* property_value = new QTableWidgetItem;
    property_name->setText(QString::fromStdWString(L"属性名"));
    property_value->setText(QString::fromStdWString(L"属性值"));
    ui->object_detail_view_->setHorizontalHeaderItem(0, property_name);
    ui->object_detail_view_->setHorizontalHeaderItem(1, property_value);

    ui->object_detail_view_->setRowCount((int)keys.size());
    for (auto& key : keys) {
        property_name = new QTableWidgetItem;
        property_name->setFlags(property_name->flags() & (~Qt::ItemIsEditable));
        property_name->setText(key);
        ui->object_detail_view_->setItem(row, 0, property_name);

        auto& value = detail.PropertyValueForKey(key);
        ShowPropertyValue(key, row, value);
        row++;
    }
    ui->object_detail_view_->blockSignals(false);
}


void ObjectPropertyWidget::InnerSetProperty(
    const QString& property_name,
    const QVariant& value,
    TcpClientImpl::SendPacketDelegate delegate) {
    if (nullptr == current_select_node_) {
        return;
    }
    pb::SetPropertyReq command;
    command.set_propertyname(property_name.toStdString());
    command.set_windowuniqueid(current_select_node_->window_unique_id());
    command.set_objectuniqueid(current_select_node_->object_unique_id());

    QByteArray body;
    QDataStream stream(&body, QIODevice::WriteOnly);
    stream << value;
    command.set_value(body.data(), body.size());

    std::string serial_data;
    command.SerializeToString(&serial_data);
    emit sendPacket(pb::PacketTypeSetPropertyReq, serial_data, std::move(delegate));
}

void ObjectPropertyWidget::OnPropertyValueEdit(int row, int column) {
    QTableWidgetItem* widget_item = ui->object_detail_view_->item(row, column);
    QVariant key = widget_item->data(PROPERTY_NAME_ROLE);
    QVariant value = widget_item->data(PROPERTY_VALUE_ROLE);
    QString text = widget_item->text();
    switch (value.type()) {
    case QVariant::Double: {
        InnerSetProperty(key.toString(), text.toDouble());
    } break;
    case QVariant::String: {
        InnerSetProperty(key.toString(), text);
    } break;
    case QVariant::Int: {
        InnerSetProperty(key.toString(), text.toInt());
    } break;
    case QVariant::Url: {

    } break;
    default: {
        qDebug() << "unsupport value:" << value.type();
    } break;
    }
}


void ObjectPropertyWidget::ShowPropertyValue(const QString& property_name,
                               int row,
                               const QVariant& value) {
    int type_id = value.type();
    BEGIN_HANDLE_PROPERTY_VALUE(type_id)
    HANDLE_PROPERTY_VALUE(QMetaType::Bool, HandleBoolProperty)
    HANDLE_PROPERTY_VALUE(QMetaType::QColor, HandleColorProperty)
    HANDLE_PROPERTY_VALUE(QMetaType::QFont, HandleFontProperty)
    HANDLE_PROPERTY_VALUE(QMetaType::QUrl, HandleUrlProperty)
    HANDLE_DEFAULT_PROPERTY_VALUE(HandleDefaultProperty);
    END_HANDLE_PROPERTY_VALUE()
}

void ObjectPropertyWidget::HandleBoolProperty(const QString& property_name,
                                int row,
                                const QVariant& value) {
    QCheckBox* checkbox = new QCheckBox(ui->object_detail_view_);
    checkbox->setChecked(value.toBool());
    checkbox->setProperty(PROPERTY_NAME, property_name);
    checkbox->setProperty(PROPERTY_VALUE, value);
    connect(checkbox, &QCheckBox::stateChanged, this, &ObjectPropertyWidget::OnProperyValueStateChange);
    ui->object_detail_view_->setCellWidget(row, 1, checkbox);
}

void ObjectPropertyWidget::HandleColorProperty(const QString& property_name,
                                 int row,
                                 const QVariant& value) {
    QPushButton* btn = new QPushButton(ui->object_detail_view_);
    QPalette palette = btn->palette();
    palette.setColor(QPalette::ButtonText, value.value<QColor>());
    btn->setPalette(palette);
    btn->setText(value.toString());
    ui->object_detail_view_->setCellWidget(row, 1, btn);
    btn->setProperty(PROPERTY_NAME, property_name);
    btn->setProperty(PROPERTY_VALUE, value);
    connect(btn, &QAbstractButton::clicked, this, &ObjectPropertyWidget::OnClickChangeColor);
}

void ObjectPropertyWidget::HandleFontProperty(const QString& property_name,
                                int row,
                                const QVariant& value) {
    QPushButton* btn = new QPushButton(ui->object_detail_view_);
    btn->setText(value.toString());
    ui->object_detail_view_->setCellWidget(row, 1, btn);
    btn->setProperty(PROPERTY_NAME, property_name);
    btn->setProperty(PROPERTY_VALUE, value);
    connect(btn, &QAbstractButton::clicked, this, &ObjectPropertyWidget::OnClickChangeFont);
}

void ObjectPropertyWidget::HandleUrlProperty(const QString& property_name,
                               int row,
                               const QVariant& value) {
    QWidget* widget = new QWidget(this);
    QHBoxLayout* hbox = new QHBoxLayout();
    QLabel* text = new QLabel(widget);
    text->setText(value.toString());
    hbox->addWidget(text);
    QPushButton* btn = new QPushButton(widget);
    btn->setText(QString::fromStdWString(L"浏览"));
    btn->setProperty(PROPERTY_NAME, property_name);
    btn->setProperty(PROPERTY_VALUE, value);
    hbox->addWidget(btn);
    hbox->setStretch(0, 1);
    hbox->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(hbox);
    connect(btn, &QAbstractButton::clicked, this, &ObjectPropertyWidget::OnClickBrowser);
    ui->object_detail_view_->setCellWidget(row, 1, widget);
}

void ObjectPropertyWidget::HandleDefaultProperty(const QString& property_name,
                                   int row,
                                   const QVariant& value) {
    QTableWidgetItem* property_value = new QTableWidgetItem;
    property_value->setText(value.toString());
    property_value->setData(PROPERTY_NAME_ROLE, property_name);
    property_value->setData(PROPERTY_VALUE_ROLE, value);
    ui->object_detail_view_->setItem(row, 1, property_value);
}


void ObjectPropertyWidget::OnClickChangeColor() {
    QPushButton* btn = (QPushButton*)sender();
    const QColor color = QColorDialog::getColor(btn->property(PROPERTY_VALUE).value<QColor>());
    if (color.isValid()) {
        InnerSetProperty(btn->property(PROPERTY_NAME).value<QString>(), color);
    }
}

void ObjectPropertyWidget::OnClickChangeFont() {
    QPushButton* btn = (QPushButton*)sender();
    bool ok = false;
    const QFont font = QFontDialog::getFont(&ok, btn->property(PROPERTY_VALUE).value<QFont>());
    if (ok) {
        InnerSetProperty(btn->property(PROPERTY_NAME).value<QString>(), font);
    }
}

void ObjectPropertyWidget::OnClickBrowser() {
    QPushButton* btn = (QPushButton*)sender();
    QString uri = btn->property(PROPERTY_VALUE).toString();
    if (uri.startsWith("qrc:") || uri.startsWith("file://")) {
        pb::GetUriContentReq request;
        request.set_uri(uri.toStdString());
        emit sendPacket(pb::PacketTypeGetUriContentReq, QObjectHelper::SerialPb(request), 0);
    }
}

void ObjectPropertyWidget::OnProperyValueStateChange(int state) {
    QCheckBox* btn = (QCheckBox*)sender();
    QVariant value(false);
    if (state == Qt::Checked) {
        value = true;
    }
    InnerSetProperty(btn->property(PROPERTY_NAME).value<QString>(), value);
}
