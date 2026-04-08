#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn (05:00-07:00)"),
    Morning     UMETA(DisplayName = "Morning (07:00-11:00)"),
    Midday      UMETA(DisplayName = "Midday (11:00-14:00)"),
    Afternoon   UMETA(DisplayName = "Afternoon (14:00-17:00)"),
    Dusk        UMETA(DisplayName = "Dusk (17:00-19:00)"),
    Night       UMETA(DisplayName = "Night (19:00-05:00)")
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear Sky"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm")
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe - Bright, welcoming"),
    Cautious    UMETA(DisplayName = "Cautious - Subtle tension"),
    Dangerous   UMETA(DisplayName = "Dangerous - Clear threat"),
    Hunted      UMETA(DisplayName = "Hunted - Maximum fear")
};

USTRUCT(BlueprintType)
struct FLightingMood
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Mood")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Mood")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Mood")
    FLinearColor SkyColor = FLinearColor(0.2f, 0.4f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Mood")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Mood")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Mood")
    float ShadowIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Mood")
    float ContrastBoost = 1.0f;
};

/**
 * Sistema central de iluminação dinâmica para o jogo jurássico
 * Controla ciclo dia/noite, clima, e atmosfera emocional
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULightingSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    ULightingSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ===== SISTEMA DE TEMPO =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeScale = 1.0f; // 1.0 = tempo real, 60.0 = 1 minuto real = 1 hora jogo

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentTimeOfDay = 12.0f; // 0-24 horas

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoAdvanceTime = true;

    // ===== SISTEMA CLIMÁTICO =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    EWeatherState CurrentWeather = EWeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float WeatherChangeInterval = 300.0f; // segundos

    // ===== SISTEMA DE AMEAÇA =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat System")
    EThreatLevel CurrentThreatLevel = EThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat System")
    float ThreatTransitionSpeed = 2.0f;

    // ===== REFERÊNCIAS DOS COMPONENTES =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Components")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Components")
    class ASkyLight* SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Components")
    class ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Components")
    class AVolumetricCloud* CloudActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Components")
    class AExponentialHeightFog* FogActor;

    // ===== CONFIGURAÇÕES DE MOOD =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Moods")
    TMap<ETimeOfDay, FLightingMood> TimeOfDayMoods;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Moods")
    TMap<EThreatLevel, FLightingMood> ThreatMoods;

    // ===== FUNÇÕES PÚBLICAS =====
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetWeatherState(EWeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetThreatLevel(EThreatLevel NewThreat);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    ETimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    FLightingMood GetCurrentMood() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void ForceUpdateLighting();

private:
    // ===== FUNÇÕES INTERNAS =====
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeatherSystem(float DeltaTime);
    void UpdateLightingMood();
    void ApplyLightingMood(const FLightingMood& Mood);
    void UpdateSunPosition();
    void UpdateSkyLight();
    void UpdateAtmosphere();
    void UpdateFog();
    void UpdateClouds();
    
    FLightingMood BlendMoods(const FLightingMood& MoodA, const FLightingMood& MoodB, float Alpha);
    
    // ===== VARIÁVEIS INTERNAS =====
    float WeatherTimer = 0.0f;
    FLightingMood CurrentMood;
    FLightingMood TargetMood;
    bool bMoodTransitioning = false;
    float MoodTransitionAlpha = 0.0f;
    
    void InitializeDefaultMoods();
    void FindLightingComponents();
};