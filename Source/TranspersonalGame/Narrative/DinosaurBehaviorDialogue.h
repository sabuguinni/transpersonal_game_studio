
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "DinosaurBehaviorDialogue.generated.h"

// ── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DinoEncounterType : uint8
{
    None            UMETA(DisplayName = "None"),
    TRexSighted     UMETA(DisplayName = "TRex Sighted"),
    RaptorPack      UMETA(DisplayName = "Raptor Pack"),
    HerbivoreHerd   UMETA(DisplayName = "Herbivore Herd"),
    NestProximity   UMETA(DisplayName = "Nest Proximity"),
    NightPredator   UMETA(DisplayName = "Night Predator"),
    AmbushWarning   UMETA(DisplayName = "Ambush Warning")
};

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Triggered   UMETA(DisplayName = "Triggered"),
    Playing     UMETA(DisplayName = "Playing"),
    Cooldown    UMETA(DisplayName = "Cooldown"),
    Exhausted   UMETA(DisplayName = "Exhausted")
};

// ── Structs ───────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DinoEncounterType EncounterType = ENarr_DinoEncounterType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float CooldownSeconds = 30.0f;
};

USTRUCT(BlueprintType)
struct FNarr_EncounterContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    ENarr_DinoEncounterType EncounterType = ENarr_DinoEncounterType::None;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    FVector DinosaurLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    float DistanceToPlayer = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    bool bIsNight = false;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative")
    int32 DinosaurCount = 1;
};

// ── Main Actor ────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ANarr_DinosaurBehaviorDialogue : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DinosaurBehaviorDialogue();

    // Trigger a warning line based on encounter context
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerEncounterDialogue(const FNarr_EncounterContext& Context);

    // Force play a specific line by row name
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void PlayDialogueLine(FName RowName);

    // Reset all cooldowns
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void ResetCooldowns();

    // Get current dialogue state
    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    ENarr_DialogueState GetDialogueState() const { return CurrentState; }

    // Check if a line is available (not on cooldown)
    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    bool IsLineAvailable(ENarr_DinoEncounterType EncounterType) const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Select best line for encounter type
    FNarr_DialogueLine* SelectBestLine(ENarr_DinoEncounterType EncounterType);

    // Internal play logic
    void PlayLine(const FNarr_DialogueLine& Line);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    UDataTable* DialogueTable;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    UAudioComponent* AudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative",
        meta = (AllowPrivateAccess = "true"))
    ENarr_DialogueState CurrentState;

    // Track cooldowns per encounter type
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    TMap<uint8, float> CooldownTimers;

    // Currently playing line data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    FNarr_DialogueLine ActiveLine;

    // Time remaining in current cooldown
    float PlaybackTimer = 0.0f;

    // Minimum distance for NPC to trigger warning (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius = 1500.0f;
};
