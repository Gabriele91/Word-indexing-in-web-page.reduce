#pragma once

#include <Config.h>

class DownloadWebPage : public QObject
{

    QString m_url;
    QString m_content;

public:
    //copy constructor
    DownloadWebPage(const DownloadWebPage& dwpage)
    {
        m_url     = dwpage.m_url;
        m_content = dwpage.m_content;
    }

    //init constructor
    DownloadWebPage(const QString& url);

    const QString& get_content() const
    {
        return m_content;
    }

    const QString& get_url() const
    {
        return m_url;
    }

	operator const QString& () const
	{
		return m_content;
	}

};


inline DownloadWebPage::DownloadWebPage(const QString& url)
{
	QNetworkAccessManager  networkMgr(this);
	QNetworkReply *reply = networkMgr.get(QNetworkRequest(QUrl(url)));
	reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

	// Wait
	QEventLoop loop;
	QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
	loop.exec();

	// Save the response
	m_content = reply->readAll();
    m_url     = url;
}