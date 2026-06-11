#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "Eng_GameplayArchitect.generated.h"

// Forward declarations
class ATranspersonalCharacter;
class ATranspersonalGameMode;
class UTranspersonalGameState;

/**
 * Core gameplay architecture validation states
 */
UENUM(BlueprintType)
enum class EEng_GameplayState : uint8
{
    Uninitialized   UMETA(DisplayName = "Uninitialized"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Ready           UMETA(DisplayName = "Ready"),
    InGame          UMETA(DisplayName = "In Game"),
    Paused          UMETA(DisplayName = "Paused"),
    Error           UMETA(DisplayName = "Error")
};

/**
 * System validation results
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTimestamp = 0.0f;

    FEng_SystemValidation()
    {
        bIsValid = false;
        SystemName = TEXT("Unknown");
        ErrorMessage = TEXT("");
        ValidationTimestamp = 0.0f;
    }
};

/**
 * Gameplay architecture requirements
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_GameplayRequirements
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Requirements")
    bool bRequireCharacterMovement = true;

    UPROPERTY(BlueprintReadWrite, Category = "Requirements")
    bool bRequireCameraSystem = true;

    UPROPERTY(BlueprintReadWrite, Category = "Requirements")
    bool bRequireSurvivalStats = true;

    UPROPERTY(BlueprintReadWrite, Category = "Requirements")
    bool bRequireInputSystem = true;

    UPROPERTY(BlueprintReadWrite, Category = "Requirements")
    bool bRequirePhysicsSystem = true;

    UPROPERTY(BlueprintReadWrite, Category = "Requirements")
    int32 MinimumActorCount = 5;

    UPROPERTY(BlueprintReadWrite, Category = "Requirements")
    float MinimumFrameRate = 30.0f;

    FEng_GameplayRequirements()
    {
        bRequireCharacterMovement = true;
        bRequireCameraSystem = true;
        bRequireSurvivalStats = true;
        bRequireInputSystem = true;
        bRequirePhysicsSystem = true;
        MinimumActorCount = 5;
        MinimumFrameRate = 30.0f;
    }
};

/**
 * Engine Architect - Gameplay Foundation System
 * 
 * This subsystem validates and manages the core gameplay architecture.
 * It ensures all fundamental systems are properly initialized and functional.
 * 
 * Key responsibilities:
 * - Validate core class compilation and loading
 * - Monitor gameplay system health
 * - Enforce architectural constraints
 * - Provide debugging and diagnostic tools
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_GameplayArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_GameplayArchitect();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Validate all core gameplay systems
     * @return True if all systems pass validation
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateGameplaySystems();

    /**
     * Get current gameplay state
     */
    UFUNCTION(BlueprintPure, Category = "Engine Architecture")
    EEng_GameplayState GetGameplayState() const { return CurrentGameplayState; }

    /**
     * Get validation results for a specific system
     */
    UFUNCTION(BlueprintPure, Category = "Engine Architecture")
    FEng_SystemValidation GetSystemValidation(const FString& SystemName) const;

    /**
     * Force re-validation of all systems
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    void ForceSystemRevalidation();

    /**
     * Get gameplay requirements configuration
     */
    UFUNCTION(BlueprintPure, Category = "Engine Architecture")
    FEng_GameplayRequirements GetGameplayRequirements() const { return GameplayRequirements; }

    /**
     * Set gameplay requirements (runtime configuration)
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetGameplayRequirements(const FEng_GameplayRequirements& NewRequirements);

    /**
     * Check if minimum viable gameplay is available
     */
    UFUNCTION(BlueprintPure, Category = "Engine Architecture")
    bool IsMinimumViableGameplayReady() const;

    /**
     * Get diagnostic information about current gameplay state
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetGameplayDiagnostics() const;

protected:
    /**
     * Current gameplay validation state
     */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EEng_GameplayState CurrentGameplayState;

    /**
     * System validation results cache
     */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TMap<FString, FEng_SystemValidation> SystemValidations;

    /**
     * Gameplay requirements configuration
     */
    UPROPERTY(BlueprintReadWrite, Category = "Configuration")
    FEng_GameplayRequirements GameplayRequirements;

    /**
     * Last validation timestamp
     */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastValidationTime;

private:
    /**
     * Validate character system
     */
    FEng_SystemValidation ValidateCharacterSystem();

    /**
     * Validate game mode system
     */
    FEng_SystemValidation ValidateGameModeSystem();

    /**
     * Validate level and world state
     */
    FEng_SystemValidation ValidateLevelSystem();

    /**
     * Validate input system
     */
    FEng_SystemValidation ValidateInputSystem();

    /**
     * Validate physics system
     */
    FEng_SystemValidation ValidatePhysicsSystem();

    /**
     * Update gameplay state based on validation results
     */
    void UpdateGameplayState();

    /**
     * Timer handle for periodic validation
     */
    FTimerHandle ValidationTimerHandle;

    /**
     * Perform periodic system validation
     */
    void PeriodicValidation();
};