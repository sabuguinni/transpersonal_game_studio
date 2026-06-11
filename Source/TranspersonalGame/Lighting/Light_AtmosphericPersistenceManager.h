#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "SharedTypes.h"
#include "Light_AtmosphericPersistenceManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float DirectionalLightIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FColor DirectionalLightColor = FColor(255, 240, 200, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricScattering = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FColor VolumetricAlbedo = FColor(200, 180, 150, 255);

    FLight_AtmosphericSettings()
    {
        DirectionalLightIntensity = 5.0f;
        DirectionalLightColor = FColor(255, 240, 200, 255);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        bVolumetricFog = true;
        VolumetricScattering = 0.2f;
        VolumetricAlbedo = FColor(200, 180, 150, 255);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericPersistenceManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericPersistenceManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FLight_AtmosphericSettings CretaceousSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    bool bAutoSaveOnChange = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    float SaveInterval = 30.0f;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void EnsureSkyAtmosphereExists();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ConfigureDirectionalLight();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetupVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void CreateLandmarkLights();

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void SaveAtmosphericState();

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void ValidateAtmosphericIntegrity();

private:
    UPROPERTY()
    ADirectionalLight* MainDirectionalLight;

    UPROPERTY()
    ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY()
    AExponentialHeightFog* HeightFogActor;

    UPROPERTY()
    TArray<AActor*> LandmarkLights;

    float LastSaveTime;
    bool bAtmosphereValidated;

    void FindAtmosphericActors();
    void RemoveLegacyAtmosphericFog();
    bool ValidateAtmosphericComponents();
};