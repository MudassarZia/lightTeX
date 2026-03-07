interface StatusBarProps {
  line: number;
  col: number;
  compileStatus: string;
  engine: string;
  fileName: string;
}

function StatusBar(props: StatusBarProps) {
  const statusColor = () => {
    switch (props.compileStatus) {
      case "Success":
        return "#4ec9b0";
      case "Error":
        return "#f44747";
      case "Compiling":
        return "#dcdcaa";
      default:
        return "inherit";
    }
  };

  const statusText = () => {
    switch (props.compileStatus) {
      case "Success":
        return "Compiled";
      case "Error":
        return "Error";
      case "Compiling":
        return "Compiling...";
      default:
        return "Ready";
    }
  };

  return (
    <div
      class="status-bar"
      style={{
        display: "flex",
        "align-items": "center",
        "justify-content": "space-between",
        padding: "2px 12px",
        "font-size": "12px",
        background: "var(--ui-statusbar-bg, #007acc)",
        color: "var(--ui-statusbar-fg, #ffffff)",
        "min-height": "22px",
      }}
    >
      <div style={{ display: "flex", gap: "16px", "align-items": "center" }}>
        <span>
          Ln {props.line}, Col {props.col}
        </span>
        <span style={{ color: statusColor() }}>{statusText()}</span>
      </div>
      <div style={{ display: "flex", gap: "16px", "align-items": "center" }}>
        <span>{props.engine}</span>
        <span>UTF-8</span>
      </div>
    </div>
  );
}

export default StatusBar;
