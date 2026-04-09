// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "TranspersonalGameCoreModule.h"
#include "Engine/Engine.h"
#include "Modules/ModuleManager.h"
#include "StudioDirectorSubsystem.h"
#include "PhysicsCore/PhysicsSystemManager.h"
#include "Performance/PerformanceManager.h"

DEFINE_LOG_CATEGORY(LogTranspersonalGameCore);

void FTranspersonalGameCoreModule::StartupModule()
{
    UE_LOG(LogTranspersonalGameCore, Warning, TEXT("TranspersonalGameCore module starting up"));
    
    // Initialize core systems
    InitializeCoreSystems();
    
    // Register with Studio Director
    RegisterWithStudioDirector();
    
    UE_LOG(LogTranspersonalGameCore, Warning, TEXT("TranspersonalGameCore module startup complete"));
}

void FTranspersonalGameCoreModule::ShutdownModule()
{
    UE_LOG(LogTranspersonalGameCore, Warning, TEXT("TranspersonalGameCore module shutting down"));
    
    // Cleanup core systems
    ShutdownCoreSystems();
    
    UE_LOG(LogTranspersonalGameCore, Warning, TEXT("TranspersonalGameCore module shutdown complete"));
}

void FTranspersonalGameCoreModule::InitializeCoreSystems()
{
    // Core systems initialization will be handled by subsystems
    UE_LOG(LogTranspersonalGameCore, Log, TEXT("Core systems initialization delegated to subsystems"));
}

void FTranspersonalGameCoreModule::ShutdownCoreSystems()
{
    // Core systems shutdown will be handled by subsystems
    UE_LOG(LogTranspersonalGameCore, Log, TEXT("Core systems shutdown delegated to subsystems"));
}

void FTranspersonalGameCoreModule::RegisterWithStudioDirector()
{
    UE_LOG(LogTranspersonalGameCore, Log, TEXT("Registering Core module with Studio Director"));
    
    // Registration will happen when subsystems are created
    // This ensures proper initialization order
}

IMPLEMENT_MODULE(FTranspersonalGameCoreModule, TranspersonalGameCore)