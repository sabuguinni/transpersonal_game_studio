#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Greeting,
    Quest,
    Trade,
    Warning,
    Lore,
    Farewell
};

UENUM(BlueprintType)
enum class ENarr_EmotionalTone : uint8
{
    Neutral,
    Friendly,
    Hostile,
    Fearful,
    Excited,
    Sad,
    Angry
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_EmotionalTone EmotionalTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , DialogueText(FText::FromString(TEXT("...")))
        , DialogueType(ENarr_DialogueType::Greeting)
        , EmotionalTone(ENarr_EmotionalTone::Neutral)
        , Duration(3.0f)
        , AudioFilePath(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueConversation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ConversationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 RequiredTribalRank;

    FNarr_DialogueConversation()
        : ConversationID(TEXT(""))
        , bIsRepeatable(true)
        , RequiredTribalRank(0)
    {}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueConversation> AvailableConversations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanInitiateDialogue;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartConversation(const FString& ConversationID, AActor* Initiator);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsInConversation() const;

private:
    UPROPERTY()
    FNarr_DialogueConversation* CurrentConversation;

    UPROPERTY()
    int32 CurrentLineIndex;

    UPROPERTY()
    AActor* ConversationPartner;
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueActor(AActor* Actor, UNarr_DialogueComponent* DialogueComponent);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void UnregisterDialogueActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<AActor*> GetNearbyDialogueActors(const FVector& Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueAudio(const FString& AudioFilePath);

private:
    UPROPERTY()
    TMap<AActor*, UNarr_DialogueComponent*> RegisteredDialogueActors;
};

#include "Narr_DialogueSystem.generated.h"