#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_GameplayFoundation.generated.h"

/**
 * Engine Architect #02 - Gameplay Foundation System
 * Core gameplay architecture for the prehistoric survival game
 * Defines the fundamental systems that enable player interaction with the world
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_GameplayMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay Metrics")
    int32 ActivePlayers = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay Metrics")
    int32 ActiveDinosaurs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay Metrics")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay Metrics")
    float WorldLoadProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay Metrics")
    bool bIsGameplayReady = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PlayerMovementConfig
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 300.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float RunSpeed = 600.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float JumpHeight = 420.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float StaminaDrainRate = 10.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float StaminaRegenRate = 15.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_WorldInteractionConfig
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    float InteractionRange = 200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    float GatheringTime = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    float CraftingSpeedMultiplier = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    int32 MaxInventorySlots = 30;
};

/**
 * Core gameplay foundation subsystem
 * Manages fundamental gameplay systems and player interaction
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_GameplayFoundation : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core gameplay management
    UFUNCTION(BlueprintCallable, Category = "Gameplay Foundation")
    void InitializeGameplaySystems();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Foundation")
    void UpdateGameplayMetrics();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Foundation")
    bool IsGameplaySystemReady() const;

    // Player configuration
    UFUNCTION(BlueprintCallable, Category = "Player Config")
    void SetPlayerMovementConfig(const FEng_PlayerMovementConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Player Config")
    FEng_PlayerMovementConfig GetPlayerMovementConfig() const;

    // World interaction
    UFUNCTION(BlueprintCallable, Category = "World Interaction")
    void SetWorldInteractionConfig(const FEng_WorldInteractionConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "World Interaction")
    FEng_WorldInteractionConfig GetWorldInteractionConfig() const;

    // Metrics and monitoring
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FEng_GameplayMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void LogGameplayStatus() const;

    // System validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    void ValidateGameplayFoundation();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Config")
    FEng_PlayerMovementConfig PlayerMovementConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Config")
    FEng_WorldInteractionConfig WorldInteractionConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FEng_GameplayMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float InitializationTime = 0.0f;

private:
    void UpdatePlayerMetrics();
    void UpdateWorldMetrics();
    void UpdatePerformanceMetrics();
    bool ValidateSystemIntegrity() const;
};

/**
 * Gameplay foundation manager component
 * Attaches to GameMode to manage core gameplay systems
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_GameplayFoundationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_GameplayFoundationComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Foundation management
    UFUNCTION(BlueprintCallable, Category = "Foundation")
    void InitializeFoundationSystems();

    UFUNCTION(BlueprintCallable, Category = "Foundation")
    void ShutdownFoundationSystems();

    UFUNCTION(BlueprintCallable, Category = "Foundation")
    bool AreFoundationSystemsReady() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void ApplyDefaultConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SaveConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void LoadConfiguration();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Foundation")
    bool bFoundationInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "Foundation")
    float FoundationStartupTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoInitialize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableMetricsLogging = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MetricsUpdateInterval = 1.0f;

private:
    float LastMetricsUpdate = 0.0f;
    void UpdateFoundationMetrics();
};