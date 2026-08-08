#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "orderscreen.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>

#include "settingsdialog.h"
#include "loginscreen.h"
#include "tokenmanager.h"

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

    QAction *settingsAction = new QAction("Settings", this);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettings);
    fileMenu->addAction(settingsAction);

    QAction *logoutAction = new QAction("Logout", this);
    connect(logoutAction, &QAction::triggered, this, &MainWindow::logout);
    fileMenu->addAction(logoutAction);
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

void MainWindow::openSettings()
{
    SettingsDialog *dialog = new SettingsDialog(this);
    dialog->exec();
    delete dialog;
}

void MainWindow::logout()
{
    // Show confirmation dialog
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Logout");
    msgBox.setText("Are you sure you want to logout?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    if (msgBox.exec() == QMessageBox::Yes) {
        // Clear token
        TokenManager::instance().clearTokens();

        // Close all subwindows
        mdiArea->closeAllSubWindows();

        // Replace central widget with login screen
        LoginScreen *loginScreen = new LoginScreen(this);
        setCentralWidget(loginScreen);
        loginScreen->show();
    }
}
