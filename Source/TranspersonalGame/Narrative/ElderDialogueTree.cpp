#include "ElderDialogueTree.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// ElderDialogueTree.cpp — Agent #15 Narrative & Dialogue
// Tribal Elder — practical tracker, survivor, not mystical
// Dialogue tree for QuestSurviveStampede
// ============================================================

AElderDialogueTree::AElderDialogueTree()
{
    PrimaryActorTick.bCanEverTick = true;

    CurrentState = ENarr_ElderDialogueState::Idle;
    bPlayerInRange = false;
    InteractionRadius = 300.0f;

    // Wire ElevenLabs TTS audio URLs generated in cycle PROD_CYCLE_AUTO_20260629_001
    AudioURL_StampedeWarning    = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782693143757_TribalElder_StampedeWarning.mp3");
    AudioURL_QuestComplete      = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782693163591_TribalElder_QuestComplete.mp3");
    AudioURL_LoreDrought        = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782693306771_TribalElder_LoreDrought.mp3");
    AudioURL_PostQuestReflection= TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782693310081_TribalElder_PostQuestReflectio.mp3");
    AudioURL_AskMoreBranch      = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782693333076_TribalElder_AskMore_Branch.mp3");
}

void AElderDialogueTree::BeginPlay()
{
    Super::BeginPlay();
    InitialiseDialogueData();
    InitialiseLoreData();
    UE_LOG(LogTemp, Log, TEXT("[ElderDialogueTree] Initialised. State: Idle. Interaction radius: %.0f"), InteractionRadius);
}

void AElderDialogueTree::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

#if WITH_EDITOR
    // Draw interaction radius in editor for placement reference
    DrawDebugSphere(GetWorld(), GetActorLocation(), InteractionRadius, 16, FColor::Yellow, false, -1.0f, 0, 1.0f);
#endif
}

void AElderDialogueTree::InitialiseDialogueData()
{
    // ---- QUEST OFFER LINES ----
    // Elder opens with urgency — stampede is imminent
    {
        FNarr_DialogueLine Line1;
        Line1.SpeakerName   = TEXT("Elder");
        Line1.LineText      = TEXT("Stranger! You must flee — the herd is panicking! A great predator stalks from the north. If the stampede reaches the valley floor, nothing survives.");
        Line1.AudioURL      = AudioURL_StampedeWarning;
        Line1.Mood          = ENarr_ElderMood::Urgent;
        Line1.DisplayDuration = 6.0f;
        QuestOfferLines.Add(Line1);

        FNarr_DialogueLine Line2;
        Line2.SpeakerName   = TEXT("Elder");
        Line2.LineText      = TEXT("I need someone fast and quiet. Reach the high ground before the herd does. Drive off the predator if you can. Will you do this?");
        Line2.AudioURL      = TEXT("");
        Line2.Mood          = ENarr_ElderMood::Urgent;
        Line2.DisplayDuration = 5.0f;
        QuestOfferLines.Add(Line2);
    }

    // ---- BRANCH: ACCEPT ----
    BranchAccept.BranchID         = TEXT("accept");
    BranchAccept.PlayerChoiceText = TEXT("I will do it.");
    BranchAccept.NextState        = ENarr_ElderDialogueState::QuestActive;
    {
        FNarr_DialogueLine R1;
        R1.SpeakerName    = TEXT("Elder");
        R1.LineText       = TEXT("Good. Move fast. The high ground is north-west — you will see the flat rocks from the ridge. Do not stop for anything.");
        R1.AudioURL       = TEXT("");
        R1.Mood           = ENarr_ElderMood::Urgent;
        R1.DisplayDuration = 4.5f;
        BranchAccept.ElderResponseLines.Add(R1);
    }

    // ---- BRANCH: REFUSE ----
    BranchRefuse.BranchID         = TEXT("refuse");
    BranchRefuse.PlayerChoiceText = TEXT("This is not my problem.");
    BranchRefuse.NextState        = ENarr_ElderDialogueState::Idle;
    {
        FNarr_DialogueLine R1;
        R1.SpeakerName    = TEXT("Elder");
        R1.LineText       = TEXT("Then you are a fool. The stampede does not care whose problem it is. When those animals run, they crush everything in their path — including you.");
        R1.AudioURL       = TEXT("");
        R1.Mood           = ENarr_ElderMood::Warning;
        R1.DisplayDuration = 5.0f;
        BranchRefuse.ElderResponseLines.Add(R1);

        FNarr_DialogueLine R2;
        R2.SpeakerName    = TEXT("Elder");
        R2.LineText       = TEXT("Come back when you change your mind. If you survive that long.");
        R2.AudioURL       = TEXT("");
        R2.Mood           = ENarr_ElderMood::Calm;
        R2.DisplayDuration = 3.5f;
        BranchRefuse.ElderResponseLines.Add(R2);
    }

    // ---- BRANCH: ASK MORE ----
    BranchAskMore.BranchID         = TEXT("ask_more");
    BranchAskMore.PlayerChoiceText = TEXT("Tell me more about this predator.");
    BranchAskMore.NextState        = ENarr_ElderDialogueState::QuestActive;
    {
        FNarr_DialogueLine R1;
        R1.SpeakerName    = TEXT("Elder");
        R1.LineText       = TEXT("So. You want to know more before you decide. Smart. Here is what I know: the predator came from the eastern ridge, not the north. It has been circling our valley for two days.");
        R1.AudioURL       = AudioURL_AskMoreBranch;
        R1.Mood           = ENarr_ElderMood::Reflective;
        R1.DisplayDuration = 7.0f;
        BranchAskMore.ElderResponseLines.Add(R1);

        FNarr_DialogueLine R2;
        R2.SpeakerName    = TEXT("Elder");
        R2.LineText       = TEXT("The stampede was not an accident — the herd was driven here deliberately. Something is pushing everything south. I do not know what yet. But I intend to find out. Will you help me?");
        R2.AudioURL       = TEXT("");
        R2.Mood           = ENarr_ElderMood::Warning;
        R2.DisplayDuration = 6.5f;
        BranchAskMore.ElderResponseLines.Add(R2);
    }

    // ---- POST-QUEST LINES ----
    {
        FNarr_DialogueLine P1;
        P1.SpeakerName    = TEXT("Elder");
        P1.LineText       = TEXT("Quest complete. You survived the stampede and drove off the predator. The herd is safe. Our tribe owes you a debt.");
        P1.AudioURL       = AudioURL_QuestComplete;
        P1.Mood           = ENarr_ElderMood::Calm;
        P1.DisplayDuration = 5.0f;
        PostQuestLines.Add(P1);

        FNarr_DialogueLine P2;
        P2.SpeakerName    = TEXT("Elder");
        P2.LineText       = TEXT("I have lived through four migrations. Each time, the signs are the same — the small animals leave first. Then the birds go silent. Then the ground shakes. If you learn to read these signs, you will survive.");
        P2.AudioURL       = AudioURL_PostQuestReflection;
        P2.Mood           = ENarr_ElderMood::Reflective;
        P2.DisplayDuration = 7.0f;
        PostQuestLines.Add(P2);

        FNarr_DialogueLine P3;
        P3.SpeakerName    = TEXT("Elder");
        P3.LineText       = TEXT("But this migration is different. Four species moving south at once — that does not happen unless something large is pushing them. Something we have not seen before.");
        P3.AudioURL       = TEXT("");
        P3.Mood           = ENarr_ElderMood::Warning;
        P3.DisplayDuration = 5.5f;
        PostQuestLines.Add(P3);
    }

    UE_LOG(LogTemp, Log, TEXT("[ElderDialogueTree] Dialogue data initialised. QuestOffer: %d lines, PostQuest: %d lines"), 
        QuestOfferLines.Num(), PostQuestLines.Num());
}

void AElderDialogueTree::InitialiseLoreData()
{
    // ---- LORE ENTRY: The Drought Migration ----
    {
        FNarr_LoreEntry Entry;
        Entry.EntryID   = TEXT("lore_drought_migration");
        Entry.Title     = TEXT("The Drought Migration");
        Entry.BodyText  = TEXT(
            "Three seasons ago, the great river to the north dried up. "
            "The prey animals followed the water south. "
            "And where the prey goes, the predators follow. "
            "The TRex that drove the herd is not lost — it is hunting. "
            "It will keep coming back until it finds what it needs, or until something stops it. "
            "The Elder has seen this pattern before. Four migrations in one lifetime. "
            "Each time, the valley fills with animals that do not belong here. "
            "Each time, the tribe must adapt or die."
        );
        Entry.AudioURL  = AudioURL_LoreDrought;
        Entry.bUnlocked = false;
        LoreEntries.Add(Entry);
    }

    // ---- LORE ENTRY: Reading Animal Signs ----
    {
        FNarr_LoreEntry Entry;
        Entry.EntryID   = TEXT("lore_animal_signs");
        Entry.Title     = TEXT("Reading the Signs");
        Entry.BodyText  = TEXT(
            "The Elder's method for predicting predator movement: "
            "1. Small ground animals (lizards, rodents) flee 2-3 days before a large predator arrives. "
            "2. Birds stop calling and leave the canopy 1 day before. "
            "3. Herbivore herds bunch together and face one direction — the direction the threat is coming FROM. "
            "4. The ground vibrates 10-15 minutes before a stampede. "
            "5. The air smells different — large predators have a distinct musk that carries on the wind. "
            "A skilled tracker reads all five signs together. Any single sign alone can be misleading."
        );
        Entry.AudioURL  = TEXT("");
        Entry.bUnlocked = false;
        LoreEntries.Add(Entry);
    }

    // ---- LORE ENTRY: The Eastern Ridge Territory ----
    {
        FNarr_LoreEntry Entry;
        Entry.EntryID   = TEXT("lore_eastern_ridge");
        Entry.Title     = TEXT("The Eastern Ridge");
        Entry.BodyText  = TEXT(
            "The eastern ridge marks the boundary of the valley's known territory. "
            "Beyond it, the Elder's tribe has never settled — the terrain is too exposed, "
            "too open for ambush predators to be avoided. "
            "The TRex that circled the valley for two days came from beyond that ridge. "
            "It means the territory east of the valley is no longer safe. "
            "Something drove it out. The Elder does not yet know what. "
            "But the pattern — multiple species moving south simultaneously — "
            "suggests a catastrophic event further north. Possibly volcanic activity, "
            "or a drought severe enough to collapse an entire ecosystem."
        );
        Entry.AudioURL  = TEXT("");
        Entry.bUnlocked = false;
        LoreEntries.Add(Entry);
    }

    UE_LOG(LogTemp, Log, TEXT("[ElderDialogueTree] Lore data initialised. %d entries."), LoreEntries.Num());
}

void AElderDialogueTree::AdvanceDialogueState(ENarr_ElderDialogueState NewState)
{
    ENarr_ElderDialogueState OldState = CurrentState;
    CurrentState = NewState;
    UE_LOG(LogTemp, Log, TEXT("[ElderDialogueTree] State: %d -> %d"), (int32)OldState, (int32)NewState);
}

void AElderDialogueTree::PlayerChoseAccept()
{
    UE_LOG(LogTemp, Log, TEXT("[ElderDialogueTree] Player chose: ACCEPT quest"));
    AdvanceDialogueState(ENarr_ElderDialogueState::BranchAccept);
    // Unlock lore after accepting
    UnlockLoreEntry(TEXT("lore_drought_migration"));
}

void AElderDialogueTree::PlayerChoseRefuse()
{
    UE_LOG(LogTemp, Log, TEXT("[ElderDialogueTree] Player chose: REFUSE quest"));
    AdvanceDialogueState(ENarr_ElderDialogueState::BranchRefuse);
}

void AElderDialogueTree::PlayerChoseAskMore()
{
    UE_LOG(LogTemp, Log, TEXT("[ElderDialogueTree] Player chose: ASK MORE"));
    AdvanceDialogueState(ENarr_ElderDialogueState::BranchAskMore);
    // Unlock eastern ridge lore when player asks more
    UnlockLoreEntry(TEXT("lore_eastern_ridge"));
}

void AElderDialogueTree::OnQuestCompleted()
{
    UE_LOG(LogTemp, Log, TEXT("[ElderDialogueTree] Quest completed — advancing to PostQuestLore"));
    AdvanceDialogueState(ENarr_ElderDialogueState::QuestComplete);
    // Unlock animal signs lore as reward for completing quest
    UnlockLoreEntry(TEXT("lore_animal_signs"));
}

FNarr_LoreEntry AElderDialogueTree::GetLoreEntry(const FString& EntryID) const
{
    for (const FNarr_LoreEntry& Entry : LoreEntries)
    {
        if (Entry.EntryID == EntryID)
        {
            return Entry;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[ElderDialogueTree] Lore entry not found: %s"), *EntryID);
    return FNarr_LoreEntry();
}

void AElderDialogueTree::UnlockLoreEntry(const FString& EntryID)
{
    for (FNarr_LoreEntry& Entry : LoreEntries)
    {
        if (Entry.EntryID == EntryID && !Entry.bUnlocked)
        {
            Entry.bUnlocked = true;
            UE_LOG(LogTemp, Log, TEXT("[ElderDialogueTree] Lore unlocked: %s — %s"), *EntryID, *Entry.Title);
            return;
        }
    }
}

FString AElderDialogueTree::GetCurrentAudioURL() const
{
    switch (CurrentState)
    {
        case ENarr_ElderDialogueState::QuestOffer:
            return AudioURL_StampedeWarning;
        case ENarr_ElderDialogueState::QuestComplete:
            return AudioURL_QuestComplete;
        case ENarr_ElderDialogueState::PostQuestLore:
            return AudioURL_PostQuestReflection;
        case ENarr_ElderDialogueState::BranchAskMore:
            return AudioURL_AskMoreBranch;
        default:
            return TEXT("");
    }
}

bool AElderDialogueTree::IsPlayerInInteractionRange(const FVector& PlayerLocation) const
{
    float DistSq = FVector::DistSquared(GetActorLocation(), PlayerLocation);
    return DistSq <= (InteractionRadius * InteractionRadius);
}

void AElderDialogueTree::DEBUG_PrintAllDialogue()
{
    UE_LOG(LogTemp, Log, TEXT("=== ELDER DIALOGUE TREE DEBUG ==="));
    UE_LOG(LogTemp, Log, TEXT("Current State: %d"), (int32)CurrentState);
    UE_LOG(LogTemp, Log, TEXT("Quest Offer Lines: %d"), QuestOfferLines.Num());
    UE_LOG(LogTemp, Log, TEXT("Post Quest Lines: %d"), PostQuestLines.Num());
    UE_LOG(LogTemp, Log, TEXT("Lore Entries: %d"), LoreEntries.Num());

    for (const FNarr_LoreEntry& Entry : LoreEntries)
    {
        UE_LOG(LogTemp, Log, TEXT("  Lore [%s] '%s' — Unlocked: %s"),
            *Entry.EntryID, *Entry.Title, Entry.bUnlocked ? TEXT("YES") : TEXT("NO"));
    }

    UE_LOG(LogTemp, Log, TEXT("Audio URLs:"));
    UE_LOG(LogTemp, Log, TEXT("  Warning: %s"), *AudioURL_StampedeWarning);
    UE_LOG(LogTemp, Log, TEXT("  Complete: %s"), *AudioURL_QuestComplete);
    UE_LOG(LogTemp, Log, TEXT("  Drought: %s"), *AudioURL_LoreDrought);
    UE_LOG(LogTemp, Log, TEXT("  Reflection: %s"), *AudioURL_PostQuestReflection);
    UE_LOG(LogTemp, Log, TEXT("  AskMore: %s"), *AudioURL_AskMoreBranch);
    UE_LOG(LogTemp, Log, TEXT("=== END DEBUG ==="));
}

void AElderDialogueTree::DEBUG_ForceQuestComplete()
{
    UE_LOG(LogTemp, Log, TEXT("[ElderDialogueTree] DEBUG: Forcing quest complete state"));
    OnQuestCompleted();
}
