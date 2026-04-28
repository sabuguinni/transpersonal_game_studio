// Copyright Transpersonal Game Studio 2026
// Primary game header implementation — log category and constants
#include "TranspersonalGame.h"

// Log category definition (declared extern in TranspersonalGame.h)
DEFINE_LOG_CATEGORY(LogTranspersonalGame);

// Note: IMPLEMENT_PRIMARY_GAME_MODULE is in TranspersonalGameModule.cpp

// Minimal module implementation for Editor loading
// The full FTranspersonalGameModule is disabled pending bug fixes
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, TranspersonalGame, TranspersonalGame);
