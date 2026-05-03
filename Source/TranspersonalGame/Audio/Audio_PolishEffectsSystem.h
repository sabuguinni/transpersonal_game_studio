#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_PolishEffectsSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_EffectType : uint8
{
    None = 0,
    ScreenShake,
    DamageFlash,
    FootstepDust,
    AudioFeedback
};

USTRUCT(BlueprintType)
struct FAudio_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    EAudio_EffectType EffectType = EAudio_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FVector Location = FVector::ZeroVector;

    FAudio_EffectData()
    {
        EffectType = EAudio_EffectType::None;
        Intensity = 1.0f;
        Duration = 1.0f;
        Location = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_PolishEffectsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_PolishEffectsSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Screen shake effects
    UFUNCTION(BlueprintCallable, Category = "Audio Polish Effects")
    void TriggerScreenShake(float Intensity, float Duration, FVector SourceLocation);

    // Damage flash effect
    UFUNCTION(BlueprintCallable, Category = "Audio Polish Effects")
    void TriggerDamageFlash(float Intensity, float Duration);

    // Footstep dust particles
    UFUNCTION(BlueprintCallable, Category = "Audio Polish Effects")
    void TriggerFootstepDust(FVector Location, float Scale = 1.0f);

    // Audio feedback for actions
    UFUNCTION(BlueprintCallable, Category = "Audio Polish Effects")
    void TriggerAudioFeedback(const FString& EventName, float Volume = 1.0f);

    // Day/night cycle control
    UFUNCTION(BlueprintCallable, Category = "Audio Polish Effects")
    void UpdateDayNightCycle(float TimeOfDay);

    // Proximity-based screen shake (for T-Rex footsteps)
    UFUNCTION(BlueprintCallable, Category = "Audio Polish Effects")
    void CheckProximityShake();

protected:
    // Screen shake properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float MaxShakeIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeRadius = 1000.0f;

    // Damage flash properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FLinearColor FlashColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashOpacity = 0.5f;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* AudioFeedbackComponent;

    // Particle system for footsteps
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    class UParticleSystem* FootstepDustEffect;

    // Day/night cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayNightSpeed = 1.0f;

    // Internal state
    bool bDamageFlashActive = false;
    float DamageFlashTimer = 0.0f;
    float DamageFlashDuration = 0.0f;

    // References
    UPROPERTY()
    class APlayerController* PlayerController;

    UPROPERTY()
    class ACharacter* PlayerCharacter;

    UPROPERTY()
    class ADirectionalLight* SunLight;

private:
    // Helper functions
    void InitializeReferences();
    void UpdateDamageFlash(float DeltaTime);
    void FindSunLight();
    float CalculateDistanceToPlayer(FVector Location);
    void ApplyScreenShakeToCamera(float Intensity);
};