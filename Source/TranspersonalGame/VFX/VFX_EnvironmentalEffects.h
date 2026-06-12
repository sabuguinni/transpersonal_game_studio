#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_EnvironmentalEffects.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature;

    FVFX_WeatherState()
    {
        RainIntensity = 0.0f;
        FogDensity = 0.0f;
        WindStrength = 0.5f;
        WindDirection = FVector(1, 0, 0);
        Temperature = 25.0f;
    }
};

UENUM(BlueprintType)
enum class EVFX_EnvironmentalType : uint8
{
    Volcanic_Ash        UMETA(DisplayName = "Volcanic Ash"),
    Pollen_Particles    UMETA(DisplayName = "Pollen"),
    Insect_Swarm        UMETA(DisplayName = "Insect Swarm"),
    Dust_Devil          UMETA(DisplayName = "Dust Devil"),
    Mist_Rising         UMETA(DisplayName = "Rising Mist"),
    Leaves_Falling      UMETA(DisplayName = "Falling Leaves"),
    Seeds_Floating      UMETA(DisplayName = "Floating Seeds")
};

UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_EnvironmentalEffects : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_EnvironmentalEffects();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    TMap<EVFX_EnvironmentalType, UNiagaraSystem*> EnvironmentalSystems;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather")
    FVFX_WeatherState CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float EffectRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxEnvironmentalEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Effects")
    TArray<UNiagaraComponent*> ActiveEnvironmentalEffects;

public:
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void UpdateWeatherState(const FVFX_WeatherState& NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SpawnEnvironmentalEffect(EVFX_EnvironmentalType EffectType, FVector Location, float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void StartRainEffect(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void StopRainEffect();

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void StartFogEffect(float Density);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void StopFogEffect();

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SpawnVolcanicAsh(FVector VolcanoLocation, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SpawnPollenCloud(FVector TreeLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SpawnInsectSwarm(FVector Location, float SwarmSize);

private:
    void InitializeEnvironmentalSystems();
    void UpdateWeatherEffects(float DeltaTime);
    void CleanupExpiredEffects();
    bool ShouldSpawnEffect(EVFX_EnvironmentalType EffectType);
    FVector GetRandomLocationInRadius(FVector Center, float Radius);

    // Weather effect components
    UPROPERTY()
    UNiagaraComponent* RainEffect;

    UPROPERTY()
    UNiagaraComponent* FogEffect;

    UPROPERTY()
    UNiagaraComponent* WindEffect;

    float WeatherUpdateTimer;
    float EffectSpawnTimer;
};