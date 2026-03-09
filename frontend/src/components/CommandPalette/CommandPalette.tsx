import { createSignal, createEffect, For, Show, onCleanup } from "solid-js";

export interface Command {
  id: string;
  label: string;
  category?: string;
  shortcut?: string;
  action: () => void;
}

interface CommandPaletteProps {
  commands: Command[];
  open: boolean;
  onClose: () => void;
}

function CommandPalette(props: CommandPaletteProps) {
  const [query, setQuery] = createSignal("");
  const [selectedIndex, setSelectedIndex] = createSignal(0);
  let inputRef: HTMLInputElement | undefined;

  const filtered = () => {
    const q = query().toLowerCase();
    if (!q) return props.commands;
    return props.commands.filter(
      (cmd) =>
        cmd.label.toLowerCase().includes(q) ||
        (cmd.category && cmd.category.toLowerCase().includes(q))
    );
  };

  createEffect(() => {
    if (props.open) {
      setQuery("");
      setSelectedIndex(0);
      setTimeout(() => inputRef?.focus(), 0);
    }
  });

  createEffect(() => {
    filtered();
    setSelectedIndex(0);
  });

  function handleKeyDown(e: KeyboardEvent) {
    const items = filtered();
    switch (e.key) {
      case "ArrowDown":
        e.preventDefault();
        setSelectedIndex((i) => Math.min(i + 1, items.length - 1));
        break;
      case "ArrowUp":
        e.preventDefault();
        setSelectedIndex((i) => Math.max(i - 1, 0));
        break;
      case "Enter":
        e.preventDefault();
        if (items[selectedIndex()]) {
          items[selectedIndex()].action();
          props.onClose();
        }
        break;
      case "Escape":
        e.preventDefault();
        props.onClose();
        break;
    }
  }

  function handleBackdropClick(e: MouseEvent) {
    if ((e.target as HTMLElement).classList.contains("command-palette-backdrop")) {
      props.onClose();
    }
  }

  return (
    <Show when={props.open}>
      <div class="command-palette-backdrop" onClick={handleBackdropClick}>
        <div class="command-palette">
          <input
            ref={inputRef}
            class="command-palette-input"
            type="text"
            placeholder="Type a command..."
            value={query()}
            onInput={(e) => setQuery(e.currentTarget.value)}
            onKeyDown={handleKeyDown}
          />
          <div class="command-palette-list">
            <For each={filtered()}>
              {(cmd, index) => (
                <div
                  class={`command-palette-item ${index() === selectedIndex() ? "selected" : ""}`}
                  onClick={() => {
                    cmd.action();
                    props.onClose();
                  }}
                  onMouseEnter={() => setSelectedIndex(index())}
                >
                  <span class="command-palette-label">{cmd.label}</span>
                  <Show when={cmd.category}>
                    <span class="command-palette-category">{cmd.category}</span>
                  </Show>
                  <Show when={cmd.shortcut}>
                    <span class="command-palette-shortcut">{cmd.shortcut}</span>
                  </Show>
                </div>
              )}
            </For>
            <Show when={filtered().length === 0}>
              <div class="command-palette-empty">No matching commands</div>
            </Show>
          </div>
        </div>
      </div>
    </Show>
  );
}

export default CommandPalette;
