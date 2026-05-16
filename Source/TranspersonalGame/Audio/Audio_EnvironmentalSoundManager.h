#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_EnvironmentalSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savanna     UMETA(DisplayName = "Savanna"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<class USoundCue*> RandomSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float RandomSoundInterval = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.7f;

    FAudio_BiomeAudioData()
    {
        AmbientLoop = nullptr;
        RandomSoundInterval = 15.0f;
        AmbientVolume = 0.7f;
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
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* RandomSoundComponent;

    // Biome audio configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioData> BiomeAudioMap;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BiomeDetectionRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MasterVolume = 1.0f;

    // Audio management functions
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayRandomBiomeSound();

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateAmbientAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    EAudio_BiomeType DetectBiomeFromLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetMasterVolume(float NewVolume);

private:
    FTimerHandle RandomSoundTimer;
    
    void InitializeBiomeAudioData();
    void StartRandomSoundTimer();
    void StopRandomSoundTimer();
    
    // Biome detection helpers
    bool IsInSavanna(FVector Location);
    bool IsInSwamp(FVector Location);
    bool IsInForest(FVector Location);
    bool IsInDesert(FVector Location);
    bool IsInMountain(FVector Location);
};

#include "Audio_EnvironmentalSoundManager.generated.h"