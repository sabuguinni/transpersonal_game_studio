#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioSystemCore.generated.h"

UENUM(BlueprintType)
enum class EAudioThreatLevel : uint8
{
    Safe = 0,
    Cautious = 1,
    Threatened = 2,
    Hunted = 3,
    Panic = 4
};

UENUM(BlueprintType)
enum class EAudioEnvironment : uint8
{
    DenseForest = 0,
    OpenPlains = 1,
    RiverSide = 2,
    CaveSystem = 3,
    Cliffside = 4
};

UENUM(BlueprintType)
enum class EDinosaurProximity : uint8
{
    None = 0,
    Distant = 1,
    Nearby = 2,
    Close = 3,
    Immediate = 4
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    EAudioThreatLevel ThreatLevel = EAudioThreatLevel::Safe;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    EAudioEnvironment Environment = EAudioEnvironment::DenseForest;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    EDinosaurProximity DinosaurProximity = EDinosaurProximity::None;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    float TimeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    float WeatherIntensity = 0.0f; // 0.0 = clear, 1.0 = storm

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    bool bIsPlayerHidden = false;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    bool bIsPlayerMoving = false;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    float PlayerHeartRate = 60.0f; // BPM for stress indication
};

/**
 * Core Audio System Manager
 * Handles adaptive music, environmental audio, and threat-based sound design
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
    void UpdateAudioState(const FAudioStateData& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetThreatLevel(EAudioThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironment(EAudioEnvironment NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void OnDinosaurSpotted(EDinosaurProximity Proximity, FVector DinosaurLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void OnPlayerHidden(bool bHidden);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(FVector Location, float ThreatLevel, bool bIsAggressive);

    // Adaptive Music System
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void TransitionToMusicState(const FString& StateName, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetMusicParameter(const FString& ParameterName, float Value);

protected:
    // Audio State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudioStateData CurrentAudioState;

    // MetaSound Assets
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> EnvironmentAmbientMetaSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> ThreatSystemMetaSound;

    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> ThreatAudioComponent;

    // Internal Functions
    void UpdateAdaptiveMusic();
    void UpdateEnvironmentalAudio();
    void UpdateThreatAudio();
    void CalculateAudioParameters();

    // Audio Parameter Calculation
    float CalculateTensionLevel() const;
    float CalculateIsolationLevel() const;
    float CalculateVulnerabilityLevel() const;
};