#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MetasoundSource.h"
#include "AudioComponent.h"
#include "AudioParameterControllerInterface.h"
#include "MetaSoundManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMetaSoundManager, Log, All);

/**
 * MetaSound Manager
 * 
 * Manages all MetaSound assets and provides a unified interface
 * for procedural audio generation and real-time parameter control.
 * 
 * This is the heart of our adaptive audio system - every sound
 * that responds to gameplay is routed through MetaSounds.
 */

UENUM(BlueprintType)
enum class EMetaSoundType : uint8
{
    AdaptiveMusic,
    DinosaurVocalization,
    EnvironmentalAmbience,
    PlayerFootsteps,
    WeatherSystem,
    TensionModulator,
    ProximityDetector,
    HeartbeatSync
};

USTRUCT(BlueprintType)
struct FMetaSoundInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMetaSoundSource* MetaSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> FloatParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, bool> BoolParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> IntParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority;

    FMetaSoundInstance()
    {
        MetaSoundAsset = nullptr;
        AudioComponent = nullptr;
        bIsActive = false;
        Priority = 1.0f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMetaSoundManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UMetaSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // MetaSound Management
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    bool CreateMetaSoundInstance(EMetaSoundType Type, const FString& InstanceName, UMetaSoundSource* MetaSoundAsset);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    bool PlayMetaSoundInstance(const FString& InstanceName, bool bLoop = false);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    bool StopMetaSoundInstance(const FString& InstanceName, float FadeOutTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    bool SetFloatParameter(const FString& InstanceName, const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    bool SetBoolParameter(const FString& InstanceName, const FString& ParameterName, bool Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    bool SetIntParameter(const FString& InstanceName, const FString& ParameterName, int32 Value);

    // Advanced Parameter Control
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    bool InterpolateFloatParameter(const FString& InstanceName, const FString& ParameterName, 
                                  float TargetValue, float InterpolationTime);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    bool TriggerMetaSoundEvent(const FString& InstanceName, const FString& TriggerName);

    // Batch Operations
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetGlobalFloatParameter(const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void StopAllMetaSoundsOfType(EMetaSoundType Type, float FadeOutTime = 1.0f);

    // Query Functions
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    bool IsMetaSoundInstanceActive(const FString& InstanceName) const;

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    float GetFloatParameter(const FString& InstanceName, const FString& ParameterName) const;

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    TArray<FString> GetActiveMetaSoundInstances() const;

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetMaxConcurrentMetaSounds(int32 MaxCount);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetCPUBudget(float MaxCPUPercentage);

protected:
    // Core MetaSound instances storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaSound")
    TMap<FString, FMetaSoundInstance> MetaSoundInstances;

    // Type-based organization
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaSound")
    TMap<EMetaSoundType, TArray<FString>> InstancesByType;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxConcurrentMetaSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxCPUPercentage;

    // Parameter interpolation tracking
    UPROPERTY()
    TMap<FString, TMap<FString, FFloatParameterInterpolation>> ActiveInterpolations;

private:
    struct FFloatParameterInterpolation
    {
        float StartValue;
        float TargetValue;
        float CurrentTime;
        float Duration;
        FString InstanceName;
        FString ParameterName;

        FFloatParameterInterpolation()
        {
            StartValue = 0.0f;
            TargetValue = 0.0f;
            CurrentTime = 0.0f;
            Duration = 1.0f;
        }
    };

    // Internal helper functions
    void UpdateParameterInterpolations(float DeltaTime);
    void ManagePerformance();
    bool ValidateMetaSoundInstance(const FString& InstanceName) const;
    void CleanupInactiveInstances();

    // Performance monitoring
    float CurrentCPUUsage;
    int32 ActiveInstanceCount;
    FTimerHandle PerformanceCheckTimer;
};