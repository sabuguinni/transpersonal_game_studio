#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core_PlayerMovementComponent.generated.h"

UENUM(BlueprintType)
enum class ECore_MovementState : uint8
{
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Falling     UMETA(DisplayName = "Falling")
};

USTRUCT(BlueprintType)
struct FCore_MovementSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SwimSpeed = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float ClimbSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpZVelocity = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StaminaCostPerSecond = 10.0f;

    FCore_MovementSettings()
    {
        WalkSpeed = 300.0f;
        RunSpeed = 600.0f;
        CrouchSpeed = 150.0f;
        SwimSpeed = 250.0f;
        ClimbSpeed = 200.0f;
        JumpZVelocity = 420.0f;
        StaminaCostPerSecond = 10.0f;
    }
};

/**
 * Enhanced movement component for TranspersonalCharacter with survival mechanics
 * Handles walking, running, crouching, swimming, climbing with stamina integration
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PlayerMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    UCore_PlayerMovementComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Movement state management
    UFUNCTION(BlueprintCallable, Category = "Core Movement")
    void SetMovementState(ECore_MovementState NewState);

    UFUNCTION(BlueprintPure, Category = "Core Movement")
    ECore_MovementState GetMovementState() const { return CurrentMovementState; }

    // Enhanced movement controls
    UFUNCTION(BlueprintCallable, Category = "Core Movement")
    void StartRunning();

    UFUNCTION(BlueprintCallable, Category = "Core Movement")
    void StopRunning();

    UFUNCTION(BlueprintCallable, Category = "Core Movement")
    void StartCrouching();

    UFUNCTION(BlueprintCallable, Category = "Core Movement")
    void StopCrouching();

    UFUNCTION(BlueprintCallable, Category = "Core Movement")
    void StartClimbing();

    UFUNCTION(BlueprintCallable, Category = "Core Movement")
    void StopClimbing();

    // Stamina integration
    UFUNCTION(BlueprintCallable, Category = "Core Movement")
    void SetStaminaComponent(class UCore_SurvivalComponent* InStaminaComponent);

    UFUNCTION(BlueprintPure, Category = "Core Movement")
    bool CanRun() const;

    UFUNCTION(BlueprintPure, Category = "Core Movement")
    bool CanClimb() const;

    // Terrain adaptation
    UFUNCTION(BlueprintCallable, Category = "Core Movement")
    void UpdateMovementForTerrain(float TerrainSteepness, bool bIsWater, bool bIsClimbable);

    // Physics integration
    UFUNCTION(BlueprintCallable, Category = "Core Movement")
    void ApplyEnvironmentalForce(FVector Force, float Duration);

protected:
    // Movement state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Movement")
    ECore_MovementState CurrentMovementState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Movement")
    FCore_MovementSettings MovementSettings;

    // Component references
    UPROPERTY()
    class UCore_SurvivalComponent* SurvivalComponent;

    // Internal state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Movement")
    bool bIsRunning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Movement")
    bool bIsClimbing;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Movement")
    float CurrentStaminaDrain;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Movement")
    FVector EnvironmentalForce;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Movement")
    float EnvironmentalForceDuration;

    // Internal methods
    void UpdateMovementSpeed();
    void HandleStaminaConsumption(float DeltaTime);
    void HandleEnvironmentalForces(float DeltaTime);
    void UpdateMovementModeForState();
    bool IsValidMovementTransition(ECore_MovementState FromState, ECore_MovementState ToState) const;
};