
#include "DinosaurBehaviorDialogue.h"
#include "Engine/DataTable.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

ANarr_DinosaurBehaviorDialogue::ANarr_DinosaurBehaviorDialogue()
{
    PrimaryActorTick.bCanEverTick = true;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudio"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    CurrentState = ENarr_DialogueState::Idle;
    PlaybackTimer = 0.0f;
    TriggerRadius = 1500.0f;
}

void ANarr_DinosaurBehaviorDialogue::BeginPlay()
{
    Super::BeginPlay();

    // Initialize cooldown map with all encounter types set to 0
    for (uint8 i = 0; i < (uint8)ENarr_DinoEncounterType::AmbushWarning + 1; ++i)
    {
        CooldownTimers.Add(i, 0.0f);
    }
}

void ANarr_DinosaurBehaviorDialogue::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Tick down cooldown timers
    for (auto& Pair : CooldownTimers)
    {
        if (Pair.Value > 0.0f)
        {
            Pair.Value = FMath::Max(0.0f, Pair.Value - DeltaTime);
        }
    }

    // Tick playback timer
    if (CurrentState == ENarr_DialogueState::Playing)
    {
        PlaybackTimer -= DeltaTime;
        if (PlaybackTimer <= 0.0f)
        {
            // Line finished — enter cooldown
            float* CooldownPtr = CooldownTimers.Find((uint8)ActiveLine.EncounterType);
            if (CooldownPtr)
            {
                *CooldownPtr = ActiveLine.CooldownSeconds;
            }
            CurrentState = ENarr_DialogueState::Cooldown;
        }
    }
    else if (CurrentState == ENarr_DialogueState::Cooldown)
    {
        // Check if all cooldowns expired → return to Idle
        bool bAllExpired = true;
        for (auto& Pair : CooldownTimers)
        {
            if (Pair.Value > 0.0f)
            {
                bAllExpired = false;
                break;
            }
        }
        if (bAllExpired)
        {
            CurrentState = ENarr_DialogueState::Idle;
        }
    }
}

void ANarr_DinosaurBehaviorDialogue::TriggerEncounterDialogue(const FNarr_EncounterContext& Context)
{
    // Only trigger if idle or cooldown allows
    if (CurrentState == ENarr_DialogueState::Playing)
    {
        return;
    }

    if (!IsLineAvailable(Context.EncounterType))
    {
        return;
    }

    // Distance check — only trigger if player is within radius
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }

    float DistToPlayer = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
    if (DistToPlayer > TriggerRadius)
    {
        return;
    }

    FNarr_DialogueLine* SelectedLine = SelectBestLine(Context.EncounterType);
    if (SelectedLine)
    {
        PlayLine(*SelectedLine);
    }
}

void ANarr_DinosaurBehaviorDialogue::PlayDialogueLine(FName RowName)
{
    if (!DialogueTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurBehaviorDialogue: No DialogueTable assigned"));
        return;
    }

    FNarr_DialogueLine* Row = DialogueTable->FindRow<FNarr_DialogueLine>(RowName, TEXT("PlayDialogueLine"));
    if (Row)
    {
        PlayLine(*Row);
    }
}

void ANarr_DinosaurBehaviorDialogue::ResetCooldowns()
{
    for (auto& Pair : CooldownTimers)
    {
        Pair.Value = 0.0f;
    }
    CurrentState = ENarr_DialogueState::Idle;
    UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorDialogue: All cooldowns reset"));
}

bool ANarr_DinosaurBehaviorDialogue::IsLineAvailable(ENarr_DinoEncounterType EncounterType) const
{
    const float* CooldownPtr = CooldownTimers.Find((uint8)EncounterType);
    if (CooldownPtr)
    {
        return *CooldownPtr <= 0.0f;
    }
    return true;
}

FNarr_DialogueLine* ANarr_DinosaurBehaviorDialogue::SelectBestLine(ENarr_DinoEncounterType EncounterType)
{
    if (!DialogueTable)
    {
        return nullptr;
    }

    FNarr_DialogueLine* BestLine = nullptr;
    float BestPriority = -1.0f;

    TArray<FNarr_DialogueLine*> AllRows;
    DialogueTable->GetAllRows<FNarr_DialogueLine>(TEXT("SelectBestLine"), AllRows);

    for (FNarr_DialogueLine* Row : AllRows)
    {
        if (!Row) continue;
        if (Row->EncounterType != EncounterType) continue;
        if (Row->Priority > BestPriority)
        {
            BestPriority = Row->Priority;
            BestLine = Row;
        }
    }

    return BestLine;
}

void ANarr_DinosaurBehaviorDialogue::PlayLine(const FNarr_DialogueLine& Line)
{
    ActiveLine = Line;
    CurrentState = ENarr_DialogueState::Playing;

    // Estimate playback duration from text length (~12 chars/sec average)
    int32 CharCount = Line.LineText.ToString().Len();
    PlaybackTimer = FMath::Max(3.0f, (float)CharCount / 12.0f);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorDialogue: Playing line — %s (%.1fs)"),
        *Line.LineText.ToString(), PlaybackTimer);

    // If AudioComponent has a sound, play it
    if (AudioComponent && AudioComponent->Sound)
    {
        AudioComponent->Play();
    }
}
