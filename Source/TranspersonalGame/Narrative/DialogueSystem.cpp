#include "DialogueSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─── UNarr_DialogueTriggerComponent ─────────────────────────────────────────

UNarr_DialogueTriggerComponent::UNarr_DialogueTriggerComponent()
{
    TriggerType    = ENarr_DialogueTriggerType::Proximity;
    AssignedSpeaker = ENarr_SpeakerRole::Narrator;
    TriggerRadius  = 500.0f;
    bOneShot       = true;
    bHasTriggered  = false;

    SetSphereRadius(TriggerRadius);
    SetCollisionProfileName(TEXT("Trigger"));
}

void UNarr_DialogueTriggerComponent::BeginPlay()
{
    Super::BeginPlay();
    OnComponentBeginOverlap.AddDynamic(this, &UNarr_DialogueTriggerComponent::OnOverlapBegin);
}

void UNarr_DialogueTriggerComponent::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Only trigger for player characters
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    if (bOneShot && bHasTriggered) return;

    ActivateTrigger(OtherActor);
}

void UNarr_DialogueTriggerComponent::ActivateTrigger(AActor* InstigatorActor)
{
    if (bOneShot && bHasTriggered) return;

    bHasTriggered = true;

    // Notify the DialogueManager in the world
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> Managers;
    UGameplayStatics::GetAllActorsOfClass(World, ANarr_DialogueManager::StaticClass(), Managers);
    for (AActor* ManagerActor : Managers)
    {
        ANarr_DialogueManager* Manager = Cast<ANarr_DialogueManager>(ManagerActor);
        if (Manager)
        {
            Manager->TriggerDialogue(AssignedSpeaker, TriggerType);
            break;
        }
    }
}

void UNarr_DialogueTriggerComponent::ResetTrigger()
{
    bHasTriggered = false;
}

// ─── ANarr_DialogueManager ───────────────────────────────────────────────────

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;
    GlobalDialogueCooldown = 8.0f;
    LastDialogueTime = -999.0f;
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    InitialiseDefaultBanks();
}

void ANarr_DialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Future: update subtitle display timers here
}

void ANarr_DialogueManager::InitialiseDefaultBanks()
{
    // ChiefHunter bank
    {
        FNarr_NPCDialogueBank ChiefBank;
        ChiefBank.Role = ENarr_SpeakerRole::ChiefHunter;

        FNarr_DialogueLine Line1;
        Line1.LineText = TEXT("The hunting grounds are ours — but only if we move before dawn.");
        Line1.Speaker = ENarr_SpeakerRole::ChiefHunter;
        Line1.TriggerType = ENarr_DialogueTriggerType::QuestStart;
        Line1.CooldownSeconds = 60.0f;
        ChiefBank.Lines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.LineText = TEXT("Stay downwind. Always downwind.");
        Line2.Speaker = ENarr_SpeakerRole::ChiefHunter;
        Line2.TriggerType = ENarr_DialogueTriggerType::CombatAlert;
        Line2.CooldownSeconds = 20.0f;
        ChiefBank.Lines.Add(Line2);

        DialogueBanks.Add(ChiefBank);
    }

    // TribalElder bank
    {
        FNarr_NPCDialogueBank ElderBank;
        ElderBank.Role = ENarr_SpeakerRole::TribalElder;

        FNarr_DialogueLine Line1;
        Line1.LineText = TEXT("Patience is the sharpest weapon we have.");
        Line1.Speaker = ENarr_SpeakerRole::TribalElder;
        Line1.TriggerType = ENarr_DialogueTriggerType::Proximity;
        Line1.CooldownSeconds = 90.0f;
        ElderBank.Lines.Add(Line1);

        DialogueBanks.Add(ElderBank);
    }

    // ScoutRunner bank
    {
        FNarr_NPCDialogueBank ScoutBank;
        ScoutBank.Role = ENarr_SpeakerRole::ScoutRunner;

        FNarr_DialogueLine Line1;
        Line1.LineText = TEXT("Two raptors on the east bank. Wade slow.");
        Line1.Speaker = ENarr_SpeakerRole::ScoutRunner;
        Line1.TriggerType = ENarr_DialogueTriggerType::CombatAlert;
        Line1.CooldownSeconds = 30.0f;
        ScoutBank.Lines.Add(Line1);

        DialogueBanks.Add(ScoutBank);
    }

    // CampBuilder bank
    {
        FNarr_NPCDialogueBank BuilderBank;
        BuilderBank.Role = ENarr_SpeakerRole::CampBuilder;

        FNarr_DialogueLine Line1;
        Line1.LineText = TEXT("Nobody leaves this camp unguarded. Nobody.");
        Line1.Speaker = ENarr_SpeakerRole::CampBuilder;
        Line1.TriggerType = ENarr_DialogueTriggerType::Proximity;
        Line1.CooldownSeconds = 45.0f;
        BuilderBank.Lines.Add(Line1);

        DialogueBanks.Add(BuilderBank);
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Initialised %d NPC banks"), DialogueBanks.Num());
}

bool ANarr_DialogueManager::CanPlayDialogue() const
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    return (CurrentTime - LastDialogueTime) >= GlobalDialogueCooldown;
}

FNarr_DialogueLine ANarr_DialogueManager::GetNextLine(ENarr_SpeakerRole Speaker)
{
    for (FNarr_NPCDialogueBank& Bank : DialogueBanks)
    {
        if (Bank.Role == Speaker && Bank.Lines.Num() > 0)
        {
            int32 Idx = Bank.CurrentLineIndex % Bank.Lines.Num();
            Bank.CurrentLineIndex++;
            return Bank.Lines[Idx];
        }
    }
    return FNarr_DialogueLine();
}

void ANarr_DialogueManager::TriggerDialogue(ENarr_SpeakerRole Speaker, ENarr_DialogueTriggerType TriggerType)
{
    if (!CanPlayDialogue()) return;

    FNarr_DialogueLine Line = GetNextLine(Speaker);
    if (Line.LineText.IsEmpty()) return;

    LastDialogueTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    UE_LOG(LogTemp, Log, TEXT("[Dialogue] %s: %s"),
        *UEnum::GetValueAsString(Speaker),
        *Line.LineText);

    // Play audio if assigned
    if (Line.VoiceAsset)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Line.VoiceAsset);
    }
}

void ANarr_DialogueManager::RegisterDialogueBank(FNarr_NPCDialogueBank Bank)
{
    // Replace existing bank for this role, or add new
    for (FNarr_NPCDialogueBank& Existing : DialogueBanks)
    {
        if (Existing.Role == Bank.Role)
        {
            Existing = Bank;
            return;
        }
    }
    DialogueBanks.Add(Bank);
}

void ANarr_DialogueManager::ResetAllDialogue()
{
    for (FNarr_NPCDialogueBank& Bank : DialogueBanks)
    {
        Bank.CurrentLineIndex = 0;
        for (FNarr_DialogueLine& Line : Bank.Lines)
        {
            Line.bPlayedThisSession = false;
        }
    }
    LastDialogueTime = -999.0f;
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: All dialogue reset"));
}
