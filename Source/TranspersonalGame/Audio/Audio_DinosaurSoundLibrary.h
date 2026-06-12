#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Audio_DinosaurSoundLibrary.generated.h"

UENUM(BlueprintType)
enum class EAudio_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pteranodon      UMETA(DisplayName = "Pteranodon"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class EAudio_DinosaurSoundType : uint8
{
    Roar            UMETA(DisplayName = "Roar"),
    Growl           UMETA(DisplayName = "Growl"),
    Footstep        UMETA(DisplayName = "Footstep"),
    Breathing       UMETA(DisplayName = "Breathing"),
    Attack          UMETA(DisplayName = "Attack"),
    Pain            UMETA(DisplayName = "Pain"),
    Death           UMETA(DisplayName = "Death"),
    Idle            UMETA(DisplayName = "Idle")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    EAudio_DinosaurSoundType SoundType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<TSoftObjectPtr<USoundCue>> SoundVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float PitchVariation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float Cooldown = 2.0f;

    FAudio_DinosaurSoundData()
    {
        SoundType = EAudio_DinosaurSoundType::Roar;
        BaseVolume = 1.0f;
        PitchVariation = 0.2f;
        MaxDistance = 5000.0f;
        Cooldown = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSpeciesProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    EAudio_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<FAudio_DinosaurSoundData> SoundLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float SpeciesVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float TerritorialRange = 2000.0f;

    FAudio_DinosaurSpeciesProfile()
    {
        Species = EAudio_DinosaurSpecies::TRex;
        SpeciesVolumeMultiplier = 1.0f;
        TerritorialRange = 2000.0f;
    }
};

/**
 * Comprehensive dinosaur sound library and playback system
 * Manages species-specific audio profiles and territorial audio behavior
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_DinosaurSoundLibrary : public UDataAsset
{
    GENERATED_BODY()

public:
    UAudio_DinosaurSoundLibrary();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    bool PlayDinosaurSound(EAudio_DinosaurSpecies Species, EAudio_DinosaurSoundType SoundType, 
                          FVector Location, AActor* SourceActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    UAudioComponent* PlayDinosaurSoundWithComponent(EAudio_DinosaurSpecies Species, 
                                                   EAudio_DinosaurSoundType SoundType,
                                                   USceneComponent* AttachComponent);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetSpeciesVolumeMultiplier(EAudio_DinosaurSpecies Species, float VolumeMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    FAudio_DinosaurSpeciesProfile* GetSpeciesProfile(EAudio_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    TArray<USoundCue*> GetSoundVariations(EAudio_DinosaurSpecies Species, EAudio_DinosaurSoundType SoundType);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<FAudio_DinosaurSpeciesProfile> SpeciesProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float GlobalDinosaurVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    bool bEnableTerritorialAudio = true;

private:
    void InitializeDefaultProfiles();
    FAudio_DinosaurSoundData* GetSoundData(FAudio_DinosaurSpeciesProfile* Profile, EAudio_DinosaurSoundType SoundType);
    USoundCue* SelectRandomSoundVariation(const TArray<TSoftObjectPtr<USoundCue>>& Variations);
    
    // Cooldown tracking
    TMap<FString, float> SoundCooldowns;
    bool IsOnCooldown(const FString& CooldownKey, float CooldownTime);
};