#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Audio_DinosaurSoundSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_DinosaurType : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Pteranodon  UMETA(DisplayName = "Pteranodon")
};

UENUM(BlueprintType)
enum class EAudio_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Alert       UMETA(DisplayName = "Alert"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Territorial UMETA(DisplayName = "Territorial"),
    Wounded     UMETA(DisplayName = "Wounded"),
    Death       UMETA(DisplayName = "Death")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TSoftObjectPtr<USoundCue> IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TSoftObjectPtr<USoundCue> AlertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TSoftObjectPtr<USoundCue> AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TSoftObjectPtr<USoundCue> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    TSoftObjectPtr<USoundCue> BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float HearingRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FootstepInterval = 1.5f;

    FAudio_DinosaurSoundData()
    {
        BaseVolume = 1.0f;
        HearingRange = 2000.0f;
        FootstepInterval = 1.5f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_DinosaurSoundSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DinosaurSoundSystem();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetDinosaurType(EAudio_DinosaurType NewType);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetDinosaurState(EAudio_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayStateSound();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayFootstepSound();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetMovementSpeed(float Speed);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void TriggerTerritorialRoar();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetVolumeMultiplier(float Multiplier);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* VocalizationComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* FootstepComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* BreathingComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Configuration")
    TMap<EAudio_DinosaurType, FAudio_DinosaurSoundData> DinosaurSoundMap;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_DinosaurType CurrentType;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_DinosaurState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float FootstepThreshold = 50.0f;

private:
    UFUNCTION()
    void UpdateFootstepTiming();

    UFUNCTION()
    void UpdateBreathingIntensity();

    float LastFootstepTime = 0.0f;
    float StateChangeTime = 0.0f;
    bool bIsMoving = false;
};