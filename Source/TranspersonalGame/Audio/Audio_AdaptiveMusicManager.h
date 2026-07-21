#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Audio_AdaptiveMusicManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Calm,
    Tension,
    Danger,
    Combat,
    Exploration,
    Dawn,
    Dusk,
    Night
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = false;

    FAudio_MusicLayer()
    {
        Volume = 1.0f;
        FadeTime = 2.0f;
        bIsActive = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_AdaptiveMusicManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveMusicManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Music state management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMusicState(EAudio_MusicState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_MusicState GetCurrentMusicState() const { return CurrentMusicState; }

    // Layer management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void EnableMusicLayer(int32 LayerIndex, float FadeInTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void DisableMusicLayer(int32 LayerIndex, float FadeOutTime = 2.0f);

    // Proximity-based music
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateProximityMusic(const TArray<AActor*>& NearbyActors);

    // Time of day music
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateTimeOfDayMusic(float TimeOfDay);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Layers")
    TArray<FAudio_MusicLayer> MusicLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Components")
    TArray<UAudioComponent*> AudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    EAudio_MusicState CurrentMusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    EAudio_MusicState TargetMusicState;

    // Transition timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float StateTransitionTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float CurrentTransitionTime = 0.0f;

    // Proximity settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float DangerDetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float TensionDetectionRadius = 3000.0f;

private:
    void InitializeAudioComponents();
    void ProcessMusicTransition(float DeltaTime);
    void UpdateLayerVolumes(float DeltaTime);
    EAudio_MusicState DetermineMusicStateFromProximity(const TArray<AActor*>& NearbyActors);
    EAudio_MusicState DetermineMusicStateFromTimeOfDay(float TimeOfDay);
};