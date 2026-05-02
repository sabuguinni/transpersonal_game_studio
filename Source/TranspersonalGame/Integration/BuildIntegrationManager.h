#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

/**
 * Integration & Build Manager - Agente #19
 * Manages build validation, actor cleanup, and system integration
 * Ensures all agent outputs work together cohesively
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CleanDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetActorCount(const FString& ActorClassName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetSystemValidationReport();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetCurrentFramerate();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsPerformanceAcceptable();

    // Integration status
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAllSystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> ValidationErrors;

protected:
    // Internal validation functions
    bool ValidateCharacterSystem();
    bool ValidateWorldGeneration();
    bool ValidateAudioSystem();
    bool ValidateVFXSystem();
    bool ValidateAISystem();

    // Actor management
    void RemoveDuplicateLighting();
    void ValidateActorIntegrity();

    // Performance tracking
    UPROPERTY()
    float FramerateHistory[60]; // Last 60 frames
    
    UPROPERTY()
    int32 FrameIndex;

    UPROPERTY()
    float AverageFramerate;

private:
    // Validation state
    bool bInitialized;
    float LastCleanupTime;
    
    // Constants
    static constexpr float VALIDATION_INTERVAL = 5.0f;
    static constexpr float MIN_ACCEPTABLE_FPS = 30.0f;
    static constexpr int32 MAX_DUPLICATE_ACTORS = 3;
};