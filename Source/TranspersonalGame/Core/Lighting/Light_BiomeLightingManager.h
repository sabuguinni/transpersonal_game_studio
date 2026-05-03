#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Light_BiomeLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_BiomeType : uint8
{
    Swamp       UMETA(DisplayName = "Pântano"),
    Forest      UMETA(DisplayName = "Floresta"),
    Savanna     UMETA(DisplayName = "Savana"),
    Desert      UMETA(DisplayName = "Deserto"),
    Mountain    UMETA(DisplayName = "Montanha Nevada")
};

USTRUCT(BlueprintType)
struct FLight_BiomeLightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunTemperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientIntensity = 1500.0f;

    FLight_BiomeLightingSettings()
    {
        SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        SunIntensity = 3.5f;
        SunTemperature = 5500.0f;
        SkyLightColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        SkyLightIntensity = 1.2f;
        FogColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        AmbientColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);
        AmbientIntensity = 1500.0f;
    }
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    TMap<ELight_BiomeType, FLight_BiomeLightingSettings> BiomeLightingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDuration = 1200.0f; // 20 minutos

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeOfDay = 0.5f; // 0.0 = meia-noite, 0.5 = meio-dia

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UDirectionalLightComponent* SunLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkyLightComponent* SkyLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UExponentialHeightFogComponent* FogComponent;

    UFUNCTION(BlueprintCallable, Category = "Biome Lighting")
    void ApplyBiomeLighting(ELight_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Biome Lighting")
    FLight_BiomeLightingSettings GetBiomeLightingSettings(ELight_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Lighting")
    void SetBiomeLightingSettings(ELight_BiomeType BiomeType, const FLight_BiomeLightingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InitializeLightingComponents();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSkyLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateFogSettings();

private:
    void SetupDefaultBiomeSettings();
    FLinearColor InterpolateLightColor(const FLinearColor& DayColor, const FLinearColor& NightColor, float TimeOfDay);
    float InterpolateLightIntensity(float DayIntensity, float NightIntensity, float TimeOfDay);
};