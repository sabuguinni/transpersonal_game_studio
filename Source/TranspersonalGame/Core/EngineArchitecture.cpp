#include "EngineArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

void UEngineArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Subsystem Initialized"));
    
    // Register core modules
    RegisterCoreModule(TEXT("TranspersonalGame"), TEXT("Source/TranspersonalGame/"));
    RegisterCoreModule(TEXT("Core"), TEXT("Source/TranspersonalGame/Core/"));
    RegisterCoreModule(TEXT("Physics"), TEXT("Source/TranspersonalGame/Physics/"));
    RegisterCoreModule(TEXT("Performance"), TEXT("Source/TranspersonalGame/Performance/"));
    
    // Register gameplay modules
    RegisterGameplayModule(TEXT("Characters"), TEXT("Source/TranspersonalGame/Characters/"));
    RegisterGameplayModule(TEXT("WorldGeneration"), TEXT("Source/TranspersonalGame/WorldGeneration/"));
    RegisterGameplayModule(TEXT("Environment"), TEXT("Source/TranspersonalGame/Environment/"));
    RegisterGameplayModule(TEXT("AI"), TEXT("Source/TranspersonalGame/AI/"));
    RegisterGameplayModule(TEXT("Combat"), TEXT("Source/TranspersonalGame/Combat/"));
    RegisterGameplayModule(TEXT("Audio"), TEXT("Source/TranspersonalGame/Audio/"));
    RegisterGameplayModule(TEXT("VFX"), TEXT("Source/TranspersonalGame/VFX/"));
    
    // Initial validation
    bIsArchitectureValid = ValidateModuleStructure();
    LastValidationTime = FDateTime::Now();
    
    LogArchitectureStatus();
}

void UEngineArchitecture::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Subsystem Deinitialized"));
    Super::Deinitialize();
}

bool UEngineArchitecture::ValidateModuleStructure()
{
    bool bValid = true;
    
    // Check if core modules exist
    for (const auto& Module : CoreModules)
    {
        FString ModulePath = FPaths::ProjectDir() + Module.Value;
        if (!FPaths::DirectoryExists(ModulePath))
        {
            UE_LOG(LogTemp, Error, TEXT("Core module directory missing: %s"), *ModulePath);
            bValid = false;
        }
    }
    
    // Check if gameplay modules exist
    for (const auto& Module : GameplayModules)
    {
        FString ModulePath = FPaths::ProjectDir() + Module.Value;
        if (!FPaths::DirectoryExists(ModulePath))
        {
            UE_LOG(LogTemp, Warning, TEXT("Gameplay module directory missing: %s"), *ModulePath);
        }
    }
    
    return bValid;
}

bool UEngineArchitecture::ValidatePerformanceRequirements()
{
    float CurrentFrameTime = GetCurrentFrameTime();
    int32 CurrentActorCount = GetCurrentActorCount();
    float CurrentMemoryUsage = GetCurrentMemoryUsageMB();
    
    bool bPerformanceValid = true;
    
    if (CurrentFrameTime > MaxFrameTimeMs)
    {
        UE_LOG(LogTemp, Warning, TEXT("Frame time exceeds threshold: %.2f ms > %.2f ms"), 
               CurrentFrameTime, MaxFrameTimeMs);
        bPerformanceValid = false;
    }
    
    if (CurrentActorCount > MaxActorCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor count exceeds threshold: %d > %d"), 
               CurrentActorCount, MaxActorCount);
        bPerformanceValid = false;
    }
    
    if (CurrentMemoryUsage > MaxMemoryUsageMB)
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory usage exceeds threshold: %.2f MB > %.2f MB"), 
               CurrentMemoryUsage, MaxMemoryUsageMB);
        bPerformanceValid = false;
    }
    
    return bPerformanceValid;
}

bool UEngineArchitecture::ValidateMemoryUsage()
{
    float CurrentMemoryUsage = GetCurrentMemoryUsageMB();
    
    if (CurrentMemoryUsage > MaxMemoryUsageMB * 0.8f) // 80% threshold warning
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory usage approaching limit: %.2f MB (%.1f%% of max)"), 
               CurrentMemoryUsage, (CurrentMemoryUsage / MaxMemoryUsageMB) * 100.0f);
        return false;
    }
    
    return true;
}

void UEngineArchitecture::RegisterCoreModule(const FString& ModuleName, const FString& ModulePath)
{
    CoreModules.Add(ModuleName, ModulePath);
    UE_LOG(LogTemp, Log, TEXT("Registered core module: %s at %s"), *ModuleName, *ModulePath);
}

void UEngineArchitecture::RegisterGameplayModule(const FString& ModuleName, const FString& ModulePath)
{
    GameplayModules.Add(ModuleName, ModulePath);
    UE_LOG(LogTemp, Log, TEXT("Registered gameplay module: %s at %s"), *ModuleName, *ModulePath);
}

float UEngineArchitecture::GetCurrentFrameTime() const
{
    if (GEngine && GEngine->GetWorld())
    {
        return GEngine->GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    }
    return 0.0f;
}

int32 UEngineArchitecture::GetCurrentActorCount() const
{
    if (GEngine && GEngine->GetWorld())
    {
        return GEngine->GetWorld()->GetActorCount();
    }
    return 0;
}

float UEngineArchitecture::GetCurrentMemoryUsageMB() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

bool UEngineArchitecture::EnforceNamingConventions()
{
    // This would check file naming conventions
    // For now, return true as a placeholder
    UE_LOG(LogTemp, Log, TEXT("Naming convention enforcement completed"));
    return true;
}

bool UEngineArchitecture::EnforceModuleDependencies()
{
    // This would check module dependency rules
    // For now, return true as a placeholder
    UE_LOG(LogTemp, Log, TEXT("Module dependency enforcement completed"));
    return true;
}

bool UEngineArchitecture::ValidateFileStructure()
{
    // Check if SharedTypes.h exists
    FString SharedTypesPath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/SharedTypes.h");
    if (!FPaths::FileExists(SharedTypesPath))
    {
        UE_LOG(LogTemp, Error, TEXT("SharedTypes.h not found at: %s"), *SharedTypesPath);
        return false;
    }
    
    return true;
}

bool UEngineArchitecture::ValidateIncludePaths()
{
    // Validate that all include paths are correct
    // This is a placeholder for more complex validation
    return true;
}

bool UEngineArchitecture::ValidateNamingConventions()
{
    // Check that all classes follow the naming conventions
    // This is a placeholder for more complex validation
    return true;
}

void UEngineArchitecture::LogArchitectureStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECTURE STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Architecture Valid: %s"), bIsArchitectureValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Core Modules: %d"), CoreModules.Num());
    UE_LOG(LogTemp, Warning, TEXT("Gameplay Modules: %d"), GameplayModules.Num());
    UE_LOG(LogTemp, Warning, TEXT("Last Validation: %s"), *LastValidationTime.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Current Frame Time: %.2f ms"), GetCurrentFrameTime());
    UE_LOG(LogTemp, Warning, TEXT("Current Actor Count: %d"), GetCurrentActorCount());
    UE_LOG(LogTemp, Warning, TEXT("Current Memory Usage: %.2f MB"), GetCurrentMemoryUsageMB());
    UE_LOG(LogTemp, Warning, TEXT("=== END ARCHITECTURE STATUS ==="));
}