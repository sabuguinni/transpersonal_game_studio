#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_EffectsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ScreenShakeSettings
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

    FAudio_ScreenShakeSettings()
    {
        Intensity = 1.0f;
        Duration = 0.5f;
        Frequency = 10.0f;
        Range = 2000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DamageFlashSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FLinearColor FlashColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashIntensity = 0.7f;

    FAudio_DamageFlashSettings()
    {
        FlashColor = FLinearColor::Red;
        FlashDuration = 0.3f;
        FlashIntensity = 0.7f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_EffectsManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_EffectsManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    FAudio_ScreenShakeSettings TRexFootstepShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    FAudio_ScreenShakeSettings RaptorFootstepShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FAudio_DamageFlashSettings DamageFlash;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* TRexFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* RaptorFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* DamageSound;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TriggerTRexFootstep(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TriggerRaptorFootstep(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TriggerDamageFlash(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TriggerScreenShake(FAudio_ScreenShakeSettings ShakeSettings, FVector EpicenterLocation);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void PlaySoundAtLocation(USoundCue* SoundCue, FVector Location, float VolumeMultiplier = 1.0f);

private:
    void ApplyScreenShake(float Intensity, float Duration, float Frequency, FVector EpicenterLocation, float Range);
    void ApplyDamageFlash(FAudio_DamageFlashSettings FlashSettings);
    
    float GetDistanceToPlayer(FVector Location);
    APlayerController* GetPlayerController();
};