#include "DialogueManager.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
// UNarr_DialogueManagerComponent
// ============================================================

UNarr_DialogueManagerComponent::UNarr_DialogueManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsDialogueActive = false;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
    ActiveSequence = nullptr;
}

void UNarr_DialogueManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogues();
}

void UNarr_DialogueManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsDialogueActive || ActiveSequence == nullptr)
    {
        return;
    }

    LineTimer -= DeltaTime;
    if (LineTimer <= 0.0f)
    {
        AdvanceLineInternal();
    }
}

bool UNarr_DialogueManagerComponent::PlaySequence(FName SequenceID)
{
    for (FNarr_DialogueSequence& Seq : DialogueLibrary)
    {
        if (Seq.SequenceID == SequenceID)
        {
            if (Seq.bHasBeenPlayed && !Seq.bCanReplay)
            {
                return false;
            }

            ActiveSequence = &Seq;
            CurrentLineIndex = 0;
            bIsDialogueActive = true;

            if (Seq.Lines.Num() > 0)
            {
                ActiveLine = Seq.Lines[0];
                LineTimer = ActiveLine.DisplayDuration;
            }
            else
            {
                EndDialogue();
                return false;
            }

            return true;
        }
    }

    return false;
}

void UNarr_DialogueManagerComponent::AdvanceLine()
{
    AdvanceLineInternal();
}

void UNarr_DialogueManagerComponent::AdvanceLineInternal()
{
    if (ActiveSequence == nullptr)
    {
        return;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= ActiveSequence->Lines.Num())
    {
        EndDialogue();
        return;
    }

    ActiveLine = ActiveSequence->Lines[CurrentLineIndex];
    LineTimer = ActiveLine.DisplayDuration;
}

void UNarr_DialogueManagerComponent::EndDialogue()
{
    if (ActiveSequence != nullptr)
    {
        ActiveSequence->bHasBeenPlayed = true;
    }

    bIsDialogueActive = false;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
    ActiveSequence = nullptr;
}

bool UNarr_DialogueManagerComponent::HasSequenceBeenPlayed(FName SequenceID) const
{
    for (const FNarr_DialogueSequence& Seq : DialogueLibrary)
    {
        if (Seq.SequenceID == SequenceID)
        {
            return Seq.bHasBeenPlayed;
        }
    }
    return false;
}

void UNarr_DialogueManagerComponent::RegisterSequence(const FNarr_DialogueSequence& Sequence)
{
    for (FNarr_DialogueSequence& Existing : DialogueLibrary)
    {
        if (Existing.SequenceID == Sequence.SequenceID)
        {
            Existing = Sequence;
            return;
        }
    }
    DialogueLibrary.Add(Sequence);
}

FNarr_DialogueLine UNarr_DialogueManagerComponent::GetCurrentLine() const
{
    return ActiveLine;
}

void UNarr_DialogueManagerComponent::InitializeDefaultDialogues()
{
    // TRex Territory Warning
    {
        FNarr_DialogueSequence TRexWarning;
        TRexWarning.SequenceID = FName("TRexTerritoryWarning");
        TRexWarning.bCanReplay = false;

        FNarr_DialogueLine Line1;
        Line1.LineText = TEXT("The T-Rex has been spotted near the eastern ridge. Stay low.");
        Line1.Speaker = ENarr_SpeakerRole::TribeElder;
        Line1.DialogueType = ENarr_DialogueType::Warning;
        Line1.DisplayDuration = 4.5f;
        TRexWarning.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.LineText = TEXT("Move through the tall grass. Do not run until you reach the river.");
        Line2.Speaker = ENarr_SpeakerRole::TribeElder;
        Line2.DialogueType = ENarr_DialogueType::Warning;
        Line2.DisplayDuration = 4.0f;
        TRexWarning.Lines.Add(Line2);

        FNarr_DialogueLine Line3;
        Line3.LineText = TEXT("Running triggers the hunt response. Walk. Breathe. Survive.");
        Line3.Speaker = ENarr_SpeakerRole::TribeElder;
        Line3.DialogueType = ENarr_DialogueType::Warning;
        Line3.DisplayDuration = 4.0f;
        TRexWarning.Lines.Add(Line3);

        DialogueLibrary.Add(TRexWarning);
    }

    // River Crossing Discovery
    {
        FNarr_DialogueSequence RiverCrossing;
        RiverCrossing.SequenceID = FName("RiverCrossingDiscovery");
        RiverCrossing.bCanReplay = false;

        FNarr_DialogueLine Line1;
        Line1.LineText = TEXT("We found flint near the river crossing.");
        Line1.Speaker = ENarr_SpeakerRole::Scout;
        Line1.DialogueType = ENarr_DialogueType::Discovery;
        Line1.DisplayDuration = 3.5f;
        RiverCrossing.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.LineText = TEXT("Craft your axe before the raptors return at dusk. Three stones, two branches.");
        Line2.Speaker = ENarr_SpeakerRole::Scout;
        Line2.DialogueType = ENarr_DialogueType::QuestGiver;
        Line2.DisplayDuration = 5.0f;
        RiverCrossing.Lines.Add(Line2);

        DialogueLibrary.Add(RiverCrossing);
    }

    // Tribe Camp Welcome
    {
        FNarr_DialogueSequence CampWelcome;
        CampWelcome.SequenceID = FName("TribeCampWelcome");
        CampWelcome.bCanReplay = false;

        FNarr_DialogueLine Line1;
        Line1.LineText = TEXT("You made it back. The herd moved north — we have until morning.");
        Line1.Speaker = ENarr_SpeakerRole::Hunter;
        Line1.DialogueType = ENarr_DialogueType::Environmental;
        Line1.DisplayDuration = 4.0f;
        CampWelcome.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.LineText = TEXT("Rest. Eat. Tomorrow we hunt.");
        Line2.Speaker = ENarr_SpeakerRole::TribeElder;
        Line2.DialogueType = ENarr_DialogueType::Environmental;
        Line2.DisplayDuration = 3.0f;
        CampWelcome.Lines.Add(Line2);

        DialogueLibrary.Add(CampWelcome);
    }

    // Flint Deposit Discovery
    {
        FNarr_DialogueSequence FlintDiscovery;
        FlintDiscovery.SequenceID = FName("FlintDepositDiscovery");
        FlintDiscovery.bCanReplay = false;

        FNarr_DialogueLine Line1;
        Line1.LineText = TEXT("Flint. Good quality. This vein will last the season.");
        Line1.Speaker = ENarr_SpeakerRole::PlayerInternal;
        Line1.DialogueType = ENarr_DialogueType::Discovery;
        Line1.DisplayDuration = 3.5f;
        FlintDiscovery.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.LineText = TEXT("Gather what you can carry. The weight will slow you — choose wisely.");
        Line2.Speaker = ENarr_SpeakerRole::Narrator;
        Line2.DialogueType = ENarr_DialogueType::Discovery;
        Line2.DisplayDuration = 4.5f;
        FlintDiscovery.Lines.Add(Line2);

        DialogueLibrary.Add(FlintDiscovery);
    }
}

// ============================================================
// ANarr_DialogueTriggerActor
// ============================================================

ANarr_DialogueTriggerActor::ANarr_DialogueTriggerActor()
{
    PrimaryActorTick.bCanEverTick = true;
    TriggerRadius = 400.0f;
    bOneShot = true;
    bHasTriggered = false;
    bPlayerInRange = false;
    TriggerSequenceID = NAME_None;
}

void ANarr_DialogueTriggerActor::BeginPlay()
{
    Super::BeginPlay();
}

void ANarr_DialogueTriggerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bOneShot && bHasTriggered)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!Player)
    {
        return;
    }

    float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    bool bCurrentlyInRange = Distance <= TriggerRadius;

    if (bCurrentlyInRange && !bPlayerInRange)
    {
        bPlayerInRange = true;
        OnPlayerEnterZone();
    }
    else if (!bCurrentlyInRange && bPlayerInRange)
    {
        bPlayerInRange = false;
    }
}

void ANarr_DialogueTriggerActor::OnPlayerEnterZone()
{
    if (bOneShot && bHasTriggered)
    {
        return;
    }

    bHasTriggered = true;

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!Player)
    {
        return;
    }

    UNarr_DialogueManagerComponent* DialogueMgr = Player->FindComponentByClass<UNarr_DialogueManagerComponent>();
    if (DialogueMgr && TriggerSequenceID != NAME_None)
    {
        DialogueMgr->PlaySequence(TriggerSequenceID);
    }
}
