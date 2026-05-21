#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_AudioType : uint8
{
    Ambient,
    Footsteps,
    Damage,
    Environmental,
    Dinosaur
};

USTRUCT(BlueprintType)
struct FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_AudioType AudioType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShouldLoop = false;

    FAudio_SoundEntry()
    {
        AudioType = EAudio_AudioType::Ambient;
        Volume = 1.0f;
        Pitch = 1.0f;
        bShouldLoop = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AudioSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AudioSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio System")
    class UAudioComponent* PrimaryAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Library")
    TArray<FAudio_SoundEntry> AudioLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float EffectsVolume = 1.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayAudioByType(EAudio_AudioType AudioType, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAudioByType(EAudio_AudioType AudioType);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayFootstepAudio(FVector FootstepLocation, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDamageAudio(FVector DamageLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayAmbientAudio(EAudio_AudioType EnvironmentType);

private:
    UPROPERTY()
    TMap<EAudio_AudioType, UAudioComponent*> ActiveAudioComponents;

    void InitializeAudioLibrary();
    UAudioComponent* CreateAudioComponent(const FAudio_SoundEntry& SoundEntry);
};