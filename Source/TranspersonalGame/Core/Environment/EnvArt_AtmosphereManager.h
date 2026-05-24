#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "EnvArt_AtmosphereManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeAtmosphere
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FRotator SunRotation = FRotator(-30.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogStartDistance = 0.0f;

    FEnvArt_BiomeAtmosphere()
    {
        // Default constructor
    }
};

UENUM(BlueprintType)
enum class EEnvArt_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Floresta    UMETA(DisplayName = "Floresta"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Pantano     UMETA(DisplayName = "Pantano"),
    Montanha    UMETA(DisplayName = "Montanha")
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AEnvArt_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UDirectionalLightComponent* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkyLightComponent* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UExponentialHeightFogComponent* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TMap<EEnvArt_BiomeType, FEnvArt_BiomeAtmosphere> BiomeAtmospheres;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDuration = 1200.0f; // seconds for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherIntensity = 0.0f; // 0-1 for weather effects

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetBiomeAtmosphere(EEnvArt_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FLinearColor GetCurrentSunColor() const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    float GetCurrentSunIntensity() const;

    UFUNCTION(CallInEditor, Category = "Development")
    void InitializeBiomePresets();

private:
    void UpdateSunPosition();
    void UpdateFogSettings();
    void InterpolateBiomeTransition(EEnvArt_BiomeType FromBiome, EEnvArt_BiomeType ToBiome, float Alpha);
    
    EEnvArt_BiomeType CurrentBiome = EEnvArt_BiomeType::Savana;
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
};