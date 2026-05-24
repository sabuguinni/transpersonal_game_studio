#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "ProductionDirector.generated.h"

UENUM(BlueprintType)
enum class EDir_MilestoneStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_MilestoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 RequiredElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 CompletedElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    FDir_MilestoneData()
    {
        Name = TEXT("Unnamed Milestone");
        Status = EDir_MilestoneStatus::NotStarted;
        RequiredElements = 1;
        CompletedElements = 0;
        Description = TEXT("No description");
    }
};

/**
 * Production Director - Coordinates and tracks milestone progress for the studio
 * Acts as a central hub for monitoring game development progress in real-time
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionDirector : public AActor
{
    GENERATED_BODY()

public:
    AProductionDirector();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* StatusDisplay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    TArray<FDir_MilestoneData> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentPhase;

public:
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneStatus(const FString& MilestoneName, EDir_MilestoneStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void IncrementMilestoneProgress(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ScanLevelForProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetMilestoneReport() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void RefreshDisplay();

private:
    void InitializeMilestones();
    void UpdateDisplayText();
    int32 CountActorsOfType(const FString& ActorType) const;
    float LastScanTime;
};

#include "ProductionDirector.generated.h"