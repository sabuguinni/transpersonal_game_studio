#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "ObjectPhysicsComponent.generated.h"

class UConsciousnessSystem;
class UPhysicsManager;

/**
 * @brief Component that makes objects respond to consciousness and reality fluctuations
 * 
 * This component can be attached to any actor to make it consciousness-responsive.
 * Objects will float, phase, or behave differently based on nearby consciousness
 * levels and reality stability. Essential for creating the living, responsive world.
 * 
 * Performance Budget: 0.1ms per object (max 0.2ms)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UObjectPhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UObjectPhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Updates object physics based on nearby consciousness
     * @param NearbyConsciousnessLevel Consciousness level of nearby entities (0.0 to 1.0)
     * @param RealityStability Current reality stability (0.0 to 1.0)
     */
    UFUNCTION(BlueprintCallable, Category = "Object Physics")
    void UpdateFromConsciousness(float NearbyConsciousnessLevel, float RealityStability);

    /**
     * @brief Sets the consciousness sensitivity of this object
     * @param Sensitivity How much the object responds to consciousness (0.0 to 2.0)
     */
    UFUNCTION(BlueprintCallable, Category = "Object Physics")
    void SetConsciousnessSensitivity(float Sensitivity);

    /**
     * @brief Enables/disables consciousness physics for this object
     * @param bEnabled Whether to enable consciousness physics
     */
    UFUNCTION(BlueprintCallable, Category = "Object Physics")
    void SetConsciousnessPhysicsEnabled(bool bEnabled);

    /**
     * @brief Gets current consciousness influence on this object
     * @return Current consciousness influence level (0.0 to 1.0)
     */
    UFUNCTION(BlueprintPure, Category = "Object Physics")
    float GetCurrentConsciousnessInfluence() const;

    /**
     * @brief Checks if object is currently affected by consciousness
     * @return True if object is consciousness-affected
     */
    UFUNCTION(BlueprintPure, Category = "Object Physics")
    bool IsConsciousnessAffected() const;

    /**
     * @brief Applies a consciousness-based impulse to the object
     * @param Direction Impulse direction
     * @param Intensity Impulse intensity (0.0 to 2.0)
     */
    UFUNCTION(BlueprintCallable, Category = "Object Physics")
    void ApplyConsciousnessImpulse(FVector Direction, float Intensity);

    /**
     * @brief Sets object to phase mode (temporary collision disable)
     * @param Duration Phase duration in seconds
     */
    UFUNCTION(BlueprintCallable, Category = "Object Physics")
    void StartPhasing(float Duration);

protected:
    /** How sensitive this object is to consciousness (0.0 = not affected, 2.0 = highly affected) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Settings")
    float ConsciousnessSensitivity = 1.0f;

    /** Minimum consciousness level needed to affect this object */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Settings")
    float MinConsciousnessThreshold = 0.3f;

    /** Whether this object can float due to consciousness */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Settings")
    bool bCanConsciousnessFloat = true;

    /** Whether this object can phase through other objects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Settings")
    bool bCanPhase = false;

    /** Maximum height this object can float (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Settings")
    float MaxFloatHeight = 200.0f;

    /** Speed of consciousness-induced floating */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Settings")
    float FloatSpeed = 100.0f;

    /** Whether consciousness physics are enabled for this object */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Settings")
    bool bConsciousnessPhysicsEnabled = true;

    /** Object type for different consciousness responses */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Settings")
    int32 ObjectType = 0; // 0=Generic, 1=Organic, 2=Crystalline, 3=Metallic

private:
    /** Cached reference to consciousness system */
    UPROPERTY()
    UConsciousnessSystem* ConsciousnessSystem;

    /** Cached reference to physics manager */
    UPROPERTY()
    UPhysicsManager* PhysicsManager;

    /** Cached primitive component for physics operations */
    UPROPERTY()
    UPrimitiveComponent* PrimitiveComponent;

    /** Current consciousness influence level */
    float CurrentConsciousnessInfluence = 0.0f;

    /** Original object location for floating calculations */
    FVector OriginalLocation;

    /** Current floating offset */
    FVector CurrentFloatOffset;

    /** Whether object is currently floating */
    bool bIsFloating = false;

    /** Whether object is currently phasing */
    bool bIsPhasing = false;

    /** Remaining phase time */
    float RemainingPhaseTime = 0.0f;

    /** Original collision settings for phase restoration */
    ECollisionEnabled::Type OriginalCollisionEnabled;

    /** Performance tracking */
    float LastFrameTime = 0.0f;

    /** Detection radius for nearby consciousness sources */
    float ConsciousnessDetectionRadius = 1000.0f;

    /**
     * @brief Detects nearby consciousness sources
     * @return Highest consciousness level detected nearby
     */
    float DetectNearbyConsciousness();

    /**
     * @brief Processes consciousness-induced floating
     * @param DeltaTime Frame delta time
     * @param ConsciousnessLevel Current consciousness influence
     */
    void ProcessConsciousnessFloating(float DeltaTime, float ConsciousnessLevel);

    /**
     * @brief Processes object phasing state
     * @param DeltaTime Frame delta time
     */
    void ProcessPhasing(float DeltaTime);

    /**
     * @brief Applies consciousness-specific effects based on object type
     * @param ConsciousnessLevel Current consciousness influence
     * @param RealityStability Current reality stability
     */
    void ApplyObjectTypeEffects(float ConsciousnessLevel, float RealityStability);

    /**
     * @brief Validates performance budget
     * @param DeltaTime Frame delta time
     */
    void ValidatePerformance(float DeltaTime);

    /**
     * @brief Initializes object physics properties
     */
    void InitializeObjectPhysics();
};