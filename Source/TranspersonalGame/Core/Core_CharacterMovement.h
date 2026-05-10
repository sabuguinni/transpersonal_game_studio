#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Core_CharacterMovement.generated.h"

/**
 * Core Character Movement System
 * Handles advanced movement mechanics for the TranspersonalCharacter
 * Includes WASD controls, running, jumping, terrain adaptation
 * Integrates with physics systems for realistic movement
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CharacterMovement : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CharacterMovement();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Movement State Management
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    ECore_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float RunSpeed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float JumpVelocity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float StaminaDrainRate;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float StaminaRegenRate;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float CurrentStamina;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float MaxStamina;

    // Terrain Adaptation
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain")
    float TerrainAdaptationSpeed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain")
    float SlopeSpeedModifier;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Terrain")
    float MaxClimbAngle;

    // Physics Integration
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Physics")
    bool bUsePhysicsIntegration;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Physics")
    float GroundFriction;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Physics")
    float AirControl;

    // Movement Functions
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementState(ECore_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ProcessMovementInput(FVector InputVector);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartRunning();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopRunning();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void Jump();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateStamina(float DeltaTime);

    // Terrain Analysis
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetTerrainSlope();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    bool IsOnValidTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    FVector GetTerrainNormal();

    // Physics Integration
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyPhysicsMovement(FVector MovementVector, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void HandleGroundContact(FHitResult HitResult);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void HandleAirMovement(float DeltaTime);

private:
    // Internal state
    bool bIsRunning;
    bool bIsJumping;
    bool bIsOnGround;
    FVector LastGroundNormal;
    float TimeSinceGroundContact;
    
    // Component references
    UPROPERTY()
    class UCharacterMovementComponent* CharacterMovementComponent;

    UPROPERTY()
    class UCapsuleComponent* CapsuleComponent;

    // Internal functions
    void InitializeMovementComponent();
    void UpdateMovementSpeed();
    void CheckGroundContact();
    void ApplyTerrainAdaptation();
    void HandleStaminaSystem(float DeltaTime);
    FVector CalculateMovementDirection(FVector InputVector);
    void ApplyMovementForces(FVector Direction, float Speed, float DeltaTime);
};