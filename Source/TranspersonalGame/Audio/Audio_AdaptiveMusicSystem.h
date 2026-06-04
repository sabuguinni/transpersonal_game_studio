#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Audio_AdaptiveMusicSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Silence,
    Exploration,
    Tribal,
    Combat,
    Danger,
    Victory
};

UENUM(BlueprintType)
enum class EAudio_IntensityLevel : uint8
{
    Calm,
    Tense,
    Intense,
    Extreme
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicTrack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_MusicState MusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_IntensityLevel IntensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float FadeOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    bool bShouldLoop;

    FAudio_MusicTrack()
    {
        MusicState = EAudio_MusicState::Silence;
        IntensityLevel = EAudio_IntensityLevel::Calm;
        FadeInTime = 2.0f;
        FadeOutTime = 1.5f;
        bShouldLoop = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_TribalZoneSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    FString TribeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    TSoftObjectPtr<USoundCue> TribalMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    float RelationshipModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    float ZoneRadius;

    FAudio_TribalZoneSettings()
    {
        TribeName = TEXT("Unknown Tribe");
        RelationshipModifier = 0.0f;
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AdaptiveMusicSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AdaptiveMusicSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* TribalAudioComponent;

    // Music system configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    TArray<FAudio_MusicTrack> MusicTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    EAudio_MusicState CurrentMusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    EAudio_IntensityLevel CurrentIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    float MasterMusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    float MasterAmbientVolume;

    // Tribal territory system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    TArray<FAudio_TribalZoneSettings> TribalZones;

    UPROPERTY(BlueprintReadOnly, Category = "Tribal Audio")
    FString CurrentTribalTerritory;

    UPROPERTY(BlueprintReadOnly, Category = "Tribal Audio")
    float CurrentTribalRelationship;

    // Adaptive music functions
    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void TransitionToMusicState(EAudio_MusicState NewState, EAudio_IntensityLevel NewIntensity = EAudio_IntensityLevel::Calm);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void SetMusicIntensity(EAudio_IntensityLevel NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void PlayTribalMusic(const FString& TribeName, float RelationshipLevel);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void StopAllMusic(float FadeOutTime = 2.0f);

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateTribalTerritory(const FString& TribeName, float RelationshipLevel);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayEnvironmentalStory(const FString& LocationName);

    // Combat audio triggers
    UFUNCTION(BlueprintCallable, Category = "Combat Audio")
    void OnDinosaurEncounter(const FString& DinosaurType, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat Audio")
    void OnCombatStart();

    UFUNCTION(BlueprintCallable, Category = "Combat Audio")
    void OnCombatEnd(bool bPlayerVictory);

private:
    // Internal music management
    FAudio_MusicTrack* FindMusicTrack(EAudio_MusicState State, EAudio_IntensityLevel Intensity);
    void CrossfadeToTrack(const FAudio_MusicTrack& NewTrack);
    void UpdateTribalAudioBasedOnRelationship(float RelationshipLevel);
    
    // Transition timing
    float TransitionTimer;
    bool bIsTransitioning;
    EAudio_MusicState TargetMusicState;
    EAudio_IntensityLevel TargetIntensity;
};