#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "Quest_SurvivalObjectiveSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalObjectiveType : uint8
{
    GatherResources     UMETA(DisplayName = "Gather Resources"),
    FindWater          UMETA(DisplayName = "Find Water Source"),
    BuildShelter       UMETA(DisplayName = "Build Shelter"),
    HuntPrey           UMETA(DisplayName = "Hunt Small Prey"),
    SurviveNight       UMETA(DisplayName = "Survive Night Cycle"),
    AvoidPredators     UMETA(DisplayName = "Avoid Predators"),
    CraftTool          UMETA(DisplayName = "Craft Basic Tool")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveStatus : uint8
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
    EQuest_SurvivalObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOptional;

    FQuest_SurvivalObjective()
    {
        ObjectiveType = EQuest_SurvivalObjectiveType::GatherResources;
        ObjectiveName = TEXT("Unnamed Objective");
        Description = TEXT("No description");
        Status = EQuest_ObjectiveStatus::NotStarted;
        RequiredCount = 1;
        CurrentCount = 0;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 200.0f;
        bIsOptional = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_SurvivalObjectiveComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_SurvivalObjectiveComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    TArray<FQuest_SurvivalObjective> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    int32 CompletedObjectivesCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    bool bAllObjectivesCompleted;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void AddObjective(EQuest_SurvivalObjectiveType ObjectiveType, const FString& Name, const FString& Description, int32 RequiredCount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void UpdateObjectiveProgress(EQuest_SurvivalObjectiveType ObjectiveType, int32 ProgressAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void CompleteObjective(EQuest_SurvivalObjectiveType ObjectiveType);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    bool IsObjectiveCompleted(EQuest_SurvivalObjectiveType ObjectiveType) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    FQuest_SurvivalObjective GetObjective(EQuest_SurvivalObjectiveType ObjectiveType) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    TArray<FQuest_SurvivalObjective> GetActiveObjectives() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void SetObjectiveTargetLocation(EQuest_SurvivalObjectiveType ObjectiveType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    float GetDistanceToObjective(EQuest_SurvivalObjectiveType ObjectiveType) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    void InitializeSurvivalObjectives();

    UFUNCTION(BlueprintCallable, Category = "Quest Objectives")
    FString GetObjectiveStatusText() const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalObjectiveManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalObjectiveManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Management")
    TArray<FQuest_SurvivalObjective> MasterObjectiveList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Management")
    bool bAutoStartObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Management")
    float ObjectiveCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* ObjectiveCompleteSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* ObjectiveFailedSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* NewObjectiveSound;

private:
    FTimerHandle ObjectiveCheckTimer;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void RegisterPlayerCharacter(class APawn* PlayerPawn);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void StartSurvivalQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void CheckObjectiveCompletion();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void OnObjectiveCompleted(EQuest_SurvivalObjectiveType CompletedObjective);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void OnObjectiveFailed(EQuest_SurvivalObjectiveType FailedObjective);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void SpawnObjectiveMarkers();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void UpdateHUDDisplay();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnSurvivalQuestStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnSurvivalQuestCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnObjectiveStatusChanged(EQuest_SurvivalObjectiveType ObjectiveType, EQuest_ObjectiveStatus NewStatus);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ObjectiveTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    AQuest_ObjectiveTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    EQuest_SurvivalObjectiveType TriggerObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    bool bCompleteOnEnter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    bool bStartOnEnter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    int32 ProgressAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* TriggerSound;

public:
    UFUNCTION()
    void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnPlayerEnteredTrigger();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnPlayerExitedTrigger();
};

#include "Quest_SurvivalObjectiveSystem.generated.h"