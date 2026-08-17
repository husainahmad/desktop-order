QT       += core gui network
QT       += printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    apiclient.cpp \
    brand.cpp \
    busyindicator.cpp \
    cacheutils.cpp \
    chain.cpp \
    dailyreportscreen.cpp \
    loginscreen.cpp \
    main.cpp \
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
    productdetailpopup.cpp \
    productimage.cpp \
    productwidget.cpp \
    salesreportscreen.cpp \
    setting.cpp \
    settingscreen.cpp \
    sku.cpp \
    skuwidget.cpp \
    store.cpp \
    tokenmanager.cpp \
    user.cpp

HEADERS += \
    apiclient.h \
    brand.h \
    busyindicator.h \
    cacheutils.h \
    chain.h \
    dailyreportscreen.h \
    loginscreen.h \
    order.h \
    ordercartwidget.h \
    orderform.h \
    orderitem.h \
    orderitemsku.h \
    orderpaymentpopup.h \
    orderpopupwindow.h \
    orderprint.h \
    orderscreen.h \
    ordertabbutton.h \
    ordersummary.h \
    ordertablewidget.h \
    product.h \
    productdetailpopup.h \
    productimage.h \
    productwidget.h \
    salesreportscreen.h \
    screenutils.h \
    setting.h \
    settingscreen.h \
    sku.h \
    skuwidget.h \
    store.h \
    tokenmanager.h \
    touchutils.h \
    user.h

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

DEFINES -= QT_NO_DEBUG_OUTPUT