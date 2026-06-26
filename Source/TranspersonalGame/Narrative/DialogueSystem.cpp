#include "DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// UNarr_DialogueComponent
// ─────────────────────────────────────────────────────────────────────────────

UNarr_DialogueComponent::UNarr_DialogueComponent()
    : NPCRole(ENarr_NPCRole::Survivor)
    , NPCName(TEXT("Unknown"))
    , CurrentState(ENarr_DialogueState::Idle)
    , ActiveTreeIndex(INDEX_NONE)
    , bDialogueActive(false)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    // Auto-populate dialogue trees based on role
    if (NPCRole == ENarr_NPCRole::Elder || NPCRole == ENarr_NPCRole::Hunter)
    {
        InitRaptorHuntDialogue();
    }
    else if (NPCRole == ENarr_NPCRole::Craftsman)
    {
        InitCraftingIntroDialogue();
    }
}

void UNarr_DialogueComponent::StartDialogue(const FString& QuestID)
{
    for (int32 i = 0; i < DialogueTrees.Num(); ++i)
    {
        if (DialogueTrees[i].QuestID == QuestID)
        {
            ActiveTreeIndex = i;
            DialogueTrees[i].CurrentLineIndex = 0;
            bDialogueActive = true;
            CurrentState = ENarr_DialogueState::QuestOffer;
            return;
        }
    }
    // Fallback: start first tree if QuestID not found
    if (DialogueTrees.Num() > 0)
    {
        ActiveTreeIndex = 0;
        DialogueTrees[0].CurrentLineIndex = 0;
        bDialogueActive = true;
        CurrentState = ENarr_DialogueState::Greeting;
    }
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentLine() const
{
    if (!bDialogueActive || ActiveTreeIndex == INDEX_NONE)
    {
        return FNarr_DialogueLine();
    }
    const FNarr_DialogueTree& Tree = DialogueTrees[ActiveTreeIndex];
    if (Tree.Lines.IsValidIndex(Tree.CurrentLineIndex))
    {
        return Tree.Lines[Tree.CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

bool UNarr_DialogueComponent::AdvanceLine()
{
    if (!bDialogueActive || ActiveTreeIndex == INDEX_NONE)
    {
        return false;
    }
    FNarr_DialogueTree& Tree = DialogueTrees[ActiveTreeIndex];
    Tree.CurrentLineIndex++;
    if (Tree.CurrentLineIndex >= Tree.Lines.Num())
    {
        EndDialogue();
        return false;
    }
    return true;
}

void UNarr_DialogueComponent::EndDialogue()
{
    bDialogueActive = false;
    ActiveTreeIndex = INDEX_NONE;
    CurrentState = ENarr_DialogueState::Idle;
}

void UNarr_DialogueComponent::SetState(ENarr_DialogueState NewState)
{
    CurrentState = NewState;
}

bool UNarr_DialogueComponent::IsDialogueActive() const
{
    return bDialogueActive;
}

void UNarr_DialogueComponent::InitRaptorHuntDialogue()
{
    FNarr_DialogueTree Tree;
    Tree.QuestID = TEXT("QUEST_RAPTOR_HUNT");

    // Line 0 — Elder Kael greeting (audio from cycle 014)
    FNarr_DialogueLine Line0;
    Line0.SpeakerName = TEXT("Elder Kael");
    Line0.DialogueText = TEXT("Listen carefully. The raptors hunt in packs. Three, maybe four of them. They will circle you, cut off your escape, then strike from behind. Your only chance is to reach high ground before they close in. Move now. Do not look back.");
    Line0.TriggerState = ENarr_DialogueState::QuestOffer;
    Line0.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782434120789_ElderHunter_Kael.mp3");
    Line0.DisplayDuration = 6.0f;
    Tree.Lines.Add(Line0);

    // Line 1 — Scout Mira intel
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Scout Mira");
    Line1.DialogueText = TEXT("I found their tracks near the river bend. Fresh. Maybe two hours old. They dragged something large — a young brachiosaur, I think. If they fed recently, they will be slower. This is our chance to push them back from the camp perimeter.");
    Line1.TriggerState = ENarr_DialogueState::QuestActive;
    Line1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782434129458_Scout_Mira.mp3");
    Line1.DisplayDuration = 6.0f;
    Tree.Lines.Add(Line1);

    // Line 2 — Quest complete / Elder reaction
    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Elder Kael");
    Line2.DialogueText = TEXT("You came back. I did not think you would. Three kills, just as the Elder asked. The tribe will eat tonight because of what you did out there. You are not just a survivor anymore. You are a hunter.");
    Line2.TriggerState = ENarr_DialogueState::QuestComplete;
    Line2.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782434159770_TribalElder_Narrator.mp3");
    Line2.DisplayDuration = 6.0f;
    Tree.Lines.Add(Line2);

    DialogueTrees.Add(Tree);
}

void UNarr_DialogueComponent::InitCraftingIntroDialogue()
{
    FNarr_DialogueTree Tree;
    Tree.QuestID = TEXT("QUEST_CRAFTING_INTRO");

    // Line 0 — Craftsman Boro teaches stone axe
    FNarr_DialogueLine Line0;
    Line0.SpeakerName = TEXT("Craftsman Boro");
    Line0.DialogueText = TEXT("The stone axe. My grandfather taught me to make one just like this. Two good rocks, one straight branch, and patience. With this, you can split bone, cut hide, and defend yourself. Keep it close. You will need it before nightfall.");
    Line0.TriggerState = ENarr_DialogueState::QuestOffer;
    Line0.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782434132013_Craftsman_Boro.mp3");
    Line0.DisplayDuration = 6.0f;
    Tree.Lines.Add(Line0);

    // Line 1 — Boro warns about the night
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Craftsman Boro");
    Line1.DialogueText = TEXT("Gather three sticks and you can make a campfire. Fire keeps the small predators away. The big ones... well. Fire slows them down. It does not stop them. Nothing stops a T-Rex except distance.");
    Line1.TriggerState = ENarr_DialogueState::Lore;
    Line1.AudioURL = TEXT("");
    Line1.DisplayDuration = 5.0f;
    Tree.Lines.Add(Line1);

    DialogueTrees.Add(Tree);
}

// ─────────────────────────────────────────────────────────────────────────────
// ANarr_NPCDialogueActor
// ─────────────────────────────────────────────────────────────────────────────

ANarr_NPCDialogueActor::ANarr_NPCDialogueActor()
    : InteractionRadius(300.0f)
{
    PrimaryActorTick.bCanEverTick = false;

    ProximityTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("ProximityTrigger"));
    RootComponent = ProximityTrigger;
    ProximityTrigger->SetSphereRadius(InteractionRadius);
    ProximityTrigger->SetCollisionProfileName(TEXT("Trigger"));

    DialogueComponent = CreateDefaultSubobject<UNarr_DialogueComponent>(TEXT("DialogueComponent"));
}

void ANarr_NPCDialogueActor::BeginPlay()
{
    Super::BeginPlay();

    ProximityTrigger->SetSphereRadius(InteractionRadius);

    ProximityTrigger->OnComponentBeginOverlap.AddDynamic(
        this, &ANarr_NPCDialogueActor::OnPlayerEnterRange);
    ProximityTrigger->OnComponentEndOverlap.AddDynamic(
        this, &ANarr_NPCDialogueActor::OnPlayerExitRange);
}

void ANarr_NPCDialogueActor::OnPlayerEnterRange(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || !DialogueComponent) return;

    // Only trigger for player characters
    if (OtherActor->ActorHasTag(TEXT("Player")))
    {
        DialogueComponent->StartDialogue(TEXT("QUEST_RAPTOR_HUNT"));
    }
}

void ANarr_NPCDialogueActor::OnPlayerExitRange(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (!OtherActor || !DialogueComponent) return;

    if (OtherActor->ActorHasTag(TEXT("Player")))
    {
        if (DialogueComponent->IsDialogueActive())
        {
            DialogueComponent->EndDialogue();
        }
    }
}
