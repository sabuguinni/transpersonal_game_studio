#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<int32> NextNodeIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsEndNode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    FNarr_DialogueNode()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        bIsEndNode = false;
        AudioFilePath = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryArc
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ArcName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ArcDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 CurrentNodeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bArcCompleted;

    FNarr_StoryArc()
    {
        ArcName = TEXT("");
        ArcDescription = TEXT("");
        CurrentNodeIndex = 0;
        bArcCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* NPCMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_StoryArc> StoryArcs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentStoryArcIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoStartDialogue;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(class APawn* PlayerPawn);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ProcessPlayerResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueNode GetCurrentDialogueNode();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStoryArc();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void SetStoryArc(int32 ArcIndex);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsStoryArcCompleted(int32 ArcIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnDialogueStarted(const FString& NPCName, const FString& FirstLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnDialogueEnded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnStoryArcCompleted(const FString& ArcName);

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    class APawn* CurrentPlayer;
    bool bInDialogue;
    
    void InitializeDefaultStoryArcs();
    void SetupInteractionComponents();
};