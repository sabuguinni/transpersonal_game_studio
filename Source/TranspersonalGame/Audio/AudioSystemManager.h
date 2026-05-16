#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "AudioSystemManager.generated.h"

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> MusicCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float CrossfadeTime;

    FAudio_BiomeAudioData()
    {
        BiomeType = EAudio_BiomeType::Savana;
        AmbientVolume = 0.7f;
        MusicVolume = 0.5f;
        CrossfadeTime = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DangerAudioSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> DinosaurRoarCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> FootstepsCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> TensionMusicCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DangerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float IntensityMultiplier;

    FAudio_DangerAudioSettings()
    {
        DangerRadius = 5000.0f;
        IntensityMultiplier = 1.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* DangerAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TArray<FAudio_BiomeAudioData> BiomeAudioSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FAudio_DangerAudioSettings DangerSettings;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bInDangerZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float BiomeDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AudioUpdateInterval;

private:
    FTimerHandle AudioUpdateTimer;
    class APawn* PlayerPawn;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetDangerState(bool bDangerous);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurRoar(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayFootstepAudio(FVector Location, bool bHeavyFootsteps = false);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    EAudio_BiomeType DetectBiomeAtLocation(FVector Location);

    UFUNCTION(BlueprintImplementableEvent, Category = "Audio System")
    void OnBiomeChanged(EAudio_BiomeType OldBiome, EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintImplementableEvent, Category = "Audio System")
    void OnDangerStateChanged(bool bNewDangerState);

private:
    UFUNCTION()
    void UpdateAudioSystem();

    void CrossfadeToNewBiome(EAudio_BiomeType NewBiome);
    void UpdateDangerAudio();
    float CalculateDistanceToNearestDinosaur();
};

#include "AudioSystemManager.generated.h"