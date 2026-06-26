#include "NarrativeDialogueSystem.h"
#include "Engine/World.h"

UNarrativeDialogueSystem::UNarrativeDialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentLineIndex = 0;
    bDialogueActive = false;
}

void UNarrativeDialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    LoadDefaultNPCs();
    LoadDefaultQuests();
}

void UNarrativeDialogueSystem::StartDialogue(const FString& NPCID)
{
    FNarr_NPCProfile* NPC = FindNPC(NPCID);
    if (!NPC)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogue: NPC '%s' not found"), *NPCID);
        return;
    }

    // Build dialogue lines for this NPC
    ActiveDialogueLines.Empty();
    CurrentLineIndex = 0;
    bDialogueActive = true;

    // Add NPC-specific lines based on role
    FNarr_DialogueLine Line;
    Line.SpeakerID = NPCID;
    Line.DisplayDuration = 5.0f;

    if (NPC->Role == TEXT("Elder"))
    {
        Line.Tone = ENarr_DialogueTone::Cautious;
        Line.LineText = FText::FromString(TEXT("The raptors hunt in threes. Do not run straight — cut left, cut right."));
        ActiveDialogueLines.Add(Line);

        Line.LineText = FText::FromString(TEXT("You found the flint. Good. Now learn to shape it — your life depends on the blade."));
        ActiveDialogueLines.Add(Line);
    }
    else if (NPC->Role == TEXT("Scout"))
    {
        Line.Tone = ENarr_DialogueTone::Urgent;
        Line.LineText = FText::FromString(TEXT("Three raptors came from the eastern ridge at dawn. Drek fought them off — barely."));
        ActiveDialogueLines.Add(Line);

        Line.LineText = FText::FromString(TEXT("The mammoth herd passed two days ago. Tracks lead north, toward the frozen valley."));
        ActiveDialogueLines.Add(Line);
    }
    else if (NPC->Role == TEXT("Hunter"))
    {
        Line.Tone = ENarr_DialogueTone::Commanding;
        Line.LineText = FText::FromString(TEXT("You want to know where the herd went? Bring me a stone axe. Then we talk."));
        ActiveDialogueLines.Add(Line);
    }
    else
    {
        Line.Tone = ENarr_DialogueTone::Informative;
        Line.LineText = FText::FromString(TEXT("Stay close to the river. Fresh water, fish, and the predators avoid the open banks."));
        ActiveDialogueLines.Add(Line);
    }

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Started dialogue with '%s' (%d lines)"),
        *NPCID, ActiveDialogueLines.Num());
}

void UNarrativeDialogueSystem::AdvanceDialogue()
{
    if (!bDialogueActive) return;

    CurrentLineIndex++;
    if (CurrentLineIndex >= ActiveDialogueLines.Num())
    {
        EndDialogue();
    }
}

void UNarrativeDialogueSystem::EndDialogue()
{
    bDialogueActive = false;
    CurrentLineIndex = 0;
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Dialogue ended"));
}

FText UNarrativeDialogueSystem::GetCurrentLineText() const
{
    if (!bDialogueActive || ActiveDialogueLines.Num() == 0) return FText::GetEmpty();
    if (CurrentLineIndex >= ActiveDialogueLines.Num()) return FText::GetEmpty();
    return ActiveDialogueLines[CurrentLineIndex].LineText;
}

FString UNarrativeDialogueSystem::GetCurrentSpeaker() const
{
    if (!bDialogueActive || ActiveDialogueLines.Num() == 0) return FString();
    if (CurrentLineIndex >= ActiveDialogueLines.Num()) return FString();
    return ActiveDialogueLines[CurrentLineIndex].SpeakerID;
}

bool UNarrativeDialogueSystem::StartQuest(const FString& QuestID)
{
    FNarr_QuestData* Quest = FindQuest(QuestID);
    if (!Quest)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogue: Quest '%s' not found"), *QuestID);
        return false;
    }

    if (Quest->Stage != ENarr_QuestStage::NotStarted)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogue: Quest '%s' already started"), *QuestID);
        return false;
    }

    Quest->Stage = ENarr_QuestStage::Active;
    ActiveQuest = *Quest;
    CompletedObjectives.Empty();

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Quest started: '%s'"), *QuestID);
    return true;
}

void UNarrativeDialogueSystem::CompleteObjective(const FString& ObjectiveID)
{
    if (ActiveQuest.Stage != ENarr_QuestStage::Active) return;

    if (!CompletedObjectives.Contains(ObjectiveID))
    {
        CompletedObjectives.Add(ObjectiveID);
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Objective completed: '%s'"), *ObjectiveID);

        // Check if all objectives done
        bool bAllDone = true;
        for (const FString& ObjID : ActiveQuest.ObjectiveIDs)
        {
            if (!CompletedObjectives.Contains(ObjID))
            {
                bAllDone = false;
                break;
            }
        }

        if (bAllDone)
        {
            CompleteActiveQuest();
        }
    }
}

void UNarrativeDialogueSystem::CompleteActiveQuest()
{
    if (ActiveQuest.Stage != ENarr_QuestStage::Active) return;

    ActiveQuest.Stage = ENarr_QuestStage::Completed;

    // Update in AllQuests array
    for (FNarr_QuestData& Q : AllQuests)
    {
        if (Q.QuestID == ActiveQuest.QuestID)
        {
            Q.Stage = ENarr_QuestStage::Completed;
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Quest COMPLETED: '%s'"), *ActiveQuest.QuestID);
}

void UNarrativeDialogueSystem::FailActiveQuest()
{
    if (ActiveQuest.Stage != ENarr_QuestStage::Active) return;

    ActiveQuest.Stage = ENarr_QuestStage::Failed;

    for (FNarr_QuestData& Q : AllQuests)
    {
        if (Q.QuestID == ActiveQuest.QuestID)
        {
            Q.Stage = ENarr_QuestStage::Failed;
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Quest FAILED: '%s'"), *ActiveQuest.QuestID);
}

void UNarrativeDialogueSystem::RegisterNPC(const FNarr_NPCProfile& NPCProfile)
{
    // Remove existing if present
    RegisteredNPCs.RemoveAll([&NPCProfile](const FNarr_NPCProfile& Existing)
    {
        return Existing.NPCID == NPCProfile.NPCID;
    });
    RegisteredNPCs.Add(NPCProfile);
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: NPC registered: '%s' (%s)"),
        *NPCProfile.NPCID, *NPCProfile.Role);
}

float UNarrativeDialogueSystem::GetNPCTrust(const FString& NPCID) const
{
    for (const FNarr_NPCProfile& NPC : RegisteredNPCs)
    {
        if (NPC.NPCID == NPCID) return NPC.TrustLevel;
    }
    return 0.0f;
}

void UNarrativeDialogueSystem::IncreaseTrust(const FString& NPCID, float Amount)
{
    for (FNarr_NPCProfile& NPC : RegisteredNPCs)
    {
        if (NPC.NPCID == NPCID)
        {
            NPC.TrustLevel = FMath::Clamp(NPC.TrustLevel + Amount, 0.0f, 1.0f);
            UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Trust increased for '%s' -> %.2f"),
                *NPCID, NPC.TrustLevel);
            return;
        }
    }
}

void UNarrativeDialogueSystem::LoadDefaultQuests()
{
    AllQuests.Empty();

    // Quest 1: First Hunt
    {
        FNarr_QuestData Q;
        Q.QuestID = TEXT("Q001_FirstHunt");
        Q.QuestTitle = FText::FromString(TEXT("First Hunt"));
        Q.QuestDescription = FText::FromString(TEXT("The tribe needs food. Hunt a small animal near the river and bring back the meat."));
        Q.Stage = ENarr_QuestStage::NotStarted;
        Q.ObjectiveIDs = { TEXT("OBJ_CraftSpear"), TEXT("OBJ_HuntAnimal"), TEXT("OBJ_ReturnMeat") };
        Q.RewardDescription = TEXT("Tribe trust +0.2, Stone knife");
        AllQuests.Add(Q);
    }

    // Quest 2: Raptor Warning
    {
        FNarr_QuestData Q;
        Q.QuestID = TEXT("Q002_RaptorWarning");
        Q.QuestTitle = FText::FromString(TEXT("Raptor Warning"));
        Q.QuestDescription = FText::FromString(TEXT("Three raptors have been spotted near the eastern ridge. Warn the outlying hunters before nightfall."));
        Q.Stage = ENarr_QuestStage::NotStarted;
        Q.ObjectiveIDs = { TEXT("OBJ_FindHunter_Drek"), TEXT("OBJ_FindHunter_Mara"), TEXT("OBJ_ReturnToElder") };
        Q.RewardDescription = TEXT("Tribe trust +0.3, Flint axe");
        AllQuests.Add(Q);
    }

    // Quest 3: Shelter Before Storm
    {
        FNarr_QuestData Q;
        Q.QuestID = TEXT("Q003_ShelterStorm");
        Q.QuestTitle = FText::FromString(TEXT("Shelter Before the Storm"));
        Q.QuestDescription = FText::FromString(TEXT("Dark clouds gather. Build a shelter strong enough to survive the storm or find the cave to the north."));
        Q.Stage = ENarr_QuestStage::NotStarted;
        Q.ObjectiveIDs = { TEXT("OBJ_GatherWood"), TEXT("OBJ_GatherHide"), TEXT("OBJ_BuildShelter") };
        Q.RewardDescription = TEXT("Survival knowledge +1, Warmth bonus");
        AllQuests.Add(Q);
    }

    // Quest 4: Track the Herd
    {
        FNarr_QuestData Q;
        Q.QuestID = TEXT("Q004_TrackHerd");
        Q.QuestTitle = FText::FromString(TEXT("Track the Herd"));
        Q.QuestDescription = FText::FromString(TEXT("The mammoth herd has moved north. Follow their tracks and report back where they are grazing."));
        Q.Stage = ENarr_QuestStage::NotStarted;
        Q.ObjectiveIDs = { TEXT("OBJ_FindTracks"), TEXT("OBJ_FollowTrail"), TEXT("OBJ_LocateHerd") };
        Q.RewardDescription = TEXT("Scout rank, Map fragment");
        AllQuests.Add(Q);
    }

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Loaded %d default quests"), AllQuests.Num());
}

void UNarrativeDialogueSystem::LoadDefaultNPCs()
{
    RegisteredNPCs.Empty();

    // Elder Kora
    {
        FNarr_NPCProfile NPC;
        NPC.NPCID = TEXT("NPC_Elder_Kora");
        NPC.NPCName = FText::FromString(TEXT("Kora"));
        NPC.Role = TEXT("Elder");
        NPC.TrustLevel = 0.7f;
        NPC.DialogueLineIDs = { TEXT("DLG_Kora_001"), TEXT("DLG_Kora_002"), TEXT("DLG_Kora_003") };
        RegisteredNPCs.Add(NPC);
    }

    // Scout Drek
    {
        FNarr_NPCProfile NPC;
        NPC.NPCID = TEXT("NPC_Scout_Drek");
        NPC.NPCName = FText::FromString(TEXT("Drek"));
        NPC.Role = TEXT("Scout");
        NPC.TrustLevel = 0.5f;
        NPC.DialogueLineIDs = { TEXT("DLG_Drek_001"), TEXT("DLG_Drek_002") };
        RegisteredNPCs.Add(NPC);
    }

    // Hunter Mara
    {
        FNarr_NPCProfile NPC;
        NPC.NPCID = TEXT("NPC_Hunter_Mara");
        NPC.NPCName = FText::FromString(TEXT("Mara"));
        NPC.Role = TEXT("Hunter");
        NPC.TrustLevel = 0.4f;
        NPC.DialogueLineIDs = { TEXT("DLG_Mara_001") };
        RegisteredNPCs.Add(NPC);
    }

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Loaded %d default NPCs"), RegisteredNPCs.Num());
}

FNarr_NPCProfile* UNarrativeDialogueSystem::FindNPC(const FString& NPCID)
{
    for (FNarr_NPCProfile& NPC : RegisteredNPCs)
    {
        if (NPC.NPCID == NPCID) return &NPC;
    }
    return nullptr;
}

FNarr_QuestData* UNarrativeDialogueSystem::FindQuest(const FString& QuestID)
{
    for (FNarr_QuestData& Q : AllQuests)
    {
        if (Q.QuestID == QuestID) return &Q;
    }
    return nullptr;
}
