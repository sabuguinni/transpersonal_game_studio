#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "Engine/DirectionalLight.h"
#include "AtmosphericVFXController.generated.h"

UENUM(BlueprintType)
enum class EAtmosphericMood : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Tense       UMETA(DisplayName = "Tense"),
    Threatening UMETA(DisplayName = "Threatening"),
    Panic       UMETA(DisplayName = "Panic")
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Misty       UMETA(DisplayName = "Misty"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Storm       UMETA(DisplayName = "Storm")
};

USTRUCT(BlueprintType)
struct FAtmosphericSettings
{
    GENERATED_BODY()

    // Partículas atmosféricas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DustIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MistIntensity = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PollenIntensity = 0.1f;

    // Efeitos de vento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WindDirection = FVector(1, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindVariation = 0.3f;

    // Efeitos de luz
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LightShaftIntensity = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AtmosphericTint = FLinearColor::White;

    FAtmosphericSettings()
    {
        DustIntensity = 0.3f;
        MistIntensity = 0.2f;
        PollenIntensity = 0.1f;
        WindDirection = FVector(1, 0, 0);
        WindStrength = 0.5f;
        WindVariation = 0.3f;
        LightShaftIntensity = 0.4f;
        AtmosphericTint = FLinearColor::White;
    }
};

/**
 * Controla efeitos atmosféricos que criam tensão e imersão
 * Responde ao estado emocional do jogo e proximidade de predadores
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAtmosphericVFXController : public AActor
{
    GENERATED_BODY()

public:
    AAtmosphericVFXController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes base
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UNiagaraComponent* DustParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UNiagaraComponent* MistParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UNiagaraComponent* PollenParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UNiagaraComponent* LightShafts;

    // Configurações atmosféricas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FAtmosphericSettings CalmSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FAtmosphericSettings TenseSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FAtmosphericSettings ThreateningSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FAtmosphericSettings PanicSettings;

    // Estado actual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EAtmosphericMood CurrentMood = EAtmosphericMood::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EWeatherState CurrentWeather = EWeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float TransitionSpeed = 2.0f;

    // Sistema de resposta a ameaças
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    float ThreatDetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    float ThreatResponseIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    bool bAutoDetectThreats = true;

private:
    // Estado interno
    FAtmosphericSettings CurrentSettings;
    FAtmosphericSettings TargetSettings;
    float TransitionProgress = 0.0f;
    
    // Cache de referências
    UPROPERTY()
    class AVFXSystemManager* VFXManager;

    // Timers para variação
    float WindVariationTimer = 0.0f;
    float DustVariationTimer = 0.0f;

public:
    // Interface pública
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetAtmosphericMood(EAtmosphericMood NewMood, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetWeatherState(EWeatherState NewWeather, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void TriggerThreatResponse(FVector ThreatLocation, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetWindDirection(FVector NewDirection, float Strength = -1.0f);

    // Sistema de detecção automática
    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void UpdateThreatDetection();

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    bool IsPlayerInDanger() const;

    // Getters
    UFUNCTION(BlueprintPure, Category = "Atmospheric State")
    EAtmosphericMood GetCurrentMood() const { return CurrentMood; }

    UFUNCTION(BlueprintPure, Category = "Atmospheric State")
    EWeatherState GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintPure, Category = "Atmospheric State")
    FVector GetCurrentWindDirection() const { return CurrentSettings.WindDirection; }

private:
    void InitializeComponents();
    void UpdateAtmosphericEffects(float DeltaTime);
    void BlendToTargetSettings(float DeltaTime);
    void ApplySettingsToComponents();
    void UpdateWindVariation(float DeltaTime);
    void UpdateDustVariation(float DeltaTime);
    
    FAtmosphericSettings GetSettingsForMood(EAtmosphericMood Mood) const;
    void ModifySettingsForWeather(FAtmosphericSettings& Settings) const;
    
    TArray<AActor*> DetectNearbyThreats() const;
    float CalculateThreatLevel(const TArray<AActor*>& Threats) const;
};