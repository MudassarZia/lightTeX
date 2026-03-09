#include "compiler/Pipeline.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace lighttex::compiler {

Compiler::Compiler(QObject *parent) : QObject(parent) {}

void Compiler::compile(const std::string &sourcePath) {
  if (status_ == CompileStatus::Compiling)
    return;

  currentSourcePath_ = sourcePath;
  status_ = CompileStatus::Compiling;
  emit compilationStarted();

  QFileInfo sourceInfo(QString::fromStdString(sourcePath));
  if (!sourceInfo.exists()) {
    CompileResult result;
    result.status = CompileStatus::Error;
    result.logOutput = "Source file not found: " + sourcePath;
    status_ = CompileStatus::Error;
    emit compilationFinished(result);
    return;
  }

  QString workDir = sourceInfo.absolutePath();
  QString outDir =
      outputDir_.empty() ? workDir : QString::fromStdString(outputDir_);

  // Build command arguments
  QStringList args;
  for (const auto &arg : engineDefaultArgs()) {
    args << QString::fromStdString(arg);
  }
  args << "-output-directory" << outDir;
  args << sourceInfo.absoluteFilePath();

  process_ = new QProcess(this);
  process_->setWorkingDirectory(workDir);

  connect(process_,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
          &Compiler::onProcessFinished);

  process_->start(QString::fromStdString(std::string(engineCommand(engine_))),
                  args);
}

void Compiler::onProcessFinished(int exitCode,
                                 QProcess::ExitStatus /*exitStatus*/) {
  CompileResult result;

  // Capture process output
  QString stdout_str = process_->readAllStandardOutput();
  QString stderr_str = process_->readAllStandardError();
  result.logOutput = stdout_str.toStdString() + stderr_str.toStdString();

  // Try to read .log file for detailed parsing
  QFileInfo sourceInfo(QString::fromStdString(currentSourcePath_));
  QString outDir = outputDir_.empty() ? sourceInfo.absolutePath()
                                      : QString::fromStdString(outputDir_);
  QString logPath = outDir + "/" + sourceInfo.completeBaseName() + ".log";

  QFile logFile(logPath);
  if (logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream stream(&logFile);
    QString logContent = stream.readAll();
    logFile.close();
    result.messages = parseLog(logContent.toStdString());
    if (!result.logOutput.empty()) {
      result.logOutput += "\n";
    }
    result.logOutput += logContent.toStdString();
  } else {
    result.messages = parseLog(result.logOutput);
  }

  // Check for PDF output
  QString pdfPath = outDir + "/" + sourceInfo.completeBaseName() + ".pdf";
  QFileInfo pdfInfo(pdfPath);

  bool hasErrors = false;
  for (const auto &msg : result.messages) {
    if (msg.kind == MessageKind::Error) {
      hasErrors = true;
      break;
    }
  }

  if (exitCode == 0 && !hasErrors && pdfInfo.exists()) {
    result.status = CompileStatus::Success;
    result.pdfPath = pdfPath.toStdString();
  } else {
    result.status = CompileStatus::Error;
    if (pdfInfo.exists()) {
      result.pdfPath = pdfPath.toStdString();
    }
  }

  status_ = result.status;
  process_->deleteLater();
  process_ = nullptr;

  emit compilationFinished(result);
}

} // namespace lighttex::compiler
