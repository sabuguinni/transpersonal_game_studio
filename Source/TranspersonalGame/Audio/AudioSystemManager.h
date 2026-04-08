#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    Safe = 0,
    Cautious = 1,
    Danger = 2,
    Imminent = 3,
    Combat = 4
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest = 0,
    OpenPlains = 1,
    Riverside = 2,
    RockyOutcrop = 3,
    Swampland = 4
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn = 0,
    Morning = 1,
    Midday = 2,
    Afternoon = 3,
    Dusk = 4,
    Night = 5
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EThreatLevel ThreatLevel = EThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType Environment = EEnvironmentType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StressLevel = 0.0f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeartRate = 60.0f; // BPM simulation

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHiding = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNearWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NearbyDinosaurCount = 0;
};

/**
 * Central audio system manager that controls adaptive music, ambient soundscapes,
 * and dynamic audio responses based on game state and player emotional state.
 */
UCLASS()
class TRANSPERSONALGAME_API AAudioSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* MusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* AmbienceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* TensionComponent;

    // MetaSound Sources for Adaptive Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    class UMetaSoundSource* AdaptiveMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    class UMetaSoundSource* AmbienceMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    class UMetaSoundSource* TensionMetaSound;

    // Audio State
    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    FAudioState CurrentAudioState;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    FAudioState PreviousAudioState;

public:
    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateThreatLevel(EThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateEnvironment(EEnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateTimeOfDay(ETimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateStressLevel(float NewStressLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetPlayerHiding(bool bHiding);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterDinosaurNearby(bool bIsNearby);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerHeartbeatIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurCall(FVector Location, class USoundCue* DinosaurCallSound);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstepSequence(FVector Location, float DinosaurSize);

private:
    // Internal Methods
    void UpdateAudioParameters();
    void TransitionMusicState();
    void UpdateAmbienceLayer();
    void UpdateTensionLayer();
    
    // Parameter calculation
    float CalculateMusicIntensity();
    float CalculateAmbienceVolume();
    float CalculateTensionLevel();
    
    // Transition timing
    UPROPERTY(EditAnywhere, Category = "Audio Transitions")
    float MusicTransitionTime = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Audio Transitions")
    float AmbienceTransitionTime = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Audio Transitions")
    float TensionTransitionTime = 1.0f;

    // Internal state tracking
    float LastStateChangeTime = 0.0f;
    bool bIsTransitioning = false;
};