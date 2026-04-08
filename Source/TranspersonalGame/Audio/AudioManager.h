#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "AudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudioState : uint8
{
    Exploration,
    Tension,
    Danger,
    Combat,
    Safe,
    Discovery,
    Domestication
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    Forest,
    Plains,
    Swamp,
    Cave,
    River,
    Cliff
};

USTRUCT(BlueprintType)
struct FDynamicMusicLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* MusicCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping = true;
};

USTRUCT(BlueprintType)
struct FAudioStateConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EEnvironmentType, FDynamicMusicLayer> MusicLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TensionThreshold = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DangerThreshold = 0.8f;
};

UCLASS()
class TRANSPERSONALGAME_API AAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    TMap<EAudioState, FAudioStateConfig> AudioStates;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbienceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* TensionComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudioState CurrentAudioState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EEnvironmentType CurrentEnvironment;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentTensionLevel;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetAudioState(EAudioState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetEnvironmentType(EEnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateTensionLevel(float NewTensionLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayDinosaurSound(class ADinosaur* Dinosaur, const FString& SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayEnvironmentalSound(FVector Location, USoundCue* SoundCue, float VolumeMultiplier = 1.0f);

private:
    void TransitionToState(EAudioState NewState, float TransitionTime);
    void UpdateMusicLayers();
    void ProcessAmbientSounds();
    
    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;
    
    float StateTransitionTimer;
    EAudioState TargetAudioState;
    bool bIsTransitioning;
};