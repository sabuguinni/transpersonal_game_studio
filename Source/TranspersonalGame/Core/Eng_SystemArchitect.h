#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_SystemArchitect.generated.h"

// Forward declarations
class UEng_ArchitectureCore;
class UCore_PhysicsManager;
class UBiomeManager;
class ADinosaurBase;
class ATranspersonalCharacter;

UENUM(BlueprintType)
enum class EEng_SystemState : uint8
{
    Uninitialized   UMETA(DisplayName = "Uninitialized"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Ready          UMETA(DisplayName = "Ready"),
    Running        UMETA(DisplayName = "Running"),
    Error          UMETA(DisplayName = "Error"),
    Shutdown       UMETA(DisplayName = "Shutdown")
};

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical       UMETA(DisplayName = "Critical"),      // Physics, Core Systems
    High          UMETA(DisplayName = "High"),          // World, Character
    Medium        UMETA(DisplayName = "Medium"),        // AI, Combat
    Low           UMETA(DisplayName = "Low"),           // Audio, VFX
    Background    UMETA(DisplayName = "Background")     // Analytics, Debug
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    EEng_SystemState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    EEng_SystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    float InitializationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    bool bIsEssential;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    TArray<FString> Dependencies;

    FEng_SystemInfo()
    {
        SystemName = TEXT("Unknown");
        State = EEng_SystemState::Uninitialized;
        Priority = EEng_SystemPriority::Medium;
        InitializationTime = 0.0f;
        bIsEssential = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 PhysicsBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    FEng_PerformanceMetrics()
    {
        FrameTime = 0.0f;
        PhysicsTime = 0.0f;
        RenderTime = 0.0f;
        ActiveActors = 0;
        PhysicsBodies = 0;
        MemoryUsageMB = 0.0f;
    }
};

/**
 * System Architect - Master coordinator for all game systems
 * Manages initialization order, dependencies, and system health
 * Ensures proper architectural compliance across all subsystems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SystemArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_SystemArchitect();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Management
    UFUNCTION(BlueprintCallable, Category = "System Architect")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    bool RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority, bool bIsEssential = false);

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    void UpdateSystemState(const FString& SystemName, EEng_SystemState NewState);

    // System Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Architect")
    EEng_SystemState GetSystemState(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Architect")
    bool IsSystemReady(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Architect")
    bool AreAllCriticalSystemsReady() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Architect")
    TArray<FEng_SystemInfo> GetAllSystemInfo() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "System Architect")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Architect")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    void LogSystemStatus() const;

    // Architectural Validation
    UFUNCTION(BlueprintCallable, Category = "System Architect")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    void EnforceArchitecturalStandards();

    // Emergency Controls
    UFUNCTION(BlueprintCallable, Category = "System Architect")
    void EmergencySystemShutdown(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    void RestartFailedSystems();

protected:
    // System Registry
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System Registry")
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    // Performance Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MetricsUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxInitializationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnablePerformanceMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnforceStrictDependencies;

private:
    // Internal system management
    void InitializeSystemsByPriority();
    void ValidateSystemDependencies();
    bool CheckSystemHealth(const FString& SystemName);
    void HandleSystemFailure(const FString& SystemName);
    
    // Performance tracking
    FTimerHandle MetricsTimerHandle;
    void UpdateMetricsTimer();
    
    // System references (weak to avoid circular dependencies)
    TWeakObjectPtr<UEng_ArchitectureCore> ArchitectureCore;
    TWeakObjectPtr<UCore_PhysicsManager> PhysicsManager;
    TWeakObjectPtr<UBiomeManager> BiomeManager;
};

/**
 * World-specific System Coordinator
 * Manages per-world system instances and coordination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_WorldSystemCoordinator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_WorldSystemCoordinator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // World System Management
    UFUNCTION(BlueprintCallable, Category = "World Systems")
    void InitializeWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World Systems")
    void UpdateWorldSystems(float DeltaTime);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Systems")
    bool AreWorldSystemsReady() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Systems")
    bool bWorldSystemsInitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float WorldSystemUpdateRate;

private:
    FTimerHandle WorldUpdateTimerHandle;
    void WorldSystemUpdateTick();
};

#include "Eng_SystemArchitect.generated.h"