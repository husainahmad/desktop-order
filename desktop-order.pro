QT       += core gui network
QT       += printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    brand.cpp \
    chain.cpp \
    loginscreen.cpp \
    main.cpp \
    mainwindow.cpp \
    order.cpp \
    ordercartwidget.cpp \
    orderform.cpp \
    orderitem.cpp \
    orderitemsku.cpp \
    orderpaymentpopup.cpp \
    orderpopupwindow.cpp \
    orderprint.cpp \
    orderscreen.cpp \
    ordersummary.cpp \
    ordertablewidget.cpp \
    product.cpp \
    productimage.cpp \
    productwidget.cpp \
    setting.cpp \
    sku.cpp \
    skuwidget.cpp \
    splashscreen.cpp \
    store.cpp \
    toast.cpp \
    user.cpp

HEADERS += \
    brand.h \
    chain.h \
    loginscreen.h \
    mainwindow.h \
    order.h \
    ordercartwidget.h \
    orderform.h \
    orderitem.h \
    orderitemsku.h \
    orderpaymentpopup.h \
    orderpopupwindow.h \
    orderprint.h \
    orderscreen.h \
    ordersummary.h \
    ordertablewidget.h \
    product.h \
    productimage.h \
    productwidget.h \
    setting.h \
    sku.h \
    skuwidget.h \
    splashscreen.h \
    store.h \
    toast.h \
    user.h

FORMS += \
    loginscreen.ui \
    mainwindow.ui \
    ordercartwidget.ui \
    orderform.ui \
    orderscreen.ui \
    ordersummary.ui \
    ordertablewidget.ui \
    productwidget.ui \
    skuwidget.ui \
    splashscreen.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

release {
    DEFINES -= QT_NO_DEBUG_OUTPUT
}

DISTFILES += \
    config/app.ini

macx {
    QMAKE_POST_LINK += cp $$PWD/config/app.ini $$OUT_PWD/config.app.ini
}

# Windows: copy app.ini after build
win32 {
    QMAKE_POST_LINK += copy /Y \"$$PWD\\config\\app.ini\" \"$$OUT_PWD\\config.app.ini\"
}

win32:LIBS += -lwinspool

