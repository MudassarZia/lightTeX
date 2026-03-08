#pragma once

#include "compiler/Engine.h"
#include "compiler/LogParser.h"

#include <QObject>
#include <QProcess>
#include <optional>
#include <string>

namespace lighttex::compiler {

enum class CompileStatus {
    Idle,
    Compiling,
    Success,
    Error
};

struct CompileResult {
    CompileStatus status = CompileStatus::Idle;
    std::vector<CompileMessage> messages;
    std::optional<std::string> pdfPath;
    std::string logOutput;
};

class Compiler : public QObject {
    Q_OBJECT

public:
    explicit Compiler(QObject* parent = nullptr);

    void setEngine(Engine engine) { engine_ = engine; }
    [[nodiscard]] Engine engine() const { return engine_; }

    void setOutputDir(const std::string& dir) { outputDir_ = dir; }
    [[nodiscard]] const std::string& outputDir() const { return outputDir_; }

    void compile(const std::string& sourcePath);

    [[nodiscard]] CompileStatus status() const { return status_; }

signals:
    void compilationStarted();
    void compilationFinished(lighttex::compiler::CompileResult result);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    Engine engine_ = Engine::PdfLatex;
    std::string outputDir_;
    CompileStatus status_ = CompileStatus::Idle;
    QProcess* process_ = nullptr;
    std::string currentSourcePath_;
};

} // namespace lighttex::compiler
