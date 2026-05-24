#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "DinosaurAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_DinosaurSpecies : uint8
{
    TRex,
    Velociraptor,
    Triceratops,
    Brachiosaurus,
    Ankylosaurus,
    Parasaurolophus,
    Pachycephalosaurus,
    Protoceratops,
    Tsintaosaurus
};

UENUM(BlueprintType)
enum class EAudio_DinosaurSoundType : uint8
{
    Roar,
    Growl,
    Footstep,
    Breathing,
    Attack,
    Death,
    Idle,
    Warning
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_DinosaurSoundType SoundType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BasePitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIs3D = true;

    FAudio_DinosaurSoundData()
    {
        Species = EAudio_DinosaurSpecies::TRex;
        SoundType = EAudio_DinosaurSoundType::Roar;
        BaseVolume = 1.0f;
        BasePitch = 1.0f;
        MaxDistance = 5000.0f;
        bIs3D = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_DinosaurAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_DinosaurAudioManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<FAudio_DinosaurSoundData> DinosaurSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float DinosaurVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FootstepVolumeBySize = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float RoarEchoDelay = 0.5f;

public:
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurSound(EAudio_DinosaurSpecies Species, EAudio_DinosaurSoundType SoundType, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayFootstepBySpecies(EAudio_DinosaurSpecies Species, FVector FootstepLocation, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayRoarWithEcho(EAudio_DinosaurSpecies Species, FVector RoarLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void StopAllDinosaurSounds();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetDinosaurVolumeMultiplier(float NewMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    float GetSpeciesSizeMultiplier(EAudio_DinosaurSpecies Species);

private:
    UPROPERTY()
    TArray<UAudioComponent*> ActiveDinosaurAudioComponents;

    void InitializeDinosaurSounds();
    FAudio_DinosaurSoundData* FindSoundData(EAudio_DinosaurSpecies Species, EAudio_DinosaurSoundType SoundType);
    UAudioComponent* CreateDinosaurAudioComponent(const FAudio_DinosaurSoundData& SoundData, FVector Location);
};