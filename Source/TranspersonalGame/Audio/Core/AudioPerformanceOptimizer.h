#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "AudioPerformanceOptimizer.generated.h"

class UAudioComponent;
class USoundBase;

UENUM(BlueprintType)
enum class EAudio_LODLevel : uint8
{
    High    UMETA(DisplayName = "High Quality"),
    Medium  UMETA(DisplayName = "Medium Quality"),
    Low     UMETA(DisplayName = "Low Quality"),
    Disabled UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveAudioSources = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 StreamingAudioSources = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AudioMemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUUsagePercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CulledAudioSources = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighQualityDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumQualityDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowQualityDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxSimultaneousAudioSources = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxStreamingAudioSources = 16;
};

/**
 * Audio Performance Optimizer Component
 * Manages audio LOD, culling, and streaming optimization for the transpersonal game
 * Ensures 60fps PC / 30fps console performance targets
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudioPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Audio Performance")
    FAudio_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Audio Performance")
    void SetLODSettings(const FAudio_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Audio Performance")
    EAudio_LODLevel CalculateAudioLOD(const FVector& ListenerLocation, const FVector& AudioSourceLocation) const;

    // Audio source management
    UFUNCTION(BlueprintCallable, Category = "Audio Performance")
    void RegisterAudioComponent(UAudioComponent* AudioComponent);

    UFUNCTION(BlueprintCallable, Category = "Audio Performance")
    void UnregisterAudioComponent(UAudioComponent* AudioComponent);

    UFUNCTION(BlueprintCallable, Category = "Audio Performance")
    void OptimizeAudioSources();

    // Streaming optimization
    UFUNCTION(BlueprintCallable, Category = "Audio Performance")
    void EnableAudioStreaming(UAudioComponent* AudioComponent, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Audio Performance")
    void PreloadCriticalAudioAssets();

    // Performance profiling
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio Performance")
    void RunPerformanceProfiler();

    UFUNCTION(BlueprintCallable, Category = "Audio Performance")
    void SetPerformanceTarget(int32 TargetFPS);

protected:
    // LOD and culling settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FAudio_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    int32 TargetFrameRate = 60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float PerformanceUpdateInterval = 0.1f;

    // Registered audio components
    UPROPERTY()
    TArray<TWeakObjectPtr<UAudioComponent>> RegisteredAudioComponents;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FAudio_PerformanceMetrics CurrentMetrics;

    UPROPERTY()
    float LastPerformanceUpdate = 0.0f;

    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    int32 MaxFrameTimeHistorySize = 60;

private:
    void UpdatePerformanceMetrics();
    void ApplyLODOptimizations();
    void CullDistantAudioSources();
    void ManageAudioStreaming();
    float CalculateAverageFrameTime() const;
    FVector GetListenerLocation() const;
    void OptimizeAudioComponentSettings(UAudioComponent* AudioComponent, EAudio_LODLevel LODLevel);
};