#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Quest_DialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FText> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> NextDialogueIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestEnd;

    FQuest_DialogueEntry()
    {
        DialogueID = "";
        DialogueText = FText::FromString("Default dialogue text");
        bIsQuestStart = false;
        bIsQuestEnd = false;
        QuestID = "";
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ConversationTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    FString ConversationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    TArray<FQuest_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    FString StartDialogueID;

    FQuest_ConversationTree()
    {
        ConversationID = "";
        StartDialogueID = "";
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_DialogueManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DialogueMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FQuest_ConversationTree> ConversationTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FString CurrentConversationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    bool bIsInConversation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FText NPCName;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartConversation(const FString& ConversationID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FQuest_DialogueEntry GetCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FText> GetCurrentResponseOptions();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsPlayerInRange();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnConversationStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnConversationEnded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueChanged(const FQuest_DialogueEntry& NewDialogue);

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    FQuest_DialogueEntry* FindDialogueEntry(const FString& DialogueID);
    FQuest_ConversationTree* FindConversationTree(const FString& ConversationID);

    UPROPERTY()
    bool bPlayerInRange;
};