#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "Audio_EffectManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_EffectType : uint8
{
    TRexProximity,
    DamageFlash,
    FootstepDust,
    AmbientNature
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeInTime = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeOutTime = 0.5f;

    FAudio_EffectData()
    {
        EffectType = EAudio_EffectType::AmbientNature;
        Volume = 1.0f;
        Pitch = 1.0f;
        FadeInTime = 0.1f;
        FadeOutTime = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_EffectManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_EffectManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    TArray<FAudio_EffectData> EffectDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    float TRexDetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    float ScreenShakeIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effects")
    float DamageFlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effects")
    FLinearColor DamageFlashColor = FLinearColor::Red;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void PlayEffect(EAudio_EffectType EffectType, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void StopEffect(EAudio_EffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Proximity")
    void CheckTRexProximity();

    UFUNCTION(BlueprintCallable, Category = "Damage Effects")
    void TriggerDamageFlash();

    UFUNCTION(BlueprintCallable, Category = "Footstep Effects")
    void CreateFootstepDust(FVector Location, float Intensity = 1.0f);

private:
    TMap<EAudio_EffectType, UAudioComponent*> ActiveEffects;
    
    UPROPERTY()
    class APlayerController* PlayerController;

    UPROPERTY()
    class APawn* PlayerPawn;

    bool bDamageFlashActive = false;
    float DamageFlashTimer = 0.0f;
};