#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/NoExportTypes.h"
#include "EnvArt_CretaceousEnvironmentPropLibrary.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_CretaceousPropType : uint8
{
    FallenLog           UMETA(DisplayName = "Fallen Log"),
    WeatheredBoulder    UMETA(DisplayName = "Weathered Boulder"),
    RockFormation       UMETA(DisplayName = "Rock Formation"),
    DeadTree            UMETA(DisplayName = "Dead Tree"),
    FernCluster         UMETA(DisplayName = "Fern Cluster"),
    MushroomGroup       UMETA(DisplayName = "Mushroom Group"),
    BrokenBranch        UMETA(DisplayName = "Broken Branch"),
    StoneCluster        UMETA(DisplayName = "Stone Cluster"),
    CycadStump          UMETA(DisplayName = "Cycad Stump"),
    FossilizedWood      UMETA(DisplayName = "Fossilized Wood")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_CretaceousPropData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    EEnvArt_CretaceousPropType PropType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    TSoftObjectPtr<UStaticMesh> MeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FVector ScaleRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    float SpawnWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    bool bRequiresGroundAlignment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    float MaxSlopeAngle;

    FEnvArt_CretaceousPropData()
    {
        PropType = EEnvArt_CretaceousPropType::FallenLog;
        ScaleRange = FVector(0.8f, 1.2f, 1.0f);
        SpawnWeight = 1.0f;
        bRequiresGroundAlignment = true;
        MaxSlopeAngle = 45.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnvArt_CretaceousEnvironmentPropLibrary : public UObject
{
    GENERATED_BODY()

public:
    UEnvArt_CretaceousEnvironmentPropLibrary();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Library")
    TArray<FEnvArt_CretaceousPropData> PropDatabase;

    UFUNCTION(BlueprintCallable, Category = "Environment Props")
    FEnvArt_CretaceousPropData GetRandomPropByType(EEnvArt_CretaceousPropType PropType);

    UFUNCTION(BlueprintCallable, Category = "Environment Props")
    FEnvArt_CretaceousPropData GetWeightedRandomProp();

    UFUNCTION(BlueprintCallable, Category = "Environment Props")
    TArray<FEnvArt_CretaceousPropData> GetPropsForBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Environment Props")
    bool ValidateSpawnLocation(const FVector& Location, const FEnvArt_CretaceousPropData& PropData);

    UFUNCTION(BlueprintCallable, Category = "Environment Props")
    FVector CalculateAlignedRotation(const FVector& SurfaceNormal, const FEnvArt_CretaceousPropData& PropData);

protected:
    void InitializePropDatabase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TMap<FString, TArray<EEnvArt_CretaceousPropType>> BiomePropTypes;
};