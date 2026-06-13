#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Audio_SystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Peaceful    UMETA(DisplayName = "Peaceful"),
    Alert       UMETA(DisplayName = "Alert"),
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat")
};

USTRUCT(BlueprintType)
struct FAudio_BiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 1.5f;
};

USTRUCT(BlueprintType)
struct FAudio_ThreatSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> ThreatMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float IntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float HeartbeatVolume = 0.3f;
};

/**
 * Audio System Manager - Handles adaptive music, ambient sounds, and threat audio
 * Provides biome-based ambient audio and dynamic threat level music
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_SystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_SystemManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome audio management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    // Audio control
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayFootstepSound(const FVector& Location, bool bIsHeavy = false);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(const FString& DinosaurType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetTimeOfDay(float TimeRatio); // 0.0 = midnight, 0.5 = noon

    // Audio zones
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterAudioZone(class AAudio_Zone* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UnregisterAudioZone(class AAudio_Zone* Zone);

protected:
    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentTimeOfDay;

    // Audio components
    UPROPERTY()
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY()
    UAudioComponent* MusicAudioComponent;

    UPROPERTY()
    UAudioComponent* ThreatAudioComponent;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EAudio_BiomeType, FAudio_BiomeSettings> BiomeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EAudio_ThreatLevel, FAudio_ThreatSettings> ThreatSettings;

    // Registered zones
    UPROPERTY()
    TArray<class AAudio_Zone*> RegisteredZones;

private:
    void UpdateAmbientAudio();
    void UpdateThreatAudio();
    void UpdateTimeBasedAudio();

    // Audio transition
    void FadeAudioComponent(UAudioComponent* Component, float TargetVolume, float FadeTime);
};