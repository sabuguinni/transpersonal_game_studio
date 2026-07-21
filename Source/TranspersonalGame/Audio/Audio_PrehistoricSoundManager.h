#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Audio_PrehistoricSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_AmbientZoneType : uint8
{
    SafeZone,
    DangerZone,
    ForestZone,
    CanyonZone,
    WaterZone
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_AmbientSoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLoop = true;

    FAudio_AmbientSoundData()
    {
        Volume = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 3.0f;
        bLoop = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    EDinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> AlertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MinSoundInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MaxSoundInterval = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float AudioRange = 2000.0f;

    FAudio_DinosaurSoundProfile()
    {
        Species = EDinosaurSpecies::TRex;
        MinSoundInterval = 5.0f;
        MaxSoundInterval = 15.0f;
        AudioRange = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_PrehistoricSoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_PrehistoricSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MasterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicAudioComponent;

    // Ambient zone configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    TMap<EAudio_AmbientZoneType, FAudio_AmbientSoundData> AmbientZoneSounds;

    // Dinosaur audio profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<FAudio_DinosaurSoundProfile> DinosaurSoundProfiles;

    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_AmbientZoneType CurrentAmbientZone;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentDangerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsInCombat;

    // Audio timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Timing")
    float AmbientUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Timing")
    float DinosaurSoundCheckInterval = 2.0f;

    // Timers
    float LastAmbientUpdate;
    float LastDinosaurSoundCheck;

public:
    // Main audio control functions
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetAmbientZone(EAudio_AmbientZoneType NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void UpdateDangerLevel(float NewDangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetCombatState(bool bInCombat);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayDinosaurSound(EDinosaurSpecies Species, const FVector& Location, bool bIsAlert = false);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayEnvironmentalSound(TSoftObjectPtr<USoundCue> SoundCue, const FVector& Location, float Volume = 1.0f);

    // Audio zone detection
    UFUNCTION(BlueprintCallable, Category = "Audio Detection")
    EAudio_AmbientZoneType DetectAmbientZone(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Detection")
    float CalculateDangerLevel(const FVector& PlayerLocation);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Audio Utility")
    void FadeToNewAmbient(EAudio_AmbientZoneType NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio Utility")
    void StopAllAmbientSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio Utility")
    FAudio_DinosaurSoundProfile GetDinosaurSoundProfile(EDinosaurSpecies Species);

protected:
    // Internal audio management
    void UpdateAmbientAudio();
    void UpdateDinosaurAudio();
    void ProcessNearbyDinosaurs();
    void HandleAudioTransitions();

    // Audio loading and caching
    void LoadAudioAssets();
    void CacheAudioComponents();

    // Distance and attenuation calculations
    float CalculateAudioDistance(const FVector& SourceLocation, const FVector& ListenerLocation);
    float CalculateVolumeAttenuation(float Distance, float MaxRange);
};