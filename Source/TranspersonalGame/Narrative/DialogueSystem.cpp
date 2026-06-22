// DialogueSystem.cpp
// Agent #15 — Narrative & Dialogue Agent
// Prehistoric survival dialogue system implementation
// PROD_CYCLE_AUTO_20260622_011

#include "DialogueSystem.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

ANarr_DialogueNPC::ANarr_DialogueNPC()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root mesh — visible NPC stand-in
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // Interaction sphere — triggers dialogue when player enters
    InteractionRadius = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRadius"));
    InteractionRadius->SetupAttachment(RootComponent);
    InteractionRadius->SetSphereRadius(300.0f);
    InteractionRadius->SetCollisionProfileName(TEXT("Trigger"));

    // Default values
    NPCName = TEXT("Tribal Elder");
    NPCRole = ENarr_NPCRole::TribalElder;
    CurrentState = ENarr_DialogueState::Idle;
    InteractionRangeRadius = 300.0f;
    CurrentSequenceIndex = 0;
    CurrentLineIndex = 0;
    bPlayerInRange = false;
    CooldownTimer = 0.0f;
    bOnCooldown = false;
}

void ANarr_DialogueNPC::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    InteractionRadius->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueNPC::OnPlayerEnterRange);
    InteractionRadius->OnComponentEndOverlap.AddDynamic(this, &ANarr_DialogueNPC::OnPlayerExitRange);

    // Populate default dialogue if none set
    if (DialogueSequences.Num() == 0)
    {
        InitDefaultDialogue();
    }
}

void ANarr_DialogueNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bOnCooldown)
    {
        CooldownTimer -= DeltaTime;
        if (CooldownTimer <= 0.0f)
        {
            bOnCooldown = false;
            CooldownTimer = 0.0f;
            CurrentState = bPlayerInRange ? ENarr_DialogueState::PlayerInRange : ENarr_DialogueState::Idle;
        }
    }
}

void ANarr_DialogueNPC::BeginDialogue()
{
    if (CurrentState == ENarr_DialogueState::Cooldown || bOnCooldown)
    {
        return;
    }

    if (DialogueSequences.Num() == 0)
    {
        return;
    }

    CurrentState = ENarr_DialogueState::Active;
    CurrentLineIndex = 0;

    // Display first line
    FNarr_DialogueLine FirstLine = GetCurrentLine();
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1, FirstLine.DisplayDuration,
            FColor::Yellow,
            FString::Printf(TEXT("[%s]: %s"), *FirstLine.SpeakerName, *FirstLine.LineText)
        );
    }
}

void ANarr_DialogueNPC::AdvanceLine()
{
    if (CurrentState != ENarr_DialogueState::Active)
    {
        return;
    }

    CurrentLineIndex++;

    if (!HasMoreLines())
    {
        EndDialogue();
        return;
    }

    FNarr_DialogueLine Line = GetCurrentLine();
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1, Line.DisplayDuration,
            FColor::Yellow,
            FString::Printf(TEXT("[%s]: %s"), *Line.SpeakerName, *Line.LineText)
        );
    }
}

void ANarr_DialogueNPC::EndDialogue()
{
    CurrentState = ENarr_DialogueState::Cooldown;
    CurrentLineIndex = 0;

    // Start cooldown
    float Cooldown = 30.0f;
    if (DialogueSequences.IsValidIndex(CurrentSequenceIndex))
    {
        Cooldown = DialogueSequences[CurrentSequenceIndex].CooldownSeconds;
    }

    bOnCooldown = true;
    CooldownTimer = Cooldown;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Gray, TEXT("[Dialogue ended]"));
    }
}

FNarr_DialogueLine ANarr_DialogueNPC::GetCurrentLine() const
{
    if (DialogueSequences.IsValidIndex(CurrentSequenceIndex))
    {
        const FNarr_DialogueSequence& Seq = DialogueSequences[CurrentSequenceIndex];
        if (Seq.Lines.IsValidIndex(CurrentLineIndex))
        {
            return Seq.Lines[CurrentLineIndex];
        }
    }
    return FNarr_DialogueLine();
}

bool ANarr_DialogueNPC::HasMoreLines() const
{
    if (DialogueSequences.IsValidIndex(CurrentSequenceIndex))
    {
        return CurrentLineIndex < DialogueSequences[CurrentSequenceIndex].Lines.Num();
    }
    return false;
}

FString ANarr_DialogueNPC::GetNPCDisplayName() const
{
    return NPCName;
}

void ANarr_DialogueNPC::AddDialogueLine(const FNarr_DialogueLine& NewLine, int32 SequenceIndex)
{
    while (DialogueSequences.Num() <= SequenceIndex)
    {
        FNarr_DialogueSequence NewSeq;
        NewSeq.SequenceID = FName(*FString::Printf(TEXT("Sequence_%d"), DialogueSequences.Num()));
        DialogueSequences.Add(NewSeq);
    }
    DialogueSequences[SequenceIndex].Lines.Add(NewLine);
}

void ANarr_DialogueNPC::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        bPlayerInRange = true;
        if (CurrentState == ENarr_DialogueState::Idle)
        {
            CurrentState = ENarr_DialogueState::PlayerInRange;
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1, 3.0f, FColor::Green,
                    FString::Printf(TEXT("Press [E] to talk to %s"), *NPCName)
                );
            }
        }
    }
}

void ANarr_DialogueNPC::OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        bPlayerInRange = false;
        if (CurrentState == ENarr_DialogueState::PlayerInRange)
        {
            CurrentState = ENarr_DialogueState::Idle;
        }
    }
}

void ANarr_DialogueNPC::InitDefaultDialogue()
{
    // Sequence 0 — Elder's first warning
    FNarr_DialogueSequence Seq0;
    Seq0.SequenceID = FName("ElderWarning");
    Seq0.bRepeatable = true;
    Seq0.CooldownSeconds = 60.0f;

    FNarr_DialogueLine Line0;
    Line0.SpeakerName = NPCName;
    Line0.LineText = TEXT("Stay close to the treeline. The big one hunts by sound.");
    Line0.DisplayDuration = 5.0f;
    Line0.bTriggersQuest = false;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = NPCName;
    Line1.LineText = TEXT("The herd moves south at dawn. Follow them — that is where we hunt.");
    Line1.DisplayDuration = 5.0f;
    Line1.bTriggersQuest = true;
    Line1.QuestID = FName("Quest_FollowHerd");

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = NPCName;
    Line2.LineText = TEXT("No fire tonight. It draws them.");
    Line2.DisplayDuration = 4.0f;
    Line2.bTriggersQuest = false;

    Seq0.Lines.Add(Line0);
    Seq0.Lines.Add(Line1);
    Seq0.Lines.Add(Line2);

    DialogueSequences.Add(Seq0);
}
