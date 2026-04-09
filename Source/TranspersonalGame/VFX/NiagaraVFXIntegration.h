#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraVFXIntegration.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNiagaraVFXIntegration, Log, All);

/**
 * VFX Quality levels for performance scaling
 */
UENUM(BlueprintType)
enum class EVFXQualityLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra")
};

/**
 * VFX categories for prehistoric environment
 */
UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Atmospheric     UMETA(DisplayName = "Atmospheric"),
    Environmental   UMETA(DisplayName = "Environmental"),
    Flora           UMETA(DisplayName = "Flora"),
    Creature        UMETA(DisplayName = "Creature"),
    Mystical        UMETA(DisplayName = "Mystical"),
    Survival        UMETA(DisplayName = "Survival")
};

/**
 * Data structure for prehistoric VFX system configuration
 */
USTRUCT(BlueprintType)
struct FPrehistoricVFXData
{
    GENERATED_BODY()

    /** Path to the Niagara system asset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FString SystemPath;

    /** VFX category for organization and performance management */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFXCategory Category;

    /** Particle spawn rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float SpawnRate;

    /** Particle lifetime in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Lifetime;

    /** Initial velocity for particles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector InitialVelocity;

    /** Whether this VFX is affected by wind */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAffectedByWind;

    FPrehistoricVFXData()
        : SystemPath(TEXT(""))
        , Category(EVFXCategory::Atmospheric)
        , SpawnRate(50.0f)
        , Lifetime(5.0f)
        , InitialVelocity(FVector::ZeroVector)
        , bAffectedByWind(true)
    {
    }

    FPrehistoricVFXData(const FString& InSystemPath, EVFXCategory InCategory, float InSpawnRate, 
                       float InLifetime, const FVector& InInitialVelocity, bool InAffectedByWind)
        : SystemPath(InSystemPath)
        , Category(InCategory)
        , SpawnRate(InSpawnRate)
        , Lifetime(InLifetime)
        , InitialVelocity(InInitialVelocity)
        , bAffectedByWind(InAffectedByWind)
    {
    }
};

/**
 * Comprehensive Niagara VFX integration system for prehistoric environment
 * Handles atmospheric effects, creature interactions, and mystical elements
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent), Category="Transpersonal VFX")
class TRANSPERSONALGAME_API UNiagaraVFXIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UNiagaraVFXIntegration();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /**
     * Spawn a prehistoric VFX effect at specified location
     * @param VFXName - Name of the VFX system to spawn
     * @param Location - World location to spawn the effect
     * @param Rotation - Rotation for the effect
     * @param AttachToActor - Optional actor to attach the effect to
     * @return The spawned Niagara component
     */
    UFUNCTION(BlueprintCallable, Category = "Prehistoric VFX")
    UNiagaraComponent* SpawnPrehistoricVFX(const FString& VFXName, const FVector& Location, 
                                          const FRotator& Rotation = FRotator::ZeroRotator, 
                                          AActor* AttachToActor = nullptr);

    /**
     * Stop all active VFX systems
     */
    UFUNCTION(BlueprintCallable, Category = "Prehistoric VFX")
    void StopAllVFX();

    /**
     * Set VFX quality level for performance scaling
     */
    UFUNCTION(BlueprintCallable, Category = "Prehistoric VFX")
    void SetVFXQuality(EVFXQualityLevel NewQualityLevel);

    /**
     * Get current VFX quality level
     */
    UFUNCTION(BlueprintPure, Category = "Prehistoric VFX")
    EVFXQualityLevel GetVFXQuality() const { return VFXQualityLevel; }

protected:
    /**
     * Initialize prehistoric VFX systems data
     */
    void InitializePrehistoricVFXSystems();

    /**
     * Load essential VFX systems into memory
     */
    void LoadEssentialVFXSystems();

    /**
     * Configure VFX parameters for a specific system
     */
    void ConfigureVFXParameters(UNiagaraComponent* VFXComponent, const FString& VFXName);

    /**
     * Configure atmospheric VFX parameters
     */
    void ConfigureAtmosphericVFX(UNiagaraComponent* VFXComponent, const FString& VFXName);

    /**
     * Configure environmental VFX parameters
     */
    void ConfigureEnvironmentalVFX(UNiagaraComponent* VFXComponent, const FString& VFXName);

    /**
     * Configure flora VFX parameters
     */
    void ConfigureFloraVFX(UNiagaraComponent* VFXComponent, const FString& VFXName);

    /**
     * Configure creature VFX parameters
     */
    void ConfigureCreatureVFX(UNiagaraComponent* VFXComponent, const FString& VFXName);

    /**
     * Configure mystical VFX parameters
     */
    void ConfigureMysticalVFX(UNiagaraComponent* VFXComponent, const FString& VFXName);

    /**
     * Update all active VFX systems
     */
    void UpdateActiveVFXSystems(float DeltaTime);

    /**
     * Update environmental influences on VFX
     */
    void UpdateEnvironmentalInfluences(float DeltaTime);

    /**
     * Manage VFX LOD based on distance and performance
     */
    void ManageVFXLOD();

    /**
     * Configure performance settings based on quality level
     */
    void ConfigurePerformanceSettings();

    /**
     * Get current wind direction from environment system
     */
    FVector GetCurrentWindDirection() const;

    /**
     * Get current wind strength from environment system
     */
    float GetCurrentWindStrength() const;

protected:
    /** VFX quality level for performance scaling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EVFXQualityLevel VFXQualityLevel;

    /** Maximum particles per VFX system */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxParticlesPerSystem;

    /** LOD distance scale multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceScale;

    /** Prehistoric VFX systems configuration data */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Systems")
    TMap<FString, FPrehistoricVFXData> PrehistoricVFXSystems;

    /** Loaded Niagara systems ready for spawning */
    UPROPERTY()
    TMap<FString, UNiagaraSystem*> LoadedVFXSystems;

    /** Currently active VFX components for management */
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;

public:
    /** Blueprint-accessible prehistoric VFX systems data */
    UPROPERTY(BlueprintReadOnly, Category = "VFX Systems")
    TMap<FString, FPrehistoricVFXData> BP_PrehistoricVFXSystems;

    /** Delegate for VFX system events */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVFXSystemEvent, const FString&, VFXName, EVFXCategory, Category);

    /** Event fired when a VFX system is spawned */
    UPROPERTY(BlueprintAssignable, Category = "VFX Events")
    FOnVFXSystemEvent OnVFXSystemSpawned;

    /** Event fired when a VFX system completes */
    UPROPERTY(BlueprintAssignable, Category = "VFX Events")
    FOnVFXSystemEvent OnVFXSystemCompleted;
};