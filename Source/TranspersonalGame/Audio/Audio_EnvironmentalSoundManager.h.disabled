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
    Savana      UMETA(DisplayName = "Savana"),
    Pantano     UMETA(DisplayName = "Pantano"), 
    Floresta    UMETA(DisplayName = "Floresta"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Montanha    UMETA(DisplayName = "Montanha")
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
    float BaseVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeDistance = 5000.0f;

    FAudio_BiomeAudioData()
    {
        BaseVolume = 0.5f;
        FadeDistance = 5000.0f;
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

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdatePlayerLocation(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetWeatherIntensity(float Intensity);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* WindAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* CreatureAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioData> BiomeAudioMap;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FVector PlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float WeatherIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float BiomeTransitionSpeed = 2.0f;

private:
    UFUNCTION()
    void TransitionToBiomeAudio();

    UFUNCTION()
    void UpdateAudioLevels();

    FTimerHandle TransitionTimerHandle;
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
};