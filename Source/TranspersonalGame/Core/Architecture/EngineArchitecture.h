#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EngineArchitecture.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - ENGINE ARCHITECTURE
 * Agent #02 - Engine Architect
 * 
 * This is the foundational architecture definition for the entire game.
 * All other systems must conform to these architectural principles.
 * 
 * CORE ARCHITECTURAL PRINCIPLES:
 * 1. Modular Design - Each system is self-contained with clear interfaces
 * 2. Performance First - 60fps PC / 30fps console mandate
 * 3. Scalability - Support for 50,000+ Mass AI agents
 * 4. Data-Driven - All gameplay parameters configurable via data assets
 * 5. Memory Efficiency - Strict memory budgets per system
 */

UENUM(BlueprintType)
enum class ETranspersonalSystemPriority : uint8
{
    Critical = 0,    // Core systems that cannot fail (Physics, Rendering)
    High = 1,        // Gameplay systems (AI, Combat, World)
    Medium = 2,      // Quality of life (Audio, VFX)
    Low = 3          // Optional systems (Analytics, Debug)
};

UENUM(BlueprintType)
enum class ETranspersonalPerformanceTier : uint8
{
    Ultra = 0,       // High-end PC (RTX 4080+, 32GB RAM)
    High = 1,        // Mid-range PC (RTX 3070, 16GB RAM)
    Medium = 2,      // Console equivalent (PS5/Xbox Series X)
    Low = 3          // Minimum spec (GTX 1060, 8GB RAM)
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSystemResourceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxCPUTimeMs = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxMemoryMB = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxDrawCalls = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxTriangles = 10000;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArchitectureConstraints
{
    GENERATED_BODY()

    // World constraints
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxWorldSizeKm = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxConcurrentActors = 100000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxMassAgents = 50000;

    // Performance constraints
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetFrameTimeMs = 16.67f; // 60fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxTotalMemoryMB = 8192;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxVRAMUsageMB = 6144;

    // LOD constraints
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LOD0Distance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LOD1Distance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LOD2Distance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CullDistance = 10000.0f;
};

/**
 * Central architecture management system
 * Enforces architectural constraints across all game systems
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngineArchitectureSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System registration and validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RegisterSystem(const FString& SystemName, ETranspersonalSystemPriority Priority, const FSystemResourceBudget& Budget);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemConstraints(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void EnforcePerformanceBudgets();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetSystemCPUTime(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetSystemMemoryUsage(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsSystemWithinBudget(const FString& SystemName);

    // Architecture constraints
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    const FArchitectureConstraints& GetArchitectureConstraints() const { return Constraints; }

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetPerformanceTier(ETranspersonalPerformanceTier NewTier);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    ETranspersonalPerformanceTier GetCurrentPerformanceTier() const { return CurrentPerformanceTier; }

protected:
    UPROPERTY()
    FArchitectureConstraints Constraints;

    UPROPERTY()
    ETranspersonalPerformanceTier CurrentPerformanceTier = ETranspersonalPerformanceTier::High;

    UPROPERTY()
    TMap<FString, FSystemResourceBudget> SystemBudgets;

    UPROPERTY()
    TMap<FString, ETranspersonalSystemPriority> SystemPriorities;

    void InitializeConstraintsForTier(ETranspersonalPerformanceTier Tier);
    void ValidateAllSystems();
    void LogArchitectureStatus();
};

/**
 * Base class for all major game systems
 * Enforces architectural compliance
 */
UCLASS(Abstract, BlueprintType)
class TRANSPERSONALGAME_API UTranspersonalSystemBase : public UObject
{
    GENERATED_BODY()

public:
    virtual void InitializeSystem() {}
    virtual void ShutdownSystem() {}
    virtual void UpdateSystem(float DeltaTime) {}

    // Performance monitoring interface
    virtual float GetCPUTimeMs() const { return 0.0f; }
    virtual int32 GetMemoryUsageMB() const { return 0; }
    virtual bool IsWithinPerformanceBudget() const { return true; }

    // System identification
    virtual FString GetSystemName() const { return TEXT("BaseSystem"); }
    virtual ETranspersonalSystemPriority GetSystemPriority() const { return ETranspersonalSystemPriority::Medium; }

protected:
    UPROPERTY()
    bool bIsInitialized = false;

    UPROPERTY()
    FSystemResourceBudget ResourceBudget;

    void RegisterWithArchitecture();
    void UnregisterFromArchitecture();
};