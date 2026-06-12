#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousPropLibrary.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_PropConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Config")
    FString PropName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Config")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Config")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Config")
    float ScaleMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Config")
    TArray<FString> PropTags;

    FEnvArt_PropConfiguration()
    {
        PropName = TEXT("DefaultProp");
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        ScaleMultiplier = 1.0f;
        PropTags.Empty();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_VegetationCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector ClusterCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float ClusterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 PlantCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bAdaptToTerrain;

    FEnvArt_VegetationCluster()
    {
        SpeciesName = TEXT("Generic_Fern");
        ClusterCenter = FVector::ZeroVector;
        ClusterRadius = 500.0f;
        PlantCount = 5;
        bAdaptToTerrain = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_MineralDeposit
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minerals")
    FString MineralType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minerals")
    FVector DepositLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minerals")
    FLinearColor IndicatorColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minerals")
    float GlowIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minerals")
    float DetectionRadius;

    FEnvArt_MineralDeposit()
    {
        MineralType = TEXT("Iron");
        DepositLocation = FVector::ZeroVector;
        IndicatorColor = FLinearColor(0.8f, 0.2f, 0.1f, 1.0f);
        GlowIntensity = 50.0f;
        DetectionRadius = 200.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousPropLibrary : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousPropLibrary();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Prop configuration arrays
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Props")
    TArray<FEnvArt_PropConfiguration> FallenLogConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Props")
    TArray<FEnvArt_PropConfiguration> WeatheredRockConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Props")
    TArray<FEnvArt_PropConfiguration> AncientBoneConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Props")
    TArray<FEnvArt_PropConfiguration> FossilizedWoodConfigs;

    // Vegetation clusters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Systems")
    TArray<FEnvArt_VegetationCluster> FernClusters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Systems")
    TArray<FEnvArt_VegetationCluster> CycadGroves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Systems")
    TArray<FEnvArt_VegetationCluster> MossPatches;

    // Mineral resource systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Systems")
    TArray<FEnvArt_MineralDeposit> IronDeposits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Systems")
    TArray<FEnvArt_MineralDeposit> CopperVeins;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Systems")
    TArray<FEnvArt_MineralDeposit> CrystalFormations;

    // Environmental storytelling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Storytelling")
    TArray<FEnvArt_PropConfiguration> StorytellingProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Storytelling")
    TArray<FString> NarrativeElements;

public:
    // Prop spawning functions
    UFUNCTION(BlueprintCallable, Category = "Prop Management")
    void SpawnFallenLog(const FVector& Location, const FRotator& Rotation, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Prop Management")
    void SpawnWeatheredRock(const FVector& Location, const FRotator& Rotation, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Prop Management")
    void SpawnAncientBones(const FVector& Location, const FRotator& Rotation, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Prop Management")
    void SpawnFossilizedWood(const FVector& Location, const FRotator& Rotation, float Scale = 1.0f);

    // Vegetation management
    UFUNCTION(BlueprintCallable, Category = "Vegetation Management")
    void CreateFernCluster(const FVector& Center, float Radius, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Vegetation Management")
    void CreateCycadGrove(const FVector& Center, float Radius, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Vegetation Management")
    void CreateMossPatch(const FVector& Center, float Radius);

    // Mineral resource management
    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void PlaceIronDeposit(const FVector& Location, float Intensity = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void PlaceCopperVein(const FVector& Location, float Intensity = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void PlaceCrystalFormation(const FVector& Location, float Intensity = 75.0f);

    // Environmental storytelling
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateStorytellingScene(const FVector& Location, const FString& StoryType);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void AddNarrativeElement(const FString& ElementDescription);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void PopulateAroundGeologicalFormations();

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void EnhanceAtmosphericDetails();

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void SetupEnvironmentalAudio();

    UFUNCTION(BlueprintCallable, Category = "Utilities", CallInEditor = true)
    void ValidatePropPlacements();

private:
    // Internal helper functions
    void InitializeDefaultConfigurations();
    void SetupPropActorProperties(AActor* PropActor, const FEnvArt_PropConfiguration& Config);
    void CreateMineralIndicatorLight(const FEnvArt_MineralDeposit& Deposit);
    FVector GetTerrainAdaptedLocation(const FVector& BaseLocation);
};