#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "BiomeLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_BiomeType : uint8
{
    Swamp,
    Forest,
    Savanna,
    Desert,
    SnowyMountain
};

USTRUCT(BlueprintType)
struct FLight_BiomeLightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-45.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogStartDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHaze = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.3f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_BiomeLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_BiomeLightingManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    ELight_BiomeType CurrentBiome = ELight_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night")
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night")
    float DayDurationMinutes = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night")
    bool bAutoAdvanceTime = true;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetBiome(ELight_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyBiomeLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_BiomeLightingSettings GetBiomeSettings(ELight_BiomeType Biome);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class ASkyLight* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class AVolumetricCloud* VolumetricClouds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TMap<ELight_BiomeType, FLight_BiomeLightingSettings> BiomeSettings;

    void InitializeLightingActors();
    void SetupBiomePresets();
    float CalculateSunAngle(float TimeHours);
    FLinearColor GetSunColorByTime(float TimeHours);
    float GetSunIntensityByTime(float TimeHours);
};

#include "BiomeLightingManager.generated.h"