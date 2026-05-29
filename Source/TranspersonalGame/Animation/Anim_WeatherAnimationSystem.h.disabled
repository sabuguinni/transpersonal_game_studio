#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "Anim_WeatherAnimationSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_WeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear Sky"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm    UMETA(DisplayName = "Thunderstorm"),
    LightSnow       UMETA(DisplayName = "Light Snow"),
    HeavySnow       UMETA(DisplayName = "Heavy Snow"),
    Fog             UMETA(DisplayName = "Fog"),
    Sandstorm       UMETA(DisplayName = "Sandstorm"),
    Wind            UMETA(DisplayName = "Strong Wind")
};

UENUM(BlueprintType)
enum class EAnim_WeatherIntensity : uint8
{
    None        UMETA(DisplayName = "None"),
    Light       UMETA(DisplayName = "Light"),
    Moderate    UMETA(DisplayName = "Moderate"),
    Heavy       UMETA(DisplayName = "Heavy"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_WeatherParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EAnim_WeatherType WeatherType = EAnim_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Duration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "-40.0", ClampMax = "50.0"))
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Humidity = 50.0f;

    FAnim_WeatherParameters()
    {
        WeatherType = EAnim_WeatherType::Clear;
        Intensity = 0.5f;
        Duration = 30.0f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        WindStrength = 1.0f;
        Temperature = 20.0f;
        Humidity = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_WeatherEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<UParticleSystem> ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<USoundCue> WeatherSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VisibilityReduction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float FogDensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float LightingIntensityMultiplier = 1.0f;

    FAnim_WeatherEffectData()
    {
        VisibilityReduction = 0.0f;
        FogColor = FLinearColor::White;
        FogDensity = 0.0f;
        LightingIntensityMultiplier = 1.0f;
    }
};

/**
 * Sistema de animação climática que controla efeitos visuais e sonoros do tempo
 * Integra com o sistema ambiental para criar uma experiência imersiva
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_WeatherAnimationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_WeatherAnimationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CONFIGURAÇÃO DE CLIMA ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
    FAnim_WeatherParameters CurrentWeatherParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
    bool bAutoWeatherTransition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration", meta = (ClampMin = "10.0", ClampMax = "600.0"))
    float WeatherTransitionDuration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float WeatherUpdateFrequency = 1.0f;

    // === EFEITOS VISUAIS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects")
    TMap<EAnim_WeatherType, FAnim_WeatherEffectData> WeatherEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual Effects")
    TObjectPtr<UParticleSystemComponent> ActiveWeatherParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual Effects")
    TObjectPtr<UAudioComponent> WeatherAudioComponent;

    // === CONTROLE DE ANIMAÇÃO ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Control")
    bool bIsWeatherAnimationActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Control")
    bool bAffectEnvironmentalAnimation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Control", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float WeatherIntensityMultiplier = 1.0f;

    // === MÉTODOS PÚBLICOS ===

    UFUNCTION(BlueprintCallable, Category = "Weather Animation")
    void SetWeatherType(EAnim_WeatherType NewWeatherType, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Animation")
    void SetWeatherParameters(const FAnim_WeatherParameters& NewParameters);

    UFUNCTION(BlueprintCallable, Category = "Weather Animation")
    void SetWeatherIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Weather Animation")
    void TransitionToWeather(EAnim_WeatherType TargetWeather, float TransitionDuration);

    UFUNCTION(BlueprintCallable, Category = "Weather Animation")
    void StartWeatherEffect(EAnim_WeatherType WeatherType, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Weather Animation")
    void StopWeatherEffect();

    UFUNCTION(BlueprintCallable, Category = "Weather Animation")
    void EnableAutoWeatherTransition(bool bEnable);

    // === CONSULTA DE ESTADO ===

    UFUNCTION(BlueprintPure, Category = "Weather Animation")
    EAnim_WeatherType GetCurrentWeatherType() const;

    UFUNCTION(BlueprintPure, Category = "Weather Animation")
    float GetCurrentWeatherIntensity() const;

    UFUNCTION(BlueprintPure, Category = "Weather Animation")
    FAnim_WeatherParameters GetCurrentWeatherParameters() const;

    UFUNCTION(BlueprintPure, Category = "Weather Animation")
    bool IsWeatherTransitioning() const;

    UFUNCTION(BlueprintPure, Category = "Weather Animation")
    float GetWeatherTransitionProgress() const;

    // === CONFIGURAÇÃO DE EFEITOS ===

    UFUNCTION(BlueprintCallable, Category = "Weather Effects")
    void SetWeatherEffect(EAnim_WeatherType WeatherType, const FAnim_WeatherEffectData& EffectData);

    UFUNCTION(BlueprintCallable, Category = "Weather Effects")
    void LoadDefaultWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather Effects")
    void UpdateVisualEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather Effects")
    void UpdateAudioEffects();

protected:
    // === MÉTODOS INTERNOS ===

    void InitializeWeatherSystem();
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateWeatherEffects(float DeltaTime);
    void ApplyWeatherToEnvironment();
    void CreateWeatherParticles(EAnim_WeatherType WeatherType);
    void UpdateWeatherAudio(EAnim_WeatherType WeatherType, float Intensity);
    void CleanupWeatherEffects();

    // === TRANSIÇÕES ===

    void StartWeatherTransition(EAnim_WeatherType FromWeather, EAnim_WeatherType ToWeather, float Duration);
    void UpdateTransitionEffects(float DeltaTime);
    void CompleteWeatherTransition();

    // === EFEITOS ESPECÍFICOS ===

    void CreateRainEffect(float Intensity);
    void CreateSnowEffect(float Intensity);
    void CreateFogEffect(float Intensity);
    void CreateSandstormEffect(float Intensity);
    void CreateThunderstormEffect(float Intensity);

private:
    // === ESTADO INTERNO ===

    UPROPERTY()
    EAnim_WeatherType PreviousWeatherType;

    UPROPERTY()
    EAnim_WeatherType TargetWeatherType;

    UPROPERTY()
    bool bIsTransitioning;

    UPROPERTY()
    float TransitionProgress;

    UPROPERTY()
    float TransitionDuration;

    UPROPERTY()
    float WeatherTimer;

    UPROPERTY()
    float LastWeatherUpdate;

    // === CACHE DE COMPONENTES ===

    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> AffectedMeshComponents;

    UPROPERTY()
    TObjectPtr<class UAnim_EnvironmentalAnimationController> EnvironmentalController;

    // === TIMERS ===

    FTimerHandle WeatherTransitionTimerHandle;
    FTimerHandle WeatherUpdateTimerHandle;
    FTimerHandle ThunderTimerHandle;
};