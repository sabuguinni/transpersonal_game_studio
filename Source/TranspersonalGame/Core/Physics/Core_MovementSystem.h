#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "Core_MovementSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCore_MovementSystem, Log, All);

/**
 * Enhanced Movement System for TranspersonalCharacter
 * 
 * Provides realistic movement mechanics for prehistoric survival:
 * - Stamina-based running with realistic depletion
 * - Stealth movement with reduced noise and speed
 * - Fear-based movement penalties (trembling, reduced speed)
 * - Terrain-adaptive movement (mud, rocks, vegetation)
 * - Injury-based movement limitations
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_MovementSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_MovementSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Movement Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Base")
    float BaseWalkSpeed = 200.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Base")
    float BaseRunSpeed = 400.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stealth")
    float SneakSpeed = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stealth")
    float SneakStaminaDrain = 5.0f;
    
    // Running System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Running")
    float RunStaminaDrain = 15.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Running")
    float MinStaminaToRun = 20.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Running")
    float StaminaRecoveryRate = 10.0f;
    
    // Fear System Impact
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Fear")
    float FearSpeedPenalty = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Fear")
    float HighFearThreshold = 70.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Fear")
    float TremblingIntensity = 2.0f;
    
    // Terrain Modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Terrain")
    float MudSpeedModifier = 0.6f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Terrain")
    float RockSpeedModifier = 0.8f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Terrain")
    float VegetationSpeedModifier = 0.7f;
    
    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
    bool bIsRunning = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
    bool bIsSneaking = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
    bool bCanRun = true;
    
    UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
    float CurrentSpeedModifier = 1.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
    FString CurrentTerrain = TEXT("Normal");

public:
    // Movement Control Functions
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartRunning();
    
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopRunning();
    
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartSneaking();
    
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopSneaking();
    
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ProcessMovementInput(const FInputActionValue& Value);
    
    // Terrain Detection
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void DetectTerrainType();
    
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ApplyTerrainModifier(const FString& TerrainType);
    
    // Fear Impact
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ApplyFearPenalty(float FearLevel);
    
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ApplyTrembling(float Intensity);
    
    // Stamina Management
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ConsumeMovementStamina(float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool HasEnoughStaminaToRun() const;
    
    // Getters
    UFUNCTION(BlueprintPure, Category = "Movement")
    float GetCurrentSpeed() const;
    
    UFUNCTION(BlueprintPure, Category = "Movement")
    float GetSpeedModifier() const { return CurrentSpeedModifier; }
    
    UFUNCTION(BlueprintPure, Category = "Movement")
    bool IsRunning() const { return bIsRunning; }
    
    UFUNCTION(BlueprintPure, Category = "Movement")
    bool IsSneaking() const { return bIsSneaking; }

private:
    // Internal References
    UPROPERTY()
    class ATranspersonalCharacter* OwnerCharacter;
    
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;
    
    // Internal State
    float LastTerrainCheck = 0.0f;
    float TerrainCheckInterval = 0.5f;
    
    FVector LastPosition;
    float TremblingTimer = 0.0f;
    
    // Internal Functions
    void UpdateMovementSpeed();
    void UpdateStamina(float DeltaTime);
    void CheckRunningConditions();
    void ApplyMovementEffects(float DeltaTime);
    FHitResult PerformGroundTrace() const;
};