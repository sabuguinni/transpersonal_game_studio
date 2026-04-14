#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "EngineArchitecture.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - ENGINE ARCHITECTURE
 * Engine Architect Agent #02
 * 
 * This is the master architecture that defines how ALL systems interact.
 * Every agent must follow these patterns and interfaces.
 * 
 * ARCHITECTURE PRINCIPLES:
 * 1. Modular Design - Each system is independent and replaceable
 * 2. Event-Driven Communication - Systems communicate via events, not direct calls
 * 3. Performance First - 60fps PC / 30fps console is non-negotiable
 * 4. Scalable Structure - Must support 50,000+ entities via Mass AI
 * 5. Hot-Reload Friendly - All systems must support runtime changes
 */

// ═══════════════════════════════════════════════════════════════
// CORE SYSTEM INTERFACE
// ═══════════════════════════════════════════════════════════════

UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SystemBase : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_SystemBase();

    // Core system lifecycle
    UFUNCTION(BlueprintCallable, Category = "System")
    virtual void InitializeSystem();

    UFUNCTION(BlueprintCallable, Category = "System")
    virtual void ShutdownSystem();

    UFUNCTION(BlueprintCallable, Category = "System")
    virtual void UpdateSystem(float DeltaTime);

    // System state management
    UFUNCTION(BlueprintCallable, Category = "System")
    EEng_SystemState GetSystemState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "System")
    void SetSystemState(EEng_SystemState NewState) { CurrentState = NewState; }

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetPerformanceMetrics() const { return PerformanceData; }

    // System communication
    UFUNCTION(BlueprintCallable, Category = "Communication")
    virtual void ReceiveSystemMessage(const FEng_SystemMessage& Message);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void SendSystemMessage(const FEng_SystemMessage& Message);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemState CurrentState = EEng_SystemState::Uninitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemPriority SystemPriority = EEng_SystemPriority::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FEng_PerformanceMetrics PerformanceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName = TEXT("BaseSystem");

    // Performance tracking
    virtual void UpdatePerformanceMetrics(float DeltaTime);
};

// ═══════════════════════════════════════════════════════════════
// ARCHITECTURE MANAGER SUBSYSTEM
// ═══════════════════════════════════════════════════════════════

UCLASS()
class TRANSPERSONALGAME_API UEng_ArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System registration and management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterSystem(UEng_SystemBase* System);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UnregisterSystem(UEng_SystemBase* System);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    UEng_SystemBase* GetSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<UEng_SystemBase*> GetAllSystems() const { return RegisteredSystems; }

    // Global system control
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateAllSystems(float DeltaTime);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetGlobalPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTier(EEng_PerformanceTier NewTier);

    // System communication hub
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void BroadcastSystemMessage(const FEng_SystemMessage& Message);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void SendSystemMessage(const FString& TargetSystem, const FEng_SystemMessage& Message);

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    bool ValidateArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void GenerateArchitectureReport();

protected:
    UPROPERTY()
    TArray<UEng_SystemBase*> RegisteredSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EEng_PerformanceTier CurrentPerformanceTier = EEng_PerformanceTier::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FEng_PerformanceMetrics GlobalPerformanceData;

    // System initialization order (critical for dependencies)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FString> SystemInitializationOrder;

    // Message queue for inter-system communication
    UPROPERTY()
    TArray<FEng_SystemMessage> MessageQueue;

    void ProcessMessageQueue();
    void InitializeSystemOrder();
};

// ═══════════════════════════════════════════════════════════════
// WORLD ARCHITECTURE SUBSYSTEM
// ═══════════════════════════════════════════════════════════════

UCLASS()
class TRANSPERSONALGAME_API UEng_WorldArchitecture : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // World Partition management
    UFUNCTION(BlueprintCallable, Category = "World")
    void InitializeWorldPartition();

    UFUNCTION(BlueprintCallable, Category = "World")
    void LoadWorldCell(const FEng_WorldCell& Cell);

    UFUNCTION(BlueprintCallable, Category = "World")
    void UnloadWorldCell(const FEng_WorldCell& Cell);

    // Streaming and LOD management
    UFUNCTION(BlueprintCallable, Category = "World")
    void UpdateWorldStreaming(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetWorldLODLevel(int32 LODLevel);

    // World state management
    UFUNCTION(BlueprintCallable, Category = "World")
    TArray<FEng_WorldCell> GetLoadedCells() const { return LoadedCells; }

    UFUNCTION(BlueprintCallable, Category = "World")
    bool IsWorldCellLoaded(int32 X, int32 Y) const;

protected:
    UPROPERTY()
    TArray<FEng_WorldCell> LoadedCells;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float CellSize = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 LoadRadius = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CurrentLODLevel = 2;

    void UpdateCellLoading(const FVector& PlayerLocation);
    FEng_WorldCell* FindWorldCell(int32 X, int32 Y);
};

// ═══════════════════════════════════════════════════════════════
// ARCHITECTURE CONSTANTS AND RULES
// ═══════════════════════════════════════════════════════════════

namespace EngineArchitectureConstants
{
    // Performance targets (non-negotiable)
    constexpr float TARGET_FPS_PC = 60.0f;
    constexpr float TARGET_FPS_CONSOLE = 30.0f;
    constexpr float MAX_FRAME_TIME_MS = 16.67f;

    // World limits
    constexpr float WORLD_SIZE_KM = 16.0f;
    constexpr float CELL_SIZE_M = 1000.0f;
    constexpr int32 MAX_LOADED_CELLS = 25;

    // Entity limits for Mass AI
    constexpr int32 MAX_ENTITIES_TOTAL = 50000;
    constexpr int32 MAX_ENTITIES_PER_CELL = 2000;
    constexpr int32 MAX_ACTIVE_ENTITIES = 1000;

    // Memory budgets (MB)
    constexpr float MEMORY_BUDGET_TEXTURES = 2048.0f;
    constexpr float MEMORY_BUDGET_MESHES = 1024.0f;
    constexpr float MEMORY_BUDGET_AUDIO = 512.0f;
    constexpr float MEMORY_BUDGET_PHYSICS = 256.0f;

    // System update frequencies
    constexpr float PHYSICS_TICK_RATE = 60.0f;
    constexpr float AI_TICK_RATE = 30.0f;
    constexpr float WORLD_STREAMING_RATE = 10.0f;
    constexpr float PERFORMANCE_MONITOR_RATE = 1.0f;
}