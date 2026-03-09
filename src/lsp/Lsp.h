#pragma once

#include "lsp/JsonRpc.h"
#include "lsp/LspTypes.h"

#include <QObject>
#include <QProcess>
#include <QTimer>

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace lighttex::lsp {

class LspClient : public QObject {
    Q_OBJECT

public:
    explicit LspClient(QObject* parent = nullptr);
    ~LspClient() override;

    bool start(const std::string& rootUri);
    void stop();
    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] bool isStarted() const;

    // Document sync
    void didOpen(const QString& uri, const QString& text);
    void didChange(const QString& uri, const QString& text);
    void didSave(const QString& uri);
    void didClose(const QString& uri);

    // Flush any pending didChange immediately (call before requests)
    void flushPendingChange();

    // Requests
    void requestCompletion(const QString& uri, int line, int character);
    void requestHover(const QString& uri, int line, int character);
    void requestDefinition(const QString& uri, int line, int character);

    [[nodiscard]] static bool isTexlabAvailable();

signals:
    void initialized();
    void completionReceived(const std::vector<CompletionItem>& items);
    void hoverReceived(const Hover& hover);
    void definitionReceived(const Location& location);
    void diagnosticsReceived(const QString& uri,
                              const std::vector<Diagnostic>& diagnostics);
    void statusChanged(const QString& status);

private slots:
    void onReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onDebouncedChange();
    void flushPendingOps();

private:
    void sendRequest(int id, const QString& method,
                     const QJsonObject& params = {});
    void sendNotification(const QString& method,
                          const QJsonObject& params = {});
    void handleResponse(const QJsonObject& msg);
    void handleNotification(const QJsonObject& msg);

    QProcess* process_ = nullptr;
    MessageParser parser_;
    int nextId_ = 1;
    bool initialized_ = false;

    // Pending request handlers
    std::map<int, std::function<void(const QJsonObject&)>> pendingRequests_;

    // Debounce for didChange
    QTimer changeTimer_;
    QString pendingChangeUri_;
    QString pendingChangeText_;

    // Queue operations until initialized
    std::vector<std::function<void()>> pendingOps_;
};

} // namespace lighttex::lsp
