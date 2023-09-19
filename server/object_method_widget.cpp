#include "object_method_widget.h"
#include "ui_objectmethodwidget.h"

ObjectMethodWidget::ObjectMethodWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ObjectMethodWidget)
{
    ui->setupUi(this);
}

ObjectMethodWidget::~ObjectMethodWidget()
{
    delete ui;
}


void ObjectMethodWidget::OnShowMethod(const MetaObjectDetail& detail, QtObjectNode* current_select_node) {
    current_select_node_ = current_select_node;
    auto& methods = detail.methods();
    int row = 0;
    ui->object_method_view_->blockSignals(true);
    ui->object_method_view_->clear();
    //table header
    QTableWidgetItem* method_name_header = new QTableWidgetItem;
    method_name_header->setText(QString::fromStdWString(L"方法名"));
    ui->object_method_view_->setHorizontalHeaderItem(0, method_name_header);

    QTableWidgetItem* method_type_header = new QTableWidgetItem;
    method_type_header->setText(QString::fromStdWString(L"类型"));
    ui->object_method_view_->setHorizontalHeaderItem(1, method_type_header);

    ui->object_method_view_->setRowCount((int)methods.size());
    for (auto iter = methods.begin(); iter != methods.end(); iter++) {
        auto method_name = new QTableWidgetItem;
        method_name->setFlags(method_name->flags() & (~Qt::ItemIsEditable));
        method_name->setText(iter.key());
        ui->object_method_view_->setItem(row, 0, method_name);

        auto method_type = new QTableWidgetItem;
        method_type->setFlags(method_name->flags() & (~Qt::ItemIsEditable));
        const MetaObjectDetail::MethodItem& value = iter.value();
        switch (value._method_type) {
        case QMetaMethod::Method:
            method_type->setText(QString::fromStdWString(L"函数"));
            break;
        case QMetaMethod::Slot: {
            method_type->setText(QString::fromStdWString(L"槽"));
        }
        break;
        case QMetaMethod::Signal: {
            method_type->setText(QString::fromStdWString(L"信号"));
        }
        break;
        default:
            break;
        }
        ui->object_method_view_->setItem(row, 1, method_type);

        row++;
    }
    ui->object_method_view_->blockSignals(false);
}
