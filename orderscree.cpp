#include "orderscree.h"
#include "ui_orderscree.h"

orderscree::orderscree(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::orderscree)
{
    ui->setupUi(this);
}

orderscree::~orderscree()
{
    delete ui;
}
