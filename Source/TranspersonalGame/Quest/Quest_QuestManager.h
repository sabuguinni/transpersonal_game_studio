#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Quest_QuestManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_QuestStatus : uint8
{
    NotStarted,
    Active,
    Completed,
    Failed,
    Abandoned
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt,           // Kill specific dinosaur
    Collect,        // Gather items/resources
    Explore,        // Reach location
    Survive,        // Survive for time period
    Craft,          // Create specific items
    Escort,         // Protect NPC
    Rescue          // Save trapped NPC
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    FQuest_QuestObjective()
    {
        ObjectiveID = "";
        Description = "";
        Type = EQuest_ObjectiveType::Hunt;
        TargetTag = "";
        RequiredCount = 1;
        CurrentCount = 0;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_QuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString NPCGiverID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EBiomeType RequiredBiome;

    FQuest_QuestData()
    {
        QuestID = "";
        Title = "";
        Description = "";
        Status = EQuest_QuestStatus::NotStarted;
        NPCGiverID = "";
        ExperienceReward = 100;
        TimeLimit = 0.0f;
        ElapsedTime = 0.0f;
        RequiredBiome = EBiomeType::Savanna;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_QuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_QuestManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AbandonQuest(const FString& QuestID);

    // Objective management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    // Quest queries
    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_QuestData GetQuest(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool HasActiveQuest(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // Quest generation
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void GenerateRandomQuest(EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateHuntQuest(const FString& TargetSpecies, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateExplorationQuest(const FVector& TargetLocation, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateSurvivalQuest(float Duration, EBiomeType Biome);

    // Event handling
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnDinosaurKilled(const FString& Species, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnItemCollected(const FString& ItemType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnLocationReached(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerDeath();

    // Quest rewards
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void GiveQuestRewards(const FQuest_QuestData& Quest);

    // Tick function
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void TickQuests(float DeltaTime);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TMap<FString, FQuest_QuestData> ActiveQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TMap<FString, FQuest_QuestData> CompletedQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_QuestData> QuestTemplates;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 MaxActiveQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 NextQuestID;

private:
    void InitializeQuestTemplates();
    void CheckQuestCompletion(FQuest_QuestData& Quest);
    FString GenerateQuestID();
    void BroadcastQuestEvent(const FString& EventType, const FQuest_QuestData& Quest);
};