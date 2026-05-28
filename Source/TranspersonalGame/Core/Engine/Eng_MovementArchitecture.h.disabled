#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Eng_MovementArchitecture.generated.h"

/**
 * MOVEMENT ARCHITECTURE MANAGER
 * Defines movement rules and physics parameters for all character types
 * Handles terrain adaptation, stamina-based movement, and environmental effects
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UEng_MovementArchitecture : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_MovementArchitecture();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // MOVEMENT PARAMETERS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Base")
    float BaseWalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Base")
    float BaseRunSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Base")
    float BaseJumpVelocity = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
    float StaminaDrainRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
    float StaminaRecoveryRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
    float MinStaminaToRun = 20.0f;

    // TERRAIN ADAPTATION
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Terrain")
    float SlopeSpeedMultiplier = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Terrain")
    float WaterSpeedMultiplier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Terrain")
    float MudSpeedMultiplier = 0.6f;

    // BIOME EFFECTS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Biome")
    TMap<EEng_BiomeType, float> BiomeSpeedModifiers;

    // FEAR SYSTEM INTEGRATION
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Fear")
    float FearSpeedBonus = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Fear")
    float FearStaminaDrainMultiplier = 2.0f;

    // MOVEMENT STATE TRACKING
    UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
    bool bIsRunning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
    bool bCanRun = true;

    UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
    float CurrentSpeedMultiplier = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
    EEng_BiomeType CurrentBiome = EEng_BiomeType::Forest;

    // ARCHITECTURE METHODS
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementParameters(class UCharacterMovementComponent* MovementComp);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateMovementForTerrain(const FHitResult& GroundHit);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateMovementForBiome(EEng_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateMovementForStamina(float CurrentStamina, float MaxStamina);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateMovementForFear(float FearLevel);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float CalculateMovementSpeed(bool bWantsToRun, float StaminaPercent, float FearLevel);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool CanPerformAction(const FString& ActionName, float StaminaCost);

    // VALIDATION METHODS
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Movement|Validation")
    void ValidateMovementSettings();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Movement|Validation")
    void LogMovementState();

private:
    // Internal state
    float LastSpeedUpdate = 0.0f;
    float SpeedUpdateInterval = 0.1f;
    
    // Cache for performance
    class UCharacterMovementComponent* CachedMovementComponent = nullptr;
    class ACharacter* OwnerCharacter = nullptr;

    // Helper methods
    void InitializeBiomeModifiers();
    float GetTerrainSpeedModifier(const FHitResult& GroundHit);
    void ApplyMovementModifiers();
};