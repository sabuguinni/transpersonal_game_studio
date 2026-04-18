#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "GameFramework/GameStateBase.h"
#include "AudioLODManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_LODLevel : uint8
{
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Minimal     UMETA(DisplayName = "Minimal Quality")
};

USTRUCT(BlueprintType)
struct FAudio_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    int32 MaxSimultaneousSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    float MaxAudibleDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    bool bEnableReverb;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    bool bEnableOcclusion;

    FAudio_LODSettings()
    {
        MaxSimultaneousSounds = 32;
        MaxAudibleDistance = 5000.0f;
        VolumeMultiplier = 1.0f;
        bEnableReverb = true;
        bEnableOcclusion = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioLODManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioLODManager();

    UFUNCTION(BlueprintCallable, Category = "Audio LOD")
    void InitializeLODSystem(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Audio LOD")
    void SetLODLevel(EAudio_LODLevel NewLODLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio LOD")
    EAudio_LODLevel GetCurrentLODLevel() const { return CurrentLODLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio LOD")
    void UpdateAudioLOD(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Audio LOD")
    void RegisterAudioComponent(UAudioComponent* AudioComp);

    UFUNCTION(BlueprintCallable, Category = "Audio LOD")
    void UnregisterAudioComponent(UAudioComponent* AudioComp);

    UFUNCTION(BlueprintCallable, Category = "Audio LOD")
    void SetPerformanceMode(bool bLowPerformanceMode);

    UFUNCTION(BlueprintCallable, Category = "Audio LOD")
    float GetCurrentAudioLoad() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    EAudio_LODLevel CurrentLODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    TMap<EAudio_LODLevel, FAudio_LODSettings> LODSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Audio LOD")
    TArray<TWeakObjectPtr<UAudioComponent>> RegisteredAudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    float LODUpdateInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Audio LOD")
    float TimeSinceLastLODUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    bool bAutoLODEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    float PerformanceThreshold;

    void InitializeLODSettings();
    void ApplyLODSettings();
    void CullDistantAudioSources();
    void OptimizeActiveAudioSources();
    EAudio_LODLevel DetermineOptimalLODLevel();
    float CalculateAudioComplexity() const;
};