#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalChallenge.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Task")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Task")
    EQuest_TaskType TaskType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Task")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Task")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Task")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Task")
    bool bIsCompleted;

    FQuest_SurvivalTask()
    {
        TaskName = TEXT("Survival Task");
        TaskType = EQuest_TaskType::Gather;
        RequiredCount = 1;
        CurrentCount = 0;
        TimeLimit = 300.0f; // 5 minutes
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalChallenge : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalChallenge();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* QuestMarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_SurvivalTask> SurvivalTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float QuestTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsQuestActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsQuestCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RemainingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_BiomeType TargetBiome;

    // Resource gathering tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    int32 StonesGathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    int32 SticksGathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    int32 BerriesGathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    int32 WaterCollected;

    // Crafting tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    int32 ToolsCrafted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    int32 SheltersBuilt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    int32 FiresLit;

    // Danger survival tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    int32 PredatorsAvoided;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    int32 DangersEscaped;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HealthMaintained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaMaintained;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateTaskProgress(EQuest_TaskType TaskType, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CheckQuestCompletion();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterResourceGathering(EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterCraftingAction(EQuest_CraftingType CraftType);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterSurvivalAction(EQuest_SurvivalAction ActionType);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FString GetQuestProgressText() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    float GetQuestCompletionPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetupBasicSurvivalQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetupAdvancedSurvivalQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetupEmergencySurvivalQuest();

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnQuestTimerExpired();

    void UpdateQuestTimer();
    void CheckTaskCompletion();
    void SpawnQuestRewards();

    FTimerHandle QuestTimerHandle;
    FTimerHandle UpdateTimerHandle;
};