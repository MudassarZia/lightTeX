#include "lsp/Lsp.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>

namespace lighttex::lsp {

LspClient::LspClient(QObject* parent) : QObject(parent) {
    changeTimer_.setSingleShot(true);
    changeTimer_.setInterval(300);
    connect(&changeTimer_, &QTimer::timeout,
            this, &LspClient::onDebouncedChange);
}

LspClient::~LspClient() {
    stop();
}

bool LspClient::isTexlabAvailable() {
    QString path = QStandardPaths::findExecutable("texlab");
    return !path.isEmpty();
}

bool LspClient::start(const std::string& rootUri) {
    if (!isTexlabAvailable()) {
        emit statusChanged("texlab: not found");
        return false;
    }

    process_ = new QProcess(this);
    connect(process_, &QProcess::readyReadStandardOutput,
            this, &LspClient::onReadyRead);
    connect(process_, QOverload<int, QProcess::ExitStatus>::of(
                          &QProcess::finished),
            this, &LspClient::onProcessFinished);

    // Discard stderr — texlab sends logs there
    connect(process_, &QProcess::readyReadStandardError,
            this, [this]() {
        process_->readAllStandardError();
    });

    process_->start("texlab", QStringList());
    if (!process_->waitForStarted(5000)) {
        emit statusChanged("texlab: failed to start");
        delete process_;
        process_ = nullptr;
        return false;
    }

    emit statusChanged("texlab: starting");

    // Send initialize request
    QJsonObject params;
    params["processId"] = static_cast<int>(QCoreApplication::applicationPid());
    params["rootUri"] = QString::fromStdString(rootUri);
    params["capabilities"] = QJsonObject{
        {"textDocument", QJsonObject{
            {"completion", QJsonObject{
                {"completionItem", QJsonObject{{"snippetSupport", true}}}
            }},
            {"hover", QJsonObject{
                {"contentFormat", QJsonArray{{"plaintext"}}}
            }},
            {"definition", QJsonObject{}},
            {"publishDiagnostics", QJsonObject{}}
        }}
    };

    int initId = nextId_;
    pendingRequests_[initId] = [this](const QJsonObject& /*result*/) {
        sendNotification("initialized");
        initialized_ = true;
        emit statusChanged("texlab");
        emit initialized();
        flushPendingOps();
    };
    sendRequest(initId, "initialize", params);

    return true;
}

void LspClient::flushPendingOps() {
    auto ops = std::move(pendingOps_);
    pendingOps_.clear();
    for (auto& op : ops) {
        op();
    }
}

void LspClient::stop() {
    if (!process_) return;

    if (initialized_) {
        int shutdownId = nextId_;
        pendingRequests_[shutdownId] = [this](const QJsonObject&) {
            sendNotification("exit");
        };
        sendRequest(shutdownId, "shutdown");
        process_->waitForFinished(3000);
    }

    if (process_->state() != QProcess::NotRunning) {
        process_->kill();
        process_->waitForFinished(1000);
    }

    delete process_;
    process_ = nullptr;
    initialized_ = false;
    pendingRequests_.clear();
    pendingOps_.clear();
}

bool LspClient::isRunning() const {
    return process_ && process_->state() == QProcess::Running && initialized_;
}

bool LspClient::isStarted() const {
    return process_ && process_->state() == QProcess::Running;
}

void LspClient::didOpen(const QString& uri, const QString& text) {
    if (!isStarted()) return;

    if (!initialized_) {
        pendingOps_.push_back([this, uri, text]() {
            didOpen(uri, text);
        });
        return;
    }

    QJsonObject params;
    params["textDocument"] = QJsonObject{
        {"uri", uri},
        {"languageId", "latex"},
        {"version", 1},
        {"text", text}
    };
    sendNotification("textDocument/didOpen", params);
}

void LspClient::didChange(const QString& uri, const QString& text) {
    if (!isRunning()) return;

    pendingChangeUri_ = uri;
    pendingChangeText_ = text;
    changeTimer_.start();
}

void LspClient::flushPendingChange() {
    if (changeTimer_.isActive()) {
        changeTimer_.stop();
        onDebouncedChange();
    }
}

void LspClient::onDebouncedChange() {
    if (!isRunning() || pendingChangeUri_.isEmpty()) return;

    QJsonObject params;
    params["textDocument"] = QJsonObject{
        {"uri", pendingChangeUri_},
        {"version", nextId_}
    };
    params["contentChanges"] = QJsonArray{
        QJsonObject{{"text", pendingChangeText_}}
    };
    sendNotification("textDocument/didChange", params);
}

void LspClient::didSave(const QString& uri) {
    if (!isRunning()) return;

    QJsonObject params;
    params["textDocument"] = QJsonObject{{"uri", uri}};
    sendNotification("textDocument/didSave", params);
}

void LspClient::didClose(const QString& uri) {
    if (!isRunning()) return;

    QJsonObject params;
    params["textDocument"] = QJsonObject{{"uri", uri}};
    sendNotification("textDocument/didClose", params);
}

void LspClient::requestCompletion(const QString& uri, int line,
                                   int character) {
    if (!isRunning()) return;

    flushPendingChange();

    QJsonObject params;
    params["textDocument"] = QJsonObject{{"uri", uri}};
    params["position"] = QJsonObject{
        {"line", line}, {"character", character}};

    int id = nextId_;
    pendingRequests_[id] = [this](const QJsonObject& result) {
        std::vector<CompletionItem> items;
        QJsonArray arr;
        if (result.contains("items")) {
            arr = result["items"].toArray();
        }
        for (const auto& v : arr) {
            items.push_back(CompletionItem::fromJson(v.toObject()));
        }
        emit completionReceived(items);
    };
    sendRequest(id, "textDocument/completion", params);
}

void LspClient::requestHover(const QString& uri, int line, int character) {
    if (!isRunning()) return;

    flushPendingChange();

    QJsonObject params;
    params["textDocument"] = QJsonObject{{"uri", uri}};
    params["position"] = QJsonObject{
        {"line", line}, {"character", character}};

    int id = nextId_;
    pendingRequests_[id] = [this](const QJsonObject& result) {
        if (!result.isEmpty()) {
            emit hoverReceived(Hover::fromJson(result));
        }
    };
    sendRequest(id, "textDocument/hover", params);
}

void LspClient::requestDefinition(const QString& uri, int line,
                                   int character) {
    if (!isRunning()) return;

    flushPendingChange();

    QJsonObject params;
    params["textDocument"] = QJsonObject{{"uri", uri}};
    params["position"] = QJsonObject{
        {"line", line}, {"character", character}};

    int id = nextId_;
    pendingRequests_[id] = [this](const QJsonObject& result) {
        if (!result.isEmpty()) {
            emit definitionReceived(Location::fromJson(result));
        }
    };
    sendRequest(id, "textDocument/definition", params);
}

void LspClient::sendRequest(int id, const QString& method,
                              const QJsonObject& params) {
    if (!process_) return;
    QByteArray data = encodeRequest(id, method, params);
    process_->write(data);
    nextId_ = id + 1;
}

void LspClient::sendNotification(const QString& method,
                                  const QJsonObject& params) {
    if (!process_) return;
    QByteArray data = encodeNotification(method, params);
    process_->write(data);
}

void LspClient::onReadyRead() {
    QByteArray data = process_->readAllStandardOutput();
    parser_.feed(data);

    while (auto msg = parser_.nextMessage()) {
        if (msg->contains("id") && !msg->contains("method")) {
            handleResponse(*msg);
        } else if (msg->contains("method")) {
            handleNotification(*msg);
        }
    }
}

void LspClient::handleResponse(const QJsonObject& msg) {
    int id = msg["id"].toInt();
    auto it = pendingRequests_.find(id);
    if (it == pendingRequests_.end()) return;

    if (msg.contains("error")) {
        pendingRequests_.erase(it);
        return;
    }

    auto resultVal = msg["result"];

    if (resultVal.isNull() || resultVal.isUndefined()) {
        it->second(QJsonObject());
        pendingRequests_.erase(it);
        return;
    }

    // Handle array results (completion and definition can return arrays)
    if (resultVal.isArray()) {
        QJsonArray arr = resultVal.toArray();
        if (!arr.isEmpty() && arr[0].toObject().contains("label")) {
            std::vector<CompletionItem> items;
            for (const auto& v : arr) {
                items.push_back(CompletionItem::fromJson(v.toObject()));
            }
            emit completionReceived(items);
            pendingRequests_.erase(it);
            return;
        }
        if (!arr.isEmpty() && arr[0].toObject().contains("uri")) {
            emit definitionReceived(Location::fromJson(arr[0].toObject()));
            pendingRequests_.erase(it);
            return;
        }
        it->second(QJsonObject());
        pendingRequests_.erase(it);
        return;
    }

    it->second(resultVal.toObject());
    pendingRequests_.erase(it);
}

void LspClient::handleNotification(const QJsonObject& msg) {
    QString method = msg["method"].toString();
    QJsonObject params = msg["params"].toObject();

    if (method == "textDocument/publishDiagnostics") {
        QString uri = params["uri"].toString();
        std::vector<Diagnostic> diagnostics;
        QJsonArray arr = params["diagnostics"].toArray();
        for (const auto& v : arr) {
            diagnostics.push_back(Diagnostic::fromJson(v.toObject()));
        }
        emit diagnosticsReceived(uri, diagnostics);
    }
}

void LspClient::onProcessFinished(int /*exitCode*/,
                                    QProcess::ExitStatus /*status*/) {
    initialized_ = false;
    emit statusChanged("texlab: stopped");
}

} // namespace lighttex::lsp
