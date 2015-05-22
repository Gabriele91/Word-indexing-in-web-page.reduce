//
//  Config.h
//  Web-page-parser
//
//  Created by Gabriele Di Bari on 22/04/15.
//  Copyright (c) 2015 Gabriele Di Bari. All rights reserved.
//
#pragma once

#if __APPLE__
    #include <qapplication.h>
    #include <QWebElement>
    #include <qwebpage.h>
    #include <qwebframe.h>
    #include <QtWebKit>
    #include <qnetworkaccessmanager.h>
    #include <qnetworkrequest.h>
#else
    #include <QtWidgets/qapplication.h>
    #include <QtWebKit/QWebElement>
    #include <QtWebKitWidgets/qwebpage.h>
    #include <QtWebKitWidgets/qwebframe.h>
    #include <QtWebKit/QtWebKit>
    #include <QtNetwork/qnetworkaccessmanager.h>
    #include <QtNetwork/qnetworkrequest.h>
#endif