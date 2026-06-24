#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NarrativeDialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    ProximityEnter   UMETA(DisplayName = "Proximity Enter"),
    ProximityExit    UMETA(DisplayName = "Proximity Exit"),
    QuestStart       UMETA(DisplayName = "Quest Start"),
    QuestComplete    UMETA(DisplayName = "Quest Complete"),
    DinoEncounter    UMETA(DisplayName = "Dino Encounter"),
    PlayerDeath      UMETA(DisplayName = "Player Death"),
    SurvivalCritical UMETA(DisplayName = "Survival Critical")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType;

    FNarr_DialogueLine()
        : CharacterName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , AudioURL(TEXT(""))
        , DisplayDuration(5.0f)
        , TriggerType(ENarr_DialogueTriggerType::ProximityEnter)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString ZoneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FVector ZoneLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bOneShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasTriggered;

    FNarr_DialogueZone()
        : ZoneID(TEXT(""))
        , ZoneLocation(FVector::ZeroVector)
        , TriggerRadius(500.0f)
        , bOneShot(true)
        , bHasTriggered(false)
    {}
};

/**
 * ANarrativeDialogueManager
 * Manages all narrative dialogue zones, voice line playback triggers,
 * and character narration for the prehistoric survival world.
 * Placed once in the level — handles proximity-based dialogue delivery.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Narrative Dialogue Manager"))
class TRANSPERSONALGAME_API ANarrativeDialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeDialogueManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === DIALOGUE ZONES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Zones")
    TArray<FNarr_DialogueZone> DialogueZones;

    // === ACTIVE STATE ===
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    FNarr_DialogueLine CurrentActiveLine;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    bool bIsDialoguePlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    float DialogueTimeRemaining;

    // === CHARACTERS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Characters")
    TMap<FString, FString> CharacterVoiceURLs;

    // === FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogueLine(const FNarr_DialogueLine& Line);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CheckProximityTriggers(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueZone(const FNarr_DialogueZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ClearActiveDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialoguePlaying() const { return bIsDialoguePlaying; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const { return CurrentActiveLine; }

    UFUNCTION(CallInEditor, Category = "Narrative|Debug")
    void PopulateDefaultDialogueZones();

protected:
    void InitializeDefaultZones();
    void UpdateDialogueTimer(float DeltaTime);

private:
    UPROPERTY()
    float ProximityCheckInterval;

    UPROPERTY()
    float TimeSinceLastProximityCheck;

    APawn* CachedPlayerPawn;
};
