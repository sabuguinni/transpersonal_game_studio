// DialogueManager.cpp
// Agent #15 — Narrative & Dialogue Agent
// Full implementation of the dialogue manager system.

#include "DialogueManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ADialogueManager::ADialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    ProximityTriggerRadius = 800.0f;
    DangerCheckInterval = 3.0f;
    DangerCheckTimer = 0.0f;
    bDialogueActive = false;

    // Populate hardcoded survival dialogue lines
    PopulateDefaultLines();
}

void ADialogueManager::BeginPlay()
{
    Super::BeginPlay();
    RegisterHardcodedLines();
}

void ADialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDialogueActive)
    {
        AdvanceDialogueTimer(DeltaTime);
    }

    DangerCheckTimer += DeltaTime;
    if (DangerCheckTimer >= DangerCheckInterval)
    {
        DangerCheckTimer = 0.0f;
        CheckProximityTriggers();
    }
}

void ADialogueManager::TriggerDialogueLine(FName LineID)
{
    if (bDialogueActive)
    {
        return; // Don't interrupt active dialogue
    }

    FNarr_DialogueLine Line = GetLineByID(LineID);
    if (Line.LineID == NAME_None)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Line ID '%s' not found"), *LineID.ToString());
        return;
    }

    if (Line.bPlayOnce && HasLineBeenPlayed(LineID))
    {
        return;
    }

    // Activate dialogue
    CurrentDialogue.LineID = Line.LineID;
    CurrentDialogue.SpeakerName = Line.SpeakerName;
    CurrentDialogue.LineText = Line.LineText;
    CurrentDialogue.RemainingTime = Line.DisplayDuration;
    CurrentDialogue.bIsActive = true;
    bDialogueActive = true;

    // Mark as played
    if (Line.bPlayOnce)
    {
        PlayedLineIDs.AddUnique(LineID);
    }

    // Play voice audio if available
    if (Line.VoiceAudio.IsValid())
    {
        USoundBase* Sound = Line.VoiceAudio.Get();
        if (Sound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), Sound);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Playing line '%s' — %s: %s"),
        *LineID.ToString(), *Line.SpeakerName, *Line.LineText);
}

void ADialogueManager::TriggerDialogueByRole(ENarr_SpeakerRole Role, ENarr_DialogueTrigger Trigger)
{
    for (const FNarr_DialogueLine& Line : HardcodedLines)
    {
        if (Line.SpeakerRole == Role && Line.TriggerType == Trigger)
        {
            if (!HasLineBeenPlayed(Line.LineID))
            {
                TriggerDialogueLine(Line.LineID);
                return;
            }
        }
    }
}

void ADialogueManager::StopCurrentDialogue()
{
    CurrentDialogue.bIsActive = false;
    CurrentDialogue.RemainingTime = 0.0f;
    bDialogueActive = false;
}

bool ADialogueManager::HasLineBeenPlayed(FName LineID) const
{
    return PlayedLineIDs.Contains(LineID);
}

FNarr_DialogueLine ADialogueManager::GetLineByID(FName LineID) const
{
    for (const FNarr_DialogueLine& Line : HardcodedLines)
    {
        if (Line.LineID == LineID)
        {
            return Line;
        }
    }

    // Check DataTable if assigned
    if (DialogueDataTable)
    {
        FNarr_DialogueLine* Row = DialogueDataTable->FindRow<FNarr_DialogueLine>(LineID, TEXT("DialogueManager"));
        if (Row)
        {
            return *Row;
        }
    }

    return FNarr_DialogueLine(); // Empty line with NAME_None
}

TArray<FNarr_DialogueLine> ADialogueManager::GetLinesByTrigger(ENarr_DialogueTrigger Trigger) const
{
    TArray<FNarr_DialogueLine> Result;
    for (const FNarr_DialogueLine& Line : HardcodedLines)
    {
        if (Line.TriggerType == Trigger)
        {
            Result.Add(Line);
        }
    }
    return Result;
}

void ADialogueManager::RegisterHardcodedLines()
{
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Registered %d hardcoded dialogue lines"), HardcodedLines.Num());
}

void ADialogueManager::DebugPlayRandomLine()
{
    if (HardcodedLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: No lines to play"));
        return;
    }

    int32 RandomIndex = FMath::RandRange(0, HardcodedLines.Num() - 1);
    TriggerDialogueLine(HardcodedLines[RandomIndex].LineID);
}

void ADialogueManager::AdvanceDialogueTimer(float DeltaTime)
{
    CurrentDialogue.RemainingTime -= DeltaTime;
    if (CurrentDialogue.RemainingTime <= 0.0f)
    {
        StopCurrentDialogue();
    }
}

void ADialogueManager::CheckProximityTriggers()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    float RadiusSq = ProximityTriggerRadius * ProximityTriggerRadius;

    for (const FNarr_DialogueLine& Line : HardcodedLines)
    {
        if (Line.TriggerType == ENarr_DialogueTrigger::OnProximity)
        {
            float DistSq = FVector::DistSquared(PlayerLocation, GetActorLocation());
            if (DistSq <= RadiusSq && !HasLineBeenPlayed(Line.LineID))
            {
                TriggerDialogueLine(Line.LineID);
                break;
            }
        }
    }
}

void ADialogueManager::PopulateDefaultLines()
{
    // Tribal Leader — danger warning
    {
        FNarr_DialogueLine Line;
        Line.LineID = FName("KARA_TREX_WARNING");
        Line.SpeakerName = TEXT("Kara");
        Line.SpeakerRole = ENarr_SpeakerRole::TribalLeader;
        Line.LineText = TEXT("Listen carefully. The T-Rex hunts by sound and smell — not sight. Stay downwind and move slowly.");
        Line.DisplayDuration = 6.0f;
        Line.TriggerType = ENarr_DialogueTrigger::OnDangerNear;
        Line.bPlayOnce = true;
        HardcodedLines.Add(Line);
    }

    // Scout — raptor nest intel
    {
        FNarr_DialogueLine Line;
        Line.LineID = FName("DREN_RAPTOR_NEST");
        Line.SpeakerName = TEXT("Dren");
        Line.SpeakerRole = ENarr_SpeakerRole::Scout;
        Line.LineText = TEXT("We found the raptor nest at dawn. Four eggs, still warm. The pack returns before nightfall.");
        Line.DisplayDuration = 5.0f;
        Line.TriggerType = ENarr_DialogueTrigger::OnQuestStart;
        Line.bPlayOnce = true;
        HardcodedLines.Add(Line);
    }

    // Elder — herd wisdom
    {
        FNarr_DialogueLine Line;
        Line.LineID = FName("MOSA_HERD_WISDOM");
        Line.SpeakerName = TEXT("Mosa");
        Line.SpeakerRole = ENarr_SpeakerRole::Elder;
        Line.LineText = TEXT("Follow the Brachiosaurus herd — not too close. They remember safe paths and water sources.");
        Line.DisplayDuration = 7.0f;
        Line.TriggerType = ENarr_DialogueTrigger::OnFirstVisit;
        Line.bPlayOnce = true;
        HardcodedLines.Add(Line);
    }

    // Player narration — intro
    {
        FNarr_DialogueLine Line;
        Line.LineID = FName("PLAYER_INTRO_NARRATION");
        Line.SpeakerName = TEXT("Survivor");
        Line.SpeakerRole = ENarr_SpeakerRole::PlayerVoice;
        Line.LineText = TEXT("First winter alone. No tribe. No fire. I survived by remembering what my mother taught me — read the ground, read the wind, read the silence.");
        Line.DisplayDuration = 8.0f;
        Line.TriggerType = ENarr_DialogueTrigger::OnFirstVisit;
        Line.bPlayOnce = true;
        HardcodedLines.Add(Line);
    }
}
