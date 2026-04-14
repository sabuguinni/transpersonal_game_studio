#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../AudioTypes.h"
#include "AudioLODManager.generated.h"

class AActor;
class UMetaSoundSource;

UENUM(BlueprintType)
enum class EAudio_LODLevel : uint8
{
    Highest = 0,    // Full quality, all layers
    High = 1,       // High quality, most layers
    Medium = 2,     // Medium quality, essential layers
    Low = 3,        // Low quality, minimal layers
    Lowest = 4      // Minimal quality, only critical audio
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    int32 MaxConcurrentSounds = 32;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    bool bEnableReverb = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    bool bEnableOcclusion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    bool bEnableSpatialization = true;

    FAudio_LODSettings()
    {
        MaxDistance = 5000.0f;
        VolumeMultiplier = 1.0f;
        MaxConcurrentSounds = 32;
        bEnableReverb = true;
        bEnableOcclusion = true;
        bEnableSpatialization = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveAudioSources = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CulledAudioSources = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AudioMemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AudioCPUUsagePercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageLatencyMS = 0.0f;

    FAudio_PerformanceMetrics()
    {
        ActiveAudioSources = 0;
        CulledAudioSources = 0;
        AudioMemoryUsageMB = 0.0f;
        AudioCPUUsagePercent = 0.0f;
        AverageLatencyMS = 0.0f;
    }
};

/**
 * Audio LOD Manager - Optimizes audio performance through distance-based LOD system
 * Manages audio quality, culling, and resource allocation based on player distance and performance metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioLODManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioLODManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Audio LOD")
    void UpdateAudioLOD(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Audio LOD")
    EAudio_LODLevel CalculateLODLevel(AActor* AudioSource, const FVector& ListenerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Audio LOD")
    void SetAudioLODLevel(UAudioComponent* AudioComponent, EAudio_LODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio LOD")
    void RegisterAudioComponent(UAudioComponent* AudioComponent, EAudio_SourceType SourceType);

    UFUNCTION(BlueprintCallable, Category = "Audio LOD")
    void UnregisterAudioComponent(UAudioComponent* AudioComponent);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FAudio_PerformanceMetrics GetPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForPerformance();

    // Distance Culling
    UFUNCTION(BlueprintCallable, Category = "Culling")
    void UpdateDistanceCulling(const FVector& ListenerLocation);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    bool ShouldCullAudioSource(const FVector& SourceLocation, const FVector& ListenerLocation, EAudio_SourceType SourceType) const;

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetLODSettings(EAudio_LODLevel LODLevel, const FAudio_LODSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    FAudio_LODSettings GetLODSettings(EAudio_LODLevel LODLevel) const;

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetGlobalAudioQuality(EAudio_LODLevel Quality);

protected:
    // LOD Settings for each level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    TMap<EAudio_LODLevel, FAudio_LODSettings> LODSettings;

    // Registered audio components
    UPROPERTY()
    TMap<UAudioComponent*, EAudio_SourceType> RegisteredComponents;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FAudio_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxAudioCPUBudget = 15.0f; // Percentage

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxAudioMemoryBudgetMB = 256.0f;

    // Update frequency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateFrequency = 0.1f; // Update every 100ms

    UPROPERTY()
    float LastLODUpdateTime = 0.0f;

private:
    void InitializeLODSettings();
    void UpdatePerformanceMetrics();
    void ApplyPerformanceOptimizations();
    float CalculateDistanceFactor(float Distance, float MaxDistance) const;
    void CullDistantAudioSources(const FVector& ListenerLocation);
};