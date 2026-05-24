#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "../SharedTypes.h"
#include "PrehistoricEnvironmentManager.generated.h"

class ULandscape;
class UMaterialParameterCollection;
class UFoliageType_InstancedStaticMesh;
class UNiagaraSystem;
class UAudioComponent;

UENUM(BlueprintType)
enum class EEnvi_EnvironmentLayer : uint8
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
struct TRANSPERSONALGAME_API FEnvArt_BiomeVegetationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<class UStaticMesh*> TreeMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<class UStaticMesh*> ShrubMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<class UStaticMesh*> GrassMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float ScaleVariation = 0.2f;

    FEnvArt_BiomeVegetationData()
    {
        VegetationDensity = 1.0f;
        ScaleVariation = 0.2f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_EnvironmentProps
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    EEnvi_EnvironmentLayer LayerType = EEnvi_EnvironmentLayer::GroundCover;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<class UStaticMesh*> FallenLogMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<class UStaticMesh*> BoneMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    float PropDensity = 0.5f;

    FEnvArt_EnvironmentProps()
    {
        LayerType = EEnvi_EnvironmentLayer::GroundCover;
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

/**
 * Manages prehistoric environment art placement and biome-specific vegetation
 * Handles the visual storytelling through environmental details
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
    virtual void Tick(float DeltaTime);

    // Core environment data per biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (AllowPrivateAccess = "true"))
    TMap<EBiomeType, FEnvArt_BiomeVegetationData> BiomeVegetationMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (AllowPrivateAccess = "true"))
    TMap<EBiomeType, FEnvArt_EnvironmentProps> BiomePropsMap;

    // Instanced mesh components for performance
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TArray<class UInstancedStaticMeshComponent*> VegetationComponents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TArray<class UInstancedStaticMeshComponent*> PropComponents;

    // Environment storytelling parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling", meta = (AllowPrivateAccess = "true"))
    float NarrativeDetailDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling", meta = (AllowPrivateAccess = "true"))
    bool bEnableEnvironmentalStorytelling = true;

    // Material management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
    class UMaterialInterface* TerrainMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
    TMap<EBiomeType, class UMaterialInterface*> BiomeMaterials;

public:
    // Environment population functions
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void PopulateBiomeVegetation(EBiomeType BiomeType, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void PopulateEnvironmentLayer(EEnvi_EnvironmentLayer LayerType, const FBox& AreaBounds);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void CreateNarrativeDetails(const FVector& Location, const FString& StoryContext);

    // Biome transition management
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void BlendBiomeVegetation(EBiomeType FromBiome, EBiomeType ToBiome, const FVector& TransitionCenter, float BlendRadius);

    // Material application
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyBiomeMaterials(EBiomeType BiomeType, class ALandscape* TargetLandscape);

    // Foliage Management
    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void PlaceFoliageInstances(const FEnvironmentLayerData& LayerData, const FBox& PlacementArea);

    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void RemoveFoliageInArea(const FVector& Center, float Radius, EEnvi_EnvironmentLayer LayerType = EEnvi_EnvironmentLayer::GroundCover);

    UFUNCTION(BlueprintCallable, Category = "Foliage")
    int32 GetFoliageInstanceCount(EEnvi_EnvironmentLayer LayerType) const;

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(int32 Level); // 0=Ultra, 1=High, 2=Medium, 3=Low

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentPerformanceMetric() const { return CurrentPerformanceScore; }

    // Environmental Storytelling
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlaceStorytellingElements(const FVector& Location, const FString& NarrativeHint);

    // Cleanup and optimization
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void OptimizeVegetationLODs();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ClearEnvironmentArea(const FVector& Center, float Radius);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugShowBiomeData();

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleLayerVisibility(EEnvi_EnvironmentLayer LayerType, bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawEnvironmentBounds(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void RegenerateLayer(EEnvi_EnvironmentLayer LayerType);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnvironmentInitialized, bool, bSuccess, FString, ErrorMessage);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLayerPopulated, EEnvi_EnvironmentLayer, LayerType, int32, InstanceCount, float, CompletionTime);
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
    TMap<EEnvi_EnvironmentLayer, TObjectPtr<AInstancedFoliageActor>> LayerFoliageActors;

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
    void InitializeBiomeData();
    void SetupInstancedComponents();
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius);
    FRotator GetRandomRotation();
    FVector GetRandomScale(float BaseScale, float Variation);
    bool IsValidPlacementLocation(const FVector& Location);
    void PlaceNarrativeDetail(const FVector& Location, const FString& DetailType);
};