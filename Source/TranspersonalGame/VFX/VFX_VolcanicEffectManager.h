#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "VFX_VolcanicEffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_VolcanicIntensity : uint8
{
    Dormant     UMETA(DisplayName = "Dormant"),
    LowActivity UMETA(DisplayName = "Low Activity"),
    Moderate    UMETA(DisplayName = "Moderate"),
    HighActivity UMETA(DisplayName = "High Activity"),
    Erupting    UMETA(DisplayName = "Erupting")
};

USTRUCT(BlueprintType)
struct FVFX_VolcanicSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    float AshDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    float WindStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    float EffectRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    bool bEnableDistanceAttenuation = true;
};

/**
 * VFX Manager for prehistoric volcanic effects including ash clouds, dust storms, and atmospheric particles
 * Provides realistic Cretaceous period volcanic activity simulation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_VolcanicEffectManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_VolcanicEffectManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Niagara Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components", meta = (AllowPrivateAccess = "true"))
    class UNiagaraComponent* AshCloudComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components", meta = (AllowPrivateAccess = "true"))
    class UNiagaraComponent* VolcanicDustComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components", meta = (AllowPrivateAccess = "true"))
    class UNiagaraComponent* GroundParticlesComponent;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* VolcanicRumbleAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* WindAshAudio;

public:
    // Volcanic Effect Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Control")
    EVFX_VolcanicIntensity CurrentIntensity = EVFX_VolcanicIntensity::LowActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Control")
    FVFX_VolcanicSettings VolcanicSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Control")
    float IntensityChangeRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Control")
    bool bAutoIntensityVariation = true;

    // Niagara System References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* AshCloudSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* VolcanicDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* GroundParticleSystem;

    // Audio Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundBase* VolcanicRumbleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundBase* WindAshSound;

    // Public Methods
    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void SetVolcanicIntensity(EVFX_VolcanicIntensity NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void TriggerVolcanicEvent(float Duration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void UpdateWindParameters(FVector NewWindDirection, float NewWindStrength);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void SetAshDensity(float NewDensity);

    UFUNCTION(BlueprintPure, Category = "Volcanic Effects")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void StartVolcanicActivity();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void StopVolcanicActivity();

private:
    // Internal state
    float CurrentAshDensity = 1.0f;
    float TargetAshDensity = 1.0f;
    float IntensityTimer = 0.0f;
    bool bVolcanicEventActive = false;
    float VolcanicEventTimer = 0.0f;
    float VolcanicEventDuration = 0.0f;

    // Internal methods
    void UpdateIntensityParameters();
    void UpdateNiagaraParameters();
    void UpdateAudioParameters();
    void HandleAutoIntensityVariation(float DeltaTime);
    float CalculateDistanceAttenuation() const;
};