#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataAsset.h"
#include "EnvArt_CretaceousPropAssetLibrary.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_CretaceousPropType : uint8
{
    FallenLog       UMETA(DisplayName = "Fallen Log"),
    Boulder         UMETA(DisplayName = "Boulder"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    DeadTree        UMETA(DisplayName = "Dead Tree"),
    Fern            UMETA(DisplayName = "Fern"),
    Mushroom        UMETA(DisplayName = "Mushroom"),
    BrokenBranch    UMETA(DisplayName = "Broken Branch"),
    StoneCluster    UMETA(DisplayName = "Stone Cluster")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_CretaceousPropData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop Data")
    EEnvArt_CretaceousPropType PropType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop Data")
    TSoftObjectPtr<UStaticMesh> PropMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop Data")
    TSoftObjectPtr<UMaterialInterface> PropMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop Data")
    FVector DefaultScale;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop Data")
    float SpawnWeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop Data")
    FString PropDescription;

    FEnvArt_CretaceousPropData()
    {
        PropType = EEnvArt_CretaceousPropType::FallenLog;
        DefaultScale = FVector(1.0f, 1.0f, 1.0f);
        SpawnWeight = 1.0f;
        PropDescription = TEXT("Cretaceous Environmental Prop");
    }
};

/**
 * Asset library for Cretaceous period environmental props
 * Contains references to fallen logs, boulders, rock formations, and other period-appropriate assets
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnvArt_CretaceousPropAssetLibrary : public UDataAsset
{
    GENERATED_BODY()

public:
    UEnvArt_CretaceousPropAssetLibrary();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fallen Logs")
    TArray<FEnvArt_CretaceousPropData> FallenLogAssets;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock Formations")
    TArray<FEnvArt_CretaceousPropData> BoulderAssets;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock Formations")
    TArray<FEnvArt_CretaceousPropData> RockFormationAssets;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dead Vegetation")
    TArray<FEnvArt_CretaceousPropData> DeadTreeAssets;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Small Props")
    TArray<FEnvArt_CretaceousPropData> SmallPropAssets;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Prop Library")
    FEnvArt_CretaceousPropData GetRandomPropOfType(EEnvArt_CretaceousPropType PropType) const;

    UFUNCTION(BlueprintCallable, Category = "Prop Library")
    TArray<FEnvArt_CretaceousPropData> GetAllPropsOfType(EEnvArt_CretaceousPropType PropType) const;

    UFUNCTION(BlueprintCallable, Category = "Prop Library")
    int32 GetPropCountOfType(EEnvArt_CretaceousPropType PropType) const;

protected:
    void InitializeDefaultAssets();
};