#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Narr_TribalDialogue.generated.h"

UENUM(BlueprintType)
enum class ENarr_TribalRole : uint8
{
    None = 0,
    Elder,
    Hunter,
    Gatherer,
    Shaman,
    Scout,
    Warrior,
    Crafter
};

UENUM(BlueprintType)
enum class ENarr_DialogueContext : uint8
{
    Greeting = 0,
    Warning,
    Teaching,
    Trading,
    Celebration,
    Mourning,
    Planning,
    Storytelling
};

USTRUCT(BlueprintType)
struct FNarr_TribalDialogueLine : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ENarr_TribalRole SpeakerRole = ENarr_TribalRole::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ENarr_DialogueContext Context = ENarr_DialogueContext::Greeting;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float EmotionalIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bRequiresPlayerResponse = false;

    FNarr_TribalDialogueLine()
    {
        SpeakerRole = ENarr_TribalRole::None;
        Context = ENarr_DialogueContext::Greeting;
        DialogueText = TEXT("");
        AudioPath = TEXT("");
        EmotionalIntensity = 1.0f;
        bRequiresPlayerResponse = false;
    }
};

UCLASS(ClassGroup=(Narrative), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_TribalDialogue : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_TribalDialogue();

    UFUNCTION(BlueprintCallable, Category = "Tribal Dialogue")
    FNarr_TribalDialogueLine GetDialogueForContext(ENarr_TribalRole Role, ENarr_DialogueContext Context);

    UFUNCTION(BlueprintCallable, Category = "Tribal Dialogue")
    TArray<FNarr_TribalDialogueLine> GetAllDialogueForRole(ENarr_TribalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Tribal Dialogue")
    void AddDialogueLine(const FNarr_TribalDialogueLine& NewLine);

    UFUNCTION(BlueprintCallable, Category = "Tribal Dialogue")
    bool HasDialogueForContext(ENarr_TribalRole Role, ENarr_DialogueContext Context) const;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Dialogue")
    TArray<FNarr_TribalDialogueLine> TribalDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Dialogue")
    ENarr_TribalRole AssignedRole = ENarr_TribalRole::None;

private:
    void InitializeDefaultDialogues();
    void LoadDialogueDatabase();
};