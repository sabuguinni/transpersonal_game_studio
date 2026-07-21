#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Quest_SocialQuestManager.generated.h"

// Forward declarations
class UCrowd_SocialDynamicsManager;
class UQuest_NPCManager;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SocialTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Trigger")
    EQuest_TriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Trigger")
    ECrowd_SocialInteractionType RequiredInteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Trigger")
    int32 MinGroupSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Trigger")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Trigger")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Trigger")
    bool bIsActive;

    FQuest_SocialTrigger()
    {
        TriggerType = EQuest_TriggerType::SocialInteraction;
        RequiredInteractionType = ECrowd_SocialInteractionType::Neutral;
        MinGroupSize = 2;
        TriggerRadius = 500.0f;
        QuestID = TEXT("");
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_TribalConflictData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Conflict")
    FString ConflictID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Conflict")
    TArray<AActor*> Tribe1Members;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Conflict")
    TArray<AActor*> Tribe2Members;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Conflict")
    FVector ConflictLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Conflict")
    float ConflictIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Conflict")
    EQuest_ConflictResolution ResolutionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Conflict")
    float StartTime;

    FQuest_TribalConflictData()
    {
        ConflictID = TEXT("");
        ConflictLocation = FVector::ZeroVector;
        ConflictIntensity = 0.5f;
        ResolutionType = EQuest_ConflictResolution::Negotiation;
        StartTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_SocialQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_SocialQuestManager();

protected:
    virtual void BeginPlay() override;

    // Social trigger system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Triggers")
    TArray<FQuest_SocialTrigger> ActiveSocialTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Triggers")
    float TriggerCheckInterval;

    // Tribal conflict tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Conflicts")
    TArray<FQuest_TribalConflictData> ActiveConflicts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Conflicts")
    int32 MaxSimultaneousConflicts;

    // Quest generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Generation")
    float QuestGenerationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Generation")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Generation")
    TArray<FString> AvailableQuestTemplates;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UCrowd_SocialDynamicsManager* SocialDynamicsRef;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UQuest_NPCManager* NPCManagerRef;

    // Timer handles
    FTimerHandle TriggerCheckTimer;
    FTimerHandle ConflictUpdateTimer;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Social trigger management
    UFUNCTION(BlueprintCallable, Category = "Social Triggers")
    void RegisterSocialTrigger(const FQuest_SocialTrigger& NewTrigger);

    UFUNCTION(BlueprintCallable, Category = "Social Triggers")
    void RemoveSocialTrigger(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Social Triggers")
    bool CheckSocialTriggers();

    // Tribal conflict management
    UFUNCTION(BlueprintCallable, Category = "Tribal Conflicts")
    void StartTribalConflict(const TArray<AActor*>& Tribe1, const TArray<AActor*>& Tribe2, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Tribal Conflicts")
    void ResolveTribalConflict(const FString& ConflictID, EQuest_ConflictResolution Resolution);

    UFUNCTION(BlueprintCallable, Category = "Tribal Conflicts")
    void UpdateActiveConflicts();

    // Quest generation based on social dynamics
    UFUNCTION(BlueprintCallable, Category = "Quest Generation")
    FString GenerateQuestFromSocialEvent(ECrowd_SocialInteractionType InteractionType, const TArray<AActor*>& InvolvedActors);

    UFUNCTION(BlueprintCallable, Category = "Quest Generation")
    TArray<FString> GetAvailableQuestsForLocation(const FVector& Location, float Radius);

    // Player intervention system
    UFUNCTION(BlueprintCallable, Category = "Player Intervention")
    bool CanPlayerInterveneInConflict(const FString& ConflictID);

    UFUNCTION(BlueprintCallable, Category = "Player Intervention")
    void PlayerInterventionChoice(const FString& ConflictID, EQuest_InterventionType InterventionType);

    // Social reputation system
    UFUNCTION(BlueprintCallable, Category = "Reputation")
    void UpdatePlayerReputationWithTribe(const FString& TribeID, float ReputationChange);

    UFUNCTION(BlueprintCallable, Category = "Reputation")
    float GetPlayerReputationWithTribe(const FString& TribeID);

private:
    // Internal helper functions
    void CheckProximityTriggers();
    void CheckInteractionTriggers();
    void ProcessConflictEscalation(FQuest_TribalConflictData& Conflict);
    FString GenerateUniqueConflictID();
    bool IsLocationSafeForQuest(const FVector& Location);

    // Social event tracking
    TMap<FString, float> TribeReputations;
    TArray<FVector> RecentConflictLocations;
    float LastTriggerCheckTime;
};