
#include "DialogueSystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// ============================================================
// UNarr_DialogueComponent
// ============================================================

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    NPCSpeaker = ENarr_DialogueSpeaker::Unknown;
    InteractionRadius = 300.0f;
    bIsInDialogue = false;
    CurrentLineIndex = 0;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentNodeID = DialogueTree.RootNodeID;
    CurrentLineIndex = 0;
}

bool UNarr_DialogueComponent::StartDialogue(AActor* Initiator)
{
    if (!Initiator || bIsInDialogue)
    {
        return false;
    }

    // Range check
    AActor* Owner = GetOwner();
    if (Owner)
    {
        float Distance = FVector::Dist(Owner->GetActorLocation(), Initiator->GetActorLocation());
        if (Distance > InteractionRadius)
        {
            return false;
        }
    }

    bIsInDialogue = true;
    CurrentNodeID = DialogueTree.RootNodeID;
    CurrentLineIndex = 0;
    return true;
}

void UNarr_DialogueComponent::EndDialogue()
{
    bIsInDialogue = false;
    CurrentNodeID = DialogueTree.RootNodeID;
    CurrentLineIndex = 0;
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentLine() const
{
    // Find current node
    for (const FNarr_DialogueNode& Node : DialogueTree.Nodes)
    {
        if (Node.NodeID == CurrentNodeID)
        {
            if (Node.Lines.IsValidIndex(CurrentLineIndex))
            {
                return Node.Lines[CurrentLineIndex];
            }
        }
    }
    return FNarr_DialogueLine();
}

bool UNarr_DialogueComponent::AdvanceDialogue()
{
    if (!bIsInDialogue)
    {
        return false;
    }

    for (const FNarr_DialogueNode& Node : DialogueTree.Nodes)
    {
        if (Node.NodeID == CurrentNodeID)
        {
            // More lines in this node?
            if (CurrentLineIndex + 1 < Node.Lines.Num())
            {
                CurrentLineIndex++;
                return true;
            }

            // Move to next node
            if (Node.bIsTerminal || Node.NextNodeIDs.Num() == 0)
            {
                EndDialogue();
                return false;
            }

            CurrentNodeID = Node.NextNodeIDs[0];
            CurrentLineIndex = 0;
            return true;
        }
    }

    EndDialogue();
    return false;
}

bool UNarr_DialogueComponent::CheckCondition(ENarr_DialogueCondition Condition, AActor* Player) const
{
    if (Condition == ENarr_DialogueCondition::None)
    {
        return true;
    }

    // Condition checks are resolved by game state — stub returns true for now
    // Full implementation hooks into TranspersonalGameState
    switch (Condition)
    {
        case ENarr_DialogueCondition::HasCraftedAxe:
        case ENarr_DialogueCondition::SurvivedNight:
        case ENarr_DialogueCondition::KilledRaptor:
        case ENarr_DialogueCondition::FoundHerdTracks:
            return true; // Resolved via GameState in full integration
        case ENarr_DialogueCondition::HealthBelow30:
        case ENarr_DialogueCondition::HungerBelow50:
            return false; // Default: not in danger state
        default:
            return true;
    }
}

// ============================================================
// ANarr_DialogueManager
// ============================================================

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    PopulateDefaultDialogue();
}

void ANarr_DialogueManager::RegisterVoiceLine(FName LineID, const FString& AudioURL)
{
    VoiceLineAudioURLs.Add(LineID, AudioURL);
}

FString ANarr_DialogueManager::GetVoiceLineURL(FName LineID) const
{
    const FString* URL = VoiceLineAudioURLs.Find(LineID);
    return URL ? *URL : FString();
}

void ANarr_DialogueManager::RegisterDialogueTree(const FNarr_DialogueTree& Tree)
{
    // Remove existing tree for this NPC if present
    AllDialogueTrees.RemoveAll([&Tree](const FNarr_DialogueTree& Existing)
    {
        return Existing.OwnerNPC == Tree.OwnerNPC;
    });
    AllDialogueTrees.Add(Tree);
}

FNarr_DialogueTree ANarr_DialogueManager::FindTreeByNPC(ENarr_DialogueSpeaker NPC) const
{
    for (const FNarr_DialogueTree& Tree : AllDialogueTrees)
    {
        if (Tree.OwnerNPC == NPC)
        {
            return Tree;
        }
    }
    return FNarr_DialogueTree();
}

void ANarr_DialogueManager::PopulateDefaultDialogue()
{
    // --- Elder Kael — survival wisdom ---
    FNarr_DialogueTree KaelTree;
    KaelTree.TreeID = FName("ElderKael_Main");
    KaelTree.OwnerNPC = ENarr_DialogueSpeaker::ElderKael;
    KaelTree.RootNodeID = FName("Kael_Root");

    // Root node — greeting
    FNarr_DialogueNode KaelRoot;
    KaelRoot.NodeID = FName("Kael_Root");
    KaelRoot.bIsTerminal = false;
    KaelRoot.NextNodeIDs.Add(FName("Kael_Raptor_Tip"));

    FNarr_DialogueLine KaelLine1;
    KaelLine1.LineID = FName("Kael_001");
    KaelLine1.Speaker = ENarr_DialogueSpeaker::ElderKael;
    KaelLine1.LineText = FText::FromString(TEXT("Stay low. Do not run. The Raptor sees movement, not stillness."));
    KaelLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782805103468_Elder_Kael_Survival_Tip.mp3");
    KaelLine1.DisplayDuration = 5.0f;
    KaelLine1.RequiredCondition = ENarr_DialogueCondition::None;
    KaelRoot.Lines.Add(KaelLine1);

    KaelTree.Nodes.Add(KaelRoot);

    // Second node — philosophy
    FNarr_DialogueNode KaelRaptorTip;
    KaelRaptorTip.NodeID = FName("Kael_Raptor_Tip");
    KaelRaptorTip.bIsTerminal = true;

    FNarr_DialogueLine KaelLine2;
    KaelLine2.LineID = FName("Kael_002");
    KaelLine2.Speaker = ENarr_DialogueSpeaker::ElderKael;
    KaelLine2.LineText = FText::FromString(TEXT("We do not name the beasts. Names give them power over your fear. Fear is information. Use it."));
    KaelLine2.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782805111003_Elder_Kael_Philosophy.mp3");
    KaelLine2.DisplayDuration = 5.5f;
    KaelLine2.RequiredCondition = ENarr_DialogueCondition::None;
    KaelRaptorTip.Lines.Add(KaelLine2);

    KaelTree.Nodes.Add(KaelRaptorTip);
    RegisterDialogueTree(KaelTree);

    // --- Scout Mira — herd tracking ---
    FNarr_DialogueTree MiraTree;
    MiraTree.TreeID = FName("ScoutMira_Main");
    MiraTree.OwnerNPC = ENarr_DialogueSpeaker::ScoutMira;
    MiraTree.RootNodeID = FName("Mira_Root");

    FNarr_DialogueNode MiraRoot;
    MiraRoot.NodeID = FName("Mira_Root");
    MiraRoot.bIsTerminal = true;

    FNarr_DialogueLine MiraLine1;
    MiraLine1.LineID = FName("Mira_001");
    MiraLine1.Speaker = ENarr_DialogueSpeaker::ScoutMira;
    MiraLine1.LineText = FText::FromString(TEXT("The herd moved south three days ago. Forty, maybe fifty of them. When the big ones move — the hunters follow."));
    MiraLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782805113200_Scout_Mira_Herd_Report.mp3");
    MiraLine1.DisplayDuration = 6.0f;
    MiraLine1.RequiredCondition = ENarr_DialogueCondition::FoundHerdTracks;
    MiraRoot.Lines.Add(MiraLine1);

    MiraTree.Nodes.Add(MiraRoot);
    RegisterDialogueTree(MiraTree);

    // Register voice line URLs
    RegisterVoiceLine(FName("Narrator_001"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782805087261_Narrator_Survival.mp3"));
    RegisterVoiceLine(FName("Kael_001"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782805103468_Elder_Kael_Survival_Tip.mp3"));
    RegisterVoiceLine(FName("Kael_002"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782805111003_Elder_Kael_Philosophy.mp3"));
    RegisterVoiceLine(FName("Mira_001"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782805113200_Scout_Mira_Herd_Report.mp3"));

    UE_LOG(LogTemp, Log, TEXT("ANarr_DialogueManager: Populated %d dialogue trees, %d voice lines"), AllDialogueTrees.Num(), VoiceLineAudioURLs.Num());
}
