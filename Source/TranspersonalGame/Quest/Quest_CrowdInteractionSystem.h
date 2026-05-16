#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "../Crowd/Crowd_MassEntityManager.h"
#include "../SharedTypes.h"
#include "Quest_CrowdInteractionSystem.generated.h"

// Quest trigger types for crowd interactions
UENUM(BlueprintType)
enum class EQuest_CrowdTriggerType : uint8
{
    DensityThreshold     UMETA(DisplayName = "Crowd Density Threshold"),
    EntityBehaviorChange UMETA(DisplayName = "Entity Behavior Change"),
    CrowdMovement        UMETA(DisplayName = "Crowd Movement Pattern"),
    EntityProximity      UMETA(DisplayName = "Entity Proximity to Player"),
    CrowdDispersion      UMETA(DisplayName = "Crowd Dispersion Event")
};

// Crowd quest objective types
UENUM(BlueprintType)
enum class EQuest_CrowdObjective : uint8
{
    FollowCrowd          UMETA(DisplayName = "Follow Crowd Movement"),
    AvoidCrowd           UMETA(DisplayName = "Avoid Crowd Areas"),
    InteractWithEntity   UMETA(DisplayName = "Interact with Specific Entity"),
    InfluenceBehavior    UMETA(DisplayName = "Influence Crowd Behavior"),
    EscortEntity         UMETA(DisplayName = "Escort Entity Through Crowd")
};

// Crowd interaction quest data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdInteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_CrowdTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_CrowdObjective ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TriggerThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredEntityCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime;

    FQuest_CrowdInteractionData()
    {
        QuestID = TEXT("");
        TriggerType = EQuest_CrowdTriggerType::DensityThreshold;
        ObjectiveType = EQuest_CrowdObjective::FollowCrowd;
        TriggerThreshold = 50.0f;
        TargetLocation = FVector::ZeroVector;
        InteractionRadius = 1000.0f;
        RequiredEntityCount = 5;
        bIsCompleted = false;
        TimeLimit = 300.0f;
        ElapsedTime = 0.0f;
    }
};

// Quest component for crowd interaction tracking
UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CrowdInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CrowdInteractionComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Crowd interaction quest management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_CrowdInteractionData> ActiveCrowdQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    ACrowd_MassEntityManager* CrowdManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CrowdCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float LastCrowdCheck;

    // Quest functions
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartCrowdQuest(const FQuest_CrowdInteractionData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteCrowdQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CheckCrowdDensityTrigger(const FQuest_CrowdInteractionData& Quest);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CheckEntityProximityTrigger(const FQuest_CrowdInteractionData& Quest);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateCrowdQuestProgress(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetNearbyEntityCount(const FVector& Location, float Radius);

private:
    void FindCrowdManager();
    void ProcessCrowdTriggers();
};

// Game instance subsystem for crowd quest management
UCLASS()
class TRANSPERSONALGAME_API UQuest_CrowdInteractionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Global crowd quest tracking
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_CrowdInteractionData> GlobalCrowdQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 CompletedCrowdQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float TotalCrowdInteractionTime;

    // Quest management functions
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterCrowdQuest(const FQuest_CrowdInteractionData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UnregisterCrowdQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_CrowdInteractionData GetCrowdQuestByID(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_CrowdInteractionData> GetActiveCrowdQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool HasActiveCrowdQuest(const FString& QuestID);

    // Crowd quest templates
    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_CrowdInteractionData CreateFollowCrowdQuest(const FVector& StartLocation, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_CrowdInteractionData CreateAvoidCrowdQuest(const FVector& DangerZone, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_CrowdInteractionData CreateEntityInteractionQuest(const FVector& TargetLocation, int32 EntityCount);
};