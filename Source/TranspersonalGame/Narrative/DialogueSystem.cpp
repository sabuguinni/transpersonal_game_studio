#include "DialogueSystem.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

// ─── UNarr_DialogueManagerComponent ─────────────────────────────────────────

UNarr_DialogueManagerComponent::UNarr_DialogueManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bDialogueActive = false;
    CurrentNodeIndex = 0;
}

void UNarr_DialogueManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    BuildElderDialogueTrees();
}

void UNarr_DialogueManagerComponent::BuildElderDialogueTrees()
{
    // ── ELDER INTRO TREE (triggers QUEST_RAPTOR_DEN) ──────────────────────
    {
        FNarr_DialogueTree ElderIntro;
        ElderIntro.TreeID = TEXT("ELDER_INTRO");
        ElderIntro.NPCID  = TEXT("Elder");
        ElderIntro.EntryNodeIndex = 0;

        // Node 0 — opening warning
        FNarr_DialogueNode N0;
        N0.NodeIndex   = 0;
        N0.SpeakerID   = TEXT("Elder");
        N0.DialogueText = TEXT("Listen. The Elder speaks only once. You are not the first to walk into that valley. Others went before you — stronger, faster, better armed. None came back. The raptors remember every human who entered their territory. They learn. They adapt. You must be smarter than them, not stronger.");
        N0.AudioURL    = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782534764127_Elder_NPC_QuestIntro.mp3");
        N0.bIsTerminal = false;

        FNarr_DialogueChoice C0A;
        C0A.ChoiceText    = TEXT("I understand. What must I do?");
        C0A.NextNodeIndex = 1;

        FNarr_DialogueChoice C0B;
        C0B.ChoiceText    = TEXT("I am not afraid of raptors.");
        C0B.NextNodeIndex = 2;

        N0.Choices.Add(C0A);
        N0.Choices.Add(C0B);
        ElderIntro.Nodes.Add(N0);

        // Node 1 — quest assignment (cooperative response)
        FNarr_DialogueNode N1;
        N1.NodeIndex      = 1;
        N1.SpeakerID      = TEXT("Elder");
        N1.DialogueText   = TEXT("Good. Study their patterns. Strike when they are separated. And if you hear that clicking sound — the one they make when they call to each other — run. Do not look back. Just run. Find their nest in the eastern valley. Destroy the eggs. All of them.");
        N1.AudioURL       = TEXT("");
        N1.OnEnterAction  = ENarr_DialogueAction::StartQuest;
        N1.ActionPayload  = TEXT("QUEST_RAPTOR_DEN");
        N1.bIsTerminal    = true;
        ElderIntro.Nodes.Add(N1);

        // Node 2 — bravado response, still assigns quest
        FNarr_DialogueNode N2;
        N2.NodeIndex      = 2;
        N2.SpeakerID      = TEXT("Elder");
        N2.DialogueText   = TEXT("Fear is not the problem. Arrogance is. The ones who were not afraid — they are bones in the valley now. Go to the eastern ridge. Find the nest. Destroy every egg you find. Come back alive. That is the only thing that matters.");
        N2.AudioURL       = TEXT("");
        N2.OnEnterAction  = ENarr_DialogueAction::StartQuest;
        N2.ActionPayload  = TEXT("QUEST_RAPTOR_DEN");
        N2.bIsTerminal    = true;
        ElderIntro.Nodes.Add(N2);

        RegisterDialogueTree(ElderIntro);
    }

    // ── QUEST_RAPTOR_DEN COMPLETE TREE ────────────────────────────────────
    {
        FNarr_DialogueTree ElderComplete;
        ElderComplete.TreeID = TEXT("ELDER_RAPTOR_DEN_COMPLETE");
        ElderComplete.NPCID  = TEXT("Elder");
        ElderComplete.EntryNodeIndex = 0;

        FNarr_DialogueNode N0;
        N0.NodeIndex   = 0;
        N0.SpeakerID   = TEXT("Elder");
        N0.DialogueText = TEXT("You destroyed the nest. Good. The pack will scatter — confused, angry, but without a home they will not stay in this valley. We have maybe three days before they regroup. Use that time. Gather what you need. When they return, and they will return, we must be ready. You proved yourself today. The tribe sees it. I see it.");
        N0.AudioURL    = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782534769428_Elder_NPC_QuestComplete.mp3");
        N0.OnEnterAction = ENarr_DialogueAction::CompleteQuest;
        N0.ActionPayload = TEXT("QUEST_RAPTOR_DEN");
        N0.bIsTerminal = true;
        ElderComplete.Nodes.Add(N0);

        RegisterDialogueTree(ElderComplete);
    }

    // ── QUEST_SURVIVE_NIGHT TREE ──────────────────────────────────────────
    {
        FNarr_DialogueTree ElderNight;
        ElderNight.TreeID = TEXT("ELDER_SURVIVE_NIGHT");
        ElderNight.NPCID  = TEXT("Elder");
        ElderNight.EntryNodeIndex = 0;

        FNarr_DialogueNode N0;
        N0.NodeIndex   = 0;
        N0.SpeakerID   = TEXT("Elder");
        N0.DialogueText = TEXT("Night is coming. You need fire. Not for warmth — for survival. The predators that hunt in darkness, they fear the flame. Find dry wood before the sun drops below the ridge. Strike the flint against iron stone until you see sparks. Feed the sparks slowly — breath on them, do not smother them. A fire that dies in the night is a death sentence. Keep it burning until dawn.");
        N0.AudioURL    = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782534797258_Elder_NPC_SurviveNight.mp3");
        N0.OnEnterAction = ENarr_DialogueAction::StartQuest;
        N0.ActionPayload = TEXT("QUEST_SURVIVE_NIGHT");
        N0.bIsTerminal = true;
        ElderNight.Nodes.Add(N0);

        RegisterDialogueTree(ElderNight);
    }

    // ── QUEST_FIRST_HUNT TREE ─────────────────────────────────────────────
    {
        FNarr_DialogueTree ElderHunt;
        ElderHunt.TreeID = TEXT("ELDER_FIRST_HUNT");
        ElderHunt.NPCID  = TEXT("Elder");
        ElderHunt.EntryNodeIndex = 0;

        FNarr_DialogueNode N0;
        N0.NodeIndex   = 0;
        N0.SpeakerID   = TEXT("Elder");
        N0.DialogueText = TEXT("The hunt is simple. Find the prey. Study it. Wait for the moment it is alone, away from the herd. Then strike fast and hard — one clean blow if you can manage it. A wounded animal that escapes will warn the others. Bring back the meat before the scavengers smell it. The tribe needs food. We are counting on you.");
        N0.AudioURL    = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782534801559_Elder_NPC_FirstHunt.mp3");
        N0.OnEnterAction = ENarr_DialogueAction::StartQuest;
        N0.ActionPayload = TEXT("QUEST_FIRST_HUNT");
        N0.bIsTerminal = true;
        ElderHunt.Nodes.Add(N0);

        RegisterDialogueTree(ElderHunt);
    }

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Built 4 Elder dialogue trees (INTRO, RAPTOR_DEN_COMPLETE, SURVIVE_NIGHT, FIRST_HUNT)"));
}

void UNarr_DialogueManagerComponent::RegisterDialogueTree(const FNarr_DialogueTree& Tree)
{
    // Remove existing tree with same ID before adding
    DialogueTrees.RemoveAll([&Tree](const FNarr_DialogueTree& Existing)
    {
        return Existing.TreeID == Tree.TreeID;
    });
    DialogueTrees.Add(Tree);
}

bool UNarr_DialogueManagerComponent::StartDialogue(const FString& TreeID)
{
    if (bDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogue: Cannot start '%s' — dialogue already active"), *TreeID);
        return false;
    }

    FNarr_DialogueTree* Tree = FindTree(TreeID);
    if (!Tree)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogue: Tree '%s' not found"), *TreeID);
        return false;
    }

    ActiveTreeID     = TreeID;
    CurrentNodeIndex = Tree->EntryNodeIndex;
    bDialogueActive  = true;

    // Execute action on entry node if present
    if (Tree->Nodes.IsValidIndex(CurrentNodeIndex))
    {
        ExecuteNodeAction(Tree->Nodes[CurrentNodeIndex]);
    }

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Started tree '%s' at node %d"), *TreeID, CurrentNodeIndex);
    return true;
}

bool UNarr_DialogueManagerComponent::AdvanceDialogue(int32 ChoiceIndex)
{
    if (!bDialogueActive)
    {
        return false;
    }

    FNarr_DialogueTree* Tree = FindTree(ActiveTreeID);
    if (!Tree)
    {
        EndDialogue();
        return false;
    }

    if (!Tree->Nodes.IsValidIndex(CurrentNodeIndex))
    {
        EndDialogue();
        return false;
    }

    const FNarr_DialogueNode& CurrentNode = Tree->Nodes[CurrentNodeIndex];

    // Terminal node — end conversation
    if (CurrentNode.bIsTerminal || CurrentNode.Choices.Num() == 0)
    {
        EndDialogue();
        return false;
    }

    // Validate choice index
    if (!CurrentNode.Choices.IsValidIndex(ChoiceIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogue: Invalid choice index %d"), ChoiceIndex);
        return false;
    }

    const FNarr_DialogueChoice& Choice = CurrentNode.Choices[ChoiceIndex];
    if (Choice.NextNodeIndex < 0)
    {
        EndDialogue();
        return false;
    }

    CurrentNodeIndex = Choice.NextNodeIndex;

    // Execute action on new node
    if (Tree->Nodes.IsValidIndex(CurrentNodeIndex))
    {
        ExecuteNodeAction(Tree->Nodes[CurrentNodeIndex]);
    }

    return true;
}

void UNarr_DialogueManagerComponent::EndDialogue()
{
    bDialogueActive  = false;
    ActiveTreeID     = TEXT("");
    CurrentNodeIndex = 0;
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Conversation ended"));
}

FNarr_DialogueNode UNarr_DialogueManagerComponent::GetCurrentNode() const
{
    const FNarr_DialogueTree* Tree = const_cast<UNarr_DialogueManagerComponent*>(this)->FindTree(ActiveTreeID);
    if (Tree && Tree->Nodes.IsValidIndex(CurrentNodeIndex))
    {
        return Tree->Nodes[CurrentNodeIndex];
    }
    return FNarr_DialogueNode();
}

bool UNarr_DialogueManagerComponent::IsDialogueActive() const
{
    return bDialogueActive;
}

FNarr_DialogueTree* UNarr_DialogueManagerComponent::FindTree(const FString& TreeID)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID)
        {
            return &Tree;
        }
    }
    return nullptr;
}

void UNarr_DialogueManagerComponent::ExecuteNodeAction(const FNarr_DialogueNode& Node)
{
    if (Node.OnEnterAction == ENarr_DialogueAction::None)
    {
        return;
    }

    switch (Node.OnEnterAction)
    {
        case ENarr_DialogueAction::StartQuest:
            UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: ACTION StartQuest('%s')"), *Node.ActionPayload);
            // Quest manager integration: broadcast event for QuestManagerComponent to pick up
            break;

        case ENarr_DialogueAction::CompleteQuest:
            UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: ACTION CompleteQuest('%s')"), *Node.ActionPayload);
            break;

        case ENarr_DialogueAction::GiveItem:
            UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: ACTION GiveItem('%s')"), *Node.ActionPayload);
            break;

        case ENarr_DialogueAction::SetFlag:
            UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: ACTION SetFlag('%s')"), *Node.ActionPayload);
            break;

        case ENarr_DialogueAction::EndConversation:
            EndDialogue();
            break;

        default:
            break;
    }
}

// ─── ANarr_NPCDialogueActor ──────────────────────────────────────────────────

ANarr_NPCDialogueActor::ANarr_NPCDialogueActor()
{
    PrimaryActorTick.bCanEverTick = false;

    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    RootComponent     = InteractionVolume;
    InteractionVolume->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
    InteractionVolume->SetCollisionProfileName(TEXT("Trigger"));

    DialogueManager = CreateDefaultSubobject<UNarr_DialogueManagerComponent>(TEXT("DialogueManager"));
}

void ANarr_NPCDialogueActor::BeginPlay()
{
    Super::BeginPlay();

    InteractionVolume->OnComponentBeginOverlap.AddDynamic(
        this, &ANarr_NPCDialogueActor::OnPlayerEnterRange);

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: NPC '%s' ready at %s"),
        *NPCID, *GetActorLocation().ToString());
}

void ANarr_NPCDialogueActor::OnPlayerEnterRange(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor)
    {
        return;
    }

    // Only trigger for player characters
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar)
    {
        return;
    }

    if (DialogueManager && !DialogueManager->IsDialogueActive())
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Player entered range of NPC '%s' — starting '%s'"),
            *NPCID, *DefaultDialogueTreeID);
        DialogueManager->StartDialogue(DefaultDialogueTreeID);
    }
}
