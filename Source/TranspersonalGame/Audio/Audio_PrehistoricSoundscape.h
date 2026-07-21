#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "GameFramework/Actor.h"
#include "Audio_PrehistoricSoundscape.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Dense Forest"),
    Plains      UMETA(DisplayName = "Open Plains"), 
    River       UMETA(DisplayName = "Riverbank"),
    Mountain    UMETA(DisplayName = "Rocky Mountains"),
    Cave        UMETA(DisplayName = "Underground Cave")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Early Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Evening     UMETA(DisplayName = "Evening"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAmbience
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> DayAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> NightAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float AttenuationRadius;

    FAudio_BiomeAmbience()
    {
        BiomeType = EAudio_BiomeType::Forest;
        BaseVolume = 0.7f;
        AttenuationRadius = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_PrehistoricSoundscape : public AActor
{
    GENERATED_BODY()

public:
    AAudio_PrehistoricSoundscape();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbienceAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Audio")
    TArray<FAudio_BiomeAmbience> BiomeAmbienceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Audio")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Audio")
    EAudio_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Audio")
    float MasterAmbienceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Audio")
    float TimeOfDayTransitionSpeed;

    // Dinosaur proximity audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> TRexApproachSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> RaptorPackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float DinosaurDetectionRadius;

public:
    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    void SetBiomeType(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    void SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    void PlayDinosaurProximityAlert(const FString& DinosaurType, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    void UpdateAmbienceVolume(float NewVolume);

private:
    void UpdateCurrentAmbience();
    void CheckDinosaurProximity();
    FAudio_BiomeAmbience* GetCurrentBiomeData();
    
    float CurrentVolumeMultiplier;
    bool bIsTransitioning;
};