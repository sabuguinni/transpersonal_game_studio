#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousSwampSystem.generated.h"

USTRUCT(BlueprintType)
struct FEnvArt_SwampVegetationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Vegetation")
    float CypressTreeDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Vegetation")
    float MossHangingChance = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Vegetation")
    FVector TreeScaleRange = FVector(0.8f, 1.4f, 2.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Vegetation")
    float FernDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Vegetation")
    float AquaticPlantCoverage = 0.4f;
};

USTRUCT(BlueprintType)
struct FEnvArt_SwampWaterConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Features")
    float WaterLevel = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Features")
    float WaterTurbidity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Features")
    FLinearColor WaterTint = FLinearColor(0.3f, 0.4f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Features")
    float MistIntensity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Features")
    float CausticStrength = 0.4f;
};

USTRUCT(BlueprintType)
struct FEnvArt_SwampAtmosphereConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float HumidityLevel = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float LightFiltering = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AmbientSoundVolume = 0.6f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousSwampSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousSwampSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SwampBoundsMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* AtmosphericLight;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp System")
    FEnvArt_SwampVegetationConfig VegetationConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp System")
    FEnvArt_SwampWaterConfig WaterConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp System")
    FEnvArt_SwampAtmosphereConfig AtmosphereConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp System")
    float SwampRadius = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp System")
    int32 MaxVegetationCount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp System")
    int32 MaxWaterFeatures = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp System")
    bool bEnableAtmosphericEffects = true;

    // Runtime state
    UPROPERTY(BlueprintReadOnly, Category = "Swamp State")
    TArray<AActor*> SpawnedVegetation;

    UPROPERTY(BlueprintReadOnly, Category = "Swamp State")
    TArray<AActor*> SpawnedWaterFeatures;

    UPROPERTY(BlueprintReadOnly, Category = "Swamp State")
    TArray<AActor*> SpawnedAtmosphericEffects;

    UPROPERTY(BlueprintReadOnly, Category = "Swamp State")
    bool bIsInitialized = false;

    // Core functions
    UFUNCTION(BlueprintCallable, Category = "Swamp System")
    void InitializeSwampBiome();

    UFUNCTION(BlueprintCallable, Category = "Swamp System")
    void SpawnSwampVegetation();

    UFUNCTION(BlueprintCallable, Category = "Swamp System")
    void CreateWaterFeatures();

    UFUNCTION(BlueprintCallable, Category = "Swamp System")
    void SetupAtmosphericEffects();

    UFUNCTION(BlueprintCallable, Category = "Swamp System")
    void UpdateSwampAtmosphere(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Swamp System")
    void CleanupSwampElements();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Swamp System")
    FVector GetRandomSwampPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Swamp System")
    bool IsValidSwampLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Swamp System")
    float GetSwampDepthAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Swamp System")
    FLinearColor GetSwampLightingColor(float TimeOfDay) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Swamp System")
    void RegenerateSwampBiome();

private:
    void SpawnCypressTrees();
    void SpawnMossAndFerns();
    void CreateSwampWater();
    void SetupVolumetricFog();
    void ConfigureSwampLighting();

    float SwampGenerationTime = 0.0f;
    bool bRegenerationRequested = false;
};