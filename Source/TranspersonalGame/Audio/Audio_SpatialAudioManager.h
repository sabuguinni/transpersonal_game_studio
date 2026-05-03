#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerBox.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundBase.h"
#include "Core/SharedTypes.h"
#include "Audio_SpatialAudioManager.generated.h"

/**
 * AUDIO AGENT #16 - SPATIAL AUDIO MANAGER
 * 
 * Manages 3D spatial audio throughout the prehistoric world.
 * Handles ambient zones, dynamic audio mixing, and environmental audio.
 * 
 * Features:
 * - Biome-specific ambient audio zones
 * - Distance-based audio attenuation
 * - Dynamic audio mixing based on player location
 * - Dinosaur proximity audio cues
 * - Environmental audio triggers
 */

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Forest = 0      UMETA(DisplayName = "Forest"),
    Plains = 1      UMETA(DisplayName = "Plains"),
    River = 2       UMETA(DisplayName = "River"),
    Swamp = 3       UMETA(DisplayName = "Swamp"),
    Mountains = 4   UMETA(DisplayName = "Mountains"),
    Cave = 5        UMETA(DisplayName = "Cave"),
    Danger = 6      UMETA(DisplayName = "Danger")
};

UENUM(BlueprintType)
enum class EAudio_Priority : uint8
{
    Background = 0  UMETA(DisplayName = "Background"),
    Ambient = 1     UMETA(DisplayName = "Ambient"),
    Gameplay = 2    UMETA(DisplayName = "Gameplay"),
    Critical = 3    UMETA(DisplayName = "Critical"),
    Emergency = 4   UMETA(DisplayName = "Emergency")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ZoneSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float MaxDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bIs3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bAutoActivate = true;

    FAudio_ZoneSettings()
    {
        ZoneType = EAudio_ZoneType::Forest;
        BaseVolume = 0.7f;
        FadeDistance = 1000.0f;
        MaxDistance = 2000.0f;
        bIs3D = true;
        bAutoActivate = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EmitterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Emitter")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Emitter")
    EAudio_Priority Priority = EAudio_Priority::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Emitter")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Emitter")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Emitter")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Emitter")
    FString AudioURL;

    FAudio_EmitterData()
    {
        Location = FVector::ZeroVector;
        Priority = EAudio_Priority::Ambient;
        Volume = 1.0f;
        Pitch = 1.0f;
        bIsActive = true;
        AudioURL = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SpatialAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SpatialAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core audio zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    TArray<FAudio_ZoneSettings> AudioZones;

    // Active audio emitters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Emitters")
    TArray<FAudio_EmitterData> AudioEmitters;

    // Audio components for different zones
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* ForestAmbientComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* PlainsAmbientComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* RiverAmbientComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* DangerAmbientComponent;

    // Player reference for distance calculations
    UPROPERTY(BlueprintReadOnly, Category = "Player")
    APawn* PlayerPawn;

    // Audio mixing settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Mixing")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Mixing")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Mixing")
    float EffectsVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Mixing")
    float VoiceVolume = 0.9f;

public:
    // Audio zone management
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void RegisterAudioZone(EAudio_ZoneType ZoneType, FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UnregisterAudioZone(EAudio_ZoneType ZoneType);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateAudioMixing(float DeltaTime);

    // Audio emitter management
    UFUNCTION(BlueprintCallable, Category = "Audio Emitters")
    void AddAudioEmitter(FVector Location, EAudio_Priority Priority, const FString& AudioURL);

    UFUNCTION(BlueprintCallable, Category = "Audio Emitters")
    void RemoveAudioEmitter(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Emitters")
    void UpdateEmitterVolumes();

    // Player proximity audio
    UFUNCTION(BlueprintCallable, Category = "Player Audio")
    void UpdatePlayerProximityAudio();

    UFUNCTION(BlueprintCallable, Category = "Player Audio")
    float CalculateDistanceAttenuation(FVector EmitterLocation, float MaxDistance);

    // Zone detection
    UFUNCTION(BlueprintCallable, Category = "Zone Detection")
    EAudio_ZoneType GetCurrentPlayerZone();

    UFUNCTION(BlueprintCallable, Category = "Zone Detection")
    void OnPlayerEnterZone(EAudio_ZoneType NewZone);

    UFUNCTION(BlueprintCallable, Category = "Zone Detection")
    void OnPlayerExitZone(EAudio_ZoneType OldZone);

    // Audio URL integration
    UFUNCTION(BlueprintCallable, Category = "Audio URLs")
    void PlayAudioFromURL(const FString& AudioURL, FVector Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio URLs")
    void StopAudioFromURL(const FString& AudioURL);

    // Volume control
    UFUNCTION(BlueprintCallable, Category = "Volume Control")
    void SetMasterVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Volume Control")
    void SetAmbientVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Volume Control")
    void SetEffectsVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Volume Control")
    void SetVoiceVolume(float NewVolume);

private:
    // Internal audio management
    void InitializeAudioZones();
    void SetupDefaultAmbientSounds();
    void UpdateZoneTransitions(float DeltaTime);
    
    // Current zone tracking
    EAudio_ZoneType CurrentZone;
    EAudio_ZoneType PreviousZone;
    float ZoneTransitionTime;
    float ZoneTransitionDuration;
};