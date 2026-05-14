#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_DinosaurSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_DinosaurType : uint8
{
    TRex           UMETA(DisplayName = "T-Rex"),
    Raptor         UMETA(DisplayName = "Raptor"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Pteranodon     UMETA(DisplayName = "Pteranodon"),
    Stegosaurus    UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class EAudio_DinosaurBehavior : uint8
{
    Idle         UMETA(DisplayName = "Idle"),
    Hunting      UMETA(DisplayName = "Hunting"),
    Feeding      UMETA(DisplayName = "Feeding"),
    Territorial  UMETA(DisplayName = "Territorial"),
    Fleeing      UMETA(DisplayName = "Fleeing"),
    Mating       UMETA(DisplayName = "Mating"),
    Dying        UMETA(DisplayName = "Dying")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSoundSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TArray<USoundCue*> IdleSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TArray<USoundCue*> HuntingSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TArray<USoundCue*> FeedingSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TArray<USoundCue*> TerritorialSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TArray<USoundCue*> FleeingSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TArray<USoundCue*> MatingSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TArray<USoundCue*> DyingSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TArray<USoundCue*> FootstepSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    float MaxAudibleDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    float VocalizationInterval;

    FAudio_DinosaurSoundSet()
    {
        BaseVolume = 1.0f;
        MaxAudibleDistance = 3000.0f;
        VocalizationInterval = 15.0f;
    }
};

UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_DinosaurSoundManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DinosaurSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* VocalizationComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* FootstepComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Settings")
    EAudio_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Settings")
    EAudio_DinosaurBehavior CurrentBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Data")
    TMap<EAudio_DinosaurType, FAudio_DinosaurSoundSet> DinosaurSoundMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bEnableRandomVocalizations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bEnableFootstepSounds;

    FTimerHandle VocalizationTimer;
    FTimerHandle FootstepTimer;

public:
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetDinosaurType(EAudio_DinosaurType NewType);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetBehavior(EAudio_DinosaurBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayVocalization(EAudio_DinosaurBehavior BehaviorType = EAudio_DinosaurBehavior::Idle);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayFootstepSound();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void StartFootstepLoop(float StepInterval = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void StopFootstepLoop();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void EnableRandomVocalizations(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void StopAllSounds();

protected:
    void InitializeDinosaurSoundData();
    void StartRandomVocalizationTimer();
    void StopRandomVocalizationTimer();
    void PlayRandomVocalization();
    TArray<USoundCue*> GetSoundsForBehavior(EAudio_DinosaurBehavior Behavior);
};