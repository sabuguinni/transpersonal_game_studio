#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

class UQuest_QuestGiver;
class AQuest_QuestNPC;

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuest_QuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_QuestManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    // Quest registration
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuestGiver(UQuest_QuestGiver* QuestGiver);

    // Survival quest system
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateSurvivalQuest(const FString& QuestID, const FText& Title, const FText& Description);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateHuntQuest(const FString& QuestID, const FString& TargetDinosaur, int32 RequiredKills);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateExplorationQuest(const FString& QuestID, const FVector& TargetLocation, float Radius);

private:
    UPROPERTY()
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY()
    TArray<FString> ActiveQuestIDs;

    UPROPERTY()
    TArray<UQuest_QuestGiver*> QuestGivers;

    void InitializeDefaultQuests();
    void CreateDefaultSurvivalQuests();
    bool ValidateQuestCompletion(const FString& QuestID);
};