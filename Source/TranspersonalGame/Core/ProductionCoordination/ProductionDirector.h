#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "../../SharedTypes.h"
#include "ProductionDirector.generated.h"

/**
 * Production Director - Central coordination actor for Milestone 1
 * Tracks progress of all 19 agents and ensures deliverables are met
 * Spawned in MinPlayableMap to provide visual feedback on production status
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionDirector : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionDirector();

protected:
    virtual void BeginPlay() override;

    // Status tracking components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    class UTextRenderComponent* StatusDisplay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    class UStaticMeshComponent* StatusIndicator;

    // Milestone 1 progress tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bCharacterMovementComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bTerrainComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bDinosaursPlaced = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bLightingComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bSurvivalHUDComplete = false;

    // Agent deliverable tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Progress")
    TMap<FString, bool> AgentDeliverables;

public:
    virtual void Tick(float DeltaTime) override;

    // Production coordination functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void MarkAgentComplete(const FString& AgentName, const FString& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetMilestone1Progress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void RefreshStatusDisplay();

private:
    void InitializeStatusDisplay();
    void UpdateStatusText();
    FString GetProgressBar(float Progress) const;
};