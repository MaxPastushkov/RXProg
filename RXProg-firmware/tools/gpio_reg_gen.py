#!/usr/bin/env python3
"""
STM32 GPIO register value generator.

Simple Tkinter GUI to build MODER, OTYPER and PUPDR register values for a
single GPIO port by picking the configuration of each of the 16 pins.

MODER  (2 bits/pin): 00 input, 01 output, 10 alternate function, 11 reset
OTYPER (1 bit/pin) : 0 push-pull, 1 open-drain
PUPDR  (2 bits/pin): 00 none, 01 pull-up, 10 pull-down
"""

import signal
import tkinter as tk
from tkinter import ttk

# (label, 2-bit value) options for the MODER register, in dropdown order.
MODER_OPTIONS = [
    ("Input", 0b00),
    ("Output", 0b01),
    ("Alternate", 0b10),
    ("Reset", 0b11),
]

# (label, 1-bit value) options for the OTYPER register.
OTYPER_OPTIONS = [
    ("Push-pull", 0b0),
    ("Open-drain", 0b1),
]

# (label, 2-bit value) options for the PUPDR register.
PUPDR_OPTIONS = [
    ("None", 0b00),
    ("Pull-up", 0b01),
    ("Pull-down", 0b10),
]

NUM_PINS = 16


def _make_lookup(options):
    """Map a dropdown label back to its register value."""
    return {label: value for label, value in options}


def _make_reverse(options):
    """Map a register value back to its dropdown label."""
    return {value: label for label, value in options}


MODER_LOOKUP = _make_lookup(MODER_OPTIONS)
OTYPER_LOOKUP = _make_lookup(OTYPER_OPTIONS)
PUPDR_LOOKUP = _make_lookup(PUPDR_OPTIONS)

MODER_REVERSE = _make_reverse(MODER_OPTIONS)
OTYPER_REVERSE = _make_reverse(OTYPER_OPTIONS)
PUPDR_REVERSE = _make_reverse(PUPDR_OPTIONS)


class GpioRegGen(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("STM32 GPIO Register Generator")
        self.resizable(False, False)

        # Per-pin Tk string variables holding the selected dropdown label.
        self.moder_vars = []
        self.otyper_vars = []
        self.pupdr_vars = []

        self._build_pin_table()
        self._build_output()
        self._recompute()

        # Ensure the window can always be closed (window-manager X button,
        # Escape key, and a fully terminated mainloop).
        self.protocol("WM_DELETE_WINDOW", self._quit)
        self.bind("<Escape>", lambda _evt: self._quit())

        # Tk's mainloop runs in C and never yields to Python, so a Ctrl+C
        # SIGINT stays queued and the window is orphaned. Route SIGINT to a
        # clean quit and tick periodically so Python can actually deliver it.
        signal.signal(signal.SIGINT, lambda _sig, _frame: self._quit())
        self._tick()

    def _tick(self):
        self._tick_id = self.after(100, self._tick)

    def _build_pin_table(self):
        table = ttk.LabelFrame(self, text="Pins", padding=8)
        table.grid(row=0, column=0, padx=10, pady=10, sticky="nsew")

        headers = ["Pin", "MODER", "OTYPER", "PUPDR"]
        for col, text in enumerate(headers):
            ttk.Label(table, text=text, font=("TkDefaultFont", 9, "bold")).grid(
                row=0, column=col, padx=6, pady=(0, 4), sticky="w"
            )

        for pin in range(NUM_PINS):
            row = pin + 1
            ttk.Label(table, text=str(pin)).grid(
                row=row, column=0, padx=6, pady=1, sticky="w"
            )

            moder_var = tk.StringVar(value=MODER_OPTIONS[0][0])
            otyper_var = tk.StringVar(value=OTYPER_OPTIONS[0][0])
            pupdr_var = tk.StringVar(value=PUPDR_OPTIONS[0][0])

            self.moder_vars.append(moder_var)
            self.otyper_vars.append(otyper_var)
            self.pupdr_vars.append(pupdr_var)

            self._add_combo(table, row, 1, moder_var, MODER_OPTIONS, width=11)
            self._add_combo(table, row, 2, otyper_var, OTYPER_OPTIONS, width=12)
            self._add_combo(table, row, 3, pupdr_var, PUPDR_OPTIONS, width=11)

    def _add_combo(self, parent, row, col, var, options, width):
        combo = ttk.Combobox(
            parent,
            textvariable=var,
            values=[label for label, _ in options],
            state="readonly",
            width=width,
        )
        combo.grid(row=row, column=col, padx=6, pady=1, sticky="w")
        combo.bind("<<ComboboxSelected>>", lambda _evt: self._recompute())

    def _build_output(self):
        out = ttk.LabelFrame(self, text="Register values", padding=8)
        out.grid(row=1, column=0, padx=10, pady=(0, 10), sticky="nsew")

        self.result_vars = {}
        for i, name in enumerate(("MODER", "OTYPER", "PUPDR")):
            ttk.Label(out, text=name, font=("TkDefaultFont", 9, "bold")).grid(
                row=i, column=0, padx=6, pady=3, sticky="w"
            )
            var = tk.StringVar(value="0x00000000")
            entry = ttk.Entry(
                out, textvariable=var, width=14,
                font=("TkFixedFont", 10),
            )
            entry.grid(row=i, column=1, padx=6, pady=3, sticky="w")
            ttk.Button(
                out, text="Copy",
                command=lambda v=var: self._copy(v.get()),
            ).grid(row=i, column=2, padx=6, pady=3)
            ttk.Button(
                out, text="Set",
                command=lambda n=name: self._apply_value(n),
            ).grid(row=i, column=3, padx=6, pady=3)
            self.result_vars[name] = var

        ttk.Button(out, text="Reset all", command=self._reset_all).grid(
            row=3, column=0, columnspan=4, pady=(8, 0)
        )

    def _recompute(self):
        moder = otyper = pupdr = 0
        for pin in range(NUM_PINS):
            moder |= MODER_LOOKUP[self.moder_vars[pin].get()] << (pin * 2)
            otyper |= OTYPER_LOOKUP[self.otyper_vars[pin].get()] << pin
            pupdr |= PUPDR_LOOKUP[self.pupdr_vars[pin].get()] << (pin * 2)

        self.result_vars["MODER"].set(f"0x{moder:08X}")
        self.result_vars["OTYPER"].set(f"0x{otyper:08X}")
        self.result_vars["PUPDR"].set(f"0x{pupdr:08X}")

    def _apply_value(self, name):
        """Decode the hex in a register's field back into the dropdowns."""
        try:
            value = int(self.result_vars[name].get().strip(), 16)
        except ValueError:
            # Restore the field to the current valid value on bad input.
            self._recompute()
            return

        if name == "OTYPER":
            for pin in range(NUM_PINS):
                bits = (value >> pin) & 0b1
                self.otyper_vars[pin].set(OTYPER_REVERSE[bits])
        else:
            target_vars = self.moder_vars if name == "MODER" else self.pupdr_vars
            reverse = MODER_REVERSE if name == "MODER" else PUPDR_REVERSE
            for pin in range(NUM_PINS):
                bits = (value >> (pin * 2)) & 0b11
                # PUPDR has no 0b11 option; leave that pin unchanged.
                if bits in reverse:
                    target_vars[pin].set(reverse[bits])

        self._recompute()

    def _reset_all(self):
        for pin in range(NUM_PINS):
            self.moder_vars[pin].set(MODER_OPTIONS[0][0])
            self.otyper_vars[pin].set(OTYPER_OPTIONS[0][0])
            self.pupdr_vars[pin].set(PUPDR_OPTIONS[0][0])
        self._recompute()

    def _copy(self, text):
        self.clipboard_clear()
        self.clipboard_append(text)

    def _quit(self):
        if getattr(self, "_tick_id", None) is not None:
            self.after_cancel(self._tick_id)
            self._tick_id = None
        self.destroy()


if __name__ == "__main__":
    app = GpioRegGen()
    try:
        app.mainloop()
    except KeyboardInterrupt:
        app._quit()
