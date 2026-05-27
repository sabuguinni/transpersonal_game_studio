#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Audio_AdaptiveMusicSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Exploration     UMETA(DisplayName = "Exploration"),
    Danger          UMETA(DisplayName = "Danger"),
    Combat          UMETA(DisplayName = "Combat"),
    Narrative       UMETA(DisplayName = "Narrative"),
    Survival        UMETA(DisplayName = "Survival")
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savana          UMETA(DisplayName = "Savana"),
    Forest          UMETA(DisplayName = "Forest"),
    Desert          UMETA(DisplayName = "Desert"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Mountain        UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Config")
    TSoftObjectPtr<USoundCue> MusicCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Config")
    float Volume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Config")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Config")
    float FadeOutTime = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Config")
    bool bLooping = true;

    FAudio_MusicConfig()
    {
        Volume = 0.7f;
        FadeInTime = 2.0f;
        FadeOutTime = 1.5f;
        bLooping = true;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_AdaptiveMusicSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveMusicSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Music state management
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetMusicState(EAudio_MusicState NewState);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void PlayNarrativeAudio(const FString& AudioURL, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void StopNarrativeAudio();

    // Danger detection system
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void OnDinosaurNearby(float Distance, const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void OnPlayerHealthLow(float HealthPercentage);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void OnCombatStart();

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void OnCombatEnd();

protected:
    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAudio_MusicState CurrentMusicState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAudio_BiomeType CurrentBiome;

    // Music configurations per state and biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Configs")
    TMap<EAudio_MusicState, FAudio_MusicConfig> MusicConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configs")
    TMap<EAudio_BiomeType, FAudio_MusicConfig> BiomeAmbientConfigs;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> NarrativeAudioComponent;

    // Transition system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionTime = 3.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
    bool bIsTransitioning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
    float TransitionTimer;

    // Danger detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger Detection")
    float DangerDetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger Detection")
    float DangerStateTimeout = 10.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Danger Detection")
    float DangerTimer;

private:
    void InitializeMusicConfigs();
    void InitializeBiomeConfigs();
    void TransitionToState(EAudio_MusicState NewState);
    void UpdateTransition(float DeltaTime);
    void PlayMusicForState(EAudio_MusicState State);
    void PlayAmbientForBiome(EAudio_BiomeType Biome);
    void CheckForDangers();
    
    EAudio_MusicState PreviousState;
    FAudio_MusicConfig* CurrentConfig;
    FAudio_MusicConfig* TargetConfig;
};