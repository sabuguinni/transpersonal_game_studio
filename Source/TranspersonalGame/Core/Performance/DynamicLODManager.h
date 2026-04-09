// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Camera/CameraComponent.h"
#include "DynamicLODManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDynamicLOD, Log, All);

/**
 * LOD calculation methods
 */
UENUM(BlueprintType)
enum class ELODCalculationMethod : uint8
{
    Distance        UMETA(DisplayName = "Distance Based"),
    ScreenSize      UMETA(DisplayName = "Screen Size Based"),
    Performance     UMETA(DisplayName = "Performance Based"),
    Hybrid          UMETA(DisplayName = "Hybrid Method")
};

/**
 * LOD quality levels
 */
UENUM(BlueprintType)
enum class ELODQualityLevel : uint8
{
    Ultra           UMETA(DisplayName = "Ultra"),
    High            UMETA(DisplayName = "High"),
    Medium          UMETA(DisplayName = "Medium"),
    Low             UMETA(DisplayName = "Low"),
    VeryLow         UMETA(DisplayName = "Very Low")
};

/**
 * LOD settings for different object types
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD3Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float ScreenSizeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bUseDynamicLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float PerformanceWeight;

    FLODSettings()
        : LOD0Distance(0.0f)
        , LOD1Distance(500.0f)
        , LOD2Distance(1500.0f)
        , LOD3Distance(3000.0f)
        , CullDistance(5000.0f)
        , ScreenSizeThreshold(0.1f)
        , bUseDynamicLOD(true)
        , PerformanceWeight(1.0f)
    {}
};

/**
 * LOD object tracking structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLODObjectInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "LOD")
    TWeakObjectPtr<UPrimitiveComponent> Component;

    UPROPERTY(BlueprintReadOnly, Category = "LOD")
    int32 CurrentLOD;

    UPROPERTY(BlueprintReadOnly, Category = "LOD")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "LOD")
    float DistanceToCamera;

    UPROPERTY(BlueprintReadOnly, Category = "LOD")
    float ScreenSize;

    UPROPERTY(BlueprintReadOnly, Category = "LOD")
    bool bIsVisible;

    UPROPERTY(BlueprintReadOnly, Category = "LOD")
    FLODSettings LODSettings;

    FLODObjectInfo()
        : Component(nullptr)
        , CurrentLOD(0)
        , LastUpdateTime(0.0f)
        , DistanceToCamera(0.0f)
        , ScreenSize(0.0f)
        , bIsVisible(true)
    {}
};

/**
 * Performance-based LOD adjustment parameters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceLODParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTimeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODBiasAdjustmentRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinLODBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxLODBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAdaptiveLOD;

    FPerformanceLODParams()
        : TargetFrameTime(16.67f) // 60 FPS
        , FrameTimeThreshold(20.0f)
        , LODBiasAdjustmentRate(0.1f)
        , MinLODBias(0.0f)
        , MaxLODBias(3.0f)
        , bEnableAdaptiveLOD(true)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLODChanged, UPrimitiveComponent*, Component, int32, OldLOD, int32, NewLOD);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLODBiasChanged, float, OldBias, float, NewBias);

/**
 * Dynamic LOD Manager System
 * Manages Level of Detail for all objects in the scene based on distance, screen size, and performance
 * Provides automatic LOD adjustment to maintain target framerate
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDynamicLODManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UDynamicLODManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // World Subsystem interface
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableDynamicLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    ELODCalculationMethod LODCalculationMethod;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    ELODQualityLevel GlobalLODQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxObjectsPerUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FPerformanceLODParams PerformanceParams;

    // LOD Settings for different object categories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    FLODSettings StaticMeshLODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    FLODSettings SkeletalMeshLODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    FLODSettings FoliageLODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    FLODSettings ParticleLODSettings;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnLODChanged OnLODChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnLODBiasChanged OnLODBiasChanged;

    // Public Methods
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void RegisterLODObject(UPrimitiveComponent* Component, const FLODSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UnregisterLODObject(UPrimitiveComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateLODForObject(UPrimitiveComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateAllLODs();

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetGlobalLODBias(float LODBias);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    float GetGlobalLODBias() const { return CurrentLODBias; }

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetLODQualityLevel(ELODQualityLevel QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    int32 GetLODForObject(UPrimitiveComponent* Component) const;

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    float GetDistanceToCamera(UPrimitiveComponent* Component) const;

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    float GetScreenSize(UPrimitiveComponent* Component) const;

    // Performance-based LOD methods
    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void EnablePerformanceBasedLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void SetTargetFrameTime(float FrameTime);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void AdjustLODForPerformance(float CurrentFrameTime);

    // Utility methods
    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<FLODObjectInfo> GetAllLODObjects() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    int32 GetTotalManagedObjects() const { return LODObjects.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ForceUpdateAllLODs();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ResetAllLODs();

    // Debug methods
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void EnableLODVisualization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogLODStatistics();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawLODDebugInfo();

private:
    // Internal state
    TArray<FLODObjectInfo> LODObjects;
    int32 CurrentUpdateIndex;
    float LastUpdateTime;
    float CurrentLODBias;
    bool bPerformanceBasedLODEnabled;
    
    // Camera tracking
    TWeakObjectPtr<UCameraComponent> CurrentCamera;
    FVector LastCameraLocation;
    FRotator LastCameraRotation;
    
    // Performance tracking
    TArray<float> RecentFrameTimes;
    float AverageFrameTime;
    bool bNeedsPerformanceAdjustment;
    
    // Timer handles
    FTimerHandle LODUpdateTimerHandle;
    FTimerHandle PerformanceCheckTimerHandle;
    
    // Debug visualization
    bool bLODVisualizationEnabled;
    TArray<FVector> DebugLODPositions;
    TArray<int32> DebugLODLevels;
    
    // Internal methods
    void UpdateLODSystem();
    void UpdateCameraReference();
    void CalculateLODForObject(FLODObjectInfo& ObjectInfo);
    int32 CalculateDistanceBasedLOD(const FLODObjectInfo& ObjectInfo) const;
    int32 CalculateScreenSizeBasedLOD(const FLODObjectInfo& ObjectInfo) const;
    int32 CalculatePerformanceBasedLOD(const FLODObjectInfo& ObjectInfo) const;
    int32 CalculateHybridLOD(const FLODObjectInfo& ObjectInfo) const;
    
    // LOD application
    void ApplyLODToStaticMesh(UStaticMeshComponent* StaticMeshComp, int32 LODLevel);
    void ApplyLODToSkeletalMesh(USkeletalMeshComponent* SkeletalMeshComp, int32 LODLevel);
    void ApplyLODToParticleSystem(UPrimitiveComponent* ParticleComp, int32 LODLevel);
    
    // Distance and screen size calculations
    float CalculateDistanceToCamera(const FVector& ObjectLocation) const;
    float CalculateScreenSize(UPrimitiveComponent* Component) const;
    FVector GetCameraLocation() const;
    FRotator GetCameraRotation() const;
    
    // Performance monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void AdjustGlobalLODBias(float TargetAdjustment);
    
    // Quality level presets
    void ApplyUltraQualitySettings();
    void ApplyHighQualitySettings();
    void ApplyMediumQualitySettings();
    void ApplyLowQualitySettings();
    void ApplyVeryLowQualitySettings();
    
    // Utility functions
    FLODSettings GetLODSettingsForComponent(UPrimitiveComponent* Component) const;
    bool IsComponentValid(UPrimitiveComponent* Component) const;
    void CleanupInvalidObjects();
    
    // Debug and visualization
    void DrawLODDebugSphere(const FVector& Location, int32 LODLevel, float Radius = 50.0f);
    void DrawLODDebugText(const FVector& Location, int32 LODLevel, float Distance);
    FColor GetLODDebugColor(int32 LODLevel) const;
    
    // Console variable management
    void UpdateLODConsoleVariables();
    void SetStaticMeshLODBias(float Bias);
    void SetSkeletalMeshLODBias(float Bias);
    void SetParticleLODBias(float Bias);
    
    // Event handling
    void OnLODChangedInternal(UPrimitiveComponent* Component, int32 OldLOD, int32 NewLOD);
    void OnLODBiasChangedInternal(float OldBias, float NewBias);
    
    // Optimization helpers
    void OptimizeLODUpdateOrder();
    void PrioritizeVisibleObjects();
    bool ShouldUpdateObject(const FLODObjectInfo& ObjectInfo) const;
};