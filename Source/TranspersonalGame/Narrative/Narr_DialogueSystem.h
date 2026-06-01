#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Warning,
    Lore,
    Tutorial,
    Tribal,
    Survival
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresTrigger;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = TEXT("");
        DialogueType = ENarr_DialogueType::Lore;
        Duration = 3.0f;
        bRequiresTrigger = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryMarker
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString MarkerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_DialogueLine> AssociatedDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bDiscovered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float DiscoveryRadius;

    FNarr_StoryMarker()
    {
        MarkerName = TEXT("Unknown Marker");
        Location = FVector::ZeroVector;
        bDiscovered = false;
        DiscoveryRadius = 500.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanRepeatDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bHasBeenTriggered;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentDialogueIndex;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void NextDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasMoreDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ResetDialogue();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueTriggered(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueCompleted();

protected:
    UFUNCTION()
    void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    void InitializeDialogueLines();
    void SetupTribalDialogue();
    void SetupSurvivalWarnings();
};