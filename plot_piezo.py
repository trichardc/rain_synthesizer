import argparse
import re
import time
from collections import deque

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import serial


READING_PATTERN = re.compile(r"^Piezo:\s*(\d+)$")
ADC_MAX = 4095
WINDOW_SECONDS = 10
MAX_SAMPLES = 400


def parse_reading(line):
    match = READING_PATTERN.fullmatch(line.strip())
    return int(match.group(1)) if match else None


def main():
    parser = argparse.ArgumentParser(description="Plot live Teensy piezo readings.")
    parser.add_argument("--port", default="COM13", help="Serial port (default: COM13)")
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()

    try:
        connection = serial.Serial(args.port, args.baud, timeout=0)
    except serial.SerialException as error:
        raise SystemExit(
            f"Could not open {args.port}. Close Serial Monitor and check the port.\n{error}"
        ) from error

    timestamps = deque(maxlen=MAX_SAMPLES)
    readings = deque(maxlen=MAX_SAMPLES)
    started_at = time.monotonic()
    pending = ""

    figure, axes = plt.subplots(figsize=(10, 5), layout="constrained")
    axes.set_title("Teensy 4.1 Piezo Sensor")
    axes.set_xlabel("Time (seconds)")
    axes.set_ylabel("ADC reading (0-4095)")
    axes.set_ylim(0, ADC_MAX)
    axes.set_xlim(0, WINDOW_SECONDS)
    axes.grid(True, alpha=0.25)

    (trace,) = axes.plot([], [], color="#16856b", linewidth=1.8)
    current_value = axes.text(
        0.99,
        0.96,
        "Waiting for readings...",
        horizontalalignment="right",
        verticalalignment="top",
        transform=axes.transAxes,
    )

    def update(_frame):
        nonlocal pending

        pending += connection.read(max(connection.in_waiting, 1)).decode(
            "utf-8", errors="ignore"
        )
        while "\n" in pending:
            line, pending = pending.split("\n", 1)
            reading = parse_reading(line)
            if reading is not None:
                timestamps.append(time.monotonic() - started_at)
                readings.append(reading)

        if timestamps:
            latest = timestamps[-1]
            trace.set_data(timestamps, readings)
            axes.set_xlim(max(0, latest - WINDOW_SECONDS), max(WINDOW_SECONDS, latest))
            current_value.set_text(f"Current: {readings[-1]}")

        return trace, current_value

    animation = FuncAnimation(
        figure,
        update,
        interval=50,
        blit=False,
        cache_frame_data=False,
    )

    try:
        plt.show()
    finally:
        connection.close()


if __name__ == "__main__":
    main()