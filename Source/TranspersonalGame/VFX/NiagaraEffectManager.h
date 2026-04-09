// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "GameplayTags.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterface.h"
#include "NiagaraParameterStore.h"
#include "VFXSubsystem.h"
#include "NiagaraEffectManager.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UNiagaraParameterStore;
class UVFXSubsystem;

UENUM(BlueprintType)
enum class ENiagaraEffectLOD : uint8
{
    Disabled,          // Effect completely disabled
    Low,               // Minimal particles, simple shaders
    Medium,            // Standard quality
    High,              // Enhanced quality
    Ultra              // Maximum quality, all features
};

UENUM(BlueprintType)
enum class ENiagaraEffectPriority : uint8
{
    Background,        // Low priority, can be culled easily
    Standard,          // Normal priority
    Important,         // High priority, rarely culled
    Critical,          // Never culled, always visible
    UI                 // UI effects, highest priority
};

USTRUCT(BlueprintType)
struct FNiagaraEffectLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    ENiagaraEffectLOD LODLevel = ENiagaraEffectLOD::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float ParticleCountMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float EmissionRateMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LifetimeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float SizeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DistanceThreshold = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableComplexShading = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableGPUSimulation = true;

    FNiagaraEffectLODSettings()
    {
        LODLevel = ENiagaraEffectLOD::Medium;
        ParticleCountMultiplier = 1.0f;
        EmissionRateMultiplier = 1.0f;
        LifetimeMultiplier = 1.0f;
        SizeMultiplier = 1.0f;
        DistanceThreshold = 5000.0f;
        bEnableCollision = true;
        bEnableComplexShading = true;
        bEnableVolumetricFog = true;
        bEnableGPUSimulation = true;
    }
};

USTRUCT(BlueprintType)
struct FNiagaraEffectInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    FString InstanceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    UNiagaraComponent* NiagaraComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    EVFXCategory Category = EVFXCategory::Environmental;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    ENiagaraEffectPriority Priority = ENiagaraEffectPriority::Standard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    FNiagaraEffectLODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    FVector SpawnScale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    AActor* AttachedActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    FName AttachSocketName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    float Duration = -1.0f; // -1 = infinite

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    bool bPaused = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    TMap<FString, float> FloatParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    TMap<FString, FVector> VectorParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    TMap<FString, FLinearColor> ColorParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Instance")
    TMap<FString, bool> BoolParameters;

    FNiagaraEffectInstance()
    {
        InstanceID = TEXT("");
        NiagaraComponent = nullptr;
        Category = EVFXCategory::Environmental;
        Priority = ENiagaraEffectPriority::Standard;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        SpawnScale = FVector::OneVector;
        AttachedActor = nullptr;
        AttachSocketName = NAME_None;
        StartTime = 0.0f;
        Duration = -1.0f;
        bAutoDestroy = true;
        bPaused = false;
    }
};

USTRUCT(BlueprintType)
struct FNiagaraEffectPool
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Pool")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Pool")
    TArray<UNiagaraComponent*> AvailableComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Pool")
    TArray<UNiagaraComponent*> ActiveComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Pool")
    int32 PoolSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Pool")
    int32 MaxPoolSize = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Pool")
    bool bAutoExpand = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Pool")
    float LastUsedTime = 0.0f;

    FNiagaraEffectPool()
    {
        PoolSize = 10;
        MaxPoolSize = 50;
        bAutoExpand = true;
        LastUsedTime = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNiagaraEffectSpawned, const FString&, InstanceID, UNiagaraComponent*, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNiagaraEffectFinished, const FString&, InstanceID, UNiagaraComponent*, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNiagaraEffectLODChanged, const FString&, InstanceID, ENiagaraEffectLOD, OldLOD, ENiagaraEffectLOD, NewLOD);

/**
 * Niagara Effect Manager - Specialized manager for Niagara particle systems
 * 
 * This manager handles:
 * - Niagara system spawning and lifecycle management
 * - Performance optimization through LOD and pooling
 * - Parameter management for dynamic effects
 * - Integration with VFX Subsystem
 * - Specialized prehistoric world effects (weather, creatures, mystical)
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNiagaraEffectManager : public UObject
{
    GENERATED_BODY()

public:
    UNiagaraEffectManager();

    // Initialization
    UFUNCTION(BlueprintCallable, Category = "Niagara Manager")
    void Initialize(UWorld* InWorld, UVFXSubsystem* InVFXSubsystem);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager")
    void Shutdown();

    // Effect Management
    UFUNCTION(BlueprintCallable, Category = "Niagara Manager")
    FString SpawnNiagaraEffect(const FVFXEffectData& EffectData, AActor* AttachActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager")
    bool StopNiagaraEffect(const FString& InstanceID, bool bFadeOut = true);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager")
    bool PauseNiagaraEffect(const FString& InstanceID);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager")
    bool ResumeNiagaraEffect(const FString& InstanceID);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager")
    UNiagaraComponent* GetNiagaraComponent(const FString& InstanceID);

    // Parameter Management
    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Parameters")
    void SetFloatParameter(const FString& InstanceID, const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Parameters")
    void SetVectorParameter(const FString& InstanceID, const FString& ParameterName, const FVector& Value);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Parameters")
    void SetColorParameter(const FString& InstanceID, const FString& ParameterName, const FLinearColor& Value);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Parameters")
    void SetBoolParameter(const FString& InstanceID, const FString& ParameterName, bool Value);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Parameters")
    void SetActorParameter(const FString& InstanceID, const FString& ParameterName, AActor* Actor);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|LOD")
    void SetEffectLOD(const FString& InstanceID, ENiagaraEffectLOD LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|LOD")
    void UpdateLODForDistance(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|LOD")
    void SetGlobalLODLevel(ENiagaraEffectLOD LODLevel);

    // Pooling
    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Pooling")
    void InitializeEffectPool(UNiagaraSystem* NiagaraSystem, int32 PoolSize = 10);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Pooling")
    UNiagaraComponent* GetPooledComponent(UNiagaraSystem* NiagaraSystem);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Pooling")
    void ReturnToPool(UNiagaraComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Pooling")
    void CleanupUnusedPools(float MaxIdleTime = 60.0f);

    // Performance
    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Performance")
    void SetMaxActiveEffects(int32 MaxEffects);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Performance")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Performance")
    void CullDistantEffects(const FVector& ViewerLocation, float CullingDistance);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Performance")
    void SetPerformanceMode(bool bLowPerformanceMode);

    // Specialized Effects
    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Specialized")
    FString CreateWeatherEffect(const FString& WeatherType, float Intensity, const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Specialized")
    FString CreateCreatureBreathEffect(AActor* CreatureActor, const FVector& MouthLocation, const FString& BreathType);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Specialized")
    FString CreateFootstepEffect(const FVector& FootLocation, const FString& SurfaceType, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Specialized")
    FString CreateMysticalEffect(const FString& EffectType, const FVector& Location, float Intensity, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Niagara Manager|Specialized")
    FString CreateTemporalDistortion(const FVector& EpicenterLocation, float Radius, float Intensity);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Niagara Manager|Events")
    FOnNiagaraEffectSpawned OnNiagaraEffectSpawned;

    UPROPERTY(BlueprintAssignable, Category = "Niagara Manager|Events")
    FOnNiagaraEffectFinished OnNiagaraEffectFinished;

    UPROPERTY(BlueprintAssignable, Category = "Niagara Manager|Events")
    FOnNiagaraEffectLODChanged OnNiagaraEffectLODChanged;

protected:
    // Core state
    UPROPERTY(BlueprintReadOnly, Category = "Niagara Manager")
    TMap<FString, FNiagaraEffectInstance> ActiveEffects;

    UPROPERTY(BlueprintReadOnly, Category = "Niagara Manager")
    TMap<UNiagaraSystem*, FNiagaraEffectPool> EffectPools;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|Config")
    int32 MaxActiveEffects = 200;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|Config")
    ENiagaraEffectLOD GlobalLODLevel = ENiagaraEffectLOD::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|Config")
    float DefaultCullingDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|Config")
    bool bEnablePooling = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|Config")
    bool bEnableAutomaticLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|Config")
    bool bLowPerformanceMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|Config")
    float LODUpdateFrequency = 0.5f; // seconds

    // LOD Distance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|LOD")
    float LowLODDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|LOD")
    float MediumLODDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|LOD")
    float HighLODDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|LOD")
    float UltraLODDistance = 500.0f;

    // Effect Libraries
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|Libraries")
    TMap<FString, TSoftObjectPtr<UNiagaraSystem>> WeatherEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|Libraries")
    TMap<FString, TSoftObjectPtr<UNiagaraSystem>> CreatureEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|Libraries")
    TMap<FString, TSoftObjectPtr<UNiagaraSystem>> MysticalEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara Manager|Libraries")
    TMap<FString, TSoftObjectPtr<UNiagaraSystem>> EnvironmentalEffects;

    // System references
    UPROPERTY()
    UWorld* World;

    UPROPERTY()
    UVFXSubsystem* VFXSubsystem;

private:
    // Internal methods
    void LoadEffectLibraries();
    void InitializeDefaultPools();
    UNiagaraComponent* CreateNiagaraComponent(UNiagaraSystem* NiagaraSystem);
    void ApplyLODSettings(UNiagaraComponent* Component, const FNiagaraEffectLODSettings& LODSettings);
    void UpdateEffectParameters(FNiagaraEffectInstance& EffectInstance);
    FString GenerateUniqueInstanceID();
    void CleanupFinishedEffects();
    ENiagaraEffectLOD CalculateLODForDistance(float Distance) const;
    void OnEffectFinished(UNiagaraComponent* Component);

    // Timers
    FTimerHandle LODUpdateTimer;
    FTimerHandle CleanupTimer;
    FTimerHandle PoolMaintenanceTimer;

    // Counters
    int32 InstanceIDCounter = 0;
};