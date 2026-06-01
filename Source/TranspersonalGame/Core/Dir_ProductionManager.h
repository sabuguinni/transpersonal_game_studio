#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Dir_ProductionManager.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    MinimumViablePrototype,
    VerticalSlice,
    AlphaPhase,
    BetaPhase,
    GoldMaster
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 EnvironmentActors;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bHasPlayableCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bHasBasicTerrain;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float FrameRate;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        DinosaurCount = 0;
        EnvironmentActors = 0;
        bHasPlayableCharacter = false;
        bHasBasicTerrain = false;
        FrameRate = 0.0f;
    }
};

/**
 * Studio Director Production Management System
 * Tracks production milestones and coordinates agent deliverables
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UDir_ProductionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Production Phase Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "Production")
    EDir_ProductionPhase GetCurrentPhase() const { return CurrentPhase; }

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateMinimumViablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GatherProductionMetrics();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    void RegisterAgentDeliverable(const FString& AgentName, const FString& DeliverableName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetPendingDeliverables();

    // Critical Production Checks
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsPlayablePrototypeReady();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionStatus();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TMap<FString, TArray<FString>> AgentDeliverables;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDir_ProductionMetrics LastMetrics;

private:
    void ValidateWorldState();
    void CheckPerformanceMetrics();
    bool HasEssentialGameplayElements();
};