#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "../Core/SharedTypes.h"
#include "EnvironmentManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> EnvironmentProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AmbientColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity;

    FEnvArt_BiomeData()
    {
        BiomeType = EBiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        AmbientColor = FLinearColor::White;
        FogDensity = 0.02f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyAtmosphereMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogHeightFalloff;

    FEnvArt_LightingSettings()
    {
        SunIntensity = 3.0f;
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        SunRotation = FRotator(-15.0f, 45.0f, 0.0f);
        SkyAtmosphereMultiplier = 1.0f;
        FogHeightFalloff = 0.2f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (AllowPrivateAccess = "true"))
    TArray<FEnvArt_BiomeData> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    FEnvArt_LightingSettings LightingSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class USceneComponent* RootSceneComponent;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SpawnEnvironmentProps();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetTimeOfDay(float TimeHours);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyBiomeAtmosphere(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
    void EditorSpawnTestProps();

protected:
    UFUNCTION()
    void SpawnPropsForBiome(const FEnvArt_BiomeData& BiomeData);

    UFUNCTION()
    void UpdateSunPosition(float TimeHours);

    UFUNCTION()
    void UpdateFogSettings(EBiomeType BiomeType);

private:
    UPROPERTY()
    TArray<AStaticMeshActor*> SpawnedProps;

    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    AExponentialHeightFog* HeightFog;

    UPROPERTY()
    ASkyAtmosphere* SkyAtmosphere;

    float CurrentTimeOfDay;
    bool bInitialized;
};