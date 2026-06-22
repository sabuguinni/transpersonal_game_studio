#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueSystem.generated.h"

// Narr_ prefix for all types per UE5 compilation rules

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Active      UMETA(DisplayName = "Active"),
    Waiting     UMETA(DisplayName = "Waiting"),
    Completed   UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Scout           UMETA(DisplayName = "Scout"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Survivor        UMETA(DisplayName = "Survivor")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bRequiresPlayerResponse;

    FNarr_DialogueLine()
        : SpeakerName(TEXT(""))
        , LineText(TEXT(""))
        , DisplayDuration(3.0f)
        , bRequiresPlayerResponse(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsQuestDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LinkedQuestID;

    FNarr_DialogueTree()
        : DialogueID(TEXT(""))
        , bIsQuestDialogue(false)
        , LinkedQuestID(TEXT(""))
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentLineIndex;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsPlayerInRange(AActor* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddDialogueTree(const FNarr_DialogueTree& NewTree);

protected:
    virtual void BeginPlay() override;

private:
    FNarr_DialogueTree* ActiveTree;
};

UCLASS()
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TriggerDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bOneShot;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bHasTriggered;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerEnterRange(AActor* Player);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
};
