#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "EnvArt_BiomeAtmosphere.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Floresta    UMETA(DisplayName = "Floresta"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Pantano     UMETA(DisplayName = "Pantano"),
    Montanha    UMETA(DisplayName = "Montanha")
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor LightColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float LightIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator LightRotation = FRotator(-45.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.005f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float TemperatureVariation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float HumidityLevel = 0.5f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_BiomeAtmosphere : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_BiomeAtmosphere();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEnvArt_BiomeType CurrentBiome = EEnvArt_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TMap<EEnvArt_BiomeType, FEnvArt_AtmosphereSettings> BiomeSettings;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<ADirectionalLight> BiomeLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<AExponentialHeightFog> BiomeFog;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetBiome(EEnvArt_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings GetCurrentAtmosphereSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void TransitionToBiome(EEnvArt_BiomeType TargetBiome, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherIntensity(float Intensity);

protected:
    UFUNCTION()
    void InitializeBiomeSettings();

    UFUNCTION()
    void CreateAtmosphereActors();

    UFUNCTION()
    void UpdateLighting();

    UFUNCTION()
    void UpdateFog();

private:
    float CurrentTransitionTime = 0.0f;
    float TargetTransitionTime = 0.0f;
    EEnvArt_BiomeType TransitionTargetBiome;
    FEnvArt_AtmosphereSettings StartSettings;
    FEnvArt_AtmosphereSettings TargetSettings;
    bool bIsTransitioning = false;

    float CurrentTimeOfDay = 12.0f; // 0-24 hours
    float WeatherIntensity = 1.0f;
};