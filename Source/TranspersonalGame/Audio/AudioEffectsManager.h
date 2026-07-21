#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "AudioEffectsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float MaxDistance = 2000.0f;

    FAudio_ScreenShakeConfig()
    {
        Intensity = 1.0f;
        Duration = 0.5f;
        Frequency = 10.0f;
        MaxDistance = 2000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DamageFlashConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FLinearColor FlashColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FadeOutTime = 0.2f;

    FAudio_DamageFlashConfig()
    {
        FlashColor = FLinearColor::Red;
        FlashIntensity = 0.8f;
        FlashDuration = 0.3f;
        FadeOutTime = 0.2f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioEffectsManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioEffectsManager();

    // Screen shake effects
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerProximityShake(FVector SourceLocation, const FAudio_ScreenShakeConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerFootstepShake(FVector FootstepLocation, float DinosaurMass = 5000.0f);

    // Damage feedback
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerDamageFlash(const FAudio_DamageFlashConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void PlayDamageAudio(USoundBase* DamageSound, float VolumeMultiplier = 1.0f);

    // Footstep particle effects
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void SpawnFootstepDust(FVector Location, FVector Normal, float ParticleScale = 1.0f);

    // Day/Night cycle audio
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void UpdateAmbientAudio(float TimeOfDay, FString BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void SetDayNightCycleSpeed(float CycleSpeedMultiplier);

protected:
    // Audio components for ambient sounds
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* DayAmbientComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* NightAmbientComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* WeatherAmbientComponent;

    // Sound assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundBase* FootstepHeavySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundBase* DamageImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundBase* HeartbeatIntenseSound;

    // Particle systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    class UParticleSystem* FootstepDustParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    class UParticleSystem* BloodSplatterParticles;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FAudio_ScreenShakeConfig DefaultShakeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FAudio_DamageFlashConfig DefaultFlashConfig;

private:
    // Internal state
    float CurrentTimeOfDay = 12.0f; // Noon
    float DayNightSpeed = 1.0f;
    FString CurrentBiome = "Savana";

    // Helper functions
    APlayerController* GetLocalPlayerController() const;
    UWorld* GetWorld() const override;
    void InitializeAudioComponents();
    float CalculateDistanceAttenuation(FVector SourceLocation, FVector ListenerLocation, float MaxDistance) const;
};