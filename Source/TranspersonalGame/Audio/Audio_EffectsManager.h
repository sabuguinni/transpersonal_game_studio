#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "Audio_EffectsManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_EffectType : uint8
{
    Footstep,
    Roar,
    Ambient,
    Impact,
    Environmental,
    UI
};

USTRUCT(BlueprintType)
struct FAudio_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effect")
    EAudio_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effect")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effect")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effect")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effect")
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effect")
    bool bIs3D = true;

    FAudio_EffectData()
    {
        EffectType = EAudio_EffectType::Ambient;
        Volume = 1.0f;
        Pitch = 1.0f;
        MaxDistance = 5000.0f;
        bIs3D = true;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    TMap<EAudio_EffectType, FAudio_EffectData> EffectLibrary;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float EffectsVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxSimultaneousEffects = 32;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void PlayEffect(EAudio_EffectType EffectType, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void PlayEffectAtActor(EAudio_EffectType EffectType, AActor* TargetActor, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void StopEffectType(EAudio_EffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMasterVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetEffectsVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    bool IsEffectPlaying(EAudio_EffectType EffectType) const;

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    int32 GetActiveEffectCount() const;

protected:
    UFUNCTION()
    void CleanupFinishedComponents();

    UFUNCTION()
    void InitializeEffectLibrary();

    UFUNCTION()
    UAudioComponent* CreateAudioComponent(const FAudio_EffectData& EffectData, FVector Location);
};