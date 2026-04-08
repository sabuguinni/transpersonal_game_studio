#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAtmosphericMood : uint8
{
    Peaceful,      // Manhã serena, luz dourada filtrada
    Tense,         // Meio-dia com sombras densas
    Threatening,   // Tarde com luz vermelha, nuvens pesadas
    Dangerous,     // Noite/tempestade, visibilidade limitada
    Dawn,          // Amanhecer místico
    Dusk           // Entardecer ominoso
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear,
    PartlyCloudy,
    Overcast,
    LightRain,
    HeavyRain,
    Storm,
    Fog
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAtmosphericSettings
{
    GENERATED_BODY()

    // Configurações do Sol
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunElevation = 45.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunAzimuth = 180.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SunColor = FLinearColor::White;
    
    // Configurações da Atmosfera
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AtmosphereHaze = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroundAlbedo = 0.3f;
    
    // Configurações de Nuvens
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudCoverage = 0.4f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudOpacity = 0.8f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CloudWindVelocity = FVector(50.0f, 0.0f, 0.0f);
    
    // Configurações de Neblina
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogHeightFalloff = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogInscatteringColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
    
    // Configurações de Pós-processamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExposureBias = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Contrast = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Saturation = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ColorGrading = FLinearColor::White;
};

/**
 * Sistema central de gerenciamento de iluminação e atmosfera
 * Controla ciclo dia/noite, clima, e transições emocionais
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes principais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UDirectionalLightComponent* SunLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class USkyLightComponent* SkyLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class USkyAtmosphereComponent* SkyAtmosphere;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UVolumetricCloudComponent* VolumetricClouds;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UExponentialHeightFogComponent* HeightFog;

public:
    // Configurações do sistema
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float TimeOfDay = 12.0f; // 0-24 horas
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayDurationMinutes = 20.0f; // Duração de um dia em minutos reais
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bAutoAdvanceTime = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeatherState = EWeatherState::PartlyCloudy;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EAtmosphericMood CurrentMood = EAtmosphericMood::Peaceful;
    
    // Presets atmosféricos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    TMap<EAtmosphericMood, FAtmosphericSettings> MoodPresets;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    TMap<EWeatherState, FAtmosphericSettings> WeatherPresets;

    // Configurações de transição
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float MoodTransitionSpeed = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float WeatherTransitionSpeed = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    UCurveFloat* TimeOfDayCurve;

    // Referência ao Post Process Volume
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Processing")
    APostProcessVolume* MainPostProcessVolume;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(float NewTimeOfDay);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetAtmosphericMood(EAtmosphericMood NewMood, bool bInstantTransition = false);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetWeatherState(EWeatherState NewWeatherState, bool bInstantTransition = false);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void TriggerLightningStrike(FVector Location);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetDynamicShadowIntensity(float Intensity);
    
    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    float GetCurrentLightLevel() const;
    
    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    bool IsNightTime() const;
    
    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    FLinearColor GetCurrentAmbientColor() const;

protected:
    // Funções internas
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateSunPosition();
    void UpdateAtmosphere();
    void UpdateClouds();
    void UpdateFog();
    void UpdatePostProcessing();
    
    void InterpolateMoodSettings(float DeltaTime);
    void InterpolateWeatherSettings(float DeltaTime);
    
    void InitializePresets();
    void SetupLumenSettings();
    void SetupVirtualShadowMaps();
    
    FAtmosphericSettings GetCurrentTargetSettings() const;
    FAtmosphericSettings BlendSettings(const FAtmosphericSettings& A, const FAtmosphericSettings& B, float Alpha) const;
    
    // Variáveis de estado interno
    FAtmosphericSettings CurrentSettings;
    FAtmosphericSettings TargetSettings;
    
    float MoodTransitionProgress = 1.0f;
    float WeatherTransitionProgress = 1.0f;
    
    // Cache para otimização
    float LastUpdateTime = 0.0f;
    float UpdateFrequency = 0.1f; // Atualizar a cada 100ms
};