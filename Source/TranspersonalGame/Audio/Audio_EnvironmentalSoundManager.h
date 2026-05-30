#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "Audio_EnvironmentalSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savana = 0,
    Pantano = 1,
    Floresta = 2,
    Deserto = 3,
    Montanha = 4
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> WindSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> CreatureSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BaseVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeDistance = 5000.0f;

    FAudio_BiomeAudioData()
    {
        BaseVolume = 0.8f;
        FadeDistance = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> MovementSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> AggressiveSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float AudioRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float VolumeMultiplier = 1.0f;

    FAudio_DinosaurAudioData()
    {
        AudioRange = 3000.0f;
        VolumeMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_EnvironmentalSoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_EnvironmentalSoundManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Biome audio management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioData> BiomeAudioMap;

    // Dinosaur audio management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TMap<FString, FAudio_DinosaurAudioData> DinosaurAudioMap;

    // Audio components for biome sounds
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* CurrentBiomeAmbient;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* CurrentBiomeWind;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* CurrentBiomeCreatures;

    // Master volume controls
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Control", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterEnvironmentalVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Control", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterDinosaurVolume = 0.8f;

    // Current biome tracking
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::Savana;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateBiomeAudio(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    EAudio_BiomeType GetBiomeFromLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location, float VolumeOverride = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Volume Control")
    void SetMasterEnvironmentalVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Volume Control")
    void SetMasterDinosaurVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void FadeBetweenBiomes(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome, float FadeTime = 2.0f);

private:
    // Internal functions
    void InitializeBiomeAudioData();
    void InitializeDinosaurAudioData();
    void UpdateVolumeBasedOnDistance(UAudioComponent* AudioComp, float Distance, float MaxDistance);

    // Fade state
    bool bIsFading = false;
    float FadeTimer = 0.0f;
    float FadeDuration = 2.0f;
    EAudio_BiomeType FadeFromBiome;
    EAudio_BiomeType FadeToBiome;
};