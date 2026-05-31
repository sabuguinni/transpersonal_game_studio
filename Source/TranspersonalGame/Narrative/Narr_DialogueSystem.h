#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "GameFramework/Actor.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Tutorial,
    Warning,
    Information,
    Quest,
    Lore
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = TEXT("");
        DialogueType = ENarr_DialogueType::Information;
        DisplayDuration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAutoAdvance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float AutoAdvanceDelay;

    FNarr_DialogueSequence()
    {
        bAutoAdvance = true;
        AutoAdvanceDelay = 4.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueSequence DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsPlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentLineIndex;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void NextLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueLine(const FString& Speaker, const FString& Text, ENarr_DialogueType Type);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasMoreLines() const;

private:
    float CurrentLineTimer;
    AActor* PlayerActor;

    void CheckPlayerProximity();
    void UpdateDialogueDisplay();
};

UCLASS()
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueSequence TriggerDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bTriggerOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequirePlayerInput;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bHasTriggered;

    UFUNCTION()
    void OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION()
    void OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(AActor* TriggeringActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ResetTrigger();
};

UCLASS()
class TRANSPERSONALGAME_API ANarr_NarrativeNPC : public AActor
{
    GENERATED_BODY()

public:
    ANarr_NarrativeNPC();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNarr_DialogueComponent* DialogueComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FString> GreetingLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FString> InformationLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float InteractionRange;

    UPROPERTY(BlueprintReadOnly, Category = "NPC")
    bool bPlayerInRange;

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void StartConversation(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void AddGreeting(const FString& Greeting);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void AddInformation(const FString& Information);

private:
    void CheckPlayerDistance();
    void SetupDefaultDialogue();
};

#include "Narr_DialogueSystem.generated.h"