#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "DinosaurAudioSystem.generated.h"

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "Tyrannosaurus Rex"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brontosaurus    UMETA(DisplayName = "Brontosaurus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Pteranodon      UMETA(DisplayName = "Pteranodon"),
    Compsognathus   UMETA(DisplayName = "Compsognathus")
};

UENUM(BlueprintType)
enum class EDinosaurSoundType : uint8
{
    Idle            UMETA(DisplayName = "Idle Breathing"),
    Walking         UMETA(DisplayName = "Footsteps"),
    Running         UMETA(DisplayName = "Running"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Calling         UMETA(DisplayName = "Calling"),
    Warning         UMETA(DisplayName = "Warning Call"),
    Aggressive      UMETA(DisplayName = "Aggressive Roar"),
    Pain            UMETA(DisplayName = "Pain Cry"),
    Death           UMETA(DisplayName = "Death Sound")
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Alert           UMETA(DisplayName = "Alert"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Mating          UMETA(DisplayName = "Mating"),
    Sleeping        UMETA(DisplayName = "Sleeping")
};

USTRUCT(BlueprintType)
struct FDinosaurAudioProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    EDinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    float Size = 1.0f; // 0.1 = tiny, 1.0 = normal, 5.0 = massive

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    float VoicePitch = 1.0f; // Individual voice variation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    float Aggressiveness = 0.5f; // How aggressive this individual is

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    float VocalFrequency = 0.5f; // How often they make sounds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    bool bIsPredator = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    float HearingRange = 2000.0f; // How far they can hear
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurAudioSystem : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurAudioSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void InitializeDinosaurAudio(const FDinosaurAudioProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void UpdateBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlaySound(EDinosaurSoundType SoundType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetDistanceToPlayer(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetPlayerVisibility(bool bCanSeePlayer);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void ReactToSound(FVector SoundLocation, float SoundIntensity);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* VoiceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MovementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* BreathingComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaSounds")
    UMetaSoundSource* DinosaurVoiceMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaSounds")
    UMetaSoundSource* DinosaurMovementMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaSounds")
    UMetaSoundSource* DinosaurBreathingMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Profile")
    FDinosaurAudioProfile AudioProfile;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    EDinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    float DistanceToPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    bool bCanSeePlayer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    float CurrentStress; // 0.0 = calm, 1.0 = maximum stress

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    float LastSoundTime;

private:
    void UpdateBreathing();
    void UpdateMovementSounds();
    void UpdateVocalBehavior();
    
    float CalculateVocalProbability();
    EDinosaurSoundType SelectAppropriateSound();
    
    void ApplySpeciesCharacteristics();
    void ApplyIndividualVariations();
    
    FTimerHandle VocalBehaviorTimer;
    FTimerHandle BreathingUpdateTimer;
    
    float NextVocalTime;
    float BreathingRate;
};