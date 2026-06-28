#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "DinosaurAudioComponent.generated.h"

UENUM(BlueprintType)
enum class EAudio_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Alert       UMETA(DisplayName = "Alert"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAudio_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Generic         UMETA(DisplayName = "Generic")
};

USTRUCT(BlueprintType)
struct FAudio_DinoSoundSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* IdleBreath = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* AlertCall = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* Roar = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* Footstep = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* Death = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* Feeding = nullptr;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurAudioComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State Management ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void SetDinoState(EAudio_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void SetDinoSpecies(EAudio_DinoSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayRoar();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayFootstep();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDeath();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayAlert();

    // --- Distance-based attenuation ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void UpdatePlayerDistance(float DistanceMeters);

    // --- Screen shake trigger ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerGroundRumble(float Intensity);

    // --- Properties ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FAudio_DinoSoundSet SoundSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    EAudio_DinoSpecies Species = EAudio_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepInterval = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float IdleBreathInterval = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float MaxHearingDistance = 5000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Dinosaur", meta = (AllowPrivateAccess = "true"))
    EAudio_DinoState CurrentState = EAudio_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Dinosaur", meta = (AllowPrivateAccess = "true"))
    float PlayerDistanceMeters = 9999.0f;

private:
    float FootstepTimer = 0.0f;
    float BreathTimer = 0.0f;

    void PlaySoundAtOwner(USoundCue* Cue, float VolumeMultiplier = 1.0f);
    void TickIdleAudio(float DeltaTime);
};
