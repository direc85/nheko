// SPDX-FileCopyrightText: Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SSOHandler.h"

#include <QHttpServerResponse>
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <QTcpServer>
#include "Logging.h"
#endif

SSOHandler::SSOHandler(QObject *)
  : server{new QHttpServer}
{
    QTimer::singleShot(120000, this, &SSOHandler::ssoFailed);

    server->route("/sso", [this](const QHttpServerRequest &req) {
        if (req.query().hasQueryItem(QStringLiteral("loginToken"))) {
            emit ssoSuccess(req.query().queryItemValue(QStringLiteral("loginToken")).toStdString());
            return tr("SSO success");
        } else {
            emit ssoFailed();
            return tr("Missing loginToken for SSO login!");
        }
    });

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    QTcpServer *tcpServer =  new QTcpServer();
    if (tcpServer->listen() && server->bind(tcpServer)) {
        this->port = tcpServer->serverPort();
    } else {
        nhlog::net()->critical("SSO listener failed: {}", tcpServer->errorString().toStdString());
        delete tcpServer;
        tcpServer = nullptr;
    }
#else
    server->listen();
    if (server->serverPorts().size() > 0)
        this->port = server->serverPorts().first();
#endif
}

SSOHandler::~SSOHandler()
{
    // work around capturing a member of a deleted object
    auto s = server;
    QTimer::singleShot(1000, [s] { s->deleteLater(); });
}

std::string
SSOHandler::url() const
{
    return "http://localhost:" + std::to_string(port) + "/sso";
}

#include "moc_SSOHandler.cpp"
