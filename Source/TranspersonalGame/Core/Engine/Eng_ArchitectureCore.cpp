#include "Eng_ArchitectureCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

UEng_ArchitectureCore::UEng_ArchitectureCore()
{
    bIsInitialized = false;
    LastFrameTime = 0.0f;
    LastActorCount = 0;
    LastValidationTime = FDateTime::Now();
}

void UEng_ArchitectureCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Core - Initializing..."));
    
    // Initialize core systems
    InitializeCoreSystems();
    
    // Setup performance monitoring
    SetupPerformanceMonitoring();
    
    // Validate module dependencies
    ValidateModuleDependencies();
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Core - Initialization Complete"));
}

void UEng_ArchitectureCore::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Core - Shutting down..."));
    
    RegisteredModules.Empty();
    SystemStates.Empty();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

bool UEng_ArchitectureCore::ValidateSystemDependencies()
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("Architecture Core not initialized"));
        return false;
    }
    
    // Check critical systems
    bool bAllSystemsValid = true;
    
    // Validate World Generation system
    if (!IsSystemModuleActive("WorldGeneration"))
    {
        UE_LOG(LogTemp, Warning, TEXT("WorldGeneration module not active"));
        bAllSystemsValid = false;
    }
    
    // Validate Character system
    if (!IsSystemModuleActive("Characters"))
    {
        UE_LOG(LogTemp, Warning, TEXT("Characters module not active"));
        bAllSystemsValid = false;
    }
    
    // Validate Physics system
    if (!IsSystemModuleActive("Physics"))
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics module not active"));
        bAllSystemsValid = false;
    }
    
    LastValidationTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Log, TEXT("System Dependencies Validation: %s"), 
           bAllSystemsValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bAllSystemsValid;
}

bool UEng_ArchitectureCore::RegisterSystemModule(const FString& ModuleName, int32 Priority)
{
    if (ModuleName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register module with empty name"));
        return false;
    }
    
    RegisteredModules.Add(ModuleName, Priority);
    SystemStates.Add(ModuleName, true); // Default to enabled
    
    UE_LOG(LogTemp, Log, TEXT("Registered module: %s (Priority: %d)"), *ModuleName, Priority);
    
    return true;
}

bool UEng_ArchitectureCore::IsSystemModuleActive(const FString& ModuleName)
{
    return RegisteredModules.Contains(ModuleName) && 
           SystemStates.Contains(ModuleName) && 
           SystemStates[ModuleName];
}

float UEng_ArchitectureCore::GetCurrentFrameTime() const
{
    if (GEngine && GEngine->GetWorld())
    {
        return GEngine->GetWorld()->GetDeltaSeconds();
    }
    return LastFrameTime;
}

int32 UEng_ArchitectureCore::GetActiveActorCount() const
{
    if (GEngine && GEngine->GetWorld())
    {
        UWorld* World = GEngine->GetWorld();
        if (World)
        {
            return World->GetActorCount();
        }
    }
    return LastActorCount;
}

FString UEng_ArchitectureCore::GetPerformanceReport() const
{
    FString Report = TEXT("=== ARCHITECTURE PERFORMANCE REPORT ===\n");
    
    Report += FString::Printf(TEXT("Frame Time: %.3f ms\n"), GetCurrentFrameTime() * 1000.0f);
    Report += FString::Printf(TEXT("Active Actors: %d\n"), GetActiveActorCount());
    Report += FString::Printf(TEXT("Registered Modules: %d\n"), RegisteredModules.Num());
    Report += FString::Printf(TEXT("Active Systems: %d\n"), SystemStates.Num());
    
    Report += TEXT("\nREGISTERED MODULES:\n");
    for (const auto& Module : RegisteredModules)
    {
        bool bEnabled = SystemStates.Contains(Module.Key) ? SystemStates[Module.Key] : false;
        Report += FString::Printf(TEXT("- %s (Priority: %d, Status: %s)\n"), 
                                  *Module.Key, Module.Value, 
                                  bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
    }
    
    return Report;
}

void UEng_ArchitectureCore::SetSystemEnabled(const FString& SystemName, bool bEnabled)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot set state for empty system name"));
        return;
    }
    
    SystemStates.Add(SystemName, bEnabled);
    
    UE_LOG(LogTemp, Log, TEXT("System %s set to %s"), 
           *SystemName, bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

bool UEng_ArchitectureCore::IsSystemEnabled(const FString& SystemName) const
{
    if (SystemStates.Contains(SystemName))
    {
        return SystemStates[SystemName];
    }
    return false;
}

void UEng_ArchitectureCore::ValidateModuleDependencies()
{
    UE_LOG(LogTemp, Log, TEXT("Validating module dependencies..."));
    
    // Register core modules with priorities
    RegisterSystemModule("Core", 100);
    RegisterSystemModule("Physics", 90);
    RegisterSystemModule("WorldGeneration", 80);
    RegisterSystemModule("Characters", 70);
    RegisterSystemModule("AI", 60);
    RegisterSystemModule("Combat", 50);
    RegisterSystemModule("Audio", 40);
    RegisterSystemModule("VFX", 30);
    RegisterSystemModule("UI", 20);
    RegisterSystemModule("QA", 10);
    
    UE_LOG(LogTemp, Log, TEXT("Module dependencies validation complete"));
}

void UEng_ArchitectureCore::InitializeCoreSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing core systems..."));
    
    // Set up default system states
    SystemStates.Empty();
    
    // Core systems always enabled
    SystemStates.Add("Core", true);
    SystemStates.Add("Physics", true);
    SystemStates.Add("Characters", true);
    
    // Optional systems - can be disabled for performance
    SystemStates.Add("WorldGeneration", true);
    SystemStates.Add("AI", true);
    SystemStates.Add("Combat", true);
    SystemStates.Add("Audio", true);
    SystemStates.Add("VFX", true);
    SystemStates.Add("UI", true);
    SystemStates.Add("QA", true);
    
    UE_LOG(LogTemp, Log, TEXT("Core systems initialization complete"));
}

void UEng_ArchitectureCore::SetupPerformanceMonitoring()
{
    UE_LOG(LogTemp, Log, TEXT("Setting up performance monitoring..."));
    
    // Initialize performance tracking variables
    LastFrameTime = 0.016f; // Target 60 FPS
    LastActorCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring setup complete"));
}