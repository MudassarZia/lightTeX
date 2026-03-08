#include "compiler/SyncTeX.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QString>

namespace lighttex::compiler {

bool SyncTeX::load(const std::string& pdfPath) {
    QString base = QString::fromStdString(pdfPath);
    base.replace(QRegularExpression(R"(\.pdf$)"), "");

    // Check for .synctex.gz or .synctex
    QFileInfo gzInfo(base + ".synctex.gz");
    if (gzInfo.exists()) {
        synctexPath_ = gzInfo.absoluteFilePath().toStdString();
        return true;
    }

    QFileInfo plainInfo(base + ".synctex");
    if (plainInfo.exists()) {
        synctexPath_ = plainInfo.absoluteFilePath().toStdString();
        return true;
    }

    synctexPath_.clear();
    return false;
}

std::optional<PdfPosition> parseForwardOutput(const std::string& output) {
    QString str = QString::fromStdString(output);

    static const QRegularExpression pageRe(R"(Page:(\d+))");
    static const QRegularExpression xRe(R"(x:([0-9.]+))");
    static const QRegularExpression yRe(R"(y:([0-9.]+))");

    auto pageMatch = pageRe.match(str);
    auto xMatch = xRe.match(str);
    auto yMatch = yRe.match(str);

    if (!pageMatch.hasMatch() || !xMatch.hasMatch() || !yMatch.hasMatch()) {
        return std::nullopt;
    }

    return PdfPosition{
        pageMatch.captured(1).toInt(),
        xMatch.captured(1).toDouble(),
        yMatch.captured(1).toDouble()
    };
}

std::optional<SourcePosition> parseInverseOutput(const std::string& output) {
    QString str = QString::fromStdString(output);

    static const QRegularExpression inputRe(R"(Input:(.+))");
    static const QRegularExpression lineRe(R"(Line:(\d+))");
    static const QRegularExpression colRe(R"(Column:(\d+))");

    auto inputMatch = inputRe.match(str);
    auto lineMatch = lineRe.match(str);

    if (!inputMatch.hasMatch() || !lineMatch.hasMatch()) {
        return std::nullopt;
    }

    int column = 0;
    auto colMatch = colRe.match(str);
    if (colMatch.hasMatch()) {
        column = colMatch.captured(1).toInt();
    }

    return SourcePosition{
        inputMatch.captured(1).trimmed().toStdString(),
        lineMatch.captured(1).toInt(),
        column
    };
}

} // namespace lighttex::compiler
