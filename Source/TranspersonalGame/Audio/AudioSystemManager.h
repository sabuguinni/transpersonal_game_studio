#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeZone : uint8
{
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseJungle     UMETA(DisplayName = "Dense Jungle"),
    RiverDelta      UMETA(DisplayName = "River Delta"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    VolcanicField   UMETA(DisplayName = "Volcanic Field"),
    CaveSystem      UMETA(DisplayName = "Cave System")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Threatened  UMETA(DisplayName = "Threatened"),
    Combat      UMETA(DisplayName = "Combat"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeZone BiomeZone = EAudio_BiomeZone::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TSoftObjectPtr<USoundBase> DayAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TSoftObjectPtr<USoundBase> NightAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float CrossfadeDuration = 3.0f;
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FName DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> AlertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepScreenShakeRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepShakeIntensity = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioSystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Biome ambient management
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetActiveBiomeZone(EAudio_BiomeZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetDayNightBlend(float NormalizedTimeOfDay);

    // Danger music system
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    // Dinosaur proximity audio
    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void OnDinosaurFootstep(FVector FootstepLocation, FName DinosaurSpecies, float DinosaurMass);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void OnDinosaurRoar(FVector RoarLocation, FName DinosaurSpecies, float Intensity);

    // Survival audio events
    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayCraftingSound(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayFireIgniteSound(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayPlayerDamageSound(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayPlayerDeathSound();

    // Screen shake trigger (called by dinosaur footsteps)
    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerScreenShake(float Intensity, float Duration);

    // Narration voice lines
    UFUNCTION(BlueprintCallable, Category = "Audio|Narration")
    void PlayNarrationLine(int32 LineIndex);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_DinosaurSoundProfile> DinosaurProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume = 1.0f;

    // Narration audio URLs (populated from ElevenLabs TTS)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narration")
    TArray<FString> NarrationAudioURLs;

private:
    UPROPERTY()
    UAudioComponent* AmbientDayComponent;

    UPROPERTY()
    UAudioComponent* AmbientNightComponent;

    UPROPERTY()
    UAudioComponent* MusicComponent;

    EAudio_BiomeZone CurrentBiomeZone;
    EAudio_DangerLevel CurrentDangerLevel;
    float CurrentDayNightBlend;

    void UpdateAmbientCrossfade(float DeltaTime);
    void UpdateMusicLayer(float DeltaTime);
    FAudio_DinosaurSoundProfile* FindDinosaurProfile(FName Species);
};
