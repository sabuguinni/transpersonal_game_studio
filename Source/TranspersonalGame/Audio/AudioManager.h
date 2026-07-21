#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "AudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bIsActive;

    FAudio_SoundZone()
    {
        Location = FVector::ZeroVector;
        Radius = 1000.0f;
        Volume = 1.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSounds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> RoarSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float RoarVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float FootstepVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MaxHearingDistance;

    FAudio_DinosaurSounds()
    {
        RoarVolume = 1.0f;
        FootstepVolume = 0.8f;
        MaxHearingDistance = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MasterAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    TArray<FAudio_SoundZone> AmbientZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TMap<EDinosaurSpecies, FAudio_DinosaurSounds> DinosaurSoundMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float VoiceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bUseMetaSounds;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDinosaurSound(EDinosaurSpecies Species, const FString& SoundType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateAmbientZone(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetMasterVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlaySurvivalNarration(const FString& NarrationKey);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StopAllAmbientSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void InitializeAudioZones();

private:
    void SetupDinosaurSounds();
    void CreateAmbientZones();
    FAudio_SoundZone* GetClosestAmbientZone(const FVector& Location);
    
    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    float LastZoneUpdateTime;
    FAudio_SoundZone* CurrentActiveZone;
};