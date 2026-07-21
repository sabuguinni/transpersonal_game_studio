// DialogueSystemManager.cpp
// Agent #15 — Narrative & Dialogue Agent
// CYCLE: PROD_CYCLE_AUTO_20260628_001
// Full implementation of dialogue trigger system and manager.

#include "DialogueSystemManager.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// ─── ANarr_DialogueTrigger ─────────────────────────────────────────────────────

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;

    TriggerVolume->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
    TriggerVolume->SetGenerateOverlapEvents(true);
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnTriggerBeginOverlap);

    CurrentState = ENarr_DialogueState::Idle;
    CurrentLineIndex = 0;
    CooldownTimer = 0.0f;
    LineTimer = 0.0f;
}

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Advance cooldown
    if (CurrentState == ENarr_DialogueState::Cooldown)
    {
        CooldownTimer -= DeltaTime;
        if (CooldownTimer <= 0.0f)
        {
            if (DialogueSequence.bOneShot == false || CurrentLineIndex == 0)
            {
                CurrentState = ENarr_DialogueState::Idle;
                CooldownTimer = 0.0f;
            }
        }
    }

    // Advance line playback
    if (CurrentState == ENarr_DialogueState::Playing)
    {
        LineTimer -= DeltaTime;
        if (LineTimer <= 0.0f)
        {
            CurrentLineIndex++;
            if (CurrentLineIndex >= DialogueSequence.Lines.Num())
            {
                // Sequence complete
                CurrentState = ENarr_DialogueState::Cooldown;
                CooldownTimer = DialogueSequence.CooldownSeconds;
                CurrentLineIndex = DialogueSequence.bOneShot ? CurrentLineIndex : 0;

                // Notify manager if present
                TArray<AActor*> Managers;
                UGameplayStatics::GetAllActorsOfClass(GetWorld(),
                    ANarr_DialogueSystemManager::StaticClass(), Managers);
                for (AActor* M : Managers)
                {
                    ANarr_DialogueSystemManager* Mgr = Cast<ANarr_DialogueSystemManager>(M);
                    if (Mgr)
                    {
                        Mgr->NotifyDialogueCompleted(this);
                    }
                }
            }
            else
            {
                // Play next line
                const FNarr_DialogueLine& NextLine = DialogueSequence.Lines[CurrentLineIndex];
                LineTimer = NextLine.PlaybackDuration;

                if (NextLine.VoiceAsset)
                {
                    UGameplayStatics::PlaySoundAtLocation(GetWorld(), NextLine.VoiceAsset,
                        GetActorLocation(), 1.0f, 1.0f, 0.0f);
                }
            }
        }
    }

    // Debug draw
    if (bDebugDraw && GetWorld() && GetWorld()->IsPlayInEditor())
    {
        FColor DrawColor = (CurrentState == ENarr_DialogueState::Playing) ?
            FColor::Green : FColor::Yellow;
        DrawDebugBox(GetWorld(), GetActorLocation(),
            TriggerVolume->GetScaledBoxExtent(), DrawColor, false, -1.0f, 0, 2.0f);
    }
}

void ANarr_DialogueTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Only trigger for player character
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    // Only trigger for locally controlled player
    if (!PlayerChar->IsLocallyControlled()) return;

    TriggerDialogue(OtherActor);
}

void ANarr_DialogueTrigger::TriggerDialogue(AActor* PlayerActor)
{
    if (!CanTrigger()) return;
    if (DialogueSequence.Lines.Num() == 0) return;

    CurrentState = ENarr_DialogueState::Playing;
    CurrentLineIndex = 0;

    const FNarr_DialogueLine& FirstLine = DialogueSequence.Lines[0];
    LineTimer = FirstLine.PlaybackDuration;

    // Play first voice line if asset is assigned
    if (FirstLine.VoiceAsset)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), FirstLine.VoiceAsset,
            GetActorLocation(), 1.0f, 1.0f, 0.0f);
    }

    // Notify manager
    TArray<AActor*> Managers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(),
        ANarr_DialogueSystemManager::StaticClass(), Managers);
    for (AActor* M : Managers)
    {
        ANarr_DialogueSystemManager* Mgr = Cast<ANarr_DialogueSystemManager>(M);
        if (Mgr)
        {
            Mgr->NotifyDialogueStarted(this);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[DialogueTrigger] Triggered: %s — Speaker: %s"),
        *DialogueSequence.SequenceID.ToString(), *GetSpeakerName());
}

void ANarr_DialogueTrigger::ResetDialogue()
{
    CurrentState = ENarr_DialogueState::Idle;
    CurrentLineIndex = 0;
    CooldownTimer = 0.0f;
    LineTimer = 0.0f;
}

bool ANarr_DialogueTrigger::CanTrigger() const
{
    return CurrentState == ENarr_DialogueState::Idle;
}

FString ANarr_DialogueTrigger::GetSpeakerName() const
{
    switch (AssignedSpeaker)
    {
    case ENarr_DialogueSpeaker::ScoutWarrior:   return TEXT("Scout Warrior");
    case ENarr_DialogueSpeaker::ElderHunter:    return TEXT("Elder Hunter");
    case ENarr_DialogueSpeaker::TribalNarrator: return TEXT("Tribal Narrator");
    case ENarr_DialogueSpeaker::RaidScout:      return TEXT("Raid Scout");
    case ENarr_DialogueSpeaker::TribalLeader:   return TEXT("Tribal Leader");
    case ENarr_DialogueSpeaker::ChiefHunter:    return TEXT("Chief Hunter");
    default:                                     return TEXT("Unknown");
    }
}

// ─── ANarr_DialogueSystemManager ──────────────────────────────────────────────

ANarr_DialogueSystemManager::ANarr_DialogueSystemManager()
{
    PrimaryActorTick.bCanEverTick = false;
    TotalDialoguesPlayed = 0;
    ActiveTrigger = nullptr;
}

void ANarr_DialogueSystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-discover all triggers in the level
    TArray<AActor*> FoundTriggers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(),
        ANarr_DialogueTrigger::StaticClass(), FoundTriggers);

    for (AActor* A : FoundTriggers)
    {
        ANarr_DialogueTrigger* Trigger = Cast<ANarr_DialogueTrigger>(A);
        if (Trigger)
        {
            RegisterTrigger(Trigger);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[DialogueSystemManager] Initialized — %d triggers registered"),
        RegisteredTriggers.Num());
}

void ANarr_DialogueSystemManager::RegisterTrigger(ANarr_DialogueTrigger* Trigger)
{
    if (!Trigger) return;
    if (RegisteredTriggers.Contains(Trigger)) return;
    RegisteredTriggers.Add(Trigger);
    UE_LOG(LogTemp, Log, TEXT("[DialogueSystemManager] Registered trigger: %s"),
        *Trigger->GetActorLabel());
}

void ANarr_DialogueSystemManager::UnregisterTrigger(ANarr_DialogueTrigger* Trigger)
{
    if (!Trigger) return;
    RegisteredTriggers.Remove(Trigger);
}

void ANarr_DialogueSystemManager::NotifyDialogueStarted(ANarr_DialogueTrigger* Trigger)
{
    ActiveTrigger = Trigger;
    TotalDialoguesPlayed++;
    UE_LOG(LogTemp, Log, TEXT("[DialogueSystemManager] Dialogue started — total played: %d"),
        TotalDialoguesPlayed);
}

void ANarr_DialogueSystemManager::NotifyDialogueCompleted(ANarr_DialogueTrigger* Trigger)
{
    if (ActiveTrigger == Trigger)
    {
        ActiveTrigger = nullptr;
    }
    UE_LOG(LogTemp, Log, TEXT("[DialogueSystemManager] Dialogue completed: %s"),
        *Trigger->GetActorLabel());
}

bool ANarr_DialogueSystemManager::IsAnyDialoguePlaying() const
{
    return ActiveTrigger != nullptr;
}

int32 ANarr_DialogueSystemManager::GetActiveTriggerCount() const
{
    int32 Count = 0;
    for (const ANarr_DialogueTrigger* T : RegisteredTriggers)
    {
        if (T && T->CurrentState == ENarr_DialogueState::Playing)
        {
            Count++;
        }
    }
    return Count;
}
