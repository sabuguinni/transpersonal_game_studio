#include "DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// Narrative & Dialogue Agent #15 — DialogueSystem.cpp
// Proximity-based dialogue trigger implementation
// Prehistoric survival game — realistic, no spiritual content
// ============================================================

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
    : TriggerContext(ENarr_DialogueContext::None)
    , TriggerRadius(500.0f)
    , bHasFired(false)
    , TimeSinceLastFire(0.0f)
    , CurrentLineIndex(0)
{
    PrimaryActorTick.bCanEverTick = true;

    // Root component
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(TriggerRadius);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));

    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnSphereBeginOverlap);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnSphereEndOverlap);

    // Default dialogue sequence — raptor warning
    DialogueSequence.SequenceID = TEXT("RaptorWarning_01");
    DialogueSequence.bOneShot = true;
    DialogueSequence.CooldownSeconds = 60.0f;

    FNarr_DialogueLine Line1;
    Line1.SpeakerID = TEXT("Scout");
    Line1.LineText = TEXT("The raptor pack has taken the eastern pass. Find another way through the valley.");
    Line1.DisplayDuration = 5.0f;
    Line1.Context = ENarr_DialogueContext::RaptorWarning;

    FNarr_DialogueLine Line2;
    Line2.SpeakerID = TEXT("Scout");
    Line2.LineText = TEXT("Move fast. Stay low. They hunt by movement.");
    Line2.DisplayDuration = 4.0f;
    Line2.Context = ENarr_DialogueContext::DangerZone;

    DialogueSequence.Lines.Add(Line1);
    DialogueSequence.Lines.Add(Line2);
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Update sphere radius from property
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(TriggerRadius);
    }
}

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Track cooldown
    if (bHasFired && !DialogueSequence.bOneShot)
    {
        TimeSinceLastFire += DeltaTime;
        if (TimeSinceLastFire >= DialogueSequence.CooldownSeconds)
        {
            ResetTrigger();
        }
    }
}

void ANarr_DialogueTrigger::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Only trigger for player characters
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    OnPlayerEnterTrigger(OtherActor);
}

void ANarr_DialogueTrigger::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    OnPlayerExitTrigger(OtherActor);
}

void ANarr_DialogueTrigger::OnPlayerEnterTrigger_Implementation(AActor* OverlappingActor)
{
    // Check if we can fire
    if (bHasFired && DialogueSequence.bOneShot) return;
    if (bHasFired && TimeSinceLastFire < DialogueSequence.CooldownSeconds) return;
    if (DialogueSequence.Lines.Num() == 0) return;

    FireDialogueSequence();
}

void ANarr_DialogueTrigger::OnPlayerExitTrigger_Implementation(AActor* OverlappingActor)
{
    // Cancel ongoing dialogue if player leaves
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
}

void ANarr_DialogueTrigger::FireDialogueSequence()
{
    if (DialogueSequence.Lines.Num() == 0) return;

    CurrentLineIndex = 0;
    bHasFired = true;
    TimeSinceLastFire = 0.0f;

    // Log first line (Blueprint will handle actual UI display)
    const FNarr_DialogueLine& FirstLine = DialogueSequence.Lines[0];
    UE_LOG(LogTemp, Log, TEXT("[Narrative] %s: %s"), *FirstLine.SpeakerID, *FirstLine.LineText);

    // Schedule advance to next line
    UWorld* World = GetWorld();
    if (World && DialogueSequence.Lines.Num() > 1)
    {
        World->GetTimerManager().SetTimer(DialogueTimerHandle, this,
            &ANarr_DialogueTrigger::AdvanceDialogue,
            FirstLine.DisplayDuration, false);
    }
}

void ANarr_DialogueTrigger::AdvanceDialogue()
{
    CurrentLineIndex++;

    if (CurrentLineIndex >= DialogueSequence.Lines.Num())
    {
        // Sequence complete
        UE_LOG(LogTemp, Log, TEXT("[Narrative] Sequence '%s' complete."), *DialogueSequence.SequenceID);
        return;
    }

    const FNarr_DialogueLine& Line = DialogueSequence.Lines[CurrentLineIndex];
    UE_LOG(LogTemp, Log, TEXT("[Narrative] %s: %s"), *Line.SpeakerID, *Line.LineText);

    // Schedule next line
    UWorld* World = GetWorld();
    if (World && CurrentLineIndex < DialogueSequence.Lines.Num() - 1)
    {
        World->GetTimerManager().SetTimer(DialogueTimerHandle, this,
            &ANarr_DialogueTrigger::AdvanceDialogue,
            Line.DisplayDuration, false);
    }
}

void ANarr_DialogueTrigger::ResetTrigger()
{
    bHasFired = false;
    TimeSinceLastFire = 0.0f;
    CurrentLineIndex = 0;

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Trigger '%s' reset."), *GetActorLabel());
}
