#include "DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

// ─────────────────────────────────────────────────────────────────────────────
// ANarr_DialogueNPC
// ─────────────────────────────────────────────────────────────────────────────

ANarr_DialogueNPC::ANarr_DialogueNPC()
    : CurrentState(ENarr_DialogueState::Idle)
    , bQuestAvailable(true)
    , bQuestCompleted(false)
    , CurrentLineIndex(0)
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    InteractionRadius = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRadius"));
    InteractionRadius->SetupAttachment(RootComponent);
    InteractionRadius->SetSphereRadius(300.0f);
    InteractionRadius->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ANarr_DialogueNPC::BeginPlay()
{
    Super::BeginPlay();
    InitializeDialogues();
    InteractionRadius->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueNPC::OnPlayerEnterRange);
}

void ANarr_DialogueNPC::InitializeDialogues()
{
    QuestDialogues.Empty();

    // ── Hunt Quest ────────────────────────────────────────────────────────────
    {
        FNarr_QuestDialogue HuntQuest;
        HuntQuest.QuestID = TEXT("QUEST_HUNT_RAPTOR_PACK");
        HuntQuest.QuestType = ENarr_QuestType::Hunt;

        // Greeting
        FNarr_DialogueLine Greet;
        Greet.SpeakerName = TEXT("Elder");
        Greet.LineText = TEXT("You are still alive. Good.");
        Greet.TriggerState = ENarr_DialogueState::Greeting;
        HuntQuest.Lines.Add(Greet);

        // Quest offer
        FNarr_DialogueLine Offer;
        Offer.SpeakerName = TEXT("Elder");
        Offer.LineText = TEXT("The raptors took Kara. Three of them, moving fast through the eastern ridge. Hunt the pack leader — the big one with the scarred flank. Bring back proof.");
        Offer.TriggerState = ENarr_DialogueState::QuestOffer;
        HuntQuest.Lines.Add(Offer);

        // Quest active reminder
        FNarr_DialogueLine Active;
        Active.SpeakerName = TEXT("Elder");
        Active.LineText = TEXT("The scarred raptor. Eastern ridge. Do not come back empty-handed.");
        Active.TriggerState = ENarr_DialogueState::QuestActive;
        HuntQuest.Lines.Add(Active);

        // Quest complete
        FNarr_DialogueLine Done;
        Done.SpeakerName = TEXT("Elder");
        Done.LineText = TEXT("You did it. The tribe will remember this. Take these — you earned them.");
        Done.TriggerState = ENarr_DialogueState::QuestDone;
        HuntQuest.Lines.Add(Done);

        QuestDialogues.Add(HuntQuest);
    }

    // ── Gather Quest ──────────────────────────────────────────────────────────
    {
        FNarr_QuestDialogue GatherQuest;
        GatherQuest.QuestID = TEXT("QUEST_GATHER_TOOLS");
        GatherQuest.QuestType = ENarr_QuestType::Gather;

        FNarr_DialogueLine Greet;
        Greet.SpeakerName = TEXT("Scout");
        Greet.LineText = TEXT("You look strong. I need a favour.");
        Greet.TriggerState = ENarr_DialogueState::Greeting;
        GatherQuest.Lines.Add(Greet);

        FNarr_DialogueLine Offer;
        Offer.SpeakerName = TEXT("Scout");
        Offer.LineText = TEXT("Three rocks, two branches. That is all I need to make a tool that will keep us alive through the cold season. The river bank to the north — that is where you will find the best flint.");
        Offer.TriggerState = ENarr_DialogueState::QuestOffer;
        GatherQuest.Lines.Add(Offer);

        FNarr_DialogueLine Active;
        Active.SpeakerName = TEXT("Scout");
        Active.LineText = TEXT("North river bank. Three rocks, two branches. Go before the raptors return.");
        Active.TriggerState = ENarr_DialogueState::QuestActive;
        GatherQuest.Lines.Add(Active);

        FNarr_DialogueLine Done;
        Done.SpeakerName = TEXT("Scout");
        Done.LineText = TEXT("Perfect. With this flint I can make a hand axe. Here — take a spear shaft. You will need it.");
        Done.TriggerState = ENarr_DialogueState::QuestDone;
        GatherQuest.Lines.Add(Done);

        QuestDialogues.Add(GatherQuest);
    }

    // ── Defend Quest ──────────────────────────────────────────────────────────
    {
        FNarr_QuestDialogue DefendQuest;
        DefendQuest.QuestID = TEXT("QUEST_DEFEND_CAMP");
        DefendQuest.QuestType = ENarr_QuestType::Defend;

        FNarr_DialogueLine Greet;
        Greet.SpeakerName = TEXT("Lookout");
        Greet.LineText = TEXT("They are coming!");
        Greet.TriggerState = ENarr_DialogueState::Greeting;
        DefendQuest.Lines.Add(Greet);

        FNarr_DialogueLine Offer;
        Offer.SpeakerName = TEXT("Lookout");
        Offer.LineText = TEXT("A raptor pack is circling the camp perimeter. Drive them back before they reach the shelters. Use fire — they fear it.");
        Offer.TriggerState = ENarr_DialogueState::QuestOffer;
        DefendQuest.Lines.Add(Offer);

        FNarr_DialogueLine Active;
        Active.SpeakerName = TEXT("Lookout");
        Active.LineText = TEXT("Keep them away from the shelters! Drive them back!");
        Active.TriggerState = ENarr_DialogueState::QuestActive;
        DefendQuest.Lines.Add(Active);

        FNarr_DialogueLine Done;
        Done.SpeakerName = TEXT("Lookout");
        Done.LineText = TEXT("They fled. Good. The camp is safe — for now.");
        Done.TriggerState = ENarr_DialogueState::QuestDone;
        DefendQuest.Lines.Add(Done);

        QuestDialogues.Add(DefendQuest);
    }

    CurrentState = bQuestAvailable ? ENarr_DialogueState::QuestOffer : ENarr_DialogueState::Idle;
    CurrentLineIndex = 0;
}

FNarr_DialogueLine ANarr_DialogueNPC::GetCurrentLine() const
{
    TArray<FNarr_DialogueLine> StateLines = GetLinesForState(CurrentState);
    if (StateLines.IsValidIndex(CurrentLineIndex))
    {
        return StateLines[CurrentLineIndex];
    }
    FNarr_DialogueLine Empty;
    Empty.SpeakerName = NPCName;
    Empty.LineText = TEXT("...");
    return Empty;
}

void ANarr_DialogueNPC::AdvanceDialogue()
{
    TArray<FNarr_DialogueLine> StateLines = GetLinesForState(CurrentState);
    CurrentLineIndex++;
    if (CurrentLineIndex >= StateLines.Num())
    {
        CurrentLineIndex = 0;
        // Advance state machine
        if (CurrentState == ENarr_DialogueState::Greeting)
        {
            CurrentState = bQuestCompleted ? ENarr_DialogueState::QuestDone : ENarr_DialogueState::QuestOffer;
        }
        else if (CurrentState == ENarr_DialogueState::QuestOffer)
        {
            CurrentState = ENarr_DialogueState::QuestActive;
        }
    }
}

void ANarr_DialogueNPC::OnQuestCompleted(const FString& QuestID)
{
    for (const FNarr_QuestDialogue& QD : QuestDialogues)
    {
        if (QD.QuestID == QuestID)
        {
            bQuestCompleted = true;
            CurrentState = ENarr_DialogueState::QuestDone;
            CurrentLineIndex = 0;
            break;
        }
    }
}

TArray<FNarr_DialogueLine> ANarr_DialogueNPC::GetLinesForState(ENarr_DialogueState State) const
{
    TArray<FNarr_DialogueLine> Result;
    for (const FNarr_QuestDialogue& QD : QuestDialogues)
    {
        for (const FNarr_DialogueLine& Line : QD.Lines)
        {
            if (Line.TriggerState == State)
            {
                Result.Add(Line);
            }
        }
    }
    return Result;
}

void ANarr_DialogueNPC::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    // Transition to greeting when player enters range
    if (CurrentState == ENarr_DialogueState::Idle)
    {
        CurrentState = ENarr_DialogueState::Greeting;
        CurrentLineIndex = 0;
    }

    FNarr_DialogueLine Line = GetCurrentLine();
    UE_LOG(LogTemp, Log, TEXT("[Dialogue] %s: %s"), *Line.SpeakerName, *Line.LineText);
}

// ─────────────────────────────────────────────────────────────────────────────
// UNarr_DialogueManager
// ─────────────────────────────────────────────────────────────────────────────

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisteredNPCs.Empty();
    UE_LOG(LogTemp, Log, TEXT("[DialogueManager] Initialized — ready to register NPCs"));
}

void UNarr_DialogueManager::Deinitialize()
{
    RegisteredNPCs.Empty();
    Super::Deinitialize();
}

void UNarr_DialogueManager::RegisterNPC(ANarr_DialogueNPC* NPC)
{
    if (NPC && !RegisteredNPCs.Contains(NPC))
    {
        RegisteredNPCs.Add(NPC);
        UE_LOG(LogTemp, Log, TEXT("[DialogueManager] Registered NPC: %s"), *NPC->NPCName);
    }
}

void UNarr_DialogueManager::UnregisterNPC(ANarr_DialogueNPC* NPC)
{
    RegisteredNPCs.Remove(NPC);
}

ANarr_DialogueNPC* UNarr_DialogueManager::FindNPCByName(const FString& Name) const
{
    for (ANarr_DialogueNPC* NPC : RegisteredNPCs)
    {
        if (NPC && NPC->NPCName == Name)
        {
            return NPC;
        }
    }
    return nullptr;
}

void UNarr_DialogueManager::NotifyQuestCompleted(const FString& QuestID)
{
    for (ANarr_DialogueNPC* NPC : RegisteredNPCs)
    {
        if (NPC)
        {
            NPC->OnQuestCompleted(QuestID);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("[DialogueManager] Quest completed broadcast: %s"), *QuestID);
}

int32 UNarr_DialogueManager::GetRegisteredNPCCount() const
{
    return RegisteredNPCs.Num();
}
