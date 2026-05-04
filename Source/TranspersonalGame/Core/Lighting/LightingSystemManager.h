#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Subsystems/WorldSubsystem.h"
#include "LightingSystemManager.generated.h"

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

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Fog         UMETA(DisplayName = "Fog"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_BiomeLightingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientColor = FLinearColor(0.2f, 0.3f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float Temperature = 5500.0f;

    FLight_BiomeLightingData()
    {
        AmbientColor = FLinearColor(0.2f, 0.3f, 0.4f, 1.0f);
        SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
        SunIntensity = 3.0f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        FogDensity = 0.01f;
        Temperature = 5500.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Referências para actores de iluminação
    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    class ASkyLight* SkyLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    class ASkyAtmosphere* Atmosphere;

    // Sistema de tempo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentTimeOfDay = 12.0f; // 0-24 horas

    UPROPERTY(BlueprintReadOnly, Category = "Time System")
    ELight_TimeOfDay CurrentTimePhase;

    // Sistema meteorológico
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    // Dados de iluminação por bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    TMap<FString, FLight_BiomeLightingData> BiomeLightingData;

public:
    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeather(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLightingForBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_BiomeLightingData GetBiomeLightingData(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InitializeLightingReferences();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateDynamicLighting();

    UFUNCTION(BlueprintPure, Category = "Time")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Time")
    ELight_TimeOfDay GetCurrentTimePhase() const { return CurrentTimePhase; }

private:
    void UpdateTimeOfDayPhase();
    void UpdateSunPosition();
    void UpdateSunIntensity();
    void UpdateSkyLightColor();
    void UpdateFogSettings();
    void UpdateAtmosphereSettings();
    void InitializeBiomeLightingData();
    
    FLinearColor CalculateSunColor(float TimeOfDay) const;
    float CalculateSunIntensity(float TimeOfDay) const;
    FRotator CalculateSunRotation(float TimeOfDay) const;
};