#pragma once

// Completed units within a named phase; zero total means the amount of work is unknown.
void loadingWork(const char* phase, unsigned done, unsigned total);
// For synchronous work between render frames, before its first expensive read.
void loadingBlock(const char* phase);
