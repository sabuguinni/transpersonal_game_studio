#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Quest_NPCDialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString OptionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ResponseText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bStartsQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    FQuest_DialogueOption()
    {
        OptionText = TEXT("");
        ResponseText = TEXT("");
        bStartsQuest = false;
        QuestID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FQuest_DialogueOption> Options;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bEndsConversation;

    FQuest_DialogueNode()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioURL = TEXT("");
        bEndsConversation = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_NPCDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_NPCDialogueSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FQuest_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 CurrentNodeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsInConversation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartConversation(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FQuest_DialogueNode GetCurrentDialogueNode() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectDialogueOption(int32 OptionIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetupBiomeSpecificDialogue(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsPlayerInRange(AActor* PlayerActor) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueNode(const FQuest_DialogueNode& NewNode);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ClearDialogue();

protected:
    UFUNCTION()
    void SetupMarshDialogue();

    UFUNCTION()
    void SetupForestDialogue();

    UFUNCTION()
    void SetupSavannaDialogue();

    UFUNCTION()
    void SetupDesertDialogue();

    UFUNCTION()
    void SetupMountainDialogue();
};