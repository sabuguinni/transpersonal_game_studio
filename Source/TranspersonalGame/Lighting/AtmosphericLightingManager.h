#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "AtmosphericLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Pantano     UMETA(DisplayName = "Pantano"), 
    Floresta    UMETA(DisplayName = "Floresta"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Montanha    UMETA(DisplayName = "Montanha")
};

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_BiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientLightColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VolumetricScatteringIntensity = 1.0f;

    FLight_BiomeSettings()
    {
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        AmbientLightColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
        VolumetricScatteringIntensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FLight_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-30.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    FLight_TimeSettings()
    {
        SunRotation = FRotator(-30.0f, 0.0f, 0.0f);
        SunIntensity = 3.0f;
        SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
        SkyLightIntensity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAtmosphericLightingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAtmosphericLightingManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetBiomeLighting(ELight_BiomeType BiomeType, const FVector& Location, float Radius = 10000.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateGlobalLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SpawnVolumetricLights(const FVector& Location, ELight_BiomeType BiomeType, int32 Count = 3);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ConfigureLumenSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting", CallInEditor)
    void InitializeAtmosphericSystem();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TMap<ELight_BiomeType, FLight_BiomeSettings> BiomeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TMap<ELight_TimeOfDay, FLight_TimeSettings> TimeSettings;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Noon;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<ADirectionalLight> MainSunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TArray<TObjectPtr<AExponentialHeightFog>> BiomeFogActors;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TArray<TObjectPtr<APointLight>> VolumetricLights;

private:
    void InitializeBiomeSettings();
    void InitializeTimeSettings();
    void FindOrCreateMainSunLight();
    void UpdateSunLighting();
    void UpdateFogSettings();
    FLight_BiomeSettings GetBiomeSettingsForLocation(const FVector& Location);
};