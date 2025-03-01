#ifndef ORDERSCREE_H
#define ORDERSCREE_H

#include <QWidget>

namespace Ui {
class orderscree;
}

class orderscree : public QWidget
{
    Q_OBJECT

public:
    explicit orderscree(QWidget *parent = nullptr);
    ~orderscree();

private:
    Ui::orderscree *ui;
};

#endif // ORDERSCREE_H
