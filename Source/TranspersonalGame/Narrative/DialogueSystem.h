#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../SharedTypes.h"
#include "DialogueSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, const FString&, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, const FString&, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueChoiceSelected, const FString&, DialogueID, int32, ChoiceIndex);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString RequiredItemType;

    FNarr_DialogueChoice()
    {
        ChoiceText = FText::FromString(TEXT("Continue"));
        NextNodeID = TEXT("");
        bRequiresItem = false;
        RequiredItemType = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsEndNode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    FNarr_DialogueNode()
    {
        NodeID = TEXT("");
        SpeakerName = FText::FromString(TEXT("Unknown"));
        DialogueText = FText::FromString(TEXT("..."));
        bIsEndNode = false;
        AudioAssetPath = TEXT("");
        DisplayDuration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString StartNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_NPCType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRepeatable;

    FNarr_DialogueTree()
    {
        DialogueID = TEXT("");
        DialogueTitle = FText::FromString(TEXT("Conversation"));
        StartNodeID = TEXT("");
        NPCType = ENarr_NPCType::Tribal_Elder;
        bRepeatable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDialogueDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Data")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UFUNCTION(BlueprintCallable, Category = "Dialogue Data")
    FNarr_DialogueTree GetDialogueTree(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue Data")
    bool HasDialogueTree(const FString& DialogueID) const;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    UDialogueDataAsset* DialogueData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DefaultDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_NPCType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& DialogueID, AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueNode GetCurrentNode() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsInDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanInteract(AActor* PlayerActor) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FString CurrentNodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    AActor* CurrentPlayer;

    bool IsChoiceAvailable(const FNarr_DialogueChoice& Choice) const;
    FNarr_DialogueNode FindNodeByID(const FString& NodeID) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDialogueManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void RegisterDialogueData(UDialogueDataAsset* DialogueData);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    UDialogueDataAsset* GetDialogueData(const FString& DataAssetName) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void CreateSurvivalDialogue(const FString& DialogueID, ENarr_NPCType NPCType, const FString& QuestID = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void CreateQuestDialogue(const FString& DialogueID, const FString& QuestID, ENarr_NPCType NPCType);

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueChoiceSelected OnDialogueChoiceSelected;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue Data")
    TMap<FString, UDialogueDataAsset*> DialogueDataAssets;

    void InitializeDefaultDialogues();
    void CreateElderDialogues();
    void CreateHunterDialogues();
    void CreateExplorerDialogues();
};