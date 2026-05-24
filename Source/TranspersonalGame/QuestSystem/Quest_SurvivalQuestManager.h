#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Quest_SurvivalQuestManager.generated.h"

class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalQuest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RequiredSurvivalTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float MinHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float MinHungerThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float MinThirstThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredDinosaurKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> RequiredResources;

    FQuest_SurvivalQuest()
    {
        QuestID = "";
        QuestName = "";
        QuestDescription = "";
        bIsActive = false;
        bIsCompleted = false;
        RequiredSurvivalTime = 300.0f; // 5 minutes
        MinHealthThreshold = 50.0f;
        MinHungerThreshold = 30.0f;
        MinThirstThreshold = 30.0f;
        RequiredDinosaurKills = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuest_SurvivalQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_SurvivalQuestManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalQuest GetQuest(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalQuest> GetActiveQuests() const;

    // Survival tracking
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateSurvivalProgress(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnDinosaurKilled(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnResourceGathered(const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckQuestCompletion();

    // Quest creation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateSurvivalQuest(const FString& QuestID, const FString& Name, const FString& Description, float SurvivalTime);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateHuntingQuest(const FString& QuestID, const FString& Name, const FString& Description, const FString& DinosaurType, int32 KillCount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateGatheringQuest(const FString& QuestID, const FString& Name, const FString& Description, const TArray<FString>& Resources);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_SurvivalQuest> ActiveQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_SurvivalQuest> CompletedQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TMap<FString, int32> DinosaurKillCounts;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TMap<FString, int32> ResourceCounts;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    float TotalSurvivalTime;

    // Player reference
    UPROPERTY()
    ATranspersonalCharacter* PlayerCharacter;

private:
    void InitializeDefaultQuests();
    bool CheckSurvivalRequirements(const FQuest_SurvivalQuest& Quest) const;
    bool CheckKillRequirements(const FQuest_SurvivalQuest& Quest) const;
    bool CheckResourceRequirements(const FQuest_SurvivalQuest& Quest) const;
    void BroadcastQuestCompletion(const FString& QuestID);
};