#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "DinosaurAudioComponent.generated.h"

UENUM(BlueprintType)
enum class EAudio_DinosaurSoundType : uint8
{
    Idle,
    Alert,
    Aggressive,
    Pain,
    Death,
    Footstep,
    Breathing
};

UENUM(BlueprintType)
enum class EAudio_DinosaurSpecies : uint8
{
    TRex,
    Velociraptor,
    Triceratops,
    Brachiosaurus,
    Ankylosaurus,
    Parasaurolophus
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSoundSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> AlertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> AggressiveSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> PainSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> DeathSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float BasePitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MaxAudibleDistance = 5000.0f;

    FAudio_DinosaurSoundSet()
    {
        BaseVolume = 1.0f;
        BasePitch = 1.0f;
        MaxAudibleDistance = 5000.0f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurAudioComponent : public UAudioComponent
{
    GENERATED_BODY()

public:
    UDinosaurAudioComponent();

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurSound(EAudio_DinosaurSoundType SoundType, float VolumeMultiplier = 1.0f, float PitchVariation = 0.1f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetDinosaurSpecies(EAudio_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayFootstepSound(float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void StartBreathingLoop();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void StopBreathingLoop();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetThreatLevel(float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    bool IsPlayingDinosaurSound() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetAudioDistance(float Distance);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    EAudio_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TMap<EAudio_DinosaurSpecies, FAudio_DinosaurSoundSet> SpeciesSoundSets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float CurrentThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float SoundCooldownTime = 2.0f;

    UPROPERTY()
    UAudioComponent* BreathingComponent;

    UPROPERTY()
    float LastSoundTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    bool bUseDistanceAttenuation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MinPitchVariation = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MaxPitchVariation = 1.2f;

private:
    void InitializeSpeciesSoundSets();
    USoundBase* GetSoundForType(EAudio_DinosaurSoundType SoundType);
    float CalculateVolumeFromThreat(float BaseThreatLevel);
    float CalculatePitchFromThreat(float BaseThreatLevel);
    bool CanPlaySound() const;
};