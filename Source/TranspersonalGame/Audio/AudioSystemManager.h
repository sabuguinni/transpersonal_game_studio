#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundType : uint8
{
    Music,
    SFX,
    Ambient,
    Voice,
    UI
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_SoundType SoundType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    FAudio_SoundInstance()
    {
        SoundID = TEXT("");
        SoundType = EAudio_SoundType::SFX;
        Volume = 1.0f;
        Pitch = 1.0f;
        bLooping = false;
        Location = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core audio functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound2D(const FString& SoundID, EAudio_SoundType SoundType, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound3D(const FString& SoundID, const FVector& Location, EAudio_SoundType SoundType, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopSound(const FString& SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCategoryVolume(EAudio_SoundType SoundType, float Volume);

    // Screen shake audio feedback
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerScreenShakeAudio(float Intensity, const FVector& EpicenterLocation);

    // Damage audio feedback
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void PlayDamageAudio(float DamageAmount, const FVector& HitLocation);

    // Footstep system
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void PlayFootstepAudio(const FVector& FootLocation, float CreatureSize, const FString& SurfaceType);

    // Day/night ambient transition
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TransitionToTimeOfDay(float TimeOfDay);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    TMap<FString, TSoftObjectPtr<USoundBase>> SoundLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    TMap<EAudio_SoundType, float> CategoryVolumes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    TMap<FString, UAudioComponent*> ActiveSounds;

    // Audio effect components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    TSoftObjectPtr<USoundBase> ScreenShakeSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    TSoftObjectPtr<USoundBase> DamageSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    TSoftObjectPtr<USoundBase> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    TSoftObjectPtr<USoundBase> DayAmbient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    TSoftObjectPtr<USoundBase> NightAmbient;

private:
    void InitializeSoundLibrary();
    void InitializeCategoryVolumes();
    UAudioComponent* CreateAudioComponent(const FVector& Location = FVector::ZeroVector);
};