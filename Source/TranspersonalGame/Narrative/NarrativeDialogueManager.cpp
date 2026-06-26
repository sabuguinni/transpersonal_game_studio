#include "NarrativeDialogueManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// ANarr_DialogueNPC Implementation
// ============================================================

ANarr_DialogueNPC::ANarr_DialogueNPC()
{
    PrimaryActorTick.bCanEverTick = false;

    // Root sphere for interaction detection
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetSphereRadius(300.0f);
    InteractionSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = InteractionSphere;

    // Visual mesh placeholder for NPC
    NPCMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCMesh"));
    NPCMesh->SetupAttachment(RootComponent);
    NPCMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));

    // Default dialogue tree — Tribal Elder Kael
    DialogueTree.NPCName = TEXT("Elder_Kael");
    DialogueTree.RequiredArc = ENarr_QuestArc::NotStarted;

    // Line 1 — Always available (survival wisdom)
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Elder Kael");
    Line1.LineText = TEXT("Before the great lizards came south, my father's father walked these plains freely. Now we move only at dawn. Clever outlasts strong every time.");
    Line1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782495455250_TribalElder_Kael.mp3");
    Line1.Condition = ENarr_DialogueCondition::Always;
    Line1.DisplayDuration = 7.0f;
    DialogueTree.Lines.Add(Line1);

    // Line 2 — After player has axe
    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Elder Kael");
    Line2.LineText = TEXT("You made the axe. Good. Now gather three sticks and build a fire before the cold comes. Without fire, the night will kill you faster than any predator.");
    Line2.Condition = ENarr_DialogueCondition::HasAxe;
    Line2.DisplayDuration = 6.0f;
    DialogueTree.Lines.Add(Line2);

    // Line 3 — After first hunt
    FNarr_DialogueLine Line3;
    Line3.SpeakerName = TEXT("Elder Kael");
    Line3.LineText = TEXT("You survived your first hunt. The tribe will eat tonight. But the Triceratops herd moves south — that means the big predators follow. We must be ready.");
    Line3.Condition = ENarr_DialogueCondition::FirstHuntDone;
    Line3.DisplayDuration = 7.0f;
    DialogueTree.Lines.Add(Line3);
}

void ANarr_DialogueNPC::BeginPlay()
{
    Super::BeginPlay();
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueNPC::OnPlayerEnterRange);
}

void ANarr_DialogueNPC::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        UE_LOG(LogTemp, Log, TEXT("NARRATIVE: Player entered dialogue range of %s — press E to talk"), *DialogueTree.NPCName);
        StartDialogue(OtherActor);
    }
}

void ANarr_DialogueNPC::StartDialogue(AActor* Interactor)
{
    if (bIsInDialogue || DialogueTree.Lines.Num() == 0) return;

    bIsInDialogue = true;
    CurrentLineIndex = 0;
    UE_LOG(LogTemp, Log, TEXT("NARRATIVE: Starting dialogue with %s"), *DialogueTree.NPCName);
}

FNarr_DialogueLine ANarr_DialogueNPC::GetNextLine(const FNarr_QuestState& QuestState)
{
    // Find next valid line based on quest state conditions
    while (CurrentLineIndex < DialogueTree.Lines.Num())
    {
        const FNarr_DialogueLine& Line = DialogueTree.Lines[CurrentLineIndex];
        CurrentLineIndex++;

        bool bConditionMet = false;
        switch (Line.Condition)
        {
            case ENarr_DialogueCondition::Always:
                bConditionMet = true;
                break;
            case ENarr_DialogueCondition::HasAxe:
                bConditionMet = QuestState.bHasStoneAxe;
                break;
            case ENarr_DialogueCondition::HasCampfire:
                bConditionMet = QuestState.bHasCampfire;
                break;
            case ENarr_DialogueCondition::FirstHuntDone:
                bConditionMet = QuestState.bFirstHuntCompleted;
                break;
            case ENarr_DialogueCondition::DinoKillCount:
                bConditionMet = QuestState.DinoKillCount >= 1;
                break;
            case ENarr_DialogueCondition::DaysSurvived:
                bConditionMet = QuestState.DaysSurvived >= 3;
                break;
            default:
                bConditionMet = true;
                break;
        }

        if (bConditionMet)
        {
            UE_LOG(LogTemp, Log, TEXT("NARRATIVE: [%s] %s"), *Line.SpeakerName, *Line.LineText);
            return Line;
        }
    }

    // No more lines — end dialogue
    EndDialogue();
    return FNarr_DialogueLine();
}

void ANarr_DialogueNPC::EndDialogue()
{
    bIsInDialogue = false;
    CurrentLineIndex = 0;
    UE_LOG(LogTemp, Log, TEXT("NARRATIVE: Dialogue ended with %s"), *DialogueTree.NPCName);
}

// ============================================================
// ANarr_QuestManager Implementation
// ============================================================

ANarr_QuestManager::ANarr_QuestManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize quest state
    QuestState.CurrentArc = ENarr_QuestArc::TribeSurvival;
    QuestState.DaysSurvived = 0;
    QuestState.DinoKillCount = 0;
    QuestState.bHasStoneAxe = false;
    QuestState.bHasCampfire = false;
    QuestState.bFirstHuntCompleted = false;
}

void ANarr_QuestManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("NARRATIVE: QuestManager initialized — Arc: TribeSurvival"));
    UE_LOG(LogTemp, Log, TEXT("NARRATIVE: Objective — Craft a Stone Axe and build a Campfire to survive the first night"));
}

void ANarr_QuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Day/night cycle tracking
    DayTimer += DeltaTime;
    if (DayTimer >= DayDuration)
    {
        DayTimer = 0.0f;
        OnDaySurvived();
    }

    // Check arc advancement each tick
    CheckArcAdvancement();
}

void ANarr_QuestManager::AdvanceArc(ENarr_QuestArc NewArc)
{
    if (NewArc <= QuestState.CurrentArc) return;

    ENarr_QuestArc OldArc = QuestState.CurrentArc;
    QuestState.CurrentArc = NewArc;

    FString OldName = GetArcDisplayName(OldArc);
    FString NewName = GetArcDisplayName(NewArc);
    UE_LOG(LogTemp, Log, TEXT("NARRATIVE: Quest arc advanced — %s → %s"), *OldName, *NewName);

    // Log new objective
    switch (NewArc)
    {
        case ENarr_QuestArc::FirstHunt:
            UE_LOG(LogTemp, Log, TEXT("NARRATIVE: New objective — Hunt a dinosaur to feed the tribe"));
            break;
        case ENarr_QuestArc::TriceratopsMigration:
            UE_LOG(LogTemp, Log, TEXT("NARRATIVE: New objective — Track the Triceratops migration route"));
            break;
        case ENarr_QuestArc::PredatorTerritory:
            UE_LOG(LogTemp, Log, TEXT("NARRATIVE: New objective — Establish a safe perimeter around the camp"));
            break;
        case ENarr_QuestArc::Completed:
            UE_LOG(LogTemp, Log, TEXT("NARRATIVE: Main story arc completed — tribe has survived the first season"));
            break;
        default:
            break;
    }
}

void ANarr_QuestManager::OnPlayerCraftedAxe()
{
    QuestState.bHasStoneAxe = true;
    UE_LOG(LogTemp, Log, TEXT("NARRATIVE: Player crafted Stone Axe — hunting now possible"));

    if (QuestState.CurrentArc == ENarr_QuestArc::TribeSurvival && QuestState.bHasCampfire)
    {
        AdvanceArc(ENarr_QuestArc::FirstHunt);
    }
}

void ANarr_QuestManager::OnPlayerCraftedCampfire()
{
    QuestState.bHasCampfire = true;
    UE_LOG(LogTemp, Log, TEXT("NARRATIVE: Player built Campfire — tribe can survive the night"));

    if (QuestState.CurrentArc == ENarr_QuestArc::TribeSurvival && QuestState.bHasStoneAxe)
    {
        AdvanceArc(ENarr_QuestArc::FirstHunt);
    }
}

void ANarr_QuestManager::OnPlayerKilledDino()
{
    QuestState.DinoKillCount++;
    UE_LOG(LogTemp, Log, TEXT("NARRATIVE: Dino kill count: %d"), QuestState.DinoKillCount);

    if (!QuestState.bFirstHuntCompleted && QuestState.DinoKillCount >= 1)
    {
        QuestState.bFirstHuntCompleted = true;
        UE_LOG(LogTemp, Log, TEXT("NARRATIVE: First hunt complete! Scout Leader Vira: 'The tribe will eat tonight.'"));

        if (QuestState.CurrentArc == ENarr_QuestArc::FirstHunt)
        {
            AdvanceArc(ENarr_QuestArc::TriceratopsMigration);
        }
    }

    // Predator territory arc after 5 kills
    if (QuestState.DinoKillCount >= 5 && QuestState.CurrentArc == ENarr_QuestArc::TriceratopsMigration)
    {
        AdvanceArc(ENarr_QuestArc::PredatorTerritory);
    }
}

void ANarr_QuestManager::OnDaySurvived()
{
    QuestState.DaysSurvived++;
    UE_LOG(LogTemp, Log, TEXT("NARRATIVE: Day %d survived"), QuestState.DaysSurvived);

    // Tracker Drek speaks after 3 days
    if (QuestState.DaysSurvived == 3)
    {
        UE_LOG(LogTemp, Log, TEXT("NARRATIVE: Tracker Drek: 'Three days in this valley. The raptors know we're here now. Time to move or fortify.'"));
    }
}

ENarr_QuestArc ANarr_QuestManager::GetCurrentArc() const
{
    return QuestState.CurrentArc;
}

FString ANarr_QuestManager::GetArcDisplayName(ENarr_QuestArc Arc) const
{
    switch (Arc)
    {
        case ENarr_QuestArc::NotStarted:            return TEXT("Not Started");
        case ENarr_QuestArc::TribeSurvival:         return TEXT("Tribe Survival");
        case ENarr_QuestArc::FirstHunt:             return TEXT("First Hunt");
        case ENarr_QuestArc::TriceratopsMigration:  return TEXT("Triceratops Migration");
        case ENarr_QuestArc::PredatorTerritory:     return TEXT("Predator Territory");
        case ENarr_QuestArc::Completed:             return TEXT("Completed");
        default:                                    return TEXT("Unknown");
    }
}

bool ANarr_QuestManager::CheckArcAdvancement()
{
    // Auto-advance to PredatorTerritory after 10 days survived
    if (QuestState.DaysSurvived >= 10 && QuestState.CurrentArc < ENarr_QuestArc::PredatorTerritory)
    {
        AdvanceArc(ENarr_QuestArc::PredatorTerritory);
        return true;
    }
    return false;
}
