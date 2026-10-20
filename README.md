# Unchair

**A terminal break reminder for developers.** Unchair tells you to stand up and drink water, adapts your work time to how you actually feel, and plays your own music with a live FFT visualizer when a break starts.

Written in **C. Linux first.**

> **Status:** Early development. v0.1 is being built. Feedback and ideas are very welcome. See [Feedback](#feedback).

<!-- Add a GIF of the break screen with the visualizer here once v0.1 runs -->

---

## Why

Developers sit for hours without noticing. Most break apps use a fixed timer that you set once and then ignore or turn off. Unchair lives in your terminal, gets your attention with sound and a desktop notification, and changes your work time based on how you feel.

## Features (planned for v0.1)

- **Stand reminders:** A work/break cycle that starts at 35 minutes of work and 5 minutes of break.
- **Water reminders:** On their own timer, every 60 minutes by default.
- **Adapts to you:** After each break, you rate how the last work block felt from 1 to 5, and Unchair lengthens or shortens the next block. It always shows *why* it changed.
- **Your own music:** Drop WAV files into a folder, and a different song plays at the start of each break.
- **Live visualizer:** Frequency and volume bars that move with the music, powered by a hand-written FFT.
- **Desktop notifications:** So you see the break even when the terminal is hidden.
- **Snooze and skip:** With limits, so breaks can't be pushed away forever.
- **Plain-text log:** A record of your day, with a quick daily summary.

## How the adjustment works

| What happened | Next work block |
|---|---|
| Skipped the break | 5 min shorter |
| Took the break, felt 1 or 2 | 5 min shorter |
| Took the break, felt 3 or no answer | Same |
| Took the break, felt 4 or 5 | 5 min longer |

The work block always stays between 25 and 50 minutes. All of these numbers can be changed in the config file.

## Requirements

- Linux
- A C compiler, such as GCC or Clang
- PulseAudio or PipeWire for audio playback
- `notify-send` for desktop notifications (optional)
- Songs in **16-bit WAV** format (other formats are planned)

## Installation

Coming with v0.1: A single downloadable Linux binary on the Releases page, plus build-from-source instructions.

## Usage

| Command | What it does |
|---|---|
| `unchair` | Start the reminder screen |
| `unchair today` | Show a summary of today |

**Keys while running:** `b` break now, `w` drank water, `p` pause, `q` quit. During a break: `d` done early, `s` snooze, `x` skip, `m` stop music.

## Your music

Put `.wav` files in:

```text
~/.config/unchair/sounds/
```

Unchair rotates through them so you don't hear the same song twice in a row. A short `water.wav` in `~/.config/unchair/` is used for water reminders.

## Configuration

Settings live in `~/.config/unchair/config` and are created with defaults on first run:

```ini
work_minutes      = 35
break_minutes     = 5
min_work_minutes  = 25
max_work_minutes  = 50
adjust_step       = 5
snooze_minutes    = 5
max_snoozes       = 2
water_minutes     = 60
song_seconds      = 30
song_order        = rotate     # or: random
```

If a setting has a mistake, Unchair tells you the line number and uses the default for that setting, so it always starts.

## Roadmap

- MP3 and OGG support
- tmux status bar countdown
- Pause automatically when you're away from the keyboard
- Weekly reports
- macOS and Windows support
- A GUI version

## Feedback

This is an early project, and your opinion shapes what comes next. Please open an issue and tell me:

- Did the timer adjustment feel helpful or annoying?
- Is WAV-only a deal-breaker for you?
- What would make you keep using it every day?

## License

MIT, see [LICENSE](LICENSE).

