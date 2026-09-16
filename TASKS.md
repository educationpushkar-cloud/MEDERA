# MedEra — Task Plan (Phase II & III completion)

Phase I is done. This is the plan for finishing Phase II and III, with the core
algorithmic pieces (storage, indexing, distance, ranking, the emergency FSM)
already written and verified — see `src/phase2_3_demo.cpp`, which compiles and
runs correctly end to end. What's left is mostly integration, data, testing,
and polish. Tasks below are split by role, matching who owns what in the
Phase I report's Team Contribution table.

Commit discipline still applies: work in small, individually-committed steps
under your own GitHub account, spread across the weeks below — not one dump
at the end. See the commit-plan conventions already used for Phase I.

---

## Pushkar Bharat Shah — Team Lead / Data & Testing

**Owns:** the data layer — same territory as his Phase I struct/array/lookup work, now extended.

- [ ] Review `models.h`, `storage.h/cpp`, `hashindex.h/cpp` — these are done and tested, but read through them so you can explain the design (data model tables, load/save format) in front of the mentor.
- [ ] Extend `storage.cpp` if additional fields are needed once the team finalises what a hospital profile should show (e.g. department list as a new field) — the pattern to follow is already there for every entity.
- [ ] Wire real staff-login verification: load `Patient`/staff records from storage and check submitted credentials against them, replacing Phase I's "capture only, no check."
- [ ] Own testing of the data layer specifically: what happens on a missing file, a malformed line, duplicate IDs. Add a few deliberately broken test files and confirm the loaders don't crash (they're written to skip bad lines rather than throw, but verify it).

**Suggested weeks:** 6–8.

---

## Divyansh Sachan — Developer

**Owns:** integration — same territory as his Phase I main-menu/control-flow work, now extended to the web layer.

- [ ] Download `httplib.h` from https://github.com/yhirose/cpp-httplib into `include/httplib.h`, and get `server.cpp` compiling. This is the first thing to do, since nothing else on the web side can be tested until it builds.
- [ ] Once it compiles, manually test each endpoint (`/api/hospitals`, `/api/emergency/nearest`, `/api/hospital/update-beds`) with a browser or `curl` before touching the frontend.
- [ ] Add the missing endpoints the frontend will need next: create an appointment, submit a rating (post-visit only — reject if the appointment isn't `Completed`), and the emergency-request lifecycle (create request → notify → accept/reject → callback → dispatch → complete), calling into `emergency_fsm.h` for every state change rather than setting `.state` directly anywhere else in the codebase.
- [ ] Keep `main.cpp` (Phase I console) working as-is — it should stay a valid, independent way to demo the core lookup even after the web layer exists.

**Suggested weeks:** 6–11.

---

## Shourya Shrivastav — Data & Testing / PPT Design & Research

**Owns:** the dataset and end-to-end testing — same territory as Phase I dataset prep and console-flow testing.

- [ ] Extend the hospital dataset to cover all three cities (Dehradun, Delhi, Kanpur) with real-enough latitude/longitude per locality, not just the five-hospital Dehradun sample in the demo — reuse the existing Phase I city/area names so continuity with the report is obvious.
- [ ] Write out test scenarios by hand and confirm each one before it's shown to the mentor: an area with zero hospitals, every hospital full (0 beds), an emergency request that gets rejected by every candidate hospital in the ranked list (confirm `fallbackToNextHospital` correctly reports "no candidates left" rather than crashing).
- [ ] Once the web frontend is up, do a full manual pass through it the same way Phase I's console flow was tested, and capture screenshots for `docs/`.
- [ ] Keep contributing to slide/report updates as Phase II/III features land, same as Phase I.

**Suggested weeks:** 7–12.

---

## Madhav Varshney — PPT Design & Research

**Owns:** frontend polish and documentation — same territory as Phase I console formatting and docs.

- [ ] Improve `web/index.html` / `style.css` beyond the current bare-bones version: add an emergency-status display (show the current state from the FSM, not just a search result), basic responsive layout, and clearer error states (e.g. "no hospitals found").
- [ ] Keep `README.md` and `TASKS.md` current as tasks are completed — check items off, update the "Current project status" section in the README as each phase's pieces land.
- [ ] Update the synopsis report's diagrams/screenshots once the web version is testable, so the report reflects the actual running system rather than only the console prototype.
- [ ] Continue background research support — e.g. confirm the ranking weights (`RankingWeights` in `ranking.h`) are reasonable defaults, or suggest better ones based on how similar systems weight distance vs. availability.

**Suggested weeks:** 8–13.

---

## What's already done (do not re-build these — verify and use them)

- `models.h/cpp` — Hospital, Doctor, Patient, Appointment, Rating, EmergencyRequest structs and enum helpers.
- `storage.h/cpp` — file-backed load/save for every entity above.
- `hashindex.h/cpp` — `"city|area"` hash-map lookup, replacing the Phase I array.
- `haversine.h/cpp` — great-circle distance formula.
- `ranking.h/cpp` — weighted scoring + bounded min-heap top-K selection.
- `emergency_fsm.h/cpp` — the full state machine, including the reject → fallback-to-next-hospital path and the callback-validation step before ambulance assignment.
- `src/phase2_3_demo.cpp` — proof that all of the above work together correctly; run this before writing new code against these modules, so you can see the expected behaviour first.
