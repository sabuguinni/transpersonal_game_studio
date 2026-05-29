#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Audio_DinosaurAudioBehavior.generated.h"

UENUM(BlueprintType)
enum class EAudio_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "T-Rex"),
    Raptor         UMETA(DisplayName = "Raptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Carnotaurus    UMETA(DisplayName = "Carnotaurus"),
    Giganotosaurus UMETA(DisplayName = "Giganotosaurus"),
    Pteranodon     UMETA(DisplayName = "Pteranodon")
};

UENUM(BlueprintType)
enum class EAudio_DinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Territorial UMETA(DisplayName = "Territorial"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Mating      UMETA(DisplayName = "Mating"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Aggressive  UMETA(DisplayName = "Aggressive")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurAudioProfile
{
    GENERATED_BODY()

    // Basic vocalizations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocalizations")
    TSoftObjectPtr<USoundCue> IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocalizations")
    TSoftObjectPtr<USoundCue> AggressiveRoar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocalizations")
    TSoftObjectPtr<USoundCue> HuntingCall;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocalizations")
    TSoftObjectPtr<USoundCue> PainCry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocalizations")
    TSoftObjectPtr<USoundCue> DeathRoar;

    // Movement sounds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    TSoftObjectPtr<USoundCue> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    TSoftObjectPtr<USoundCue> BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    TSoftObjectPtr<USoundCue> HeartbeatSound;

    // Audio parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float VocalizationRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FootstepRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float VocalizationFrequency = 30.0f; // Seconds between vocalizations

    FAudio_DinosaurAudioProfile()
    {
        BaseVolume = 1.0f;
        VocalizationRange = 2000.0f;
        FootstepRange = 1000.0f;
        VocalizationFrequency = 30.0f;
    }
};

/**
 * Audio behavior component for dinosaurs that handles species-specific vocalizations,
 * movement sounds, and behavioral audio cues based on AI state
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_DinosaurAudioBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DinosaurAudioBehavior();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    EAudio_DinosaurSpecies DinosaurSpecies = EAudio_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    FAudio_DinosaurAudioProfile AudioProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    bool bEnableProceduralAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    bool bEnableFootstepAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    bool bEnableBreathingAudio = true;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* VocalizationAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MovementAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* BreathingAudioComponent;

    // State tracking
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_DinosaurBehaviorState CurrentBehaviorState = EAudio_DinosaurBehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastVocalizationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float MovementSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsMoving = false;

    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetBehaviorState(EAudio_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void TriggerVocalization(EAudio_DinosaurBehaviorState ForState = EAudio_DinosaurBehaviorState::Idle);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void TriggerFootstep();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void TriggerPainCry();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void TriggerDeathRoar();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetMovementSpeed(float Speed);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void UpdateAudioProfile();

    UFUNCTION(BlueprintPure, Category = "Dinosaur Audio")
    float GetVocalizationRange() const { return AudioProfile.VocalizationRange; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur Audio")
    float GetFootstepRange() const { return AudioProfile.FootstepRange; }

private:
    // Internal methods
    void InitializeAudioComponents();
    void LoadSpeciesAudioProfile();
    void UpdateMovementAudio(float DeltaTime);
    void UpdateBreathingAudio(float DeltaTime);
    void UpdateProceduralVocalizations(float DeltaTime);
    
    USoundCue* GetSoundForBehaviorState(EAudio_DinosaurBehaviorState State) const;
    float GetVocalizationFrequencyForState(EAudio_DinosaurBehaviorState State) const;
    
    // Audio timing
    float VocalizationTimer = 0.0f;
    float FootstepTimer = 0.0f;
    float BreathingTimer = 0.0f;
    
    // Movement tracking
    FVector LastPosition = FVector::ZeroVector;
    float FootstepInterval = 1.0f; // Seconds between footsteps when moving
    
    // Species-specific parameters
    void ConfigureForTRex();
    void ConfigureForRaptor();
    void ConfigureForTriceratops();
    void ConfigureForBrachiosaurus();
    void ConfigureForCarnotaurus();
    void ConfigureForGiganotosaurus();
    void ConfigureForPteranodon();
    
    bool bIsInitialized = false;
};