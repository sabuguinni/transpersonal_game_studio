#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "TranspersonalMovementComponent.generated.h"

class UConsciousnessSystem;
class UPhysicsManager;

/**
 * @brief Enhanced character movement component that responds to consciousness states
 * 
 * This component extends UE5's CharacterMovementComponent to create movement that
 * feels different based on the player's consciousness level. Higher consciousness
 * enables floating, phase-walking, and reality-bending movement mechanics.
 * 
 * Performance Budget: 0.3ms per frame (max 0.5ms)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTranspersonalMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    UTranspersonalMovementComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Updates movement parameters based on consciousness level
     * @param ConsciousnessLevel Current consciousness level (0.0 to 1.0)
     */
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Movement")
    void UpdateMovementFromConsciousness(float ConsciousnessLevel);

    /**
     * @brief Enables consciousness-based floating movement
     * @param bEnable Whether to enable floating
     */
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Movement")
    void SetConsciousnessFloating(bool bEnable);

    /**
     * @brief Attempts to phase through solid objects based on consciousness
     * @param Direction Movement direction
     * @return True if phasing was successful
     */
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Movement")
    bool AttemptPhaseWalk(FVector Direction);

    /**
     * @brief Gets current movement speed multiplier from consciousness
     * @return Speed multiplier (0.5 to 2.0)
     */
    UFUNCTION(BlueprintPure, Category = "Transpersonal Movement")
    float GetConsciousnessSpeedMultiplier() const;

    /**
     * @brief Checks if character is currently in consciousness floating mode
     * @return True if floating via consciousness
     */
    UFUNCTION(BlueprintPure, Category = "Transpersonal Movement")
    bool IsConsciousnessFloating() const;

    /**
     * @brief Applies consciousness-based movement impulse
     * @param Direction Impulse direction
     * @param Intensity Impulse intensity (0.0 to 2.0)
     */
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Movement")
    void ApplyConsciousnessImpulse(FVector Direction, float Intensity);

protected:
    /** Base walking speed when consciousness is neutral */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Movement")
    float BaseWalkSpeed = 600.0f;

    /** Maximum speed multiplier at high consciousness */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Movement")
    float MaxSpeedMultiplier = 2.0f;

    /** Minimum speed multiplier at low consciousness */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Movement")
    float MinSpeedMultiplier = 0.5f;

    /** Floating speed when in consciousness floating mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Movement")
    float ConsciousnessFloatSpeed = 400.0f;

    /** Air control multiplier based on consciousness */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Movement")
    float ConsciousnessAirControl = 1.0f;

    /** Gravity scale modifier for consciousness effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Movement")
    float ConsciousnessGravityScale = 1.0f;

    /** Whether consciousness floating is currently enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Movement")
    bool bConsciousnessFloatingEnabled = false;

    /** Phase walk cooldown in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Movement")
    float PhaseWalkCooldown = 2.0f;

    /** Minimum consciousness level required for phase walking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Movement")
    float MinPhaseWalkConsciousness = 0.7f;

private:
    /** Cached reference to consciousness system */
    UPROPERTY()
    UConsciousnessSystem* ConsciousnessSystem;

    /** Cached reference to physics manager */
    UPROPERTY()
    UPhysicsManager* PhysicsManager;

    /** Current consciousness level cache */
    float CurrentConsciousnessLevel = 0.5f;

    /** Current speed multiplier */
    float CurrentSpeedMultiplier = 1.0f;

    /** Time since last phase walk attempt */
    float TimeSinceLastPhaseWalk = 0.0f;

    /** Whether character is currently phase walking */
    bool bIsPhaseWalking = false;

    /** Phase walk duration */
    float PhaseWalkDuration = 0.5f;

    /** Remaining phase walk time */
    float RemainingPhaseWalkTime = 0.0f;

    /** Performance tracking */
    float LastFrameTime = 0.0f;

    /**
     * @brief Updates speed based on consciousness level
     * @param ConsciousnessLevel Current consciousness level
     */
    void UpdateSpeed(float ConsciousnessLevel);

    /**
     * @brief Updates air control based on consciousness
     * @param ConsciousnessLevel Current consciousness level
     */
    void UpdateAirControl(float ConsciousnessLevel);

    /**
     * @brief Updates gravity effects based on consciousness
     * @param ConsciousnessLevel Current consciousness level
     */
    void UpdateGravityEffects(float ConsciousnessLevel);

    /**
     * @brief Processes consciousness floating movement
     * @param DeltaTime Frame delta time
     */
    void ProcessConsciousnessFloating(float DeltaTime);

    /**
     * @brief Processes phase walking state
     * @param DeltaTime Frame delta time
     */
    void ProcessPhaseWalking(float DeltaTime);

    /**
     * @brief Validates performance budget
     * @param DeltaTime Frame delta time
     */
    void ValidatePerformance(float DeltaTime);

public:
    // Override movement functions for consciousness integration
    virtual void UpdateFromCompressedFlags(uint8 Flags) override;
    virtual float GetMaxSpeed() const override;
    virtual float GetMaxAcceleration() const override;
    virtual void PhysWalking(float deltaTime, int32 Iterations) override;
    virtual void PhysFalling(float deltaTime, int32 Iterations) override;
};