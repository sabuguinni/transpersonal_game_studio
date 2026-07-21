#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Quest_SettlementManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_SettlementStatus : uint8
{
    Establishing    UMETA(DisplayName = "Establishing"),
    Growing         UMETA(DisplayName = "Growing"),
    Thriving        UMETA(DisplayName = "Thriving"),
    Declining       UMETA(DisplayName = "Declining"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_SettlementRole : uint8
{
    Leader          UMETA(DisplayName = "Tribal Leader"),
    CraftMaster     UMETA(DisplayName = "Craft Master"),
    Scout           UMETA(DisplayName = "Scout"),
    Guard           UMETA(DisplayName = "Guard"),
    Elder           UMETA(DisplayName = "Elder"),
    Citizen         UMETA(DisplayName = "Citizen")
};

UENUM(BlueprintType)
enum class EQuest_SettlementResource : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Food            UMETA(DisplayName = "Food"),
    Hide            UMETA(DisplayName = "Hide"),
    Tools           UMETA(DisplayName = "Tools"),
    Medicine        UMETA(DisplayName = "Medicine")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SettlementNPC
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement NPC")
    EQuest_SettlementRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement NPC")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement NPC")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement NPC")
    TArray<FString> AvailableQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement NPC")
    bool bIsAvailable;

    FQuest_SettlementNPC()
    {
        NPCName = TEXT("Unknown");
        Role = EQuest_SettlementRole::Citizen;
        Position = FVector::ZeroVector;
        TrustLevel = 50.0f;
        bIsAvailable = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SettlementResource
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Resource")
    EQuest_SettlementResource ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Resource")
    int32 CurrentAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Resource")
    int32 MaxCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Resource")
    float ConsumptionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Resource")
    FVector StorageLocation;

    FQuest_SettlementResource()
    {
        ResourceType = EQuest_SettlementResource::Stone;
        CurrentAmount = 0;
        MaxCapacity = 100;
        ConsumptionRate = 1.0f;
        StorageLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SettlementBuilding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Building")
    FString BuildingName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Building")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Building")
    bool bIsConstructed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Building")
    float ConstructionProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Building")
    TArray<FQuest_SettlementResource> RequiredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Building")
    FString AssignedNPC;

    FQuest_SettlementBuilding()
    {
        BuildingName = TEXT("Unknown Building");
        Location = FVector::ZeroVector;
        bIsConstructed = false;
        ConstructionProgress = 0.0f;
        AssignedNPC = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SettlementManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SettlementManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Settlement Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    FString SettlementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    EQuest_SettlementStatus SettlementStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    int32 Population;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    float SettlementMorale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    float DefenseRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    FVector SettlementCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    float SettlementRadius;

    // NPCs and Roles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement NPCs")
    TArray<FQuest_SettlementNPC> SettlementNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement NPCs")
    FQuest_SettlementNPC TribalLeader;

    // Resources
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Resources")
    TArray<FQuest_SettlementResource> SettlementResources;

    // Buildings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Buildings")
    TArray<FQuest_SettlementBuilding> SettlementBuildings;

    // Quest Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Quests")
    TArray<FString> ActiveSettlementQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement Quests")
    TArray<FString> CompletedSettlementQuests;

    // Settlement Management Functions
    UFUNCTION(BlueprintCallable, Category = "Settlement Management")
    void InitializeSettlement(const FString& Name, const FVector& Center);

    UFUNCTION(BlueprintCallable, Category = "Settlement Management")
    void AddNPCToSettlement(const FQuest_SettlementNPC& NewNPC);

    UFUNCTION(BlueprintCallable, Category = "Settlement Management")
    void RemoveNPCFromSettlement(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Settlement Management")
    void UpdateSettlementStatus();

    UFUNCTION(BlueprintCallable, Category = "Settlement Management")
    void ProcessResourceConsumption(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Settlement Management")
    bool AddResource(EQuest_SettlementResource ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Settlement Management")
    bool ConsumeResource(EQuest_SettlementResource ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Settlement Management")
    int32 GetResourceAmount(EQuest_SettlementResource ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Settlement Management")
    void AddBuilding(const FQuest_SettlementBuilding& NewBuilding);

    UFUNCTION(BlueprintCallable, Category = "Settlement Management")
    void UpdateBuildingConstruction(const FString& BuildingName, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Settlement Management")
    bool IsBuildingComplete(const FString& BuildingName);

    // Leadership Functions
    UFUNCTION(BlueprintCallable, Category = "Settlement Leadership")
    void AssignTribalLeader(const FQuest_SettlementNPC& Leader);

    UFUNCTION(BlueprintCallable, Category = "Settlement Leadership")
    void ProcessLeadershipDecisions();

    UFUNCTION(BlueprintCallable, Category = "Settlement Leadership")
    void HandleSettlementCrisis(const FString& CrisisType);

    // Quest Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Settlement Quests")
    void StartSettlementQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Settlement Quests")
    void CompleteSettlementQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Settlement Quests")
    TArray<FString> GetAvailableSettlementQuests();

    UFUNCTION(BlueprintCallable, Category = "Settlement Quests")
    bool IsPlayerWelcomeInSettlement();

    // Event Functions
    UFUNCTION(BlueprintImplementableEvent, Category = "Settlement Events")
    void OnSettlementStatusChanged(EQuest_SettlementStatus NewStatus);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settlement Events")
    void OnResourceDepleted(EQuest_SettlementResource ResourceType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settlement Events")
    void OnBuildingCompleted(const FString& BuildingName);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settlement Events")
    void OnNewNPCArrived(const FQuest_SettlementNPC& NewNPC);

private:
    // Internal tracking
    float LastUpdateTime;
    float ResourceUpdateInterval;
    
    // Helper functions
    void UpdateMorale();
    void UpdateDefenseRating();
    void CheckSettlementNeeds();
    void ProcessNPCInteractions();
};