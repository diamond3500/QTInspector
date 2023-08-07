#ifndef FLOAT_COVER_MASK_H
#define FLOAT_COVER_MASK_H

#include <QWidget>

namespace Ui {
class FloatCoverMask;
}

class FloatCoverMask : public QWidget {
    Q_OBJECT

public:
  explicit FloatCoverMask(QWidget* parent = nullptr);
 ~FloatCoverMask();

private:
  Ui::FloatCoverMask* ui;
};

#endif // FLOAT_COVER_MASK_H
