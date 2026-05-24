#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Math/Vector.h"
#include "Audio_SpatialAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SpatialAudioType : uint8
{
    RadioTransmission   UMETA(DisplayName = "Radio Transmission"),
    EnvironmentalAmbient UMETA(DisplayName = "Environmental Ambient"),
    DinosaurVocalization UMETA(DisplayName = "Dinosaur Vocalization"),
    ExpeditionChatter   UMETA(DisplayName = "Expedition Chatter"),
    EmergencyAlert      UMETA(DisplayName = "Emergency Alert")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SpatialAudioSource
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float MaxRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    EAudio_SpatialAudioType AudioType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bIsLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bIsActive;

    FAudio_SpatialAudioSource()
    {
        Location = FVector::ZeroVector;
        MaxRange = 5000.0f;
        Volume = 1.0f;
        AudioType = EAudio_SpatialAudioType::EnvironmentalAmbient;
        AudioURL = "";
        bIsLooping = false;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_RadioStationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Station")
    FString StationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Station")
    FVector StationLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Station")
    float TransmissionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Station")
    TArray<FString> VoiceAssetURLs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Station")
    float TransmissionInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Station")
    bool bIsOperational;

    FAudio_RadioStationData()
    {
        StationName = "Unknown Station";
        StationLocation = FVector::ZeroVector;
        TransmissionRange = 8000.0f;
        TransmissionInterval = 300.0f; // 5 minutes
        bIsOperational = true;
    }
};

/**
 * Spatial Audio Manager for 3D positioned audio sources, radio transmissions, and environmental audio
 * Integrates with the narrative radio communication system for immersive expedition team broadcasts
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_SpatialAudioManager : public UObject
{
    GENERATED_BODY()

public:
    UAudio_SpatialAudioManager();

    // Core spatial audio management
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void InitializeSpatialAudio(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void UpdatePlayerPosition(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    int32 AddSpatialAudioSource(const FAudio_SpatialAudioSource& AudioSource);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void RemoveSpatialAudioSource(int32 SourceID);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void UpdateSpatialAudioSource(int32 SourceID, const FAudio_SpatialAudioSource& UpdatedSource);

    // Radio station management
    UFUNCTION(BlueprintCallable, Category = "Radio System")
    void RegisterRadioStation(const FAudio_RadioStationData& StationData);

    UFUNCTION(BlueprintCallable, Category = "Radio System")
    void PlayRadioTransmission(const FString& StationName, const FString& AudioURL);

    UFUNCTION(BlueprintCallable, Category = "Radio System")
    void SetRadioStationOperational(const FString& StationName, bool bOperational);

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetBiomeAmbientAudio(const FVector& Location, float Radius, const FString& AudioURL);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void TriggerDinosaurVocalization(const FVector& Location, const FString& DinosaurType, float Intensity);

    // Distance-based audio processing
    UFUNCTION(BlueprintCallable, Category = "Audio Processing")
    float CalculateAudioVolume(const FVector& SourceLocation, const FVector& ListenerLocation, float MaxRange) const;

    UFUNCTION(BlueprintCallable, Category = "Audio Processing")
    bool IsAudioSourceInRange(const FVector& SourceLocation, const FVector& ListenerLocation, float MaxRange) const;

    // Integration with narrative system
    UFUNCTION(BlueprintCallable, Category = "Narrative Integration")
    void IntegrateWithRadioCommunicationSystem();

    UFUNCTION(BlueprintCallable, Category = "Narrative Integration")
    void ProcessExpeditionBroadcast(const FString& Message, const FVector& OriginLocation);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spatial Audio")
    TArray<FAudio_SpatialAudioSource> ActiveAudioSources;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Radio System")
    TArray<FAudio_RadioStationData> RadioStations;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    FVector CurrentPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float GlobalAudioVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float RadioStaticVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float EnvironmentalAudioVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxAudioDistance;

private:
    UPROPERTY()
    UWorld* WorldContext;

    UPROPERTY()
    TMap<int32, class UAudioComponent*> AudioComponents;

    int32 NextSourceID;

    // Timer handles for periodic transmissions
    FTimerHandle RadioTransmissionTimer;

    // Internal audio processing
    void UpdateAudioSources();
    void ProcessRadioTransmissions();
    UAudioComponent* CreateAudioComponent(const FAudio_SpatialAudioSource& AudioSource);
    void CleanupInactiveAudioSources();
};