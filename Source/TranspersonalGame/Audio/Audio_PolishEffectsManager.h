#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_PolishEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_EffectIntensity : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ScreenShakeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    TSubclassOf<UCameraShakeBase> ShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float MaxDistance = 2000.0f;

    FAudio_ScreenShakeData()
    {
        ShakeClass = nullptr;
        Intensity = 1.0f;
        Duration = 0.5f;
        MaxDistance = 2000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DamageFlashData
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

    FAudio_DamageFlashData()
    {
        FlashColor = FLinearColor::Red;
        FlashIntensity = 0.8f;
        FlashDuration = 0.3f;
        FadeOutTime = 0.2f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AAudio_PolishEffectsManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_PolishEffectsManager();

protected:
    virtual void BeginPlay() override;

    // Screen Shake System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake", meta = (AllowPrivateAccess = "true"))
    TMap<EAudio_EffectIntensity, FAudio_ScreenShakeData> ScreenShakeSettings;

    // Damage Flash System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash", meta = (AllowPrivateAccess = "true"))
    TMap<EAudio_EffectIntensity, FAudio_DamageFlashData> DamageFlashSettings;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* FootstepAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* ImpactAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* EnvironmentalAudioComponent;

    // Timers
    FTimerHandle DamageFlashTimer;
    FTimerHandle ScreenShakeTimer;

public:
    // Screen Shake Functions
    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    void TriggerScreenShake(EAudio_EffectIntensity Intensity, FVector SourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    void TriggerDinosaurFootstepShake(FVector DinosaurLocation, float DinosaurMass);

    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    void TriggerImpactShake(FVector ImpactLocation, float ImpactForce);

    // Damage Flash Functions
    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    void TriggerDamageFlash(EAudio_EffectIntensity Intensity, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    void TriggerHealthWarningFlash();

    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    void TriggerCriticalDamageFlash();

    // Audio Feedback Functions
    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    void PlayFootstepAudio(FVector Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    void PlayImpactAudio(FVector Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    void PlayEnvironmentalAudio(FVector Location, float Range);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    float CalculateDistanceIntensity(FVector SourceLocation, float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    void SetEffectIntensityMultiplier(float Multiplier);

private:
    void InitializeScreenShakeSettings();
    void InitializeDamageFlashSettings();
    void StopDamageFlash();
    void ApplyDamageFlashToViewport(const FAudio_DamageFlashData& FlashData);

    float EffectIntensityMultiplier = 1.0f;
    bool bDamageFlashActive = false;
};