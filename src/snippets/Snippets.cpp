#include "snippets/Snippets.h"

#include <QFile>
#include <fstream>
#include <regex>
#include <sstream>
#include <toml++/toml.hpp>

namespace lighttex::snippets {

SnippetManager::SnippetManager() {
    loadDefaults();
}

void SnippetManager::loadDefaults() {
    // Try loading from Qt resource
    QFile res(":/snippets/latex.toml");
    if (res.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(res.readAll());
        loadFromToml(content.toStdString());
        return;
    }

    // Fallback: built-in defaults
    snippets_ = {
        {"\\begin", "Begin Environment",
         "\\begin{${1:environment}}\n\t$0\n\\end{${1:environment}}"},
        {"\\frac", "Fraction", "\\frac{${1:num}}{${2:den}}$0"},
        {"\\fig", "Figure",
         "\\begin{figure}[${1:htbp}]\n\t\\centering\n\t\\includegraphics"
         "[width=${2:0.8}\\textwidth]{${3:file}}\n\t\\caption{${4:caption}}"
         "\n\t\\label{fig:${5:label}}\n\\end{figure}$0"},
        {"\\tab", "Table",
         "\\begin{table}[${1:htbp}]\n\t\\centering\n\t\\begin{tabular}"
         "{${2:cc}}\n\t\t$0\n\t\\end{tabular}\n\t\\caption{${3:caption}}"
         "\n\t\\label{tab:${4:label}}\n\\end{table}"},
        {"\\eq", "Equation",
         "\\begin{equation}\n\t${1:equation}\n\t\\label{eq:${2:label}}"
         "\n\\end{equation}$0"},
        {"\\enum", "Enumerate",
         "\\begin{enumerate}\n\t\\item $0\n\\end{enumerate}"},
        {"\\item", "Itemize",
         "\\begin{itemize}\n\t\\item $0\n\\end{itemize}"},
        {"\\sec", "Section", "\\section{${1:title}}\n$0"},
        {"\\subsec", "Subsection", "\\subsection{${1:title}}\n$0"},
        {"\\cite", "Citation", "\\cite{${1:key}}$0"},
        {"\\ref", "Reference", "\\ref{${1:label}}$0"},
        {"\\label", "Label", "\\label{${1:label}}$0"},
        {"\\emph", "Emphasize", "\\emph{${1:text}}$0"},
        {"\\textbf", "Bold", "\\textbf{${1:text}}$0"},
        {"\\textit", "Italic", "\\textit{${1:text}}$0"},
        {"\\href", "Hyperlink", "\\href{${1:url}}{${2:text}}$0"},
        {"\\url", "URL", "\\url{${1:url}}$0"},
        {"\\doc", "Document",
         "\\documentclass[${1:12pt}]{${2:article}}\n\n"
         "\\begin{document}\n\n$0\n\n\\end{document}"},
    };
}

bool SnippetManager::loadFromToml(const std::string& tomlStr) {
    try {
        auto tbl = toml::parse(tomlStr);

        auto snippetArr = tbl["snippet"];
        if (!snippetArr.is_array_of_tables()) return false;

        snippets_.clear();
        auto* arr = snippetArr.as_array();
        for (auto& elem : *arr) {
            auto* t = elem.as_table();
            if (!t) continue;

            Snippet s;
            if (auto trigger = (*t)["trigger"].value<std::string>())
                s.trigger = *trigger;
            if (auto label = (*t)["label"].value<std::string>())
                s.label = *label;
            if (auto body = (*t)["body"].value<std::string>())
                s.body = *body;

            if (!s.trigger.empty() && !s.body.empty()) {
                snippets_.push_back(std::move(s));
            }
        }
        return true;
    } catch (const toml::parse_error&) {
        return false;
    }
}

bool SnippetManager::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::ostringstream ss;
    ss << file.rdbuf();
    return loadFromToml(ss.str());
}

const Snippet* SnippetManager::findByTrigger(const std::string& trigger) const {
    for (const auto& s : snippets_) {
        if (s.trigger == trigger) {
            return &s;
        }
    }
    return nullptr;
}

std::string SnippetManager::expandBody(const std::string& body) {
    // Replace ${N:placeholder} with just the placeholder text
    // Replace $N and $0 with empty string
    std::string result;
    size_t i = 0;

    while (i < body.size()) {
        if (body[i] == '$' && i + 1 < body.size()) {
            if (body[i + 1] == '{') {
                // ${N:placeholder} pattern
                size_t colonPos = body.find(':', i + 2);
                size_t closePos = body.find('}', i + 2);
                if (colonPos != std::string::npos &&
                    closePos != std::string::npos &&
                    colonPos < closePos) {
                    // Extract placeholder
                    result += body.substr(colonPos + 1, closePos - colonPos - 1);
                    i = closePos + 1;
                    continue;
                } else if (closePos != std::string::npos) {
                    // ${N} with no placeholder
                    i = closePos + 1;
                    continue;
                }
            } else if (body[i + 1] >= '0' && body[i + 1] <= '9') {
                // $N — skip
                i += 2;
                continue;
            }
        }
        result += body[i];
        ++i;
    }

    return result;
}

} // namespace lighttex::snippets
