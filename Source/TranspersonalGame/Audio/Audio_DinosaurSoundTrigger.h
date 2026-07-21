#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Audio_DinosaurSoundTrigger.generated.h"

UENUM(BlueprintType)
enum class EAudio_DinosaurType : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus UMETA(DisplayName = "Stegosaurus"),
    Pteranodon  UMETA(DisplayName = "Pteranodon")
};

UENUM(BlueprintType)
enum class EAudio_DinosaurBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Territorial UMETA(DisplayName = "Territorial"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Mating      UMETA(DisplayName = "Mating")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSoundSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TArray<class USoundBase*> IdleSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TArray<class USoundBase*> AggressiveSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TArray<class USoundBase*> FootstepSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TArray<class USoundBase*> BreathingSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MinPlayInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxPlayInterval = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float VolumeMultiplier = 1.0f;

    FAudio_DinosaurSoundSet()
    {
        MinPlayInterval = 5.0f;
        MaxPlayInterval = 15.0f;
        VolumeMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_DinosaurSoundTrigger : public AActor
{
    GENERATED_BODY()

public:
    AAudio_DinosaurSoundTrigger();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* DinosaurAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Settings")
    EAudio_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Settings")
    EAudio_DinosaurBehavior CurrentBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Configuration")
    FAudio_DinosaurSoundSet SoundSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Settings")
    float TriggerRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Settings")
    bool bPlayerTriggered = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxAudibleDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bUse3DSound = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float PitchVariation = 0.1f;

private:
    FTimerHandle SoundPlayTimer;
    float NextSoundTime = 0.0f;
    bool bIsPlayerInRange = false;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                              bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetDinosaurBehavior(EAudio_DinosaurBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayRandomSound(EAudio_DinosaurBehavior BehaviorType);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayFootstepSound();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayAggressiveSound();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetTriggerRadius(float NewRadius);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void StartAutomaticSounds();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void StopAutomaticSounds();

private:
    void ScheduleNextSound();
    void PlayScheduledSound();
    USoundBase* GetRandomSoundFromArray(const TArray<USoundBase*>& SoundArray);
    void UpdateAudioSettings();
};