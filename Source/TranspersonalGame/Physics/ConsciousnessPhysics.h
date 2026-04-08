/**
 * @file ConsciousnessPhysics.h
 * @brief Core physics system for consciousness expansion mechanics
 * @author Core Systems Programmer
 * @date 2024
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "ConsciousnessPhysics.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConsciousnessLayerChanged, int32, NewLayer, float, TransitionTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRealityDistortionApplied, float, DistortionStrength);

/**
 * @enum EConsciousnessLayer
 * @brief Defines the 4 layers of reality perception
 */
UENUM(BlueprintType)
enum class EConsciousnessLayer : uint8
{
    Physical    UMETA(DisplayName = "Physical Reality"),
    Emotional   UMETA(DisplayName = "Emotional Layer"),
    Mental      UMETA(DisplayName = "Mental Layer"),
    Spiritual   UMETA(DisplayName = "Spiritual Layer")
};

/**
 * @struct FConsciousnessPhysicsSettings
 * @brief Configuration settings for consciousness physics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FConsciousnessPhysicsSettings
{
    GENERATED_BODY()

    /** Gravity multiplier for each consciousness layer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Physics")
    TMap<EConsciousnessLayer, float> GravityMultipliers;

    /** Time dilation factor for each layer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temporal")
    TMap<EConsciousnessLayer, float> TimeDilationFactors;

    /** Physics material overrides for each layer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<EConsciousnessLayer, class UPhysicalMaterial*> LayerMaterials;

    /** Maximum reality distortion strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distortion", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float MaxDistortionStrength = 2.0f;

    /** Transition time between layers in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float LayerTransitionTime = 1.5f;

    FConsciousnessPhysicsSettings()
    {
        // Default gravity multipliers
        GravityMultipliers.Add(EConsciousnessLayer::Physical, 1.0f);
        GravityMultipliers.Add(EConsciousnessLayer::Emotional, 0.7f);
        GravityMultipliers.Add(EConsciousnessLayer::Mental, 0.4f);
        GravityMultipliers.Add(EConsciousnessLayer::Spiritual, 0.1f);

        // Default time dilation
        TimeDilationFactors.Add(EConsciousnessLayer::Physical, 1.0f);
        TimeDilationFactors.Add(EConsciousnessLayer::Emotional, 0.9f);
        TimeDilationFactors.Add(EConsciousnessLayer::Mental, 0.6f);
        TimeDilationFactors.Add(EConsciousnessLayer::Spiritual, 0.3f);
    }
};

/**
 * @class UConsciousnessPhysicsComponent
 * @brief Component that manages physics behavior based on consciousness state
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UConsciousnessPhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UConsciousnessPhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Current consciousness layer */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Consciousness")
    EConsciousnessLayer CurrentLayer = EConsciousnessLayer::Physical;

    /** Physics settings configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FConsciousnessPhysicsSettings PhysicsSettings;

    /** Current reality distortion strength (0.0 to MaxDistortionStrength) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Distortion")
    float CurrentDistortionStrength = 0.0f;

    /** Is currently transitioning between layers */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
    bool bIsTransitioning = false;

    /** Delegates */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnConsciousnessLayerChanged OnLayerChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRealityDistortionApplied OnDistortionApplied;

    /**
     * @brief Transition to a new consciousness layer
     * @param NewLayer Target consciousness layer
     * @param TransitionDuration Override transition time (optional)
     */
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void TransitionToLayer(EConsciousnessLayer NewLayer, float TransitionDuration = -1.0f);

    /**
     * @brief Apply reality distortion effect
     * @param DistortionStrength Strength of distortion (0.0 to MaxDistortionStrength)
     * @param Duration Duration of effect in seconds
     */
    UFUNCTION(BlueprintCallable, Category = "Distortion")
    void ApplyRealityDistortion(float DistortionStrength, float Duration = 3.0f);

    /**
     * @brief Get current gravity multiplier for this layer
     * @return Current gravity multiplier
     */
    UFUNCTION(BlueprintPure, Category = "Physics")
    float GetCurrentGravityMultiplier() const;

    /**
     * @brief Get current time dilation factor
     * @return Current time dilation factor
     */
    UFUNCTION(BlueprintPure, Category = "Temporal")
    float GetCurrentTimeDilation() const;

    /**
     * @brief Check if actor can phase through objects in current layer
     * @return True if phasing is enabled
     */
    UFUNCTION(BlueprintPure, Category = "Physics")
    bool CanPhaseThrough() const;

    /**
     * @brief Get physics material for current layer
     * @return Physics material or nullptr if not set
     */
    UFUNCTION(BlueprintPure, Category = "Materials")
    class UPhysicalMaterial* GetCurrentLayerMaterial() const;

private:
    /** Transition state tracking */
    EConsciousnessLayer TargetLayer = EConsciousnessLayer::Physical;
    EConsciousnessLayer PreviousLayer = EConsciousnessLayer::Physical;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 1.5f;

    /** Distortion effect tracking */
    float DistortionTimeRemaining = 0.0f;
    float TargetDistortionStrength = 0.0f;

    /** Cached component references */
    UPROPERTY()
    class UPrimitiveComponent* PrimitiveComponent = nullptr;

    /** Internal methods */
    void UpdatePhysicsProperties();
    void UpdateGravity();
    void UpdateTimeDilation();
    void UpdateMaterialProperties();
    void ProcessLayerTransition(float DeltaTime);
    void ProcessDistortionEffect(float DeltaTime);
    float InterpolateLayerValues(float PhysicalValue, float TargetValue, float Progress) const;
};

/**
 * @class AConsciousnessPhysicsManager
 * @brief World-level manager for consciousness physics systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AConsciousnessPhysicsManager : public AActor
{
    GENERATED_BODY()

public:
    AConsciousnessPhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    /** Global physics settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    FConsciousnessPhysicsSettings GlobalSettings;

    /** Performance budget in milliseconds per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float PerformanceBudgetMS = 1.5f;

    /** Maximum number of consciousness components to update per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "10", ClampMax = "500"))
    int32 MaxComponentsPerFrame = 100;

    /**
     * @brief Register a consciousness physics component with the manager
     * @param Component Component to register
     */
    UFUNCTION(BlueprintCallable, Category = "Management")
    void RegisterComponent(UConsciousnessPhysicsComponent* Component);

    /**
     * @brief Unregister a consciousness physics component
     * @param Component Component to unregister
     */
    UFUNCTION(BlueprintCallable, Category = "Management")
    void UnregisterComponent(UConsciousnessPhysicsComponent* Component);

    /**
     * @brief Apply global reality distortion to all registered components
     * @param DistortionStrength Global distortion strength
     * @param Duration Effect duration
     */
    UFUNCTION(BlueprintCallable, Category = "Global Effects")
    void ApplyGlobalDistortion(float DistortionStrength, float Duration);

    /**
     * @brief Get current performance metrics
     * @param OutUpdateTimeMS Time spent updating components this frame
     * @param OutRegisteredComponents Number of registered components
     */
    UFUNCTION(BlueprintPure, Category = "Performance")
    void GetPerformanceMetrics(float& OutUpdateTimeMS, int32& OutRegisteredComponents) const;

private:
    /** Registered consciousness components */
    UPROPERTY()
    TArray<UConsciousnessPhysicsComponent*> RegisteredComponents;

    /** Performance tracking */
    float LastUpdateTimeMS = 0.0f;
    int32 CurrentUpdateIndex = 0;

    /** Update components in batches for performance */
    void UpdateComponentsBatched(float DeltaTime);
};