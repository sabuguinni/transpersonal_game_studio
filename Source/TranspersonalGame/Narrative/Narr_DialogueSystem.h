#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueSystem.generated.h"

// Dialogue line data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        AudioFilePath = TEXT("");
        Duration = 3.0f;
    }
};

// Dialogue sequence data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TriggerCondition;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("DefaultSequence");
        bRepeatable = false;
        TriggerCondition = TEXT("None");
    }
};

// Narrative context for environmental storytelling
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LocationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> DiscoveredLore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DangerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> AvailableResources;

    FNarr_NarrativeContext()
    {
        LocationName = TEXT("Unknown Territory");
        BiomeType = TEXT("Wilderness");
        DangerLevel = 0.5f;
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

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DialogueMarker;

    // Dialogue data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_NarrativeContext CurrentContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsActive;

    // Current dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    int32 CurrentSequenceIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    bool bIsPlayingDialogue;

public:
    // Dialogue control functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayNextDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanInteract() const;

    // Narrative context functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateNarrativeContext(const FNarr_NarrativeContext& NewContext);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_NarrativeContext GetCurrentContext() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddDiscoveredLore(const FString& LoreEntry);

    // Environmental storytelling
    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void TriggerEnvironmentalNarrative(const FString& EventType);

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    TArray<FString> GetContextualHints() const;

    // Interaction events
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    // Helper functions
    void InitializeDefaultDialogue();
    void SetupInteractionComponents();
    FNarr_DialogueSequence* FindDialogueSequence(const FString& SequenceID);
    void ProcessDialogueTriggers();
};