#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "Audio_SystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_AtmosphereType : uint8
{
    Forest_Calm         UMETA(DisplayName = "Forest Calm"),
    Forest_Tense        UMETA(DisplayName = "Forest Tense"),
    Predator_Territory  UMETA(DisplayName = "Predator Territory"),
    Water_Source        UMETA(DisplayName = "Water Source"),
    Safe_Zone          UMETA(DisplayName = "Safe Zone"),
    Danger_Zone        UMETA(DisplayName = "Danger Zone")
};

USTRUCT(BlueprintType)
struct FAudio_TriggerZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    EAudio_AtmosphereType AtmosphereType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    bool bIsActive;

    FAudio_TriggerZone()
    {
        Location = FVector::ZeroVector;
        Radius = 500.0f;
        AtmosphereType = EAudio_AtmosphereType::Forest_Calm;
        VolumeMultiplier = 1.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct FAudio_NarrativeClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString ClipName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    TSoftObjectPtr<USoundCue> AudioClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    bool bPlayOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    bool bHasPlayed;

    FAudio_NarrativeClip()
    {
        ClipName = TEXT("DefaultClip");
        Priority = 1.0f;
        bPlayOnce = true;
        bHasPlayed = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* NarrativeAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* EffectsAudioComponent;

    // Audio Trigger Zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    TArray<FAudio_TriggerZone> TriggerZones;

    // Narrative Audio Clips
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    TArray<FAudio_NarrativeClip> NarrativeClips;

    // Current Audio State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_AtmosphereType CurrentAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentTensionLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsNarrativePlaying;

    // Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float NarrativeVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float EffectsVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FadeTime;

private:
    // Internal state
    FVector LastPlayerLocation;
    float TensionUpdateTimer;
    int32 CurrentTriggerZoneIndex;

public:
    // Audio Control Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetAtmosphere(EAudio_AtmosphereType NewAtmosphere, bool bFadeTransition = true);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayNarrativeClip(const FString& ClipName, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void StopNarrative();

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetTensionLevel(float TensionLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void UpdatePlayerLocation(FVector PlayerLocation);

    // Trigger Zone Management
    UFUNCTION(BlueprintCallable, Category = "Audio Triggers")
    void AddTriggerZone(FVector Location, float Radius, EAudio_AtmosphereType AtmosphereType);

    UFUNCTION(BlueprintCallable, Category = "Audio Triggers")
    void RemoveTriggerZone(int32 ZoneIndex);

    UFUNCTION(BlueprintCallable, Category = "Audio Triggers")
    int32 GetActiveZoneForLocation(FVector Location);

    // Volume Control
    UFUNCTION(BlueprintCallable, Category = "Audio Volume")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Volume")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Volume")
    void SetNarrativeVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Volume")
    void SetEffectsVolume(float Volume);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Utility")
    bool IsNarrativePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Audio Utility")
    float GetCurrentTensionLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Audio Utility")
    EAudio_AtmosphereType GetCurrentAtmosphere() const;

protected:
    // Internal helper functions
    void UpdateAmbientAudio();
    void UpdateTensionBasedOnLocation(FVector PlayerLocation);
    void CrossfadeAmbientAudio(USoundCue* NewSound);
    FAudio_NarrativeClip* FindNarrativeClip(const FString& ClipName);
    void InitializeDefaultTriggerZones();
    void InitializeDefaultNarrativeClips();
};