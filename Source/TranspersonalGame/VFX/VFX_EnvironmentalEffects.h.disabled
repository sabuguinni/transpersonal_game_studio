#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "VFX_EnvironmentalEffects.generated.h"

UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear = 0,
    Rain,
    Storm,
    Fog,
    Dust,
    Snow
};

UENUM(BlueprintType) 
enum class EVFX_BiomeType : uint8
{
    Savana = 0,
    Forest,
    Swamp,
    Desert,
    Mountain
};

USTRUCT(BlueprintType)
struct FVFX_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    float ParticleCount = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    float WindStrength = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    FLinearColor ParticleColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FVFX_BiomeEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    EVFX_BiomeType BiomeType = EVFX_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    float AmbientParticleChance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 0.3f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    TArray<FString> ParticleEffectNames;
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_EnvironmentalEffects : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_EnvironmentalEffects();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Weather System
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetWeatherType(EVFX_WeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void UpdateWeatherIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetWindDirection(FVector NewWindDirection);

    // Biome System
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetCurrentBiome(EVFX_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SpawnBiomeAmbientEffect(FVector Location);

    // Dynamic Effects
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SpawnDustCloud(FVector Location, float Size = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SpawnWaterSplash(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SpawnFireEffect(FVector Location, float Duration = 10.0f);

    // Atmospheric Effects
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void UpdateFogSettings(FLinearColor Color, float Density);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void CreateVolumetricLightRays(FVector SunDirection, float Intensity);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    EVFX_WeatherType CurrentWeatherType = EVFX_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    FVFX_WeatherSettings WeatherSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    EVFX_BiomeType CurrentBiome = EVFX_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TArray<FVFX_BiomeEffectData> BiomeEffectData;

    // Niagara Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* RainEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* DustEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* FogEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* AmbientEffect;

    // Effect Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Settings")
    float EffectUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Settings")
    float MaxEffectDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Settings")
    int32 MaxActiveEffects = 50;

private:
    float LastEffectUpdate = 0.0f;
    TArray<class UNiagaraComponent*> ActiveEffects;

    void UpdateWeatherEffects();
    void UpdateBiomeEffects();
    void CleanupExpiredEffects();
    void InitializeBiomeData();
};