#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/PointLight.h"
#include "Camera/CameraShakeBase.h"
#include "AudioEffectsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ScreenShakeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Range = 2000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DamageFlashData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FLinearColor FlashColor = FLinearColor(1.0f, 0.1f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashIntensity = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashDuration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FadeSpeed = 10.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FootstepEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float ParticleScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float SoundVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float ShakeIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float EffectRange = 1000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioEffectsManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioEffectsManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPointLightComponent* DamageFlashLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbientAudioComponent;

    // Screen Shake System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    FAudio_ScreenShakeData DefaultShakeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    FAudio_ScreenShakeData DinosaurFootstepShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    FAudio_ScreenShakeData TRexProximityShake;

    // Damage Flash System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FAudio_DamageFlashData DamageFlashSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Damage Flash")
    bool bIsDamageFlashing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Damage Flash")
    float CurrentFlashTimer = 0.0f;

    // Footstep Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Effects")
    FAudio_FootstepEffectData PlayerFootstepData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Effects")
    FAudio_FootstepEffectData DinosaurFootstepData;

    // Audio Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* DinosaurFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* PlayerFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* DamageSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* TRexRoarSound;

public:
    // Screen Shake Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerScreenShake(const FAudio_ScreenShakeData& ShakeData, const FVector& EpicenterLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerDinosaurFootstepShake(const FVector& FootstepLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerTRexProximityShake(const FVector& TRexLocation, float Distance);

    // Damage Flash Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerDamageFlash();

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void StopDamageFlash();

    // Footstep Effect Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerFootstepEffect(const FVector& FootstepLocation, bool bIsPlayer = true);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerDinosaurFootstep(const FVector& FootstepLocation, float DinosaurSize = 1.0f);

    // Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void PlaySoundAtLocation(USoundBase* Sound, const FVector& Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void SetAmbientAudio(USoundBase* AmbientSound, float Volume = 1.0f);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    float GetDistanceToPlayer(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    bool IsPlayerInRange(const FVector& Location, float Range) const;

private:
    void UpdateDamageFlash(float DeltaTime);
    void ApplyScreenShake(const FAudio_ScreenShakeData& ShakeData, const FVector& EpicenterLocation);
    class APlayerController* GetPlayerController() const;
};