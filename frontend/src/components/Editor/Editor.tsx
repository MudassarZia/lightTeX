import { onMount, onCleanup, createEffect } from "solid-js";
import { EditorState } from "@codemirror/state";
import { EditorView, keymap, lineNumbers, highlightActiveLine } from "@codemirror/view";
import { defaultKeymap, history, historyKeymap } from "@codemirror/commands";
import { searchKeymap, highlightSelectionMatches } from "@codemirror/search";
import { bracketMatching } from "@codemirror/language";

interface EditorProps {
  content: string;
  onContentChange: (content: string) => void;
  onCursorChange: (line: number, col: number) => void;
}

/**
 * EditorSurface abstraction: CodeMirror 6 implementation.
 * This component wraps CodeMirror 6 behind a stable interface.
 * In the future, it can be replaced with a custom canvas renderer
 * without changing the rest of the application.
 */
function Editor(props: EditorProps) {
  let containerRef: HTMLDivElement | undefined;
  let view: EditorView | undefined;
  let isExternalUpdate = false;

  const theme = EditorView.theme({
    "&": {
      height: "100%",
      fontSize: "14px",
      backgroundColor: "var(--editor-bg, #1e1e1e)",
      color: "var(--editor-fg, #d4d4d4)",
    },
    ".cm-content": {
      fontFamily: "'JetBrains Mono', 'Fira Code', 'Consolas', monospace",
      caretColor: "var(--editor-cursor, #aeafad)",
    },
    ".cm-cursor": {
      borderLeftColor: "var(--editor-cursor, #aeafad)",
    },
    "&.cm-focused .cm-selectionBackground, .cm-selectionBackground": {
      backgroundColor: "var(--editor-selection, #264f78) !important",
    },
    ".cm-activeLine": {
      backgroundColor: "var(--editor-line-highlight, #2a2d2e)",
    },
    ".cm-gutters": {
      backgroundColor: "var(--editor-gutter, #1e1e1e)",
      color: "var(--editor-gutter-fg, #858585)",
      border: "none",
    },
    ".cm-activeLineGutter": {
      backgroundColor: "var(--editor-line-highlight, #2a2d2e)",
    },
  });

  onMount(() => {
    if (!containerRef) return;

    const updateListener = EditorView.updateListener.of((update) => {
      if (update.docChanged && !isExternalUpdate) {
        const content = update.state.doc.toString();
        props.onContentChange(content);
      }
      if (update.selectionSet) {
        const pos = update.state.selection.main.head;
        const line = update.state.doc.lineAt(pos);
        props.onCursorChange(line.number, pos - line.from + 1);
      }
    });

    const state = EditorState.create({
      doc: props.content || "",
      extensions: [
        lineNumbers(),
        highlightActiveLine(),
        history(),
        bracketMatching(),
        highlightSelectionMatches(),
        keymap.of([...defaultKeymap, ...historyKeymap, ...searchKeymap]),
        theme,
        updateListener,
      ],
    });

    view = new EditorView({
      state,
      parent: containerRef,
    });
  });

  createEffect(() => {
    const newContent = props.content;
    if (view && newContent !== view.state.doc.toString()) {
      isExternalUpdate = true;
      view.dispatch({
        changes: {
          from: 0,
          to: view.state.doc.length,
          insert: newContent,
        },
      });
      isExternalUpdate = false;
    }
  });

  onCleanup(() => {
    view?.destroy();
  });

  return (
    <div
      ref={containerRef}
      style={{ height: "100%", overflow: "hidden" }}
    />
  );
}

export default Editor;
