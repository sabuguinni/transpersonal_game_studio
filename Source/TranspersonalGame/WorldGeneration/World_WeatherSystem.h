#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_WeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear_Hot       UMETA(DisplayName = "Clear Hot"),
    Foggy_Humid     UMETA(DisplayName = "Foggy Humid"),
    Rainy_Dense     UMETA(DisplayName = "Rainy Dense"),
    Sandstorm_Dry   UMETA(DisplayName = "Sandstorm Dry"),
    Snowy_Windy     UMETA(DisplayName = "Snowy Windy")
};

USTRUCT(BlueprintType)
struct FWorld_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType WeatherType = EWorld_WeatherType::Clear_Hot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection = FVector(1, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Humidity = 0.5f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_WeatherSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_WeatherSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UExponentialHeightFogComponent* FogComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    FWorld_WeatherSettings WeatherSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    float EffectRadius = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    bool bDynamicWeather = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    float WeatherTransitionSpeed = 1.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherType(EWorld_WeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void ApplyWeatherSettings();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void CreateWindEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    FWorld_WeatherSettings GetCurrentWeatherSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    bool IsPlayerInWeatherZone(class APawn* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(EWorld_WeatherType TargetWeather, float TransitionTime);

private:
    void InitializeWeatherForBiome();
    void UpdateDynamicWeather(float DeltaTime);
    FWorld_WeatherSettings GetDefaultSettingsForBiome(EBiomeType Biome) const;

    UPROPERTY()
    TArray<class APointLight*> WindMarkers;

    float CurrentTransitionTime = 0.0f;
    float TargetTransitionTime = 0.0f;
    bool bIsTransitioning = false;
    FWorld_WeatherSettings TargetWeatherSettings;
    FWorld_WeatherSettings StartWeatherSettings;
};