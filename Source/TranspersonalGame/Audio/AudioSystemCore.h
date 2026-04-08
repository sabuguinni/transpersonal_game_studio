#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioMixerBlueprintLibrary.h"
#include "AudioSystemCore.generated.h"

UENUM(BlueprintType)
enum class EAudioEnvironmentType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    Cave            UMETA(DisplayName = "Cave"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Canyon          UMETA(DisplayName = "Canyon")
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    Safe            UMETA(DisplayName = "Safe"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Danger          UMETA(DisplayName = "Danger"),
    Imminent        UMETA(DisplayName = "Imminent Threat"),
    Combat          UMETA(DisplayName = "Combat")
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn            UMETA(DisplayName = "Dawn"),
    Morning         UMETA(DisplayName = "Morning"),
    Midday          UMETA(DisplayName = "Midday"),
    Afternoon       UMETA(DisplayName = "Afternoon"),
    Dusk            UMETA(DisplayName = "Dusk"),
    Night           UMETA(DisplayName = "Night"),
    DeepNight       UMETA(DisplayName = "Deep Night")
};

USTRUCT(BlueprintType)
struct FAudioEnvironmentSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USoundSubmix* EnvironmentSubmix;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UReverbEffect* ReverbEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EchoIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HighFrequencyAbsorption = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class USoundCue*> AmbientSounds;
};

USTRUCT(BlueprintType)
struct FAdaptiveMusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EThreatLevel ThreatLevel = EThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudioEnvironmentType Environment = EAudioEnvironmentType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Tension = 0.0f; // 0.0 = calm, 1.0 = maximum tension

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPlayerHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDinosaurNearby = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NearbyDinosaurCount = 0;
};

/**
 * Core audio system that manages adaptive music, environmental audio, and dynamic soundscapes
 * Based on the principle that audio should be felt, not noticed
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemCore();

    // Core System Functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAdaptiveMusicState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EAudioEnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetThreatLevel(EThreatLevel NewThreatLevel, float TransitionTime = 2.0f);

    // Adaptive Music System
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void StartAdaptiveMusic();

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void StopAdaptiveMusic(float FadeOutTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void UpdateMusicTension(float NewTension, float TransitionTime = 1.0f);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayEnvironmentalSound(USoundCue* SoundCue, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void StartAmbientLoop(EAudioEnvironmentType Environment);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void StopAmbientLoop(float FadeOutTime = 2.0f);

    // Dinosaur Audio System
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void RegisterDinosaurPresence(FVector DinosaurLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void UnregisterDinosaurPresence(FVector DinosaurLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurSound(USoundCue* DinosaurSound, FVector Location, float MaxDistance = 2000.0f);

protected:
    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAdaptiveMusicState CurrentAudioState;

    // Environment settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment Settings")
    TMap<EAudioEnvironmentType, FAudioEnvironmentSettings> EnvironmentSettings;

    // Adaptive music components
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive Music")
    class UMetaSoundSource* AdaptiveMusicMetaSound;

    UPROPERTY(BlueprintReadOnly, Category = "Adaptive Music")
    class UAudioComponent* MusicAudioComponent;

    // Ambient audio components
    UPROPERTY(BlueprintReadOnly, Category = "Ambient Audio")
    TArray<UAudioComponent*> AmbientAudioComponents;

    // Audio submixes
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Mixing")
    class USoundSubmix* MasterSubmix;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Mixing")
    class USoundSubmix* MusicSubmix;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Mixing")
    class USoundSubmix* AmbientSubmix;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Mixing")
    class USoundSubmix* SFXSubmix;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Mixing")
    class USoundSubmix* DinosaurSubmix;

private:
    // Internal functions
    void UpdateEnvironmentalAudio();
    void UpdateAdaptiveMusic();
    void CalculateAudioParameters();
    
    // Transition management
    void StartAudioTransition(float Duration);
    bool IsTransitioning() const;

    // Audio parameter calculation
    float CalculateTensionFromThreats() const;
    float CalculateEnvironmentalInfluence() const;
    float CalculateTimeOfDayInfluence() const;

    // Registered dinosaur threats
    TArray<FVector> RegisteredDinosaurLocations;
    
    // Transition state
    bool bIsTransitioning = false;
    float TransitionStartTime = 0.0f;
    float TransitionDuration = 0.0f;
    FAdaptiveMusicState TransitionStartState;
    FAdaptiveMusicState TransitionTargetState;
};