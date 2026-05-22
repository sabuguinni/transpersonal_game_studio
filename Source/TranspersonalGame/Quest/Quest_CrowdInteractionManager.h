#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Components/BoxComponent.h"
#include "SharedTypes.h"
#include "Quest_CrowdInteractionManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_CrowdInteractionType : uint8
{
    AvoidDetection,
    FindNPC,
    GatherInformation,
    DeliverMessage,
    EscortThroughCrowd,
    MAX
};

USTRUCT(BlueprintType)
struct FQuest_CrowdObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_CrowdInteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CrowdDensityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionReward;

    FQuest_CrowdObjective()
    {
        ObjectiveName = TEXT("Default Objective");
        InteractionType = EQuest_CrowdInteractionType::FindNPC;
        TargetLocation = FVector::ZeroVector;
        CrowdDensityThreshold = 50.0f;
        bIsCompleted = false;
        CompletionReward = 100.0f;
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

public:
    virtual void Tick(float DeltaTime) override;

    // Quest Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_CrowdObjective> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bQuestSystemActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float QuestUpdateInterval;

    // Crowd Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Detection")
    float CrowdDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Detection")
    int32 MaxCrowdEntitiesTracked;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Detection")
    int32 CurrentCrowdCount;

    // Quest Functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartCrowdQuest(const FString& QuestName, EQuest_CrowdInteractionType InteractionType, FVector TargetLoc);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteObjective(const FString& ObjectiveName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckCrowdDensityAtLocation(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateQuestProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FVector> GetNearbyNPCLocations(FVector PlayerLocation, float SearchRadius);

    // Crowd Interaction Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    bool IsPlayerDetectedByCrowd(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    void TriggerCrowdReaction(FVector ReactionCenter, float ReactionRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    FVector FindSafestPathThroughCrowd(FVector StartLocation, FVector EndLocation);

    // Quest Validation
    UFUNCTION(BlueprintCallable, Category = "Quest Validation")
    bool ValidateQuestCompletion(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Quest Validation")
    float CalculateQuestReward(const FString& QuestName);

private:
    // Internal quest tracking
    float LastQuestUpdateTime;
    TArray<FVector> TrackedNPCLocations;
    TMap<FString, float> QuestStartTimes;

    // Helper functions
    void ScanForNearbyEntities();
    void UpdateCrowdDensityMap();
    bool IsLocationSafeFromCrowd(FVector Location);
};