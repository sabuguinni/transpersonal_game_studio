#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "LightingController.generated.h"

UENUM(BlueprintType)
enum class ELight_BiomeType : uint8
{
    Swamp,
    Forest,
    Savanna,
    Desert,
    Mountain
};

USTRUCT(BlueprintType)
struct FLight_BiomeLightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyColor = FLinearColor(0.5f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogHeightFalloff = 0.2f;

    FLight_BiomeLightingSettings()
    {
        SunColor = FLinearColor::White;
        SunIntensity = 3.0f;
        SkyColor = FLinearColor(0.5f, 0.8f, 1.0f);
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingController : public AActor
{
    GENERATED_BODY()

public:
    ALightingController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TMap<ELight_BiomeType, FLight_BiomeLightingSettings> BiomeLightingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current Biome")
    ELight_BiomeType CurrentBiome = ELight_BiomeType::Savanna;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetBiome(ELight_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InitializeLightingActors();

private:
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    AActor* SkyAtmosphere;

    UPROPERTY()
    AActor* SkyLight;

    UPROPERTY()
    AActor* HeightFog;

    void UpdateSunPosition();
    void UpdateAtmosphere();
    void UpdateFog();
    void FindOrCreateLightingActors();
    FLinearColor InterpolateDayNightColor(const FLinearColor& DayColor, const FLinearColor& NightColor, float TimeRatio);
    float CalculateSunAngle() const;
    void InitializeBiomeSettings();
};