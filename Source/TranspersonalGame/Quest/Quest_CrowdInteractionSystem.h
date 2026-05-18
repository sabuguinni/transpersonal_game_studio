#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "../Shared/SharedTypes.h"
#include "Quest_CrowdInteractionSystem.generated.h"

// Forward declarations
class UCrowd_MassEntityProcessor;
class UQuest_MissionObjectiveSystem;

UENUM(BlueprintType)
enum class EQuest_CrowdInteractionType : uint8
{
    EscortMission,
    CrowdControl,
    CelebrationEvent,
    PanicResponse,
    Migration,
    Festival,
    Emergency,
    TradeGathering
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdInteractionData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    EQuest_CrowdInteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    FString InteractionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    int32 RequiredCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    bool bIsRepeatable;

    FQuest_CrowdInteractionData()
    {
        InteractionType = EQuest_CrowdInteractionType::EscortMission;
        InteractionName = TEXT("Default Interaction");
        Description = TEXT("Default crowd interaction");
        RequiredCrowdSize = 10;
        InteractionRadius = 1000.0f;
        Duration = 60.0f;
        ExperienceReward = 100;
        bIsRepeatable = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ActiveCrowdInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Interaction")
    FQuest_CrowdInteractionData InteractionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Interaction")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Interaction")
    float TimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Interaction")
    int32 CurrentCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Interaction")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Interaction")
    TArray<AActor*> ParticipatingCrowdActors;

    FQuest_ActiveCrowdInteraction()
    {
        TargetLocation = FVector::ZeroVector;
        TimeRemaining = 0.0f;
        CurrentCrowdSize = 0;
        bIsActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CrowdInteractionManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CrowdInteractionManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    UDataTable* CrowdInteractionDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    TArray<FQuest_ActiveCrowdInteraction> ActiveInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    float InteractionCheckRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    float InteractionCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    int32 MaxSimultaneousInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    bool bDebugMode;

private:
    float LastInteractionCheckTime;
    TArray<AActor*> NearbyPlayerActors;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    bool StartCrowdInteraction(EQuest_CrowdInteractionType InteractionType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    void EndCrowdInteraction(int32 InteractionIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    TArray<AActor*> GetCrowdActorsInRadius(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    bool CanStartInteraction(EQuest_CrowdInteractionType InteractionType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    void UpdateCrowdBehaviorForInteraction(const FQuest_ActiveCrowdInteraction& Interaction);

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    int32 GetActiveCrowdInteractionCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    FQuest_CrowdInteractionData GetInteractionDataByType(EQuest_CrowdInteractionType InteractionType);

protected:
    void CheckForNewInteractionOpportunities();
    void UpdateActiveInteractions(float DeltaTime);
    void ProcessInteractionCompletion(int32 InteractionIndex);
    void TriggerCrowdResponse(const FQuest_ActiveCrowdInteraction& Interaction);
};

UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CrowdInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CrowdInteractionComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    bool bCanParticipateInInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    TArray<EQuest_CrowdInteractionType> SupportedInteractionTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    float InteractionInfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    int32 InteractionPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    bool bIsCurrentlyParticipating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Interaction")
    EQuest_CrowdInteractionType CurrentInteractionType;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    bool CanParticipateInInteraction(EQuest_CrowdInteractionType InteractionType) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    void JoinCrowdInteraction(EQuest_CrowdInteractionType InteractionType, FVector InteractionLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    void LeaveCrowdInteraction();

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    void SetInteractionBehavior(EQuest_CrowdInteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    bool IsParticipatingInInteraction() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    EQuest_CrowdInteractionType GetCurrentInteractionType() const;
};