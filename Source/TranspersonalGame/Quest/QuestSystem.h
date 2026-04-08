#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "QuestSystem.generated.h"

class UQuestComponent;
class UQuestObjective;
class ATranspersonalPlayerController;

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    Survival,           // Sobrevivência básica - comida, abrigo
    Observation,        // Observar comportamentos de dinossauros
    Discovery,          // Descobrir locais, recursos, gemas
    Domestication,      // Domesticar espécies pequenas
    Ecosystem,          // Interagir com ecossistema
    MainStory,          // História principal - encontrar a gema
    Emergency           // Missões geradas por eventos dinâmicos
};

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    NotStarted,
    Active,
    Completed,
    Failed,
    Abandoned
};

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
    Kill,
    Collect,
    Interact,
    Observe,
    Survive,
    Reach,
    Craft,
    Domesticate,
    Discover
};

USTRUCT(BlueprintType)
struct FQuestReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Experience = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer UnlockTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText RewardDescription;
};

USTRUCT(BlueprintType)
struct FQuestObjectiveData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EObjectiveType ObjectiveType = EObjectiveType::Interact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TargetCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeLimit = 0.0f; // 0 = sem limite

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOptional = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetRadius = 100.0f;
};

USTRUCT(BlueprintType)
struct FQuestData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuestType QuestType = EQuestType::Survival;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FQuestObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer PrerequisiteTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer QuestTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FQuestReward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 1; // 1-10, maior = mais importante

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeLimit = 0.0f; // 0 = sem limite

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRepeatable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoStart = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString QuestGiverID; // Para NPCs que dão missões

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText QuestGiverDialogue;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, FName, QuestID, EQuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveUpdated, FName, QuestID, int32, ObjectiveIndex, int32, CurrentProgress);

/**
 * Sistema central de missões que gerencia todas as quest do jogo
 * Integra com o sistema Mass para criar missões dinâmicas baseadas no comportamento dos dinossauros
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuestSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Delegates
    UPROPERTY(BlueprintAssignable)
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable)
    FOnObjectiveUpdated OnObjectiveUpdated;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AbandonQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    EQuestStatus GetQuestStatus(FName QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FName> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FName> GetAvailableQuests() const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(FName QuestID, int32 ObjectiveIndex, int32 Progress = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(FName QuestID, int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    int32 GetObjectiveProgress(FName QuestID, int32 ObjectiveIndex) const;

    // Dynamic Quest Generation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FName GenerateDynamicQuest(EQuestType QuestType, const FGameplayTagContainer& ContextTags);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void ProcessEcosystemEvents(const FGameplayTagContainer& EventTags, FVector Location);

    // Data Access
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuestData GetQuestData(FName QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void LoadQuestDatabase(UDataTable* QuestTable);

    // Save/Load
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void SaveQuestProgress(class USaveGame* SaveGame);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void LoadQuestProgress(class USaveGame* SaveGame);

protected:
    UPROPERTY()
    TMap<FName, FQuestData> QuestDatabase;

    UPROPERTY()
    TMap<FName, EQuestStatus> QuestStatuses;

    UPROPERTY()
    TMap<FName, TArray<int32>> ObjectiveProgress;

    UPROPERTY()
    TMap<FName, float> QuestStartTimes;

    // Dynamic Quest Generation
    FName GenerateSurvivalQuest(const FGameplayTagContainer& ContextTags);
    FName GenerateObservationQuest(const FGameplayTagContainer& ContextTags);
    FName GenerateDiscoveryQuest(const FGameplayTagContainer& ContextTags);
    FName GenerateDomesticationQuest(const FGameplayTagContainer& ContextTags);
    FName GenerateEmergencyQuest(const FGameplayTagContainer& ContextTags);

    void CheckQuestPrerequisites();
    void CheckQuestTimeouts();
    void NotifyQuestStatusChange(FName QuestID, EQuestStatus NewStatus);

private:
    FTimerHandle QuestUpdateTimer;
    int32 DynamicQuestCounter = 0;
};