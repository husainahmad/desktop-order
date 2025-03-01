#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "orderscreen.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    // Create MDI Area
    mdiArea = new QMdiArea(this);
    setCentralWidget(mdiArea);

    // Create Menu
    QMenu *fileMenu = menuBar()->addMenu("File");
    QAction *newOrderAction = new QAction("New Order", this);
    connect(newOrderAction, &QAction::triggered, this, &MainWindow::openOrderScreen);
    fileMenu->addAction(newOrderAction);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openOrderScreen()
{
    // Create a new OrderScreen inside MDI Area
    OrderScreen *orderScreen = new OrderScreen();
    QMdiSubWindow *subWindow = mdiArea->addSubWindow(orderScreen);

    subWindow->setWindowTitle("New Order");
    subWindow->setAttribute(Qt::WA_DeleteOnClose); // Delete when closed
    subWindow->showMaximized(); // Open in maximized mode
}
