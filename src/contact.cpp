// Copyright (C) 2014 - 2017 Leslie Zhai <xiang.zhai@i-soft.com.cn>

#include <KLocalizedString>

#ifndef NDEBUG                                                                      
#include <QFile>                                                                   
#endif                                                                             
#include <QJsonDocument>                                                           
#include <QJsonObject>                                                             
#include <QJsonArray>
#include <time.h>

#include "contact.h"
#include "globaldeclarations.h"

Contact::Contact(HttpPost* parent) 
  : HttpPost(parent), 
    m_v2(false)
{
#ifndef NDEBUG
    qDebug() << "DEBUG:" << __PRETTY_FUNCTION__;
#endif
}

Contact::~Contact() 
{
#ifndef NDEBUG
    qDebug() << "DEBUG:" << __PRETTY_FUNCTION__;
#endif
    m_clear();
}

void Contact::m_clear() 
{ 
    Q_FOREACH (QObject* obj, m_contactList) {
        if (obj) {
            delete obj;
            obj = Q_NULLPTR;
        }
    }
    m_contactList.clear(); 
}

void Contact::m_post(QString host) 
{
    m_clear();

    QString url = host + WX_CGI_PATH + "webwxgetcontact?r=" + 
        QString::number(time(NULL));
#ifndef NDEBUG
    qDebug() << "DEBUG:" << __PRETTY_FUNCTION__ << url;
#endif
    QString json = "{}";
    HttpPost::post(url, json, true);
}

void Contact::post() { m_v2 = false; m_post(WX_SERVER_HOST); }

void Contact::postV2() { m_v2 = true; m_post(WX_V2_SERVER_HOST); }

void Contact::finished(QNetworkReply* reply) 
{
    QString replyStr(reply->readAll());
#ifndef NDEBUG
    qDebug() << "DEBUG:" << __PRETTY_FUNCTION__;
    qDebug() << "DEBUG:" << replyStr;
    QFile file("contact.json");                                                 
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {                       
        QTextStream out(&file);                                                    
        out << replyStr;                                                           
        file.close();                                                              
    }                                                                              
#endif
    QJsonDocument doc = QJsonDocument::fromJson(replyStr.toUtf8());                
    if (!doc.isObject()) { Q_EMIT error(); return; }                                 
    QJsonObject obj = doc.object();                                                
    QJsonArray arr = obj["MemberList"].toArray();                              
    m_contactList.append(new UserObject("groupsend", i18n("Group Send"), ""));
    Q_FOREACH (const QJsonValue & val, arr) {
        QJsonObject user = val.toObject();                                         
        m_contactList.append(new UserObject(user["UserName"].toString(), 
                   user["NickName"].toString(), 
                   m_v2 ? WX_V2_SERVER_HOST + user["HeadImgUrl"].toString() : 
                        WX_SERVER_HOST + user["HeadImgUrl"].toString()));
    }                                                                              
    Q_EMIT contactListChanged();
}

QString Contact::getNickName(QString userName) 
{
    for (QObject* obj : m_contactList) {
        UserObject* contact = qobject_cast<UserObject*>(obj);
        if (contact->userName() == userName)
            return contact->nickName();
    }
    return i18n("WeChat");
}

QString Contact::getHeadImgUrl(QString userName) 
{
    for (QObject* obj : m_contactList) {
        UserObject* contact = qobject_cast<UserObject*>(obj);
        if (contact->userName() == userName)
            return contact->headImgUrl();
    }
    return "";
}
