// NarrativeDialogueSystem.cpp
// Transpersonal Game Studio — Agent #15 Narrative & Dialogue
// Full implementation of the NPC dialogue and narrative trigger system

#include "NarrativeDialogueSystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
// UNarr_DialogueLibrary
// ============================================================

UNarr_DialogueLibrary::UNarr_DialogueLibrary()
{
    DialogueLines.Empty();
}

void UNarr_DialogueLibrary::AddLine(const FNarr_DialogueLine& Line)
{
    DialogueLines.Add(Line);
}

bool UNarr_DialogueLibrary::GetLineByID(const FName& LineID, FNarr_DialogueLine& OutLine) const
{
    for (const FNarr_DialogueLine& Line : DialogueLines)
    {
        if (Line.LineID == LineID)
        {
            OutLine = Line;
            return true;
        }
    }
    return false;
}

TArray<FNarr_DialogueLine> UNarr_DialogueLibrary::GetLinesByTrigger(ENarr_DialogueTriggerType TriggerType) const
{
    TArray<FNarr_DialogueLine> Result;
    for (const FNarr_DialogueLine& Line : DialogueLines)
    {
        if (Line.TriggerType == TriggerType)
        {
            Result.Add(Line);
        }
    }
    return Result;
}

TArray<FNarr_DialogueLine> UNarr_DialogueLibrary::GetLinesBySpeaker(ENarr_SpeakerRole SpeakerRole) const
{
    TArray<FNarr_DialogueLine> Result;
    for (const FNarr_DialogueLine& Line : DialogueLines)
    {
        if (Line.SpeakerRole == SpeakerRole)
        {
            Result.Add(Line);
        }
    }
    return Result;
}

// ============================================================
// ANarr_DialogueTriggerActor
// ============================================================

ANarr_DialogueTriggerActor::ANarr_DialogueTriggerActor()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(300.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));

    TriggerType = ENarr_DialogueTriggerType::Proximity;
    SpeakerRole = ENarr_SpeakerRole::Scout;
    bHasTriggered = false;
    bRepeatTrigger = false;
    CooldownSeconds = 30.0f;
    bIsOnCooldown = false;
}

void ANarr_DialogueTriggerActor::BeginPlay()
{
    Super::BeginPlay();
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTriggerActor::OnSphereBeginOverlap);
}

void ANarr_DialogueTriggerActor::OnSphereBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || bIsOnCooldown)
    {
        return;
    }

    // Only trigger for the player pawn
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (OtherActor != PlayerPawn)
    {
        return;
    }

    if (bHasTriggered && !bRepeatTrigger)
    {
        return;
    }

    FireDialogue();
}

void ANarr_DialogueTriggerActor::FireDialogue()
{
    if (!DialogueLibrary)
    {
        return;
    }

    TArray<FNarr_DialogueLine> Lines = DialogueLibrary->GetLinesByTrigger(TriggerType);
    if (Lines.Num() == 0)
    {
        return;
    }

    // Pick a random line from matching trigger type
    int32 Index = FMath::RandRange(0, Lines.Num() - 1);
    FNarr_DialogueLine& SelectedLine = Lines[Index];

    OnDialogueTriggered.Broadcast(SelectedLine);
    bHasTriggered = true;

    if (bRepeatTrigger)
    {
        bIsOnCooldown = true;
        GetWorldTimerManager().SetTimer(
            CooldownTimerHandle,
            this,
            &ANarr_DialogueTriggerActor::ResetCooldown,
            CooldownSeconds,
            false
        );
    }
}

void ANarr_DialogueTriggerActor::ResetCooldown()
{
    bIsOnCooldown = false;
}

// ============================================================
// UNarr_DialogueManagerComponent
// ============================================================

UNarr_DialogueManagerComponent::UNarr_DialogueManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    ActiveLineIndex = 0;
    bIsDialogueActive = false;
    LineDisplayDuration = 4.0f;
}

void UNarr_DialogueManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    ActiveLines.Empty();
}

void UNarr_DialogueManagerComponent::StartDialogueSequence(const TArray<FNarr_DialogueLine>& Lines)
{
    if (Lines.Num() == 0 || bIsDialogueActive)
    {
        return;
    }

    ActiveLines = Lines;
    ActiveLineIndex = 0;
    bIsDialogueActive = true;

    DisplayCurrentLine();
}

void UNarr_DialogueManagerComponent::DisplayCurrentLine()
{
    if (ActiveLineIndex >= ActiveLines.Num())
    {
        EndDialogue();
        return;
    }

    const FNarr_DialogueLine& CurrentLine = ActiveLines[ActiveLineIndex];
    OnLineDisplayed.Broadcast(CurrentLine);

    // Auto-advance after display duration
    GetWorld()->GetTimerManager().SetTimer(
        LineAdvanceTimerHandle,
        this,
        &UNarr_DialogueManagerComponent::AdvanceLine,
        LineDisplayDuration,
        false
    );
}

void UNarr_DialogueManagerComponent::AdvanceLine()
{
    ActiveLineIndex++;
    DisplayCurrentLine();
}

void UNarr_DialogueManagerComponent::EndDialogue()
{
    bIsDialogueActive = false;
    ActiveLines.Empty();
    ActiveLineIndex = 0;
    OnDialogueEnded.Broadcast();
}

void UNarr_DialogueManagerComponent::SkipCurrentLine()
{
    if (!bIsDialogueActive)
    {
        return;
    }

    GetWorld()->GetTimerManager().ClearTimer(LineAdvanceTimerHandle);
    AdvanceLine();
}

bool UNarr_DialogueManagerComponent::IsDialogueActive() const
{
    return bIsDialogueActive;
}

// ============================================================
// ANarr_NPCDialogueActor — NPC that speaks contextual lines
// ============================================================

ANarr_NPCDialogueActor::ANarr_NPCDialogueActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.0f);
    InteractionSphere->SetCollisionProfileName(TEXT("Trigger"));

    DialogueManager = CreateDefaultSubobject<UNarr_DialogueManagerComponent>(TEXT("DialogueManager"));

    SpeakerRole = ENarr_SpeakerRole::TribalElder;
    NPCName = FText::FromString(TEXT("Elder"));
    bCanInteract = true;
    InteractionCooldown = 20.0f;
    bOnInteractionCooldown = false;
}

void ANarr_NPCDialogueActor::BeginPlay()
{
    Super::BeginPlay();
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_NPCDialogueActor::OnPlayerEnterRange);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_NPCDialogueActor::OnPlayerExitRange);
}

void ANarr_NPCDialogueActor::OnPlayerEnterRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (OtherActor == PlayerPawn)
    {
        OnPlayerNearby.Broadcast(true);
    }
}

void ANarr_NPCDialogueActor::OnPlayerExitRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bOnlyTouchingTrigger)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (OtherActor == PlayerPawn)
    {
        OnPlayerNearby.Broadcast(false);
    }
}

void ANarr_NPCDialogueActor::Interact()
{
    if (!bCanInteract || bOnInteractionCooldown || !DialogueLibrary)
    {
        return;
    }

    TArray<FNarr_DialogueLine> Lines = DialogueLibrary->GetLinesBySpeaker(SpeakerRole);
    if (Lines.Num() == 0)
    {
        return;
    }

    DialogueManager->StartDialogueSequence(Lines);

    bOnInteractionCooldown = true;
    GetWorldTimerManager().SetTimer(
        InteractionCooldownHandle,
        this,
        &ANarr_NPCDialogueActor::ResetInteractionCooldown,
        InteractionCooldown,
        false
    );
}

void ANarr_NPCDialogueActor::ResetInteractionCooldown()
{
    bOnInteractionCooldown = false;
}
