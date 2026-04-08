// Copyright Transpersonal Game Studio 2026

#include "TranspersonalGame.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogTranspersonalGame);

class FTranspersonalGameModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

void FTranspersonalGameModule::StartupModule()
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Transpersonal Game Studio - Jurassic Survival Game Module Started"));
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Target Performance: %d FPS PC, %d FPS Console"), 
           TranspersonalGameConstants::TARGET_FPS_PC, 
           TranspersonalGameConstants::TARGET_FPS_CONSOLE);
    UE_LOG(LogTranspersonalGame, Warning, TEXT("World Scale: %.1f km (%.0f UU)"), 
           TranspersonalGameConstants::WORLD_SIZE_KM, 
           TranspersonalGameConstants::WORLD_SIZE_UU);
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Max Mass Entities: %d"), 
           TranspersonalGameConstants::MAX_MASS_ENTITIES);
}

void FTranspersonalGameModule::ShutdownModule()
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Transpersonal Game Module Shutdown"));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FTranspersonalGameModule, TranspersonalGame, "TranspersonalGame");