#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/ParticleSystemComponent.h"
#include "Components/PointLightComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/Engine.h"
#include "VFXEffectComponent.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None                UMETA(DisplayName = "None"),
    ConsciousnessEnergy UMETA(DisplayName = "Consciousness Energy"),
    MeditationAura      UMETA(DisplayName = "Meditation Aura"),
    ElementalFire       UMETA(DisplayName = "Elemental Fire"),
    WaterFlow           UMETA(DisplayName = "Water Flow"),
    TransformationBurst UMETA(DisplayName = "Transformation Burst"),
    WeatherEffect       UMETA(DisplayName = "Weather Effect"),
    ImpactEffect        UMETA(DisplayName = "Impact Effect")
};

USTRUCT(BlueprintType)
struct FVFX_EffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FLinearColor EffectColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Duration = -1.0f; // -1 for infinite

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoActivate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Scale = 1.0f;

    FVFX_EffectSettings()
    {
        EffectType = EVFX_EffectType::None;
        Intensity = 1.0f;
        EffectColor = FLinearColor::White;
        Duration = -1.0f;
        bAutoActivate = true;
        Scale = 1.0f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_EffectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_EffectComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Effect Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effect")
    FVFX_EffectSettings EffectSettings;

    // Niagara System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effect")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    // Particle System (fallback)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effect")
    TSoftObjectPtr<UParticleSystem> ParticleSystem;

    // Effect Control Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Effect")
    void PlayEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX Effect")
    void StopEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX Effect")
    void SetEffectType(EVFX_EffectType NewType);

    UFUNCTION(BlueprintCallable, Category = "VFX Effect")
    void SetEffectIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Effect")
    void SetEffectColor(FLinearColor NewColor);

    UFUNCTION(BlueprintCallable, Category = "VFX Effect")
    bool IsEffectPlaying() const;

    // Component Management
    UFUNCTION(BlueprintCallable, Category = "VFX Effect")
    void CreateEffectComponents();

    UFUNCTION(BlueprintCallable, Category = "VFX Effect")
    void UpdateEffectParameters();

protected:
    // Component References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TObjectPtr<UNiagaraComponent> NiagaraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TObjectPtr<UParticleSystemComponent> ParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TObjectPtr<UPointLightComponent> LightComponent;

    // Internal State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    bool bIsPlaying = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    float CurrentDuration = 0.0f;

    // Effect Configuration
    void ConfigureConsciousnessEffect();
    void ConfigureMeditationAura();
    void ConfigureElementalFire();
    void ConfigureWaterFlow();
    void ConfigureTransformationBurst();
    void ConfigureWeatherEffect();
    void ConfigureImpactEffect();

    // Component Setup
    void SetupNiagaraComponent();
    void SetupParticleComponent();
    void SetupLightComponent();

    // Parameter Updates
    void UpdateNiagaraParameters();
    void UpdateParticleParameters();
    void UpdateLightParameters();
};