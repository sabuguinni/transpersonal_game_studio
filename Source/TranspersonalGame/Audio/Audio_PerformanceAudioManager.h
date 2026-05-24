#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "AudioDevice.h"
#include "SharedTypes.h"
#include "Audio_PerformanceAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_PerformanceTier : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Minimal     UMETA(DisplayName = "Minimal Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_PerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Performance")
    int32 MaxSimultaneousAudioSources = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Performance")
    float AudioCullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Performance")
    float EnvironmentalAudioUpdateRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Performance")
    bool bEnableAudioOcclusion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Performance")
    bool bEnableReverbZones = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Performance")
    int32 MaxConcurrentFootstepSounds = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Performance")
    float DinosaurAudioLODDistance = 2000.0f;

    FAudio_PerformanceSettings()
    {
        MaxSimultaneousAudioSources = 64;
        AudioCullDistance = 5000.0f;
        EnvironmentalAudioUpdateRate = 0.1f;
        bEnableAudioOcclusion = true;
        bEnableReverbZones = true;
        MaxConcurrentFootstepSounds = 8;
        DinosaurAudioLODDistance = 2000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    float HighQualityDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    float MediumQualityDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    float LowQualityDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio LOD")
    float CullDistance = 5000.0f;

    FAudio_LODSettings()
    {
        HighQualityDistance = 500.0f;
        MediumQualityDistance = 1500.0f;
        LowQualityDistance = 3000.0f;
        CullDistance = 5000.0f;
    }
};

/**
 * Performance-optimized audio manager for prehistoric survival game
 * Handles audio LOD, culling, and performance scaling for 60fps target
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_PerformanceAudioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_PerformanceAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EAudio_PerformanceTier PerformanceTier = EAudio_PerformanceTier::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FAudio_PerformanceSettings PerformanceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FAudio_LODSettings LODSettings;

    // Audio Source Management
    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 ActiveAudioSources = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 CulledAudioSources = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float AudioPerformanceScore = 1.0f;

    // Core Performance Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Performance")
    void SetPerformanceTier(EAudio_PerformanceTier NewTier);

    UFUNCTION(BlueprintCallable, Category = "Audio Performance")
    void OptimizeAudioForFramerate(float TargetFramerate = 60.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Performance")
    void CullDistantAudioSources(const FVector& ListenerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Performance")
    float CalculateAudioLOD(const FVector& SourceLocation, const FVector& ListenerLocation);

    // Audio Source Registration
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    bool RegisterAudioSource(UAudioComponent* AudioComponent, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UnregisterAudioSource(UAudioComponent* AudioComponent);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateAudioSourceLOD(UAudioComponent* AudioComponent, float Distance);

    // Dinosaur-Specific Audio Optimization
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void OptimizeDinosaurAudio(const FVector& DinosaurLocation, float DinosaurSize);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    bool ShouldPlayDinosaurFootstep(const FVector& FootstepLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void ManageDinosaurAmbientSounds(float DeltaTime);

    // Environmental Audio Performance
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateEnvironmentalAudio(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void OptimizeWindAndWeatherSounds();

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void CullInaudibleEnvironmentalSources();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    float GetAudioPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void LogAudioPerformanceStats();

    UFUNCTION(BlueprintPure, Category = "Performance Monitoring")
    bool IsAudioPerformanceOptimal() const;

private:
    // Internal performance tracking
    TArray<TWeakObjectPtr<UAudioComponent>> RegisteredAudioSources;
    TMap<UAudioComponent*, float> AudioSourcePriorities;
    TMap<UAudioComponent*, float> AudioSourceDistances;
    
    float LastPerformanceCheck = 0.0f;
    float PerformanceCheckInterval = 1.0f;
    
    int32 FramesSinceLastOptimization = 0;
    float AverageFrameTime = 0.0167f; // 60fps baseline
    
    // Internal optimization functions
    void UpdatePerformanceSettings();
    void ApplyPerformanceTierSettings();
    void CleanupInvalidAudioSources();
    void PrioritizeAudioSources(const FVector& ListenerLocation);
    float CalculateAudioPriority(UAudioComponent* AudioComponent, float Distance);
    void ApplyAudioLODToComponent(UAudioComponent* AudioComponent, float LODLevel);
};