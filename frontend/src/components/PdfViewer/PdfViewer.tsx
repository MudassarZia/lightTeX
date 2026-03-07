import { createSignal, createEffect, Show } from "solid-js";

interface PdfViewerProps {
  pdfPath: string | null;
}

/**
 * PDF preview pane.
 * For v0.1, displays the PDF path and a placeholder.
 * Will be replaced with MuPDF canvas rendering.
 */
function PdfViewer(props: PdfViewerProps) {
  return (
    <div class="pdf-viewer" style={{
      display: "flex",
      "flex-direction": "column",
      "align-items": "center",
      "justify-content": "center",
      height: "100%",
      padding: "20px",
      color: "var(--editor-fg, #d4d4d4)",
    }}>
      <Show
        when={props.pdfPath}
        fallback={
          <div style={{ "text-align": "center", opacity: 0.5 }}>
            <p style={{ "font-size": "18px", "margin-bottom": "8px" }}>PDF Preview</p>
            <p style={{ "font-size": "13px" }}>
              Compile a .tex file to see the preview here
            </p>
          </div>
        }
      >
        <div style={{ "text-align": "center" }}>
          <p style={{ "font-size": "14px", "margin-bottom": "8px" }}>
            PDF compiled successfully
          </p>
          <p style={{ "font-size": "12px", opacity: 0.7 }}>
            {props.pdfPath}
          </p>
          <p style={{
            "font-size": "12px",
            "margin-top": "16px",
            opacity: 0.5,
          }}>
            MuPDF rendering will be integrated here
          </p>
        </div>
      </Show>
    </div>
  );
}

export default PdfViewer;
