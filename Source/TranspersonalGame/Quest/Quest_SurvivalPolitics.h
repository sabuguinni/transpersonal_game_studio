#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalPolitics.generated.h"

UENUM(BlueprintType)
enum class EQuest_TribalRelation : uint8
{
    Hostile     UMETA(DisplayName = "Hostile"),
    Neutral     UMETA(DisplayName = "Neutral"),
    Friendly    UMETA(DisplayName = "Friendly"),
    Allied      UMETA(DisplayName = "Allied"),
    Dependent   UMETA(DisplayName = "Dependent")
};

UENUM(BlueprintType)
enum class EQuest_PoliticalEvent : uint8
{
    ResourceDispute     UMETA(DisplayName = "Resource Dispute"),
    TerritoryConflict   UMETA(DisplayName = "Territory Conflict"),
    TradeNegotiation    UMETA(DisplayName = "Trade Negotiation"),
    AllianceProposal    UMETA(DisplayName = "Alliance Proposal"),
    LeadershipChallenge UMETA(DisplayName = "Leadership Challenge"),
    MigrationRequest    UMETA(DisplayName = "Migration Request")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_TribalFaction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Politics")
    FString FactionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Politics")
    EQuest_TribalRelation RelationToPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Politics")
    int32 Influence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Politics")
    int32 Population;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Politics")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Politics")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Politics")
    TArray<FString> ControlledResources;

    FQuest_TribalFaction()
    {
        FactionName = TEXT("Unknown Tribe");
        RelationToPlayer = EQuest_TribalRelation::Neutral;
        Influence = 50;
        Population = 25;
        TerritoryCenter = FVector::ZeroVector;
        TerritoryRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_PoliticalQuest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Political Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Political Quest")
    EQuest_PoliticalEvent EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Political Quest")
    FString InvolvedFaction1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Political Quest")
    FString InvolvedFaction2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Political Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Political Quest")
    TArray<FString> PossibleOutcomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Political Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Political Quest")
    bool bIsActive;

    FQuest_PoliticalQuest()
    {
        QuestID = TEXT("POL_001");
        EventType = EQuest_PoliticalEvent::ResourceDispute;
        InvolvedFaction1 = TEXT("Unknown");
        InvolvedFaction2 = TEXT("Unknown");
        QuestDescription = TEXT("A political situation requires your attention");
        TimeLimit = 300.0f;
        bIsActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalPolitics : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalPolitics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === TRIBAL FACTION MANAGEMENT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Politics")
    TArray<FQuest_TribalFaction> TribalFactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Politics")
    TArray<FQuest_PoliticalQuest> ActivePoliticalQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Politics")
    int32 PlayerInfluence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Politics")
    FString PlayerTribalStatus;

    // === POLITICAL QUEST GENERATION ===
    UFUNCTION(BlueprintCallable, Category = "Survival Politics")
    void GenerateRandomPoliticalEvent();

    UFUNCTION(BlueprintCallable, Category = "Survival Politics")
    void ProcessPlayerDecision(const FString& QuestID, const FString& PlayerChoice);

    UFUNCTION(BlueprintCallable, Category = "Survival Politics")
    void UpdateTribalRelations(const FString& FactionName, int32 RelationChange);

    UFUNCTION(BlueprintCallable, Category = "Survival Politics")
    FQuest_TribalFaction GetFactionByName(const FString& FactionName);

    // === TERRITORY MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Survival Politics")
    bool IsPlayerInFactionTerritory(const FVector& PlayerLocation, FString& OutFactionName);

    UFUNCTION(BlueprintCallable, Category = "Survival Politics")
    void HandleTerritoryViolation(const FString& FactionName);

    // === RESOURCE POLITICS ===
    UFUNCTION(BlueprintCallable, Category = "Survival Politics")
    void InitiateResourceDispute(const FString& Faction1, const FString& Faction2, const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Survival Politics")
    void ProposeTradeAgreement(const FString& FactionName, const TArray<FString>& OfferedResources);

    // === ALLIANCE SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "Survival Politics")
    void FormAlliance(const FString& Faction1, const FString& Faction2);

    UFUNCTION(BlueprintCallable, Category = "Survival Politics")
    void BreakAlliance(const FString& Faction1, const FString& Faction2);

    UFUNCTION(BlueprintCallable, Category = "Survival Politics")
    bool AreFactionsAllied(const FString& Faction1, const FString& Faction2);

private:
    // === INTERNAL SYSTEMS ===
    void InitializeDefaultFactions();
    void UpdatePoliticalQuests(float DeltaTime);
    void CheckQuestTimeouts();
    FString GenerateQuestID();

    // === QUEST TRACKING ===
    UPROPERTY()
    float LastEventTime;

    UPROPERTY()
    int32 NextQuestID;

    UPROPERTY()
    TMap<FString, EQuest_TribalRelation> FactionAlliances;
};