// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTags.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"
#include "MetasoundSource.h"
#include "Components/AudioComponent.h"
#include "MetaSoundManager.generated.h"

class UAudioSubsystem;
class UMetasoundParameterPack;

UENUM(BlueprintType)
enum class EMetaSoundType : uint8
{
    Ambient,            // Environmental ambience
    Music,              // Adaptive music layers
    SFX,                // Sound effects
    Creature,           // Dinosaur vocalizations
    Weather,            // Weather and climate
    Vegetation,         // Plant life sounds
    Geological,         // Earth and stone sounds
    Water,              // Water-based sounds
    Temporal,           // Time-related effects
    UI,                 // Interface sounds
    Foley,              // Movement and interaction
    Synthesis,          // Procedural generation
    Processing,         // Real-time effects
    Spatial,            // 3D positioning
    Interactive         // Player-responsive
};

UENUM(BlueprintType)
enum class EMetaSoundParameterType : uint8
{
    Float,              // Floating point value
    Int,                // Integer value
    Bool,               // Boolean value
    Trigger,            // Event trigger
    String,             // String value
    Object,             // Object reference
    Audio,              // Audio buffer
    Time                // Time value
};

USTRUCT(BlueprintType)
struct FMetaSoundParameter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    FString ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    EMetaSoundParameterType ParameterType = EMetaSoundParameterType::Float;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    float FloatValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    int32 IntValue = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    bool BoolValue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    FString StringValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    float MinValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    float MaxValue = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    float DefaultValue = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    bool bIsModulatable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
    FString Description;

    FMetaSoundParameter()
    {
        ParameterName = TEXT("");
        ParameterType = EMetaSoundParameterType::Float;
        FloatValue = 0.0f;
        IntValue = 0;
        BoolValue = false;
        StringValue = TEXT("");
        MinValue = 0.0f;
        MaxValue = 1.0f;
        DefaultValue = 0.5f;
        bIsModulatable = true;
        Description = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FMetaSoundConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FString ConfigurationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EMetaSoundType SoundType = EMetaSoundType::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TSoftObjectPtr<UMetaSoundSource> MetaSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FMetaSoundParameter> Parameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FGameplayTag> Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float BasePitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bIs3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 Priority = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FString Description;

    FMetaSoundConfiguration()
    {
        ConfigurationID = TEXT("");
        SoundType = EMetaSoundType::Ambient;
        BaseVolume = 1.0f;
        BasePitch = 1.0f;
        bIs3D = true;
        bLooping = true;
        Priority = 100;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
        MaxDistance = 10000.0f;
        Description = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FMetaSoundInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    FString InstanceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    FString ConfigurationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    UAudioComponent* AudioComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    TMap<FString, float> CurrentParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    float Duration = -1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    bool bIsFadingIn = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    bool bIsFadingOut = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
    float FadeProgress = 0.0f;

    FMetaSoundInstance()
    {
        InstanceID = TEXT("");
        ConfigurationID = TEXT("");
        AudioComponent = nullptr;
        Location = FVector::ZeroVector;
        StartTime = 0.0f;
        Duration = -1.0f;
        bIsActive = false;
        bIsFadingIn = false;
        bIsFadingOut = false;
        FadeProgress = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FMetaSoundModulation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modulation")
    FString ModulationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modulation")
    FString TargetParameter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modulation")
    FString SourceParameter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modulation")
    float ModulationAmount = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modulation")
    float ModulationRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modulation")
    bool bBipolar = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modulation")
    bool bIsActive = true;

    FMetaSoundModulation()
    {
        ModulationID = TEXT("");
        TargetParameter = TEXT("");
        SourceParameter = TEXT("");
        ModulationAmount = 1.0f;
        ModulationRate = 1.0f;
        bBipolar = false;
        bIsActive = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMetaSoundInstanceStarted, const FString&, InstanceID, const FString&, ConfigurationID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMetaSoundInstanceFinished, const FString&, InstanceID, const FString&, ConfigurationID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMetaSoundParameterChanged, const FString&, InstanceID, const FString&, ParameterName, float, NewValue);

/**
 * MetaSound Manager - Manages MetaSound instances and parameters
 * Provides high-level interface for procedural audio and real-time synthesis
 * Handles parameter modulation and sample-accurate control
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UMetaSoundManager : public UObject
{
    GENERATED_BODY()

public:
    UMetaSoundManager();

    // Initialization
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void Initialize(UAudioSubsystem* InAudioSubsystem);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void Shutdown();

    // Configuration Management
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void RegisterConfiguration(const FMetaSoundConfiguration& Configuration);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void UnregisterConfiguration(const FString& ConfigurationID);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    bool HasConfiguration(const FString& ConfigurationID) const;

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    FMetaSoundConfiguration GetConfiguration(const FString& ConfigurationID) const;

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    TArray<FString> GetConfigurationsByType(EMetaSoundType SoundType) const;

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    TArray<FString> GetConfigurationsByTag(const FGameplayTag& Tag) const;

    // Instance Management
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    FString CreateInstance(const FString& ConfigurationID, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void DestroyInstance(const FString& InstanceID);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void DestroyAllInstances();

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void DestroyInstancesByConfiguration(const FString& ConfigurationID);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void DestroyInstancesByType(EMetaSoundType SoundType);

    // Playback Control
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void PlayInstance(const FString& InstanceID);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void StopInstance(const FString& InstanceID, float FadeOutTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void PauseInstance(const FString& InstanceID);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void ResumeInstance(const FString& InstanceID);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    bool IsInstanceActive(const FString& InstanceID) const;

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    bool IsInstancePlaying(const FString& InstanceID) const;

    // Parameter Control
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetInstanceParameter(const FString& InstanceID, const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetInstanceParameterInt(const FString& InstanceID, const FString& ParameterName, int32 Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetInstanceParameterBool(const FString& InstanceID, const FString& ParameterName, bool Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetInstanceParameterString(const FString& InstanceID, const FString& ParameterName, const FString& Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void TriggerInstanceParameter(const FString& InstanceID, const FString& TriggerName);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    float GetInstanceParameter(const FString& InstanceID, const FString& ParameterName) const;

    // Batch Parameter Control
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetMultipleParameters(const FString& InstanceID, const TMap<FString, float>& Parameters);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetParameterForAllInstances(const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetParameterForInstancesByType(EMetaSoundType SoundType, const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetParameterForInstancesByTag(const FGameplayTag& Tag, const FString& ParameterName, float Value);

    // Modulation System
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void AddModulation(const FString& InstanceID, const FMetaSoundModulation& Modulation);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void RemoveModulation(const FString& InstanceID, const FString& ModulationID);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetModulationAmount(const FString& InstanceID, const FString& ModulationID, float Amount);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetModulationRate(const FString& InstanceID, const FString& ModulationID, float Rate);

    // Spatial Audio
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetInstanceLocation(const FString& InstanceID, const FVector& NewLocation);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetInstanceAttenuation(const FString& InstanceID, float MaxDistance, float FalloffDistance);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetInstanceSpatialization(const FString& InstanceID, bool bEnable3D);

    // Volume and Pitch
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetInstanceVolume(const FString& InstanceID, float Volume, float FadeTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetInstancePitch(const FString& InstanceID, float Pitch);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void FadeInstanceVolume(const FString& InstanceID, float TargetVolume, float FadeTime);

    // Performance and Optimization
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetMaxConcurrentInstances(int32 MaxInstances);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void CullDistantInstances(const FVector& ListenerLocation, float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetInstancePriority(const FString& InstanceID, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    int32 GetActiveInstanceCount() const;

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    int32 GetActiveInstanceCountByType(EMetaSoundType SoundType) const;

    // Query Functions
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    TArray<FString> GetActiveInstances() const;

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    TArray<FString> GetInstancesByConfiguration(const FString& ConfigurationID) const;

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    TArray<FString> GetInstancesByType(EMetaSoundType SoundType) const;

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    FMetaSoundInstance GetInstanceData(const FString& InstanceID) const;

    // Debug and Analysis
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void DebugPrintInstances() const;

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void DebugPrintConfiguration(const FString& ConfigurationID) const;

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetDebugVisualization(bool bEnabled);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "MetaSound|Events")
    FOnMetaSoundInstanceStarted OnInstanceStarted;

    UPROPERTY(BlueprintAssignable, Category = "MetaSound|Events")
    FOnMetaSoundInstanceFinished OnInstanceFinished;

    UPROPERTY(BlueprintAssignable, Category = "MetaSound|Events")
    FOnMetaSoundParameterChanged OnParameterChanged;

protected:
    // Internal management
    void UpdateInstances(float DeltaTime);
    void UpdateModulations(float DeltaTime);
    void ProcessFades(float DeltaTime);
    void ManageInstanceLifetime();
    void OptimizePerformance();

    // Audio component management
    UAudioComponent* CreateAudioComponent(const FMetaSoundConfiguration& Configuration, const FVector& Location);
    void UpdateAudioComponent(UAudioComponent* Component, const FMetaSoundInstance& Instance);
    void ReleaseAudioComponent(UAudioComponent* Component);

    // Parameter management
    void ApplyParametersToComponent(UAudioComponent* Component, const TMap<FString, float>& Parameters);
    void UpdateModulationForInstance(FMetaSoundInstance& Instance, float DeltaTime);

private:
    // Configuration storage
    UPROPERTY()
    TMap<FString, FMetaSoundConfiguration> RegisteredConfigurations;

    // Instance management
    UPROPERTY()
    TMap<FString, FMetaSoundInstance> ActiveInstances;

    UPROPERTY()
    TArray<UAudioComponent*> PooledAudioComponents;

    UPROPERTY()
    int32 NextInstanceID = 0;

    UPROPERTY()
    int32 MaxConcurrentInstances = 64;

    // Modulation system
    UPROPERTY()
    TMap<FString, TArray<FMetaSoundModulation>> InstanceModulations;

    // Performance tracking
    UPROPERTY()
    float LastUpdateTime = 0.0f;

    UPROPERTY()
    float LastOptimizationTime = 0.0f;

    UPROPERTY()
    bool bDebugVisualizationEnabled = false;

    // Subsystem reference
    UPROPERTY()
    UAudioSubsystem* AudioSubsystem = nullptr;

    // Timer handles
    FTimerHandle UpdateTimer;
    FTimerHandle OptimizationTimer;
};