/**
 * @file WorldPhysicsManager.h
 * @brief Global physics manager for 50km² world streaming and optimization
 * 
 * Manages physics simulation across the large game world with performance optimization
 * for 200+ NPCs and streaming world chunks. Implements spatial partitioning and
 * dynamic LOD for physics objects.
 * 
 * @author Core Systems Programmer
 * @version 1.0
 * @date 2024
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "WorldPhysicsManager.generated.h"

UENUM(BlueprintType)
enum class EPhysicsLOD : uint8
{
    Full        UMETA(DisplayName = "Full Physics"),
    Reduced     UMETA(DisplayName = "Reduced Physics"),
    Kinematic   UMETA(DisplayName = "Kinematic Only"),
    Static      UMETA(DisplayName = "Static Collision"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct FPhysicsChunk
{
    GENERATED_BODY()

    /** Chunk coordinates in world grid */
    UPROPERTY(BlueprintReadOnly)
    FIntPoint ChunkCoords;

    /** World bounds of this chunk */
    UPROPERTY(BlueprintReadOnly)
    FBox ChunkBounds;

    /** Current LOD level for this chunk */
    UPROPERTY(BlueprintReadOnly)
    EPhysicsLOD CurrentLOD;

    /** Number of active physics objects in chunk */
    UPROPERTY(BlueprintReadOnly)
    int32 ActivePhysicsObjects;

    /** Distance from player */
    UPROPERTY(BlueprintReadOnly)
    float DistanceFromPlayer;

    /** Is chunk currently loaded */
    UPROPERTY(BlueprintReadOnly)
    bool bIsLoaded;

    /** Last update time */
    UPROPERTY(BlueprintReadOnly)
    float LastUpdateTime;

    FPhysicsChunk()
    {
        ChunkCoords = FIntPoint::ZeroValue;
        ChunkBounds = FBox(EForceInit::ForceInit);
        CurrentLOD = EPhysicsLOD::Disabled;
        ActivePhysicsObjects = 0;
        DistanceFromPlayer = 0.0f;
        bIsLoaded = false;
        LastUpdateTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FPhysicsPerformanceSettings
{
    GENERATED_BODY()

    /** Chunk size in Unreal units (default: 200000 = 2km) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Streaming", meta = (ClampMin = "50000", ClampMax = "500000"))
    float ChunkSize = 200000.0f;

    /** Maximum distance for full physics (in chunks) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Distances", meta = (ClampMin = "1", ClampMax = "10"))
    int32 FullPhysicsDistance = 2;

    /** Maximum distance for reduced physics (in chunks) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Distances", meta = (ClampMin = "2", ClampMax = "20"))
    int32 ReducedPhysicsDistance = 5;

    /** Maximum distance for kinematic physics (in chunks) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Distances", meta = (ClampMin = "5", ClampMax = "50"))
    int32 KinematicPhysicsDistance = 10;

    /** Target physics update frequency (Hz) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "30", ClampMax = "120"))
    float TargetPhysicsFrequency = 60.0f;

    /** Maximum physics objects per chunk */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "50", ClampMax = "1000"))
    int32 MaxPhysicsObjectsPerChunk = 200;

    /** Enable adaptive LOD based on performance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    bool bEnableAdaptiveLOD = true;

    /** Target frame time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive", meta = (ClampMin = "10.0", ClampMax = "33.0"))
    float TargetFrameTime = 16.67f; // 60 FPS

    /** Physics budget per frame (milliseconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1.0", ClampMax = "10.0"))
    float PhysicsBudgetMs = 4.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsChunkLoaded, FIntPoint, ChunkCoords, EPhysicsLOD, LODLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsChunkUnloaded, FIntPoint, ChunkCoords);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPhysicsLODChanged, FIntPoint, ChunkCoords, EPhysicsLOD, OldLOD, EPhysicsLOD, NewLOD);

/**
 * @class UWorldPhysicsManager
 * @brief Manages physics simulation across the 50km² game world
 * 
 * This subsystem handles:
 * - Physics chunk streaming and LOD management
 * - Performance optimization for large worlds
 * - Spatial partitioning for efficient collision detection
 * - Dynamic physics budget allocation
 */
UCLASS()
class TRANSPERSONALGAME_API UWorldPhysicsManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UWorldPhysicsManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // UWorldSubsystem interface
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

protected:
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UWorldPhysicsManager, STATGROUP_Tickables); }

public:
    /** Performance settings for physics management */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Physics")
    FPhysicsPerformanceSettings PerformanceSettings;

    /** Currently loaded physics chunks */
    UPROPERTY(BlueprintReadOnly, Category = "World State")
    TMap<FIntPoint, FPhysicsChunk> LoadedChunks;

    /** Events for chunk loading/unloading */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsChunkLoaded OnPhysicsChunkLoaded;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsChunkUnloaded OnPhysicsChunkUnloaded;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsLODChanged OnPhysicsLODChanged;

    /**
     * Get physics chunk coordinates for a world position
     * @param WorldPosition Position in world space
     * @return Chunk coordinates
     */
    UFUNCTION(BlueprintPure, Category = "World Physics")
    FIntPoint GetChunkCoordinates(const FVector& WorldPosition) const;

    /**
     * Get current physics LOD for a world position
     * @param WorldPosition Position in world space
     * @return Current LOD level
     */
    UFUNCTION(BlueprintPure, Category = "World Physics")
    EPhysicsLOD GetPhysicsLODAtPosition(const FVector& WorldPosition) const;

    /**
     * Force load physics chunk at coordinates
     * @param ChunkCoords Chunk coordinates to load
     * @param ForcedLOD LOD level to force (optional)
     */
    UFUNCTION(BlueprintCallable, Category = "World Physics")
    void LoadPhysicsChunk(const FIntPoint& ChunkCoords, EPhysicsLOD ForcedLOD = EPhysicsLOD::Full);

    /**
     * Unload physics chunk at coordinates
     * @param ChunkCoords Chunk coordinates to unload
     */
    UFUNCTION(BlueprintCallable, Category = "World Physics")
    void UnloadPhysicsChunk(const FIntPoint& ChunkCoords);

    /**
     * Set physics LOD for a specific chunk
     * @param ChunkCoords Target chunk coordinates
     * @param NewLOD New LOD level
     */
    UFUNCTION(BlueprintCallable, Category = "World Physics")
    void SetChunkPhysicsLOD(const FIntPoint& ChunkCoords, EPhysicsLOD NewLOD);

    /**
     * Get current physics performance metrics
     * @param OutFrameTime Current frame time in ms
     * @param OutPhysicsTime Time spent on physics in ms
     * @param OutActiveChunks Number of active physics chunks
     */
    UFUNCTION(BlueprintPure, Category = "Performance")
    void GetPhysicsPerformanceMetrics(float& OutFrameTime, float& OutPhysicsTime, int32& OutActiveChunks) const;

    /**
     * Register a physics object with the manager
     * @param PhysicsObject The object to register
     * @param WorldPosition Object's world position
     */
    UFUNCTION(BlueprintCallable, Category = "World Physics")
    void RegisterPhysicsObject(UPrimitiveComponent* PhysicsObject, const FVector& WorldPosition);

    /**
     * Unregister a physics object from the manager
     * @param PhysicsObject The object to unregister
     */
    UFUNCTION(BlueprintCallable, Category = "World Physics")
    void UnregisterPhysicsObject(UPrimitiveComponent* PhysicsObject);

    /**
     * Update player position for LOD calculations
     * @param NewPlayerPosition Current player world position
     */
    UFUNCTION(BlueprintCallable, Category = "World Physics")
    void UpdatePlayerPosition(const FVector& NewPlayerPosition);

private:
    /** Current player position for LOD calculations */
    FVector PlayerPosition;

    /** Registered physics objects by chunk */
    TMap<FIntPoint, TArray<TWeakObjectPtr<UPrimitiveComponent>>> ChunkPhysicsObjects;

    /** Performance tracking */
    float LastFrameTime;
    float LastPhysicsTime;
    float PhysicsTimeAccumulator;
    int32 FrameCounter;

    /** Update frequency control */
    float TimeSinceLastUpdate;
    float UpdateInterval;

    /** Update physics chunks based on player position */
    void UpdatePhysicsChunks();

    /** Calculate appropriate LOD for chunk distance */
    EPhysicsLOD CalculateLODForDistance(float DistanceInChunks) const;

    /** Apply LOD settings to chunk objects */
    void ApplyLODToChunk(const FIntPoint& ChunkCoords, EPhysicsLOD NewLOD);

    /** Update performance metrics */
    void UpdatePerformanceMetrics(float DeltaTime);

    /** Perform adaptive LOD adjustments based on performance */
    void PerformAdaptiveLODAdjustment();

    /** Get chunk bounds from coordinates */
    FBox GetChunkBounds(const FIntPoint& ChunkCoords) const;

    /** Calculate distance between chunks */
    float CalculateChunkDistance(const FIntPoint& ChunkA, const FIntPoint& ChunkB) const;
};