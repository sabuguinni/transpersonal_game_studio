// DialogueManager.cpp
// Agent #15 — Narrative & Dialogue Agent
// Full implementation of UNarr_DialogueManagerComponent

#include "DialogueManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UNarr_DialogueManagerComponent::UNarr_DialogueManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Check every 0.5s — not every frame
}

void UNarr_DialogueManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    // Seed the dialogue library with default lines (text only — audio assigned via Blueprint)
    FNarr_DialogueLine DangerLine;
    DangerLine.Trigger = ENarr_DialogueTrigger::DangerNear;
    DangerLine.Speaker = ENarr_CharacterRole::HunterElder;
    DangerLine.LineText = TEXT("Something is wrong. The birds stopped singing. Move to high ground — now.");
    DangerLine.CooldownSeconds = 90.0f;
    DangerLine.Priority = 2.0f;
    DialogueLibrary.Add(DangerLine);

    FNarr_DialogueLine HungerLine;
    HungerLine.Trigger = ENarr_DialogueTrigger::PlayerLowHunger;
    HungerLine.Speaker = ENarr_CharacterRole::HuntCaller;
    HungerLine.LineText = TEXT("The river bends east past the dead tree — that is where the herd crosses at dawn.");
    HungerLine.CooldownSeconds = 120.0f;
    HungerLine.Priority = 1.0f;
    DialogueLibrary.Add(HungerLine);

    FNarr_DialogueLine StampedeWarning;
    StampedeWarning.Trigger = ENarr_DialogueTrigger::StampedeWarning;
    StampedeWarning.Speaker = ENarr_CharacterRole::TrailReader;
    StampedeWarning.LineText = TEXT("The stampede is coming. I felt it in the ground — thousands of them. Run for the rocks!");
    StampedeWarning.CooldownSeconds = 180.0f;
    StampedeWarning.Priority = 3.0f;
    DialogueLibrary.Add(StampedeWarning);

    FNarr_DialogueLine NestLine;
    NestLine.Trigger = ENarr_DialogueTrigger::NestDiscovered;
    NestLine.Speaker = ENarr_CharacterRole::HunterElder;
    NestLine.LineText = TEXT("Do not touch the eggs. The mother is never far. She watches from the tree line.");
    NestLine.CooldownSeconds = 300.0f;
    NestLine.Priority = 1.5f;
    DialogueLibrary.Add(NestLine);

    FNarr_DialogueLine CampLine;
    CampLine.Trigger = ENarr_DialogueTrigger::CampFound;
    CampLine.Speaker = ENarr_CharacterRole::ScoutLeader;
    CampLine.LineText = TEXT("We found the old camp — bones of those who came before us. This place will be our home now.");
    CampLine.CooldownSeconds = 600.0f;
    CampLine.Priority = 1.0f;
    DialogueLibrary.Add(CampLine);

    FNarr_DialogueLine SurvivalLine;
    SurvivalLine.Trigger = ENarr_DialogueTrigger::DangerNear;
    SurvivalLine.Speaker = ENarr_CharacterRole::SurvivalElder;
    SurvivalLine.LineText = TEXT("The beast that hunts at night remembers faces. Make yourself dangerous. Make yourself loud.");
    SurvivalLine.CooldownSeconds = 90.0f;
    SurvivalLine.Priority = 1.8f;
    DialogueLibrary.Add(SurvivalLine);
}

void UNarr_DialogueManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    ElapsedTime += DeltaTime;

    // Auto-clear playing state after estimated duration
    if (CurrentState.bIsPlaying && ElapsedTime - CurrentState.LastPlayedTime > 20.0f)
    {
        CurrentState.bIsPlaying = false;
    }
}

void UNarr_DialogueManagerComponent::TriggerDialogue(ENarr_DialogueTrigger Trigger)
{
    if (Trigger == ENarr_DialogueTrigger::None) return;

    // Global cooldown check
    if (CurrentState.bIsPlaying) return;
    if (ElapsedTime - CurrentState.LastPlayedTime < GlobalCooldownSeconds) return;

    // Trigger-specific cooldown check
    if (IsTriggerOnCooldown(Trigger)) return;

    FNarr_DialogueLine* BestLine = FindBestLine(Trigger);
    if (BestLine)
    {
        PlayLine(*BestLine);
    }
}

void UNarr_DialogueManagerComponent::RegisterDialogueLine(FNarr_DialogueLine Line)
{
    DialogueLibrary.Add(Line);
}

FNarr_DialogueState UNarr_DialogueManagerComponent::GetDialogueState() const
{
    return CurrentState;
}

bool UNarr_DialogueManagerComponent::IsTriggerOnCooldown(ENarr_DialogueTrigger Trigger) const
{
    const float* LastTime = TriggerCooldownMap.Find(Trigger);
    if (!LastTime) return false;

    // Find cooldown for this trigger
    float CooldownDuration = 120.0f;
    for (const FNarr_DialogueLine& Line : DialogueLibrary)
    {
        if (Line.Trigger == Trigger)
        {
            CooldownDuration = Line.CooldownSeconds;
            break;
        }
    }

    return (ElapsedTime - *LastTime) < CooldownDuration;
}

void UNarr_DialogueManagerComponent::StopDialogue()
{
    CurrentState.bIsPlaying = false;
}

FNarr_DialogueLine* UNarr_DialogueManagerComponent::FindBestLine(ENarr_DialogueTrigger Trigger)
{
    FNarr_DialogueLine* BestLine = nullptr;
    float BestPriority = -1.0f;

    for (FNarr_DialogueLine& Line : DialogueLibrary)
    {
        if (Line.Trigger != Trigger) continue;
        if (Line.Priority > BestPriority)
        {
            BestPriority = Line.Priority;
            BestLine = &Line;
        }
    }

    return BestLine;
}

void UNarr_DialogueManagerComponent::PlayLine(FNarr_DialogueLine& Line)
{
    CurrentState.LastTrigger = Line.Trigger;
    CurrentState.LastPlayedTime = ElapsedTime;
    CurrentState.bIsPlaying = true;
    CurrentState.CurrentLineText = Line.LineText;

    TriggerCooldownMap.Add(Line.Trigger, ElapsedTime);

    // Play audio if asset is assigned
    if (Line.VoiceAsset && GetOwner())
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            Line.VoiceAsset,
            GetOwner()->GetActorLocation(),
            1.0f,  // Volume
            1.0f,  // Pitch
            0.0f   // StartTime
        );
    }

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Playing line: [%s] '%s'"),
        *UEnum::GetValueAsString(Line.Speaker),
        *Line.LineText);
}
