#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Quest_DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<int32> NextNodeIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestGiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float EmotionIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ESurvivalThreat ThreatLevel;

    FQuest_DialogueNode()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = TEXT("");
        bIsQuestGiver = false;
        QuestID = TEXT("");
        EmotionIntensity = 0.5f;
        ThreatLevel = ESurvivalThreat::None;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FQuest_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 CurrentNodeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EBiomeType AssociatedBiome;

    FQuest_DialogueTree()
    {
        TreeID = TEXT("");
        CurrentNodeIndex = 0;
        bIsCompleted = false;
        AssociatedBiome = EBiomeType::Savanna;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_DialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FQuest_DialogueTree DialogueTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanInteract;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ESurvivalRole NPCRole;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FQuest_DialogueNode GetCurrentDialogueNode();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectPlayerResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableResponses();

protected:
    virtual void BeginPlay() override;

private:
    bool bDialogueActive;
    AActor* CurrentPlayer;
};

UCLASS()
class TRANSPERSONALGAME_API UQuest_DialogueSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TMap<FString, FQuest_DialogueTree> GlobalDialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FString> CompletedDialogues;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void RegisterDialogueTree(const FString& TreeID, const FQuest_DialogueTree& Tree);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    FQuest_DialogueTree GetDialogueTree(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void MarkDialogueCompleted(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool IsDialogueCompleted(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    TArray<UQuest_DialogueComponent*> FindNearbyDialogueNPCs(AActor* PlayerActor, float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void CreateSurvivalDialogues();

protected:
    void SetupTribalDialogues();
    void SetupHunterDialogues();
    void SetupShamanDialogues();
    void SetupTraderDialogues();
};