// Copyright Transpersonal Game Studio. All Rights Reserved.
// PrehistoricEnvironmentManager.h - Core environment art management system

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataAsset.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "FoliageType.h"
#include "InstancedFoliageActor.h"
#include "PrehistoricEnvironmentManager.generated.h"

class ULandscape;
class UMaterialParameterCollection;
class UFoliageType_InstancedStaticMesh;
class UNiagaraSystem;
class UAudioComponent;

UENUM(BlueprintType)
enum class EEnvironmentLayer : uint8
{
    GroundCover      UMETA(DisplayName = "Ground Cover"),
    Undergrowth      UMETA(DisplayName = "Undergrowth"),
    MidCanopy        UMETA(DisplayName = "Mid Canopy"),
    CanopyTop        UMETA(DisplayName = "Canopy Top"),
    Rocks            UMETA(DisplayName = "Rocks"),
    DeadFallage      UMETA(DisplayName = "Dead Fallage"),
    WaterFeatures    UMETA(DisplayName = "Water Features"),
    Atmospheric      UMETA(DisplayName = "Atmospheric")
};

UENUM(BlueprintType)
enum class EEnvironmentMood : uint8
{
    Peaceful         UMETA(DisplayName = "Peaceful"),
    Mysterious       UMETA(DisplayName = "Mysterious"),
    Threatening      UMETA(DisplayName = "Threatening"),
    Ancient          UMETA(DisplayName = "Ancient"),
    Lush             UMETA(DisplayName = "Lush"),
    Desolate         UMETA(DisplayName = "Desolate"),
    Primordial       UMETA(DisplayName = "Primordial")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvironmentAssetData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TSoftObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float MinScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float MaxScale = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float SpawnProbability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float MinSlopeAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float MaxSlopeAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float MinAltitude = -1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float MaxAltitude = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering")
    bool bUseClusterPlacement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering", meta = (EditCondition = "bUseClusterPlacement"))
    float ClusterRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering", meta = (EditCondition = "bUseClusterPlacement"))
    int32 MinClusterSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering", meta = (EditCondition = "bUseClusterPlacement"))
    int32 MaxClusterSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableNanite = true;

    FEnvironmentAssetData()
    {
        MinScale = 0.8f;
        MaxScale = 1.2f;
        SpawnProbability = 1.0f;
        MinSlopeAngle = 0.0f;
        MaxSlopeAngle = 45.0f;
        MinAltitude = -1000.0f;
        MaxAltitude = 1000.0f;
        bUseClusterPlacement = false;
        ClusterRadius = 500.0f;
        MinClusterSize = 3;
        MaxClusterSize = 8;
        CullDistance = 5000.0f;
        bEnableNanite = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvironmentLayerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    EEnvironmentLayer LayerType = EEnvironmentLayer::GroundCover;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    FString LayerName = TEXT("Environment Layer");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<FEnvironmentAssetData> LayerAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
    float BaseDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
    float DensityVariation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    bool bUseNoiseDistribution = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (EditCondition = "bUseNoiseDistribution"))
    float NoiseScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (EditCondition = "bUseNoiseDistribution"))
    float NoiseThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxInstancesPerComponent = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem = true;

    FEnvironmentLayerData()
    {
        LayerType = EEnvironmentLayer::GroundCover;
        LayerName = TEXT("Environment Layer");
        BaseDensity = 1.0f;
        DensityVariation = 0.2f;
        bUseNoiseDistribution = true;
        NoiseScale = 0.01f;
        NoiseThreshold = 0.3f;
        MaxInstancesPerComponent = 1000;
        bEnableLODSystem = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvironmentMoodSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    EEnvironmentMood MoodType = EEnvironmentMood::Peaceful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientLightColor = FLinearColor(0.2f, 0.3f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientLightIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    TSoftObjectPtr<UNiagaraSystem> AtmosphericParticles;

    FEnvironmentMoodSettings()
    {
        MoodType = EEnvironmentMood::Peaceful;
        AmbientLightColor = FLinearColor(0.2f, 0.3f, 0.4f, 1.0f);
        AmbientLightIntensity = 0.5f;
        FogDensity = 0.1f;
        FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
        WindStrength = 1.0f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        AmbientVolume = 0.5f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPrehistoricEnvironmentDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FString EnvironmentName = TEXT("Prehistoric Environment");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layers")
    TArray<FEnvironmentLayerData> EnvironmentLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialParameterCollection> GlobalMaterialParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> TerrainMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    TArray<FEnvironmentMoodSettings> MoodPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxDrawDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTotalInstances = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableHierarchicalLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bEnableEnvironmentalStorytelling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float StorytellingDensity = 0.1f;
};

/**
 * APrehistoricEnvironmentManager
 * 
 * Core actor responsible for managing all environment art in prehistoric biomes.
 * Handles foliage placement, material management, atmospheric effects, and environmental storytelling.
 * 
 * Key Features:
 * - Layered foliage system with performance optimization
 * - Dynamic material parameter management
 * - Mood-based environmental effects
 * - Environmental storytelling through prop placement
 * - Integration with PCG and World Partition
 * - Real-time performance monitoring and adjustment
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrehistoricEnvironmentManager : public AActor
{
    GENERATED_BODY()

public:
    APrehistoricEnvironmentManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

public:
    // Environment Management Interface
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void InitializeEnvironment(UPrehistoricEnvironmentDataAsset* EnvironmentData);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void PopulateEnvironmentLayer(EEnvironmentLayer LayerType, const FBox& AreaBounds);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetEnvironmentMood(EEnvironmentMood NewMood, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateMaterialParameters();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void RefreshEnvironmentArea(const FVector& Center, float Radius);

    // Foliage Management
    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void PlaceFoliageInstances(const FEnvironmentLayerData& LayerData, const FBox& PlacementArea);

    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void RemoveFoliageInArea(const FVector& Center, float Radius, EEnvironmentLayer LayerType = EEnvironmentLayer::GroundCover);

    UFUNCTION(BlueprintCallable, Category = "Foliage")
    int32 GetFoliageInstanceCount(EEnvironmentLayer LayerType) const;

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(int32 Level); // 0=Ultra, 1=High, 2=Medium, 3=Low

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentPerformanceMetric() const { return CurrentPerformanceScore; }

    // Environmental Storytelling
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlaceStorytellingElements(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateDinosaurNest(const FVector& Location, float Size);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateAbandonedCampsite(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateBoneScatter(const FVector& Location, float Radius);

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleLayerVisibility(EEnvironmentLayer LayerType, bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawEnvironmentBounds(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void RegenerateLayer(EEnvironmentLayer LayerType);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnvironmentInitialized, bool, bSuccess, FString, ErrorMessage);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLayerPopulated, EEnvironmentLayer, LayerType, int32, InstanceCount, float, CompletionTime);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoodChanged, EEnvironmentMood, OldMood, EEnvironmentMood, NewMood);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEnvironmentInitialized OnEnvironmentInitialized;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnLayerPopulated OnLayerPopulated;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMoodChanged OnMoodChanged;

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    // Environment Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TObjectPtr<UPrehistoricEnvironmentDataAsset> EnvironmentDataAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EEnvironmentMood CurrentMood = EEnvironmentMood::Peaceful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float EnvironmentScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 PerformanceLevel = 1; // 0=Ultra, 1=High, 2=Medium, 3=Low

    // Internal Management
    UPROPERTY()
    TMap<EEnvironmentLayer, TObjectPtr<AInstancedFoliageActor>> LayerFoliageActors;

    UPROPERTY()
    TObjectPtr<UMaterialParameterCollection> MaterialParameterCollection;

    UPROPERTY()
    TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

    // Performance Tracking
    float CurrentPerformanceScore = 100.0f;
    float LastPerformanceUpdate = 0.0f;
    static constexpr float PerformanceUpdateInterval = 1.0f;

    // Mood Transition
    bool bMoodTransitionInProgress = false;
    float MoodTransitionProgress = 0.0f;
    float MoodTransitionDuration = 2.0f;
    EEnvironmentMood TargetMood = EEnvironmentMood::Peaceful;
    EEnvironmentMood PreviousMood = EEnvironmentMood::Peaceful;

    // Debug Visualization
    bool bDebugDrawBounds = false;
    TArray<FBox> DebugBounds;

private:
    // Internal helper functions
    void InitializeFoliageActors();
    void SetupMaterialParameterCollection();
    void UpdateMoodTransition(float DeltaTime);
    void UpdatePerformanceMetrics();
    
    // Foliage placement helpers
    bool IsValidPlacementLocation(const FVector& Location, const FEnvironmentAssetData& AssetData) const;
    FTransform GenerateInstanceTransform(const FVector& Location, const FEnvironmentAssetData& AssetData) const;
    void PlaceClusteredInstances(const FEnvironmentAssetData& AssetData, const FVector& ClusterCenter, int32 ClusterSize);
    
    // Environmental storytelling helpers
    void PlaceRandomStoryElement(const FVector& Location);
    TArray<FVector> GenerateStorytellingLocations(const FVector& Center, float Radius, int32 Count) const;
    
    // Performance optimization helpers
    void CullDistantInstances();
    void AdjustLODDistances();
    void OptimizeInstanceCounts();
};