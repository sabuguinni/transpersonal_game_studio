#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Quest_InteractiveQuestSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalObjectiveType : uint8
{
    WaterCollection     UMETA(DisplayName = "Water Collection"),
    Hunting            UMETA(DisplayName = "Hunting"),
    Crafting           UMETA(DisplayName = "Crafting"),
    Building           UMETA(DisplayName = "Building"),
    Exploration        UMETA(DisplayName = "Exploration"),
    ResourceGathering  UMETA(DisplayName = "Resource Gathering")
};

UENUM(BlueprintType)
enum class EQuest_ProgressState : uint8
{
    NotStarted         UMETA(DisplayName = "Not Started"),
    InProgress         UMETA(DisplayName = "In Progress"),
    Completed          UMETA(DisplayName = "Completed"),
    Failed             UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_SurvivalObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ProgressState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentAmount;

    FQuest_SurvivalObjective()
    {
        ObjectiveName = TEXT("");
        Description = TEXT("");
        ObjectiveType = EQuest_SurvivalObjectiveType::WaterCollection;
        TargetLocation = FVector::ZeroVector;
        CurrentState = EQuest_ProgressState::NotStarted;
        CompletionProgress = 0.0f;
        RequiredAmount = 1;
        CurrentAmount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_InteractiveQuestMarker : public AActor
{
    GENERATED_BODY()

public:
    AQuest_InteractiveQuestMarker();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* MarkerLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_SurvivalObjective AssociatedObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsActive;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ActivateMarker();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void DeactivateMarker();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjectiveProgress(int32 NewAmount);

    UFUNCTION(BlueprintPure, Category = "Quest")
    FQuest_SurvivalObjective GetObjective() const { return AssociatedObjective; }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    AQuest_SurvivalTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_SurvivalObjectiveType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOneTimeUse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasBeenTriggered;

public:
    UFUNCTION()
    void OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void TriggerQuest(AActor* TriggeringActor);

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool CanTrigger() const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_InteractiveQuestSystem : public UObject
{
    GENERATED_BODY()

public:
    UQuest_InteractiveQuestSystem();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_SurvivalObjective> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_SurvivalObjective> CompletedObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<AQuest_InteractiveQuestMarker*> QuestMarkers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<AQuest_SurvivalTrigger*> QuestTriggers;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeQuestSystem();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void AddObjective(const FQuest_SurvivalObjective& NewObjective);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteObjective(const FString& ObjectiveName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjectiveProgress(const FString& ObjectiveName, int32 NewAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateQuestMarker(const FVector& Location, const FQuest_SurvivalObjective& Objective);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateQuestTrigger(const FVector& Location, EQuest_SurvivalObjectiveType TriggerType);

    UFUNCTION(BlueprintPure, Category = "Quest System")
    TArray<FQuest_SurvivalObjective> GetActiveObjectives() const { return ActiveObjectives; }

    UFUNCTION(BlueprintPure, Category = "Quest System")
    TArray<FQuest_SurvivalObjective> GetCompletedObjectives() const { return CompletedObjectives; }

    UFUNCTION(BlueprintPure, Category = "Quest System")
    int32 GetTotalObjectivesCount() const;

    UFUNCTION(BlueprintPure, Category = "Quest System")
    float GetOverallProgress() const;
};