#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"
#include "SharedTypes.h"
#include "EnvArt_FallenLogAsset.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_LogDecayState : uint8
{
    Fresh           UMETA(DisplayName = "Fresh"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Mossy           UMETA(DisplayName = "Mossy"),
    Decomposing     UMETA(DisplayName = "Decomposing"),
    Fossilized      UMETA(DisplayName = "Fossilized")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LogMaterialData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* BaseMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UTexture2D* BarkTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UTexture2D* MossTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    float MossIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    float WeatheringAmount;

    FEnvArt_LogMaterialData()
    {
        BaseMaterial = nullptr;
        BarkTexture = nullptr;
        MossTexture = nullptr;
        MossIntensity = 0.5f;
        WeatheringAmount = 0.3f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_FallenLogAsset : public AStaticMeshActor
{
    GENERATED_BODY()

public:
    AEnvArt_FallenLogAsset();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    EEnvArt_LogDecayState DecayState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    float LogLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    float LogDiameter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    bool bHasHollowSection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    bool bCanHidePlayer;

    // Material System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    FEnvArt_LogMaterialData MaterialData;

    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    UMaterialInstanceDynamic* DynamicMaterial;

    // Environmental Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bAdaptToTerrain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float TerrainAdaptionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bSpawnMossParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MossParticleIntensity;

    // Interaction System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanBeClimbed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanProvideWood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    int32 WoodResourceAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanSpawnInsects;

    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "Log Management")
    void SetDecayState(EEnvArt_LogDecayState NewDecayState);

    UFUNCTION(BlueprintCallable, Category = "Log Management")
    void UpdateMaterialProperties();

    UFUNCTION(BlueprintCallable, Category = "Log Management")
    void AdaptToTerrainSurface();

    UFUNCTION(BlueprintCallable, Category = "Log Management")
    void SpawnMossEffects();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetBiomeSpecificProperties(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool CanPlayerHide() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    int32 HarvestWood();

    UFUNCTION(BlueprintPure, Category = "Log Properties")
    float GetLogVolume() const;

    UFUNCTION(BlueprintPure, Category = "Log Properties")
    bool IsFullyDecomposed() const;

private:
    void InitializeMaterials();
    void UpdateDecayVisuals();
    void HandleTerrainAdaption();
    void SpawnEnvironmentalEffects();

    // Internal State
    float CurrentWeatheringLevel;
    float MossGrowthTimer;
    bool bMaterialsInitialized;
    FVector OriginalScale;
};