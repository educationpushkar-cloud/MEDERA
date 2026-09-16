# MedEra

**A service-based platform for patient–hospital connectivity, appointment management and emergency coordination.**

MedEra connects patients and hospitals through one common interface. Patients can search hospitals, book appointments, and rate their visit; hospital administrators keep their own departments, doctors, bed availability, and ambulance status current. An emergency module tracks a request from the moment it's raised through hospital validation to ambulance dispatch.

Team ID: **DSCPP-III-2026-T223** · Department of CSE, Graphic Era (Deemed to be University), Dehradun

---

## Problem statement / objective

During a medical emergency, the time spent finding a hospital with an actual open bed is often as costly as the treatment delay itself. This same friction shows up in routine care too — patients have no shared way to check which hospital has the right department, doctor, or available bed without calling directly.

MedEra's objective is to build a single platform where patients can search, book, and rate hospitals, hospitals can maintain their own live information, and emergency requests are tracked through a defined workflow — starting from a console-based Phase I prototype and extending to a full web platform with location-based emergency coordination by Phase III.

## Team members

| Member | Roll No. | Role |
|---|---|---|
| Pushkar Bharat Shah | 2028313 | Team Lead / Data & Testing |
| Divyansh Sachan | 2027764 | Developer |
| Shourya Shrivastav | 2029085 | Data & Testing / PPT Design & Research |
| Madhav Varshney | 2029065 | PPT Design & Research |

**Mentor:** Ms. Nishtha Bhatt, Dept. of CSE, Graphic Era (Deemed to be University)

## Technologies / tools used

- **Phase I (current):** C++ (STL), GCC / g++, VS Code, Git & GitHub
- **Phase II (planned):** File-backed / lightweight SQL storage, hash-map indexing, HTML/CSS
- **Phase III (planned):** Haversine-based geolocation, weighted best-match ranking, finite-state emergency workflow

## Project setup / installation instructions

**Requirements:** a C++ compiler (GCC/g++) supporting C++11 or later.

```bash
# Clone the repository
git clone https://github.com/<org-or-username>/MedEra.git
cd MedEra

# Compile
g++ -Iinclude src/main.cpp src/hospital.cpp -o medera

# Run
./medera        # on Linux/macOS
medera.exe       # on Windows
```

> Note: `clearscreen()` calls `system("cls")`, which is Windows-specific. On Linux/macOS the program still runs correctly; it just won't clear the terminal between screens.

## Major features / modules

- **Appointment lookup** — City → Area → Hospital drill-down with direct-index O(1) lookup over a fixed dataset of 3 cities, 4 localities each, 3 hospitals per locality (36 records).
- **Hospital staff login** — captures staff name, ID, phone, and password (credential verification is a Phase II item).
- **Emergency fast-path** — captures a contact number and acknowledges the request; the full location-aware, hospital-validated workflow is a Phase II/III feature.

## Current project status / progress

**Phase I — complete.** Console-based C++ prototype implementing the three modules above over a fixed in-memory dataset, compiled and tested end to end.

**Phase II — core logic complete, integration in progress.** File-backed storage (`storage.h/cpp`), hash-indexed lookup (`hashindex.h/cpp`), and the entity data model (`models.h/cpp`) are written and tested via `phase2_3_demo.cpp`. The web API (`server.cpp`) and frontend (`web/`) are written and need `httplib.h` added locally, then testing.

**Phase III — core logic complete, integration in progress.** Haversine distance (`haversine.h/cpp`), weighted best-match ranking with bounded-heap top-K (`ranking.h/cpp`), and the emergency finite-state machine including hospital reject/fallback and callback-validation (`emergency_fsm.h/cpp`) are written and verified end to end in `phase2_3_demo.cpp`. Wiring these into the live web server/frontend is the remaining integration work.

See `docs/` for the full Phase I synopsis report and project proposal, and see `TASKS.md` for the current task breakdown by teammate.

## Repository structure

```
MedEra/
├── include/
│   ├── function.h        # Phase I prototypes
│   ├── models.h           # Phase II/III entity structs (Hospital, Doctor, Patient, ...)
│   ├── storage.h          # Phase II file-backed load/save layer
│   ├── hashindex.h        # Phase II hash-indexed lookup ("city|area")
│   ├── haversine.h        # Phase III distance formula
│   ├── ranking.h          # Phase III weighted scoring + top-K
│   └── emergency_fsm.h    # Phase III emergency state machine
├── src/
│   ├── main.cpp            # Phase I entry point
│   ├── hospital.cpp        # Phase I dataset and console logic
│   ├── models.cpp          # Phase II/III enum<->string helpers
│   ├── storage.cpp
│   ├── hashindex.cpp
│   ├── haversine.cpp
│   ├── ranking.cpp
│   ├── emergency_fsm.cpp
│   ├── phase2_3_demo.cpp  # Runnable proof that the above modules work together
│   └── server.cpp          # Phase II web API (needs include/httplib.h -- see below)
├── web/
│   ├── index.html          # Phase II frontend
│   ├── style.css
│   └── app.js
├── data/                    # Generated at runtime (hospitals.txt etc.) -- not committed
├── docs/                    # Synopsis report, proposal, screenshots
└── README.md
```

## Building each part

**Phase I (console prototype):**
```bash
g++ -Iinclude src/main.cpp src/hospital.cpp -o medera
./medera
```

**Phase II/III core logic (storage, indexing, distance, ranking, FSM) — proof-of-concept demo:**
```bash
g++ -std=c++17 -Iinclude src/models.cpp src/storage.cpp src/hashindex.cpp \
    src/haversine.cpp src/ranking.cpp src/emergency_fsm.cpp \
    src/phase2_3_demo.cpp -o phase2_3_demo
./phase2_3_demo
```

**Phase II web server (one-time setup required):**
1. Download `httplib.h` from https://github.com/yhirose/cpp-httplib (single header) into `include/httplib.h`.
2. Build:
   ```bash
   g++ -std=c++17 -Iinclude src/models.cpp src/storage.cpp src/hashindex.cpp \
       src/haversine.cpp src/ranking.cpp src/emergency_fsm.cpp \
       src/server.cpp -lpthread -o medera_server
   ./medera_server
   ```
3. Open `http://localhost:8080` in a browser.

> `server.cpp` was written against the documented cpp-httplib API but has not been compiled in the environment this project was drafted in (no network access to fetch the header). Test it as the first task once it's in the repo — see the task plan below.

