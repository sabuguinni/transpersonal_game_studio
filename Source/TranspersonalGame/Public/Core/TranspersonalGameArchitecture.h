// Copyright 2024 Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "TranspersonalGameArchitecture.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - CORE ARCHITECTURE
 * 
 * This file defines the foundational architecture for the entire game.
 * All systems must follow these patterns and constraints.
 * 
 * ARCHITECTURE PRINCIPLES:
 * 1. Modular Design - Each system is self-contained with clear interfaces
 * 2. Performance First - 60fps PC / 30fps Console is non-negotiable
 * 3. Scalability - Support for 50,000+ Mass AI agents
 * 4. Data-Driven - All gameplay parameters configurable via Data Assets
 * 5. Memory Efficient - Strict memory budgets for each system
 */

// Forward declarations
class UTranspersonalCoreSubsystem;
class UPhysicsSystemManager;
class UWorldGenerationManager;
class UMassSimulationManager;
class UConsciousnessSystem;

/**
 * Core architecture constants and limits
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTranspersonalArchitectureLimits
{
    GENERATED_BODY()

    // Performance Limits
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxSimultaneousActors = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxMassAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MinFrameRate = 30.0f;

    // Memory Limits (in MB)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory")
    int32 MaxTextureMemory = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory")
    int32 MaxMeshMemory = 1024;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory")
    int32 MaxAudioMemory = 512;

    // World Limits
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World")
    float MaxWorldSize = 16000.0f; // 16km x 16km

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World")
    float WorldPartitionCellSize = 1600.0f; // 1.6km cells

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World")
    int32 MaxLODLevels = 5;
};

/**
 * System priorities for initialization and tick order
 */
UENUM(BlueprintType)
enum class ETranspersonalSystemPriority : uint8
{
    Critical = 0,    // Core systems (Physics, Memory)
    High = 1,        // Gameplay systems (AI, Combat)
    Medium = 2,      // World systems (Environment, Weather)
    Low = 3,         // Cosmetic systems (VFX, Audio)
    Background = 4   // Non-essential systems (Analytics, Debug)
};

/**
 * Base interface for all Transpersonal Game systems
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UTranspersonalSystemInterface : public UInterface
{
    GENERATED_BODY()
};

class TRANSPERSONALGAME_API ITranspersonalSystemInterface
{
    GENERATED_BODY()

public:
    // System lifecycle
    virtual void InitializeSystem() = 0;
    virtual void ShutdownSystem() = 0;
    virtual void TickSystem(float DeltaTime) = 0;
    
    // System info
    virtual FString GetSystemName() const = 0;
    virtual ETranspersonalSystemPriority GetSystemPriority() const = 0;
    virtual bool IsSystemCritical() const = 0;
    
    // Performance monitoring
    virtual float GetSystemCPUUsage() const { return 0.0f; }
    virtual int32 GetSystemMemoryUsage() const { return 0; }
    virtual bool IsSystemHealthy() const { return true; }
};

/**
 * Core subsystem that manages all other systems
 * This is the central nervous system of the game
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UTranspersonalCoreSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // System management
    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Core")
    void RegisterSystem(TScriptInterface<ITranspersonalSystemInterface> System);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Core")
    void UnregisterSystem(TScriptInterface<ITranspersonalSystemInterface> System);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Core")
    TArray<TScriptInterface<ITranspersonalSystemInterface>> GetAllSystems() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Performance")
    float GetTotalCPUUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Performance")
    int32 GetTotalMemoryUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Performance")
    bool AreAllSystemsHealthy() const;

    // Architecture limits
    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Architecture")
    const FTranspersonalArchitectureLimits& GetArchitectureLimits() const { return ArchitectureLimits; }

    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Architecture")
    void SetArchitectureLimits(const FTranspersonalArchitectureLimits& NewLimits);

protected:
    // Registered systems
    UPROPERTY()
    TArray<TScriptInterface<ITranspersonalSystemInterface>> RegisteredSystems;

    // Architecture configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    FTranspersonalArchitectureLimits ArchitectureLimits;

    // Performance tracking
    UPROPERTY()
    float LastFrameTime;

    UPROPERTY()
    int32 FrameCounter;

private:
    void TickAllSystems(float DeltaTime);
    void SortSystemsByPriority();
    void ValidateSystemHealth();
};

/**
 * Architecture validation utilities
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UTranspersonalArchitectureValidator : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Validation")
    static bool ValidateWorldSize(float WorldSize);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Validation")
    static bool ValidateActorCount(int32 ActorCount);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Validation")
    static bool ValidateMemoryUsage(int32 MemoryUsageMB);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Validation")
    static bool ValidateFrameRate(float CurrentFPS);

    // System requirements check
    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Validation")
    static bool CheckSystemRequirements();

    // Performance profiling
    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Profiling")
    static void StartPerformanceProfile(const FString& ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal|Profiling")
    static void EndPerformanceProfile(const FString& ProfileName);
};

/**
 * Macro definitions for consistent logging and debugging
 */
#define TRANSPERSONAL_LOG(Verbosity, Format, ...) \
    UE_LOG(LogTemp, Verbosity, TEXT("[TranspersonalGame] " Format), ##__VA_ARGS__)

#define TRANSPERSONAL_LOG_SYSTEM(SystemName, Verbosity, Format, ...) \
    UE_LOG(LogTemp, Verbosity, TEXT("[%s] " Format), *SystemName, ##__VA_ARGS__)

#define TRANSPERSONAL_ENSURE(Condition, Format, ...) \
    if (!ensure(Condition)) { \
        TRANSPERSONAL_LOG(Error, Format, ##__VA_ARGS__); \
    }

#define TRANSPERSONAL_CHECK_PERFORMANCE(System) \
    if (System && System->GetSystemCPUUsage() > 16.67f) { \
        TRANSPERSONAL_LOG(Warning, "System %s exceeding CPU budget: %.2f ms", \
            *System->GetSystemName(), System->GetSystemCPUUsage()); \
    }