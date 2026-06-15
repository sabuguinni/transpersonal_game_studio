#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerBox.h"
#include "Sound/SoundBase.h"
#include "Audio_NarrativeAudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_NarrativeClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString ClipName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    USoundBase* AudioClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    bool bPlayOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float CooldownTime;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative Audio")
    bool bHasPlayed;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative Audio")
    float LastPlayTime;

    FAudio_NarrativeClip()
    {
        ClipName = TEXT("DefaultClip");
        AudioClip = nullptr;
        TriggerLocation = TEXT("Default");
        TriggerRadius = 500.0f;
        bPlayOnce = false;
        CooldownTime = 30.0f;
        bHasPlayed = false;
        LastPlayTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_AmbientZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    USoundBase* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    FVector ZoneLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    bool bLooping;

    UPROPERTY(BlueprintReadOnly, Category = "Ambient Audio")
    bool bIsActive;

    FAudio_AmbientZone()
    {
        ZoneName = TEXT("DefaultZone");
        AmbientSound = nullptr;
        ZoneLocation = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        VolumeMultiplier = 1.0f;
        bLooping = true;
        bIsActive = false;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AAudio_NarrativeAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_NarrativeAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* NarrativeAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbientAudioComponent;

public:
    // Narrative Audio Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative System")
    TArray<FAudio_NarrativeClip> NarrativeClips;

    // Ambient Audio Zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient System")
    TArray<FAudio_AmbientZone> AmbientZones;

    // System Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float NarrativeVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FadeOutTime;

    // Player Reference
    UPROPERTY(BlueprintReadOnly, Category = "Player")
    class APawn* PlayerPawn;

    // Audio Control Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayNarrativeClip(const FString& ClipName);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void StopNarrativeAudio();

    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void SetAmbientZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void StopAmbientAudio();

    // Trigger System
    UFUNCTION(BlueprintCallable, Category = "Audio Triggers")
    void CheckNarrativeTriggers();

    UFUNCTION(BlueprintCallable, Category = "Audio Triggers")
    void CheckAmbientZones();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterNarrativeClip(const FAudio_NarrativeClip& NewClip);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterAmbientZone(const FAudio_AmbientZone& NewZone);

    UFUNCTION(BlueprintPure, Category = "Audio System")
    bool IsNarrativeClipAvailable(const FString& ClipName);

    UFUNCTION(BlueprintPure, Category = "Audio System")
    float GetDistanceToPlayer(const FVector& Location);

private:
    // Internal state
    FString CurrentNarrativeClip;
    FString CurrentAmbientZone;
    float LastTriggerCheckTime;
    float TriggerCheckInterval;

    // Helper functions
    void InitializeAudioComponents();
    void UpdatePlayerReference();
    bool CanPlayNarrativeClip(const FAudio_NarrativeClip& Clip);
    void FadeAudioComponent(UAudioComponent* AudioComp, float TargetVolume, float Duration);
};