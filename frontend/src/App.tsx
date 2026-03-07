import { createSignal, onMount } from "solid-js";
import { invoke } from "@tauri-apps/api/core";
import Editor from "./components/Editor/Editor";
import PdfViewer from "./components/PdfViewer/PdfViewer";
import StatusBar from "./components/StatusBar/StatusBar";

import "./App.css";

function App() {
  const [content, setContent] = createSignal("");
  const [fileName, setFileName] = createSignal("Untitled");
  const [filePath, setFilePath] = createSignal("");
  const [compileStatus, setCompileStatus] = createSignal<string>("idle");
  const [line, setLine] = createSignal(1);
  const [col, setCol] = createSignal(1);
  const [pdfPath, setPdfPath] = createSignal<string | null>(null);

  onMount(async () => {
    // Load theme on startup
    try {
      const vars = await invoke<Record<string, string>>("get_theme");
      applyTheme(vars);
    } catch (e) {
      console.error("Failed to load theme:", e);
    }
  });

  function applyTheme(vars: Record<string, string>) {
    const root = document.documentElement;
    for (const [key, value] of Object.entries(vars)) {
      root.style.setProperty(key, value);
    }
  }

  async function handleOpenFile() {
    // For now, use a prompt. Will use Tauri file dialog later.
    const path = prompt("Enter file path:");
    if (!path) return;

    try {
      const info = await invoke<{ name: string; path: string; content: string }>(
        "open_file",
        { path }
      );
      setContent(info.content);
      setFileName(info.name);
      setFilePath(info.path);
    } catch (e) {
      console.error("Failed to open file:", e);
    }
  }

  async function handleSave() {
    try {
      await invoke("save_file");
    } catch (e) {
      console.error("Failed to save:", e);
    }
  }

  async function handleCompile() {
    setCompileStatus("compiling");
    try {
      const result = await invoke<{
        status: string;
        messages: Array<{ kind: string; message: string; line?: number }>;
        pdf_path: string | null;
        log_output: string;
      }>("compile");

      setCompileStatus(result.status);
      if (result.pdf_path) {
        setPdfPath(result.pdf_path);
      }
    } catch (e) {
      setCompileStatus("error");
      console.error("Compile error:", e);
    }
  }

  function handleContentChange(newContent: string) {
    setContent(newContent);
  }

  function handleCursorChange(newLine: number, newCol: number) {
    setLine(newLine);
    setCol(newCol);
  }

  return (
    <div class="app">
      <div class="toolbar">
        <button onClick={handleOpenFile} title="Open File">Open</button>
        <button onClick={handleSave} title="Save">Save</button>
        <button onClick={handleCompile} title="Compile">Compile</button>
        <span class="filename">{fileName()}</span>
      </div>
      <div class="main-content">
        <div class="editor-pane">
          <Editor
            content={content()}
            onContentChange={handleContentChange}
            onCursorChange={handleCursorChange}
          />
        </div>
        <div class="pdf-pane">
          <PdfViewer pdfPath={pdfPath()} />
        </div>
      </div>
      <StatusBar
        line={line()}
        col={col()}
        compileStatus={compileStatus()}
        engine="pdfLaTeX"
        fileName={fileName()}
      />
    </div>
  );
}

export default App;
