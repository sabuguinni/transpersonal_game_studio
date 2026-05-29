#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "Audio_SpatialAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_AudioCategory : uint8
{
    Ambient     UMETA(DisplayName = "Ambient"),
    Dinosaur    UMETA(DisplayName = "Dinosaur"),
    Player      UMETA(DisplayName = "Player"),
    Environment UMETA(DisplayName = "Environment"),
    Music       UMETA(DisplayName = "Music"),
    UI          UMETA(DisplayName = "UI"),
    Narration   UMETA(DisplayName = "Narration"),
    Combat      UMETA(DisplayName = "Combat")
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Mountain    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundWave> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_AudioCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationRadius = 1000.0f;

    FAudio_SoundEntry()
    {
        SoundName = TEXT("");
        Category = EAudio_AudioCategory::Ambient;
        Volume = 1.0f;
        Pitch = 1.0f;
        AttenuationRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAmbience
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_SoundEntry> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeDistance = 2000.0f;

    FAudio_BiomeAmbience()
    {
        BiomeType = EAudio_BiomeType::Savana;
        BaseVolume = 0.7f;
        FadeDistance = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_SpatialAudioManager : public UObject
{
    GENERATED_BODY()

public:
    UAudio_SpatialAudioManager();

    // Core audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void InitializeAudioSystem(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySoundAtLocation(const FAudio_SoundEntry& SoundEntry, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayNarrationSound(const FString& SoundName, USoundWave* SoundWave);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSoundsInCategory(EAudio_AudioCategory Category);

    // Biome-specific audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCurrentBiome(EAudio_BiomeType NewBiome, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateBiomeAmbience(const FVector& PlayerLocation);

    // Dinosaur audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurSound(const FString& DinosaurType, const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterDinosaurActor(AActor* DinosaurActor, const FString& DinosaurType);

    // Volume control
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_AudioCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetCategoryVolume(EAudio_AudioCategory Category) const;

    // Distance-based audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    float CalculateDistanceAttenuation(const FVector& SoundLocation, const FVector& ListenerLocation, float MaxDistance) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsLocationOccluded(const FVector& SoundLocation, const FVector& ListenerLocation) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    TArray<FAudio_BiomeAmbience> BiomeAmbiences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    TMap<EAudio_AudioCategory, float> CategoryVolumes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    TMap<FString, TSoftObjectPtr<USoundWave>> DinosaurSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    TMap<FString, TSoftObjectPtr<USoundWave>> NarrationSounds;

private:
    void InitializeBiomeAmbiences();
    void InitializeDinosaurSounds();
    void InitializeNarrationSounds();
    void CleanupInactiveComponents();
    UAudioComponent* CreateAudioComponent(UWorld* World, const FVector& Location);
};