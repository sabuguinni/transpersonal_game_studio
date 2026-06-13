#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NPCDialogueComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_DialogueType : uint8
{
    Greeting        UMETA(DisplayName = "Greeting"),
    Information     UMETA(DisplayName = "Information"),
    Quest           UMETA(DisplayName = "Quest"),
    Trade           UMETA(DisplayName = "Trade"),
    Warning         UMETA(DisplayName = "Warning"),
    Farewell        UMETA(DisplayName = "Farewell"),
    Hostile         UMETA(DisplayName = "Hostile"),
    Friendly        UMETA(DisplayName = "Friendly")
};

USTRUCT(BlueprintType)
struct FNPC_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPC_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RequiredRelationship;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequiredQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority;

    FNPC_DialogueLine()
    {
        DialogueType = ENPC_DialogueType::Greeting;
        Text = TEXT("");
        AudioPath = TEXT("");
        RequiredRelationship = 0.0f;
        bRequiresQuest = false;
        RequiredQuestID = TEXT("");
        Priority = 1;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DialogueResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ResponseText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NextDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEndsConversation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RelationshipChange;

    FNPC_DialogueResponse()
    {
        ResponseText = TEXT("");
        NextDialogueID = TEXT("");
        bEndsConversation = false;
        RelationshipChange = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FNPC_DialogueLine DialogueLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FNPC_DialogueResponse> Responses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRootNode;

    FNPC_DialogueNode()
    {
        DialogueID = TEXT("");
        bIsRootNode = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCDialogueComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // DIALOGUE SYSTEM
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNPC_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CurrentDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bInConversation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    AActor* ConversationPartner;

    // VOICE SETTINGS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString VoiceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float VoicePitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float VoiceSpeed;

    // PERSONALITY TRAITS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Friendliness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Chattiness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Helpfulness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Suspicion;

    // DIALOGUE FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartConversation(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNPC_DialogueLine GetCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FNPC_DialogueResponse> GetAvailableResponses();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueNode(const FNPC_DialogueNode& NewNode);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNPC_DialogueLine GetGreeting(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNPC_DialogueLine GetRandomDialogue(ENPC_DialogueType Type);

    // CONTEXT FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "Context")
    bool CanStartConversation(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Context")
    float GetRelationshipWith(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Context")
    bool HasRequiredQuest(const FString& QuestID);

private:
    // INTERNAL STATE
    float LastConversationTime;
    int32 ConversationCount;
    
    // HELPER FUNCTIONS
    void InitializeDefaultDialogue();
    FNPC_DialogueNode* FindDialogueNode(const FString& DialogueID);
    bool MeetsRequirements(const FNPC_DialogueLine& DialogueLine, AActor* Player);
    FString GenerateContextualResponse(AActor* Player);
};