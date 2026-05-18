#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "AudioEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_EffectType : uint8
{
    ScreenShake     UMETA(DisplayName = "Screen Shake"),
    DamageFlash     UMETA(DisplayName = "Damage Flash"),
    FootstepDust    UMETA(DisplayName = "Footstep Dust"),
    ProximityRumble UMETA(DisplayName = "Proximity Rumble")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    float Range = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    bool bEnabled = true;

    FAudio_EffectSettings()
    {
        Intensity = 1.0f;
        Duration = 2.0f;
        Range = 1000.0f;
        bEnabled = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioEffectsManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioEffectsManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* ProximityAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* DamageAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particle Components")
    class UParticleSystemComponent* FootstepDustComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    FAudio_EffectSettings TRexShakeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    FAudio_EffectSettings DamageFlashSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    FAudio_EffectSettings FootstepDustSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    TSubclassOf<UCameraShakeBase> TRexCameraShakeClass;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerTRexProximityEffect(FVector TRexLocation, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerDamageFlashEffect(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerFootstepDustEffect(FVector FootstepLocation, float CreatureSize);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void UpdateDayNightCycle(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void SetEffectEnabled(EAudio_EffectType EffectType, bool bEnabled);

private:
    float CurrentTimeOfDay = 12.0f; // Start at noon
    bool bDamageFlashActive = false;
    float DamageFlashTimer = 0.0f;

    void UpdateScreenShakeIntensity(float Distance);
    void PlayProximityWarningSound(float Distance);
    void CreateFootstepDustParticles(FVector Location, float Size);
    void RotateSunLight(float TimeOfDay);
};