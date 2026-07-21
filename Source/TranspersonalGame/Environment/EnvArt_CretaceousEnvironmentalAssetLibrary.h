#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousEnvironmentalAssetLibrary.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_EnvironmentalAssetData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Data")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Data")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Data")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Data")
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Data")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Data")
    TSoftObjectPtr<UNiagaraSystem> ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Data")
    FVector DefaultScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Data")
    bool bHasCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Data")
    bool bCastsShadows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Data")
    int32 LODLevels;

    FEnvArt_EnvironmentalAssetData()
    {
        AssetName = TEXT("DefaultAsset");
        Description = TEXT("Environmental asset for Cretaceous period");
        DefaultScale = FVector::OneVector;
        bHasCollision = true;
        bCastsShadows = true;
        LODLevels = 3;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_FallenLogAssetSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Logs")
    FEnvArt_EnvironmentalAssetData LargeFallenLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Logs")
    FEnvArt_EnvironmentalAssetData MediumFallenLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Logs")
    FEnvArt_EnvironmentalAssetData SmallFallenBranch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Logs")
    FEnvArt_EnvironmentalAssetData MossyFallenLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Logs")
    FEnvArt_EnvironmentalAssetData RottenLogWithHole;

    FEnvArt_FallenLogAssetSet()
    {
        LargeFallenLog.AssetName = TEXT("Large Cretaceous Fallen Log");
        LargeFallenLog.Description = TEXT("Massive fallen conifer log with weathered bark and moss growth");
        LargeFallenLog.DefaultScale = FVector(1.5f, 1.5f, 1.2f);

        MediumFallenLog.AssetName = TEXT("Medium Fallen Log");
        MediumFallenLog.Description = TEXT("Medium-sized fallen tree trunk with natural decay");
        MediumFallenLog.DefaultScale = FVector(1.0f, 1.0f, 1.0f);

        SmallFallenBranch.AssetName = TEXT("Small Fallen Branch");
        SmallFallenBranch.Description = TEXT("Small fallen branch suitable for ground scatter");
        SmallFallenBranch.DefaultScale = FVector(0.6f, 0.6f, 0.6f);

        MossyFallenLog.AssetName = TEXT("Moss-Covered Fallen Log");
        MossyFallenLog.Description = TEXT("Heavily moss-covered fallen log for swamp areas");
        MossyFallenLog.DefaultScale = FVector(1.2f, 1.2f, 1.0f);

        RottenLogWithHole.AssetName = TEXT("Rotten Log with Hollow");
        RottenLogWithHole.Description = TEXT("Decayed log with hollow center for small creature shelter");
        RottenLogWithHole.DefaultScale = FVector(1.1f, 1.1f, 0.9f);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_RockFormationAssetSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formations")
    FEnvArt_EnvironmentalAssetData LargeBoulder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formations")
    FEnvArt_EnvironmentalAssetData SandstoneOutcrop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formations")
    FEnvArt_EnvironmentalAssetData LimestoneFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formations")
    FEnvArt_EnvironmentalAssetData WeatheredRockCluster;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formations")
    FEnvArt_EnvironmentalAssetData FossilBearingRock;

    FEnvArt_RockFormationAssetSet()
    {
        LargeBoulder.AssetName = TEXT("Large Weathered Boulder");
        LargeBoulder.Description = TEXT("Massive boulder with natural weathering patterns");
        LargeBoulder.DefaultScale = FVector(2.0f, 2.0f, 1.8f);

        SandstoneOutcrop.AssetName = TEXT("Sandstone Outcrop");
        SandstoneOutcrop.Description = TEXT("Layered sandstone rock formation");
        SandstoneOutcrop.DefaultScale = FVector(1.5f, 1.5f, 1.2f);

        LimestoneFormation.AssetName = TEXT("Limestone Formation");
        LimestoneFormation.Description = TEXT("Eroded limestone rock with natural cavities");
        LimestoneFormation.DefaultScale = FVector(1.8f, 1.8f, 1.5f);

        WeatheredRockCluster.AssetName = TEXT("Weathered Rock Cluster");
        WeatheredRockCluster.Description = TEXT("Group of smaller weathered rocks");
        WeatheredRockCluster.DefaultScale = FVector(1.2f, 1.2f, 1.0f);

        FossilBearingRock.AssetName = TEXT("Fossil-Bearing Rock");
        FossilBearingRock.Description = TEXT("Rock formation with visible fossil imprints");
        FossilBearingRock.DefaultScale = FVector(1.3f, 1.3f, 1.1f);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_VegetationAssetSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FEnvArt_EnvironmentalAssetData LargeFernCluster;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FEnvArt_EnvironmentalAssetData CycadPlant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FEnvArt_EnvironmentalAssetData HorsetailRush;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FEnvArt_EnvironmentalAssetData AncientTreeStump;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FEnvArt_EnvironmentalAssetData DeadTreeTrunk;

    FEnvArt_VegetationAssetSet()
    {
        LargeFernCluster.AssetName = TEXT("Large Cretaceous Fern Cluster");
        LargeFernCluster.Description = TEXT("Dense cluster of prehistoric ferns");
        LargeFernCluster.DefaultScale = FVector(1.3f, 1.3f, 1.3f);

        CycadPlant.AssetName = TEXT("Cretaceous Cycad Plant");
        CycadPlant.Description = TEXT("Ancient cycad plant with palm-like fronds");
        CycadPlant.DefaultScale = FVector(1.1f, 1.1f, 1.4f);

        HorsetailRush.AssetName = TEXT("Horsetail Rush Cluster");
        HorsetailRush.Description = TEXT("Primitive horsetail plants in marshy areas");
        HorsetailRush.DefaultScale = FVector(0.8f, 0.8f, 1.2f);

        AncientTreeStump.AssetName = TEXT("Ancient Tree Stump");
        AncientTreeStump.Description = TEXT("Weathered stump of ancient conifer tree");
        AncientTreeStump.DefaultScale = FVector(1.4f, 1.4f, 1.0f);

        DeadTreeTrunk.AssetName = TEXT("Standing Dead Tree");
        DeadTreeTrunk.Description = TEXT("Bare trunk of dead tree, habitat for insects");
        DeadTreeTrunk.DefaultScale = FVector(1.0f, 1.0f, 2.5f);
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEnvArt_CretaceousEnvironmentalAssetLibrary : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Collections")
    FEnvArt_FallenLogAssetSet FallenLogs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Collections")
    FEnvArt_RockFormationAssetSet RockFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Collections")
    FEnvArt_VegetationAssetSet Vegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Settings")
    float DefaultSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Settings")
    int32 MaxAssetsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Settings")
    float MinDistanceBetweenAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxLODDistance;

    UFUNCTION(BlueprintCallable, Category = "Asset Library")
    TArray<FEnvArt_EnvironmentalAssetData> GetAssetsForBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Asset Library")
    FEnvArt_EnvironmentalAssetData GetRandomAssetForBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Asset Library")
    TArray<FEnvArt_EnvironmentalAssetData> GetFallenLogAssets() const;

    UFUNCTION(BlueprintCallable, Category = "Asset Library")
    TArray<FEnvArt_EnvironmentalAssetData> GetRockFormationAssets() const;

    UFUNCTION(BlueprintCallable, Category = "Asset Library")
    TArray<FEnvArt_EnvironmentalAssetData> GetVegetationAssets() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void ValidateAssetPaths();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void GenerateAssetReport();

public:
    UEnvArt_CretaceousEnvironmentalAssetLibrary();

protected:
    void InitializeDefaultSettings();
};