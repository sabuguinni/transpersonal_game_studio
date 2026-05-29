#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Audio_SpatialAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambient,
    Dinosaur,
    Player,
    Environment,
    Music,
    UI,
    Narration,
    Combat
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SpatialSound
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_SoundCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority;

    FAudio_SpatialSound()
    {
        SoundCue = nullptr;
        Location = FVector::ZeroVector;
        MaxDistance = 5000.0f;
        Volume = 1.0f;
        Category = EAudio_SoundCategory::Ambient;
        bIsLooping = false;
        Priority = 50;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAmbience
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<USoundCue*> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CrossfadeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseVolume;

    FAudio_BiomeAmbience()
    {
        BiomeName = TEXT("Default");
        CrossfadeDistance = 10000.0f;
        BaseVolume = 0.7f;
    }
};

/**
 * Spatial Audio Manager for prehistoric survival game
 * Handles 3D positioned audio, biome transitions, and distance-based attenuation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_SpatialAudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_SpatialAudioManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core audio functions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySoundAtLocation(USoundCue* SoundCue, const FVector& Location, 
        float Volume = 1.0f, float Pitch = 1.0f, bool bAutoDestroy = true);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySpatialSound(const FAudio_SpatialSound& SpatialSound);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSoundsByCategory(EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

    // Biome audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterBiomeAmbience(const FAudio_BiomeAmbience& BiomeAmbience);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdatePlayerLocation(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TransitionToBiome(const FString& BiomeName, float TransitionTime = 3.0f);

    // Dinosaur audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurSound(USoundCue* DinosaurSound, const FVector& DinosaurLocation, 
        float ThreatLevel = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstepSound(const FVector& Location, float CreatureSize = 1.0f);

    // Narration system
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayNarration(USoundCue* NarrationSound, bool bPauseOtherAudio = false);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopNarration();

    // Audio occlusion
    UFUNCTION(BlueprintCallable, Category = "Audio")
    float CalculateOcclusion(const FVector& SourceLocation, const FVector& ListenerLocation);

    // Audio priority system
    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool CanPlaySound(int32 Priority, EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CleanupFinishedSounds();

protected:
    // Category volume controls
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    // Biome management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FAudio_BiomeAmbience> RegisteredBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    FString CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    UAudioComponent* CurrentAmbienceComponent;

    // Active sounds tracking
    UPROPERTY()
    TArray<UAudioComponent*> ActiveSounds;

    // Player location for distance calculations
    UPROPERTY()
    FVector PlayerLocation;

    // Narration control
    UPROPERTY()
    UAudioComponent* NarrationComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    int32 MaxConcurrentSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float GlobalVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float OcclusionTraceDistance;

private:
    void InitializeCategoryVolumes();
    FString DetermineBiomeFromLocation(const FVector& Location);
    void UpdateAmbienceBasedOnLocation();
    void RemoveFinishedComponent(UAudioComponent* Component);
};