# Working on this personal fork

Read `HANDOFF.md`, then `HANDOFF.local.md` if present, before starting work. The local file contains this PC's working
toolchain, converted-data locations, emulator setup, and device transfer details. It is intentionally ignored by Git.
Read `DEVELOPMENT.md` for commands and `FORK.md` for repository identity.

- This checkout is the personal fork. Keep work on its `main` branch unless asked otherwise. Preserve upstream history
  and licenses. The separate `ArmstrongThomas/mw3ds` stylus repository/branch must remain unchanged.
- Review upstream changes before merging. `upstream` is the original project; no personal publishing remote is
  currently configured. Do not publish or push to the stylus repository as a substitute.
- Never commit converted game assets, saves, DSP firmware, build products, `.env`, or `HANDOFF.local.md`.
- Preserve the existing Morrowind typography, palette, frames, and splash art. Full-screen loading status/progress
  belongs on the bottom screen; in-game loading feedback remains on top. Support touch and physical controls.
- Keep camera, map, and button gesture ownership separate. Preserve left-handed mirroring and live gameplay during
  minimap interaction. Do not remove audio thread checks or loading/music reentrancy guards.
- Use native tests for fast correctness checks, the 3DS executable in an isolated emulator for platform checks, and
  real hardware for timing/audio claims. Distinguish Tribunal and vanilla test data. Do not claim emulator FPS as
  hardware evidence or missing-DSP runs as audible music validation.
- Do not control the user's desktop. Prefer CLI tests; use a separate emulator copy without stopping an instance
  the user is testing. Launch a visible emulator only when requested. Send builds to the console when the user says
  netloader is ready or explicitly requests the transfer.
- After meaningful changes, update `HANDOFF.md` and relevant specs with current work, evidence, limitations, and next
  steps. Update `HANDOFF.local.md` when local setup changes. Leave a local commit/checkpoint before a handoff.
