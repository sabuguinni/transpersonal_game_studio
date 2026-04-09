#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "FoliageType.h"
#include "InstancedFoliageActor.h"
#include "FoliageManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPrehistoricFoliageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    TSoftObjectPtr<UStaticMesh> FoliageMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    TSoftObjectPtr<UMaterialInterface> FoliageMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    FString FoliageType; // "Fern", "ConifersTree", "CycadPalm", "Moss", "Vine"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float MinScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float MaxScale = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    bool bCastShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    bool bNaniteEnabled = true;

    FPrehistoricFoliageData()
    {
        FoliageType = TEXT("Fern");
        MinScale = 0.8f;
        MaxScale = 1.2f;
        Density = 1.0f;
        bCastShadows = true;
        bNaniteEnabled = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AFoliageManager : public AActor
{
    GENERATED_BODY()

public:
    AFoliageManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Foliage")
    TArray<FPrehistoricFoliageData> PrehistoricVegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Settings")
    float GlobalFoliageDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Settings")
    float CullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Settings")
    bool bEnableWind = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Settings")
    float WindStrength = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootSceneComponent;

public:
    UFUNCTION(BlueprintCallable, Category = "Foliage Management")
    void PopulateFoliageInRadius(FVector Center, float Radius, const FString& BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Foliage Management")
    void ClearFoliageInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Foliage Management")
    void UpdateFoliageDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Foliage Management")
    void SetBiomeFoliage(const FString& BiomeType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Foliage Events")
    void OnFoliagePopulated(const FString& BiomeType, int32 InstanceCount);

private:
    UPROPERTY()
    TObjectPtr<AInstancedFoliageActor> FoliageActor;

    void InitializeFoliageTypes();
    void CreateFoliageInstance(const FPrehistoricFoliageData& FoliageData, FVector Location, FRotator Rotation, FVector Scale);
    FPrehistoricFoliageData GetFoliageForBiome(const FString& BiomeType) const;
};