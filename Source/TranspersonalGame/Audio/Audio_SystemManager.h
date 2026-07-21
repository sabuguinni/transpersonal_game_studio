#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Audio_SystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicLayer : uint8
{
    Ambient     UMETA(DisplayName = "Ambient"),
    Tension     UMETA(DisplayName = "Tension"),
    Combat      UMETA(DisplayName = "Combat"),
    Exploration UMETA(DisplayName = "Exploration"),
    Stealth     UMETA(DisplayName = "Stealth")
};

UENUM(BlueprintType)
enum class EAudio_SFXCategory : uint8
{
    Footsteps   UMETA(DisplayName = "Footsteps"),
    Environment UMETA(DisplayName = "Environment"),
    Dinosaurs   UMETA(DisplayName = "Dinosaurs"),
    UI          UMETA(DisplayName = "UI"),
    Combat      UMETA(DisplayName = "Combat"),
    Crafting    UMETA(DisplayName = "Crafting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_MusicLayer CurrentLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float TensionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float FadeTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    bool bIsTransitioning;

    FAudio_MusicState()
    {
        CurrentLayer = EAudio_MusicLayer::Ambient;
        TensionLevel = 0.0f;
        FadeTime = 2.0f;
        bIsTransitioning = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SpatialConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
    float MaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
    float AttenuationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
    bool bUseOcclusion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
    float OcclusionStrength;

    FAudio_SpatialConfig()
    {
        MaxDistance = 5000.0f;
        AttenuationRadius = 1000.0f;
        bUseOcclusion = true;
        OcclusionStrength = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_SystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Music System
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetMusicLayer(EAudio_MusicLayer NewLayer, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetTensionLevel(float TensionValue);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void PlayStinger(USoundCue* StingerSound);

    // SFX System
    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    UAudioComponent* PlaySFXAtLocation(USoundWave* Sound, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    UAudioComponent* PlaySFXAttached(USoundWave* Sound, USceneComponent* AttachComponent, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void StopAllSFXOfCategory(EAudio_SFXCategory Category);

    // Footstep System
    UFUNCTION(BlueprintCallable, Category = "Audio|Footsteps")
    void PlayFootstepSound(FVector Location, float Weight, bool bIsRunning);

    UFUNCTION(BlueprintCallable, Category = "Audio|Footsteps")
    void RegisterFootstepSurface(const FString& SurfaceType, USoundWave* WalkSound, USoundWave* RunSound);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void PlayEnvironmentalLoop(USoundWave* LoopSound, FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void StopEnvironmentalLoop(USoundWave* LoopSound);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    FAudio_MusicState MusicState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Config")
    FAudio_SpatialConfig SpatialConfig;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    TMap<EAudio_SFXCategory, TArray<UAudioComponent*>> CategoryComponents;

    UPROPERTY()
    TMap<FString, USoundWave*> FootstepWalkSounds;

    UPROPERTY()
    TMap<FString, USoundWave*> FootstepRunSounds;

    UPROPERTY()
    TArray<UAudioComponent*> EnvironmentalLoops;

private:
    void UpdateMusicLayers();
    void CleanupFinishedComponents();
    FString GetSurfaceTypeAtLocation(FVector Location);
};