#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousEnvironmentalPropManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_CretaceousPropData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FString PropName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    TSoftObjectPtr<UStaticMesh> MeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FVector MinScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FVector MaxScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    float SpawnProbability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    TArray<EBiomeType> AllowedBiomes;

    FEnvArt_CretaceousPropData()
    {
        PropName = TEXT("DefaultProp");
        MinScale = FVector(0.8f, 0.8f, 0.8f);
        MaxScale = FVector(1.2f, 1.2f, 1.2f);
        SpawnProbability = 0.5f;
        AllowedBiomes.Add(EBiomeType::Forest);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_PropSpawnSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    int32 MaxPropsPerArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float MinDistanceBetweenProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float TerrainConformance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    bool bUseRandomRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    bool bSnapToGround;

    FEnvArt_PropSpawnSettings()
    {
        MaxPropsPerArea = 50;
        MinDistanceBetweenProps = 500.0f;
        TerrainConformance = 0.8f;
        bUseRandomRotation = true;
        bSnapToGround = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_CretaceousEnvironmentalPropManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_CretaceousEnvironmentalPropManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Library")
    TArray<FEnvArt_CretaceousPropData> FallenLogProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Library")
    TArray<FEnvArt_CretaceousPropData> RockFormationProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Library")
    TArray<FEnvArt_CretaceousPropData> AncientTreeStumpProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Library")
    TArray<FEnvArt_CretaceousPropData> CretaceousFernProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    FEnvArt_PropSpawnSettings SpawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance;

    UFUNCTION(BlueprintCallable, Category = "Prop Management")
    void SpawnPropsInArea(FVector CenterLocation, float Radius, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Prop Management")
    void ClearPropsInArea(FVector CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Prop Management")
    void UpdatePropLOD(float PlayerDistance);

    UFUNCTION(BlueprintCallable, Category = "Prop Management")
    TArray<AActor*> GetActiveProps() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void GenerateDefaultPropLibrary();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TArray<AActor*> SpawnedProps;

    UPROPERTY()
    TMap<EBiomeType, TArray<FEnvArt_CretaceousPropData*>> BiomePropMap;

    void InitializePropLibrary();
    AActor* SpawnSingleProp(const FEnvArt_CretaceousPropData& PropData, FVector Location, FRotator Rotation);
    bool IsValidSpawnLocation(FVector Location, float MinDistance);
    FVector GetGroundLocation(FVector Location);
    void OptimizePropPerformance();
};