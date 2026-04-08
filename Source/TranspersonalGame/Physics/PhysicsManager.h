#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsManager.generated.h"

class UConsciousnessSystem;
class URealitySystem;

/**
 * @brief Core physics manager that bridges consciousness states with physical reality
 * 
 * This system manages the fundamental physics parameters that respond to consciousness
 * levels, creating emergent gameplay where the player's mental state directly affects
 * the physical world's behavior.
 * 
 * Performance Budget: 0.8ms per frame (max 1.2ms)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Updates global physics parameters based on consciousness state
     * @param ConsciousnessLevel Current consciousness level (0.0 to 1.0)
     * @param RealityStability Current reality stability (0.0 to 1.0)
     */
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Physics")
    void UpdatePhysicsFromConsciousness(float ConsciousnessLevel, float RealityStability);

    /**
     * @brief Applies consciousness-influenced forces to an actor
     * @param Actor Target actor to influence
     * @param ConsciousnessIntensity Intensity of consciousness effect (0.0 to 2.0)
     */
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Physics")
    void ApplyConsciousnessForce(AActor* Actor, float ConsciousnessIntensity);

    /**
     * @brief Creates reality distortion field around a location
     * @param Location Center of distortion
     * @param Radius Effect radius in Unreal units
     * @param Intensity Distortion intensity (0.0 to 1.0)
     * @param Duration Effect duration in seconds
     */
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Physics")
    void CreateRealityDistortion(FVector Location, float Radius, float Intensity, float Duration);

    /**
     * @brief Gets current gravity multiplier based on consciousness state
     * @return Gravity multiplier (0.1 to 2.0)
     */
    UFUNCTION(BlueprintPure, Category = "Transpersonal Physics")
    float GetConsciousnessGravityMultiplier() const;

    /**
     * @brief Enables/disables transpersonal physics effects
     * @param bEnabled Whether to enable effects
     */
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Physics")
    void SetTranspersonalPhysicsEnabled(bool bEnabled);

protected:
    /** Base gravity value (cm/s²) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Parameters")
    float BaseGravity = -980.0f;

    /** Minimum gravity multiplier during high consciousness states */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Parameters")
    float MinGravityMultiplier = 0.1f;

    /** Maximum gravity multiplier during low consciousness states */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Parameters")
    float MaxGravityMultiplier = 2.0f;

    /** Time dilation factor based on consciousness (0.1 to 2.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Parameters")
    float ConsciousnessTimeDilation = 1.0f;

    /** Air resistance multiplier for consciousness effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Parameters")
    float ConsciousnessAirResistance = 1.0f;

    /** Whether transpersonal physics are currently enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Parameters")
    bool bTranspersonalPhysicsEnabled = true;

private:
    /** Cached reference to consciousness system */
    UPROPERTY()
    UConsciousnessSystem* ConsciousnessSystem;

    /** Cached reference to reality system */
    UPROPERTY()
    URealitySystem* RealitySystem;

    /** Current gravity multiplier */
    float CurrentGravityMultiplier = 1.0f;

    /** Active reality distortions */
    TArray<struct FRealityDistortion> ActiveDistortions;

    /** Performance tracking */
    float LastFrameTime = 0.0f;
    float AverageFrameTime = 0.0f;

    /**
     * @brief Updates gravity based on consciousness level
     * @param ConsciousnessLevel Current consciousness level
     */
    void UpdateGravity(float ConsciousnessLevel);

    /**
     * @brief Updates time dilation effects
     * @param RealityStability Current reality stability
     */
    void UpdateTimeDilation(float RealityStability);

    /**
     * @brief Processes active reality distortions
     * @param DeltaTime Frame delta time
     */
    void ProcessRealityDistortions(float DeltaTime);

    /**
     * @brief Validates performance budget
     * @param DeltaTime Frame delta time
     */
    void ValidatePerformance(float DeltaTime);
};

/**
 * @brief Structure representing a reality distortion field
 */
USTRUCT(BlueprintType)
struct FRealityDistortion
{
    GENERATED_BODY()

    /** Center location of distortion */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    /** Effect radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 500.0f;

    /** Current intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 1.0f;

    /** Remaining duration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RemainingDuration = 0.0f;

    /** Distortion type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DistortionType = 0;

    FRealityDistortion()
    {
        Location = FVector::ZeroVector;
        Radius = 500.0f;
        Intensity = 1.0f;
        RemainingDuration = 0.0f;
        DistortionType = 0;
    }
};