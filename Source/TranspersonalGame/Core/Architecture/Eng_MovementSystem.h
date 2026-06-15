#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Eng_MovementSystem.generated.h"

/**
 * Enhanced movement system for prehistoric survival gameplay
 * Provides advanced movement mechanics including stamina management,
 * terrain adaptation, and survival-based movement modifiers
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_MovementSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_MovementSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Movement state tracking
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement State")
    bool bIsRunning = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement State")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement State")
    bool bIsClimbing = false;

    // Movement speeds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Speeds")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Speeds")
    float RunSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Speeds")
    float CrouchSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Speeds")
    float SwimSpeed = 200.0f;

    // Stamina system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
    float CurrentStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float StaminaDrainRate = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float StaminaRegenRate = 15.0f;

    // Terrain adaptation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float SlopeSpeedMultiplier = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float WaterSpeedMultiplier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MudSpeedMultiplier = 0.6f;

    // Character movement component reference
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

public:
    // Movement control functions
    UFUNCTION(BlueprintCallable, Category = "Movement Control")
    void StartRunning();

    UFUNCTION(BlueprintCallable, Category = "Movement Control")
    void StopRunning();

    UFUNCTION(BlueprintCallable, Category = "Movement Control")
    void StartCrouching();

    UFUNCTION(BlueprintCallable, Category = "Movement Control")
    void StopCrouching();

    // Stamina management
    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool CanRun() const;

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void RegenerateStamina(float DeltaTime);

    // Terrain detection
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetTerrainSpeedModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    bool IsOnSlope() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    bool IsInWater() const;

    // Movement state queries
    UFUNCTION(BlueprintPure, Category = "Movement State")
    bool IsMoving() const;

    UFUNCTION(BlueprintPure, Category = "Movement State")
    float GetCurrentSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Movement State")
    FVector GetMovementDirection() const;

private:
    // Internal update functions
    void UpdateMovementSpeed();
    void UpdateStamina(float DeltaTime);
    void CheckTerrainConditions();
    
    // Cached values
    FVector LastFrameLocation;
    float CurrentSpeedModifier = 1.0f;
    bool bWasMovingLastFrame = false;
};