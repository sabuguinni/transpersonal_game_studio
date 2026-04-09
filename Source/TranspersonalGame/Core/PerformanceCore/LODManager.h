// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "LODManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLODManager, Log, All);

/** LOD calculation method */
UENUM(BlueprintType)
enum class ELODCalculationMethod : uint8
{
    Distance        UMETA(DisplayName = "Distance Based"),
    ScreenSize      UMETA(DisplayName = "Screen Size Based"),
    Performance     UMETA(DisplayName = "Performance Based"),
    Hybrid          UMETA(DisplayName = "Hybrid Method")
};

/** LOD quality levels */
UENUM(BlueprintType)
enum class ELODQuality : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Minimal     UMETA(DisplayName = "Minimal Quality")
};

/** Dynamic LOD settings for objects */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDynamicLODSettings
{
    GENERATED_BODY()

    /** Distance thresholds for each LOD level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<float> DistanceThresholds = {500.0f, 1000.0f, 2000.0f, 4000.0f};

    /** Screen size thresholds for each LOD level (0.0-1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<float> ScreenSizeThresholds = {0.5f, 0.25f, 0.125f, 0.0625f};

    /** Performance impact multipliers for each LOD level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<float> PerformanceMultipliers = {1.0f, 0.75f, 0.5f, 0.25f};

    /** Enable automatic LOD transitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bAutoLODTransitions = true;

    /** LOD transition smoothness (0.0-1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TransitionSmoothness = 0.5f;

    /** Minimum LOD level (never go below this) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0", ClampMax = "4"))
    int32 MinLODLevel = 0;

    /** Maximum LOD level (never go above this) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0", ClampMax = "4"))
    int32 MaxLODLevel = 4;
};

/** LOD object registration data */
USTRUCT()
struct TRANSPERSONALGAME_API FLODObjectData
{
    GENERATED_BODY()

    /** The mesh component being managed */
    UPROPERTY()
    TWeakObjectPtr<UMeshComponent> MeshComponent;

    /** Current LOD level */
    int32 CurrentLOD = 0;

    /** Target LOD level */
    int32 TargetLOD = 0;

    /** Last distance to viewer */
    float LastDistance = 0.0f;

    /** Last screen size */
    float LastScreenSize = 0.0f;

    /** Priority for LOD updates (higher = more important) */
    int32 Priority = 0;

    /** Custom LOD settings for this object */
    FDynamicLODSettings LODSettings;

    /** Whether this object is currently visible */
    bool bIsVisible = true;

    /** Last update time */
    float LastUpdateTime = 0.0f;
};

/**
 * @brief LOD Manager - Dynamic Level of Detail management system
 * 
 * Manages LOD (Level of Detail) for all registered mesh components to optimize
 * rendering performance. Supports distance-based, screen-size-based, and
 * performance-based LOD calculation methods.
 * 
 * Key Features:
 * - Automatic LOD level calculation based on distance and screen size
 * - Performance-aware LOD adjustment
 * - Smooth LOD transitions
 * - Custom LOD settings per object type
 * - Mass LOD updates for large numbers of objects
 * - Integration with Nanite virtualized geometry
 * 
 * @author Performance Optimizer — Agent #4
 * @version 1.0 — March 2026
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ULODManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Get the LOD manager instance */
    UFUNCTION(BlueprintPure, Category = "LOD Management", CallInEditor = true)
    static ULODManager* Get(const UObject* WorldContext);

    /** Register a mesh component for LOD management */
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void RegisterMeshComponent(UMeshComponent* MeshComponent, int32 Priority = 0, 
                              const FDynamicLODSettings& CustomSettings = FDynamicLODSettings());

    /** Unregister a mesh component from LOD management */
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UnregisterMeshComponent(UMeshComponent* MeshComponent);

    /** Set global LOD calculation method */
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetLODCalculationMethod(ELODCalculationMethod Method);

    /** Set global LOD quality level */
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetLODQuality(ELODQuality Quality);

    /** Update LOD for all registered objects */
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateAllLODs(FVector ViewerLocation, FVector ViewerDirection = FVector::ForwardVector);

    /** Force LOD level for a specific component */
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void ForceLODLevel(UMeshComponent* MeshComponent, int32 LODLevel);

    /** Get current LOD level for a component */
    UFUNCTION(BlueprintPure, Category = "LOD Management")
    int32 GetCurrentLOD(UMeshComponent* MeshComponent) const;

    /** Set LOD update frequency in Hz */
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetLODUpdateFrequency(float FrequencyHz);

    /** Enable/disable automatic LOD updates */
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetAutoLODUpdatesEnabled(bool bEnabled);

    /** Get LOD statistics */
    UFUNCTION(BlueprintPure, Category = "LOD Management")
    void GetLODStatistics(int32& TotalObjects, int32& LOD0Count, int32& LOD1Count, 
                         int32& LOD2Count, int32& LOD3Count, int32& LOD4Count) const;

    /** Set default LOD settings */
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetDefaultLODSettings(const FDynamicLODSettings& Settings);

    /** Calculate optimal LOD level for a component */
    UFUNCTION(BlueprintPure, Category = "LOD Management")
    int32 CalculateOptimalLOD(UMeshComponent* MeshComponent, FVector ViewerLocation, 
                             FVector ViewerDirection = FVector::ForwardVector) const;

    /** Enable/disable LOD for specific component types */
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetComponentTypeLODEnabled(TSubclassOf<UMeshComponent> ComponentClass, bool bEnabled);

    /** Set performance budget for LOD system */
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetPerformanceBudget(float MaxUpdateTimeMs);

    /** Get current performance impact of LOD system */
    UFUNCTION(BlueprintPure, Category = "LOD Management")
    float GetLODSystemPerformanceMs() const;

protected:
    /** Current LOD calculation method */
    UPROPERTY(BlueprintReadOnly, Category = "LOD Management")
    ELODCalculationMethod CurrentCalculationMethod = ELODCalculationMethod::Hybrid;

    /** Current LOD quality level */
    UPROPERTY(BlueprintReadOnly, Category = "LOD Management")
    ELODQuality CurrentQuality = ELODQuality::Medium;

    /** Default LOD settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Management")
    FDynamicLODSettings DefaultLODSettings;

    /** LOD update frequency in Hz */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Management", meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float LODUpdateFrequency = 10.0f;

    /** Enable automatic LOD updates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Management")
    bool bAutoLODUpdatesEnabled = true;

    /** Maximum time per frame for LOD updates (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Management", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float MaxLODUpdateTimeMs = 2.0f;

    /** Enable LOD transitions smoothing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Management")
    bool bSmoothLODTransitions = true;

    /** LOD bias for performance adjustment (-2 to +2) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Management", meta = (ClampMin = "-2", ClampMax = "2"))
    int32 LODBias = 0;

private:
    /** Registered LOD objects */
    TMap<TWeakObjectPtr<UMeshComponent>, FLODObjectData> RegisteredObjects;

    /** Current viewer location for LOD calculations */
    FVector CurrentViewerLocation = FVector::ZeroVector;

    /** Current viewer direction for LOD calculations */
    FVector CurrentViewerDirection = FVector::ForwardVector;

    /** Timer handle for LOD updates */
    FTimerHandle LODUpdateTimer;

    /** Performance tracking */
    float LastLODUpdateTime = 0.0f;
    float AverageLODUpdateTime = 0.0f;

    /** Component type LOD enablement */
    TMap<TSubclassOf<UMeshComponent>, bool> ComponentTypeLODEnabled;

    /** LOD update batch size for performance */
    int32 LODUpdateBatchSize = 100;

    /** Current batch index for LOD updates */
    int32 CurrentBatchIndex = 0;

    /** Initialize default settings */
    void InitializeDefaultSettings();

    /** Update LOD for a batch of objects */
    void UpdateLODBatch();

    /** Calculate LOD level based on distance */
    int32 CalculateDistanceBasedLOD(const FLODObjectData& ObjectData, float Distance) const;

    /** Calculate LOD level based on screen size */
    int32 CalculateScreenSizeBasedLOD(const FLODObjectData& ObjectData, float ScreenSize) const;

    /** Calculate LOD level based on performance */
    int32 CalculatePerformanceBasedLOD(const FLODObjectData& ObjectData, float PerformanceRatio) const;

    /** Apply LOD level to mesh component */
    void ApplyLODToComponent(UMeshComponent* MeshComponent, int32 LODLevel);

    /** Calculate screen size for a component */
    float CalculateScreenSize(UMeshComponent* MeshComponent, FVector ViewerLocation) const;

    /** Get distance from viewer to component */
    float GetDistanceToViewer(UMeshComponent* MeshComponent, FVector ViewerLocation) const;

    /** Check if component should use LOD management */
    bool ShouldManageComponentLOD(UMeshComponent* MeshComponent) const;

    /** Cleanup invalid object references */
    void CleanupInvalidReferences();

    /** Apply quality settings to LOD settings */
    FDynamicLODSettings ApplyQualityToSettings(const FDynamicLODSettings& BaseSettings) const;

    /** Get performance ratio for performance-based LOD */
    float GetCurrentPerformanceRatio() const;
};