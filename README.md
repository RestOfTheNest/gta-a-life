
GTA San Andreas: A-Life & Municipal Simulation Engine (Archived)      

A native C++ reverse-engineering experiment built on top of Plugin-SDK for Grand Theft Auto: San Andreas (1.0 US HOODLUM).

The project aimed to integrate an autonomous A-Life ecosystem coupled with an evolving municipal macroeconomic state machine (treasury, localized crime index, dynamic social unrest, civil protests, and autonomous gang wars).

    PROJECT STATUS: ARCHIVED / UNMAINTAINED (AS-IS) > Active development has been terminated. The repository is preserved for research, code archaeology, and reference for GTA SA reverse engineers.

The Reality: AI-Assisted Architecture & Why It Stalled

This codebase was developed as an experimental hybrid workflow: human high-level architectural direction, in-game debugging, and reverse-engineering combined with LLM / AI agents writing substantial portions of the C++ boilerplate, state machines, math formulas, and MoonLoader/Lua glue code.

While the AI was competent at cranking out mathematical models and basic SDK wrappers, it hit a concrete wall when dealing with GTA SA's brittle, 20-year-old single-threaded architecture (RenderWare). The project turned into an unsustainable game of whack-a-mole:
1. The "Whack-a-Mole" AI Regression Loop

    AI agents frequently lacked internal memory of the engine's subtle constraints. Fixing one bug often silently broke two others.

    Typical examples:

        Swapping internal radar sprite IDs (e.g., binding RADAR_SPRITE_RACE [a racing trophy] instead of RADAR_SPRITE_FLAG / riot markers).

        Arbitrarily wiping critical asynchronous model streaming checks during refactoring, causing blank blips where zero ped entities spawned.

        Attempting to invoke high-level script commands or task allocators in code branches where model assets were not synchronously locked in memory.

2. Low-Level Memory & Engine Lifecycle Violations (0xC0000005)

    Task Manager Collisions: GTA SA's native CTaskManager and ped intelligence pipelines expect strictly orchestrated task destruction. Forcing task switches or clearing weapons mid-combat frequently triggered Access Violations (e.g., at 0x004D464E).

    Entity Pool Reallocation: The engine’s entity pools (CPedPool, CVehiclePool) are fundamentally fragile. Despawning or repooling combatants while distant native AI routines were querying them led to random dangling pointer dereferences.

3. Asynchronous Model Streaming Starvation

    GTA SA handles asset loading via CStreaming. Spawning complex dynamic incidents (multiple ped variations + specific weapon models like AK-47s) on high player movement speeds frequently led to streaming queue starvation.

    When streaming couldn't keep up, the logic either blocked the main thread causing micro-stutters or silently failed to materialize entities, leaving "ghost" radar icons across the map.

4. Toolchain & MSBuild PDB Locking Headaches

    Building native .asi binaries against modern MSVC toolsets (v143 / v145) alongside Plugin-SDK repeatedly deadlocked the symbol server (mspdbsrv.exe), throwing fatal compiler error C1090 on vc145.pdb.

    Resolving this required strictly serializing compilation (/m:1) and forcing /Z7 (/p:DebugInformationFormat=OldStyle), which slowed down the rapid-iteration test loop significantly.

Implemented Systems

Despite the architectural hurdles, several core subsystems were successfully implemented:

    Municipal Telemetry Engine (municipal.cpp): Real-time monitoring and HUD rendering of City Treasury balance, localized District Crime Indices, and city-wide Public Unrest.

    Incident Lifecycle Dispatcher (municipal_incidents.cpp): Spatial-anchor-based incident manager spawning civil protests and gang turf conflicts within Los Santos.

    Civil Riots: Scripted crowd behavior utilizing animation queues (RIOT_ANGRY, RIOT_CHANT) and custom banner/weapon assignment at city landmarks (e.g., City Hall).

    Faction Clashes: Autonomous firefights between rival gang factions (Ballas vs. Grove Street Families) dynamically responding to regional crime spikes.

    Crash Diagnostics (crash_handler.cpp): Structured Exception Handling (SEH) harness designed to log native register states and fault offsets upon unhandled memory access violations.
    
To access real-time municipal telemetry, economic triggers, and crisis dispatch controls, run the companion backend service and open http://localhost:8080 (or your configured Cloudflare tunnel) in any web browser.

Building from Source
Requirements

    Visual Studio 2022 (Desktop development with C++, MSVC v143/v145).

    CMake 3.20+.

    Plugin-SDK configured for GTA San Andreas.

    Target binary: GTA San Andreas 1.0 US.

Compile Command

To avoid MSBuild symbol server deadlocks during compilation, always pass the OldStyle debug flag:
PowerShell

cmake -B build -A Win32
cmake --build build --config Release -- /p:DebugInformationFormat=OldStyle /m:1

License

This project is open-sourced under the MIT License. Use the coordinates, logic, or engine hooks at your own risk.
