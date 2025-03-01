#include "splasscreen.h"
#include "ui_splasscreen.h"

SplasScreen::SplasScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SplasScreen)
{
    ui->setupUi(this);
}

SplasScreen::~SplasScreen()
{
    delete ui;
}
