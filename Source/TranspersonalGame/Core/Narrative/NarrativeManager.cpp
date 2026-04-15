#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    DialogueState = ENarr_DialogueState::Inactive;
    CurrentLineIndex = 0;
    DialogueStartTime = 0.0f;
    CurrentNPCName = TEXT("");
    CurrentSequenceID = TEXT("");
    
    LoadDefaultNPCProfiles();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager initialized with %d NPC profiles"), NPCProfiles.Num());
}

void UNarrativeManager::Deinitialize()
{
    EndDialogue();
    NPCProfiles.Empty();
    
    Super::Deinitialize();
}

bool UNarrativeManager::StartDialogue(const FString& NPCName, const FString& SequenceID)
{
    if (DialogueState == ENarr_DialogueState::InProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue - another dialogue is already in progress"));
        return false;
    }

    if (!NPCProfiles.Contains(NPCName))
    {
        UE_LOG(LogTemp, Error, TEXT("NPC profile not found: %s"), *NPCName);
        return false;
    }

    FNarr_NPCProfile& NPCProfile = NPCProfiles[NPCName];
    
    // Find the requested dialogue sequence
    FNarr_DialogueSequence* FoundSequence = nullptr;
    for (FNarr_DialogueSequence& Sequence : NPCProfile.AvailableDialogues)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            FoundSequence = &Sequence;
            break;
        }
    }

    if (!FoundSequence)
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue sequence not found: %s for NPC: %s"), *SequenceID, *NPCName);
        return false;
    }

    // Check if dialogue was already completed and is not repeatable
    if (!FoundSequence->bIsRepeatable && IsDialogueCompleted(NPCName, SequenceID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue already completed and not repeatable: %s"), *SequenceID);
        return false;
    }

    CurrentNPCName = NPCName;
    CurrentSequenceID = SequenceID;
    CurrentLineIndex = 0;
    DialogueState = ENarr_DialogueState::InProgress;
    DialogueStartTime = GetWorld()->GetTimeSeconds();

    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s with NPC: %s"), *SequenceID, *NPCName);
    return true;
}

void UNarrativeManager::EndDialogue()
{
    if (DialogueState == ENarr_DialogueState::InProgress)
    {
        DialogueState = ENarr_DialogueState::Completed;
        
        if (!CurrentNPCName.IsEmpty() && !CurrentSequenceID.IsEmpty())
        {
            MarkDialogueCompleted(CurrentNPCName, CurrentSequenceID);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Ended dialogue: %s"), *CurrentSequenceID);
    }

    CurrentNPCName = TEXT("");
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    DialogueState = ENarr_DialogueState::Inactive;
}

bool UNarrativeManager::IsDialogueActive() const
{
    return DialogueState == ENarr_DialogueState::InProgress;
}

FNarr_DialogueLine UNarrativeManager::GetCurrentDialogueLine() const
{
    if (!IsDialogueActive() || CurrentNPCName.IsEmpty() || CurrentSequenceID.IsEmpty())
    {
        return FNarr_DialogueLine();
    }

    const FNarr_NPCProfile* NPCProfile = NPCProfiles.Find(CurrentNPCName);
    if (!NPCProfile)
    {
        return FNarr_DialogueLine();
    }

    for (const FNarr_DialogueSequence& Sequence : NPCProfile->AvailableDialogues)
    {
        if (Sequence.SequenceID == CurrentSequenceID)
        {
            if (CurrentLineIndex >= 0 && CurrentLineIndex < Sequence.DialogueLines.Num())
            {
                return Sequence.DialogueLines[CurrentLineIndex];
            }
            break;
        }
    }

    return FNarr_DialogueLine();
}

void UNarrativeManager::AdvanceDialogue()
{
    if (!IsDialogueActive())
    {
        return;
    }

    const FNarr_NPCProfile* NPCProfile = NPCProfiles.Find(CurrentNPCName);
    if (!NPCProfile)
    {
        EndDialogue();
        return;
    }

    for (const FNarr_DialogueSequence& Sequence : NPCProfile->AvailableDialogues)
    {
        if (Sequence.SequenceID == CurrentSequenceID)
        {
            CurrentLineIndex++;
            
            if (CurrentLineIndex >= Sequence.DialogueLines.Num())
            {
                EndDialogue();
            }
            
            UE_LOG(LogTemp, Log, TEXT("Advanced dialogue to line %d"), CurrentLineIndex);
            return;
        }
    }

    EndDialogue();
}

void UNarrativeManager::RegisterNPC(const FString& NPCName, const FNarr_NPCProfile& Profile)
{
    NPCProfiles.Add(NPCName, Profile);
    UE_LOG(LogTemp, Log, TEXT("Registered NPC: %s with %d dialogue sequences"), *NPCName, Profile.AvailableDialogues.Num());
}

FNarr_NPCProfile UNarrativeManager::GetNPCProfile(const FString& NPCName) const
{
    const FNarr_NPCProfile* FoundProfile = NPCProfiles.Find(NPCName);
    return FoundProfile ? *FoundProfile : FNarr_NPCProfile();
}

void UNarrativeManager::UpdateNPCMood(const FString& NPCName, ENarr_NPCMood NewMood)
{
    FNarr_NPCProfile* NPCProfile = NPCProfiles.Find(NPCName);
    if (NPCProfile)
    {
        NPCProfile->CurrentMood = NewMood;
        UE_LOG(LogTemp, Log, TEXT("Updated NPC %s mood to %d"), *NPCName, (int32)NewMood);
    }
}

void UNarrativeManager::UpdateNPCTrust(const FString& NPCName, float TrustDelta)
{
    FNarr_NPCProfile* NPCProfile = NPCProfiles.Find(NPCName);
    if (NPCProfile)
    {
        NPCProfile->TrustLevel = FMath::Clamp(NPCProfile->TrustLevel + TrustDelta, 0.0f, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("Updated NPC %s trust level to %f"), *NPCName, NPCProfile->TrustLevel);
    }
}

void UNarrativeManager::MarkDialogueCompleted(const FString& NPCName, const FString& SequenceID)
{
    FNarr_NPCProfile* NPCProfile = NPCProfiles.Find(NPCName);
    if (NPCProfile)
    {
        NPCProfile->CompletedDialogues.AddUnique(SequenceID);
        UE_LOG(LogTemp, Log, TEXT("Marked dialogue completed: %s for NPC: %s"), *SequenceID, *NPCName);
    }
}

bool UNarrativeManager::IsDialogueCompleted(const FString& NPCName, const FString& SequenceID) const
{
    const FNarr_NPCProfile* NPCProfile = NPCProfiles.Find(NPCName);
    return NPCProfile && NPCProfile->CompletedDialogues.Contains(SequenceID);
}

TArray<FString> UNarrativeManager::GetAvailableDialogues(const FString& NPCName) const
{
    TArray<FString> AvailableSequences;
    
    const FNarr_NPCProfile* NPCProfile = NPCProfiles.Find(NPCName);
    if (NPCProfile)
    {
        for (const FNarr_DialogueSequence& Sequence : NPCProfile->AvailableDialogues)
        {
            if (Sequence.bIsRepeatable || !IsDialogueCompleted(NPCName, Sequence.SequenceID))
            {
                AvailableSequences.Add(Sequence.SequenceID);
            }
        }
    }

    return AvailableSequences;
}

void UNarrativeManager::LoadDefaultNPCProfiles()
{
    CreateSurvivalDialogues();
    
    RegisterNPC(TEXT("ElderHunter"), CreateElderHunterProfile());
    RegisterNPC(TEXT("ScoutRunner"), CreateScoutRunnerProfile());
    RegisterNPC(TEXT("TribeChief"), CreateTribeChiefProfile());
}

void UNarrativeManager::CreateSurvivalDialogues()
{
    // This method sets up the core survival-focused dialogue system
    UE_LOG(LogTemp, Log, TEXT("Creating survival-focused dialogue sequences"));
}

FNarr_NPCProfile UNarrativeManager::CreateElderHunterProfile()
{
    FNarr_NPCProfile Profile;
    Profile.NPCName = TEXT("Elder Hunter");
    Profile.NPCRole = TEXT("Experienced Tracker");
    Profile.CurrentMood = ENarr_NPCMood::Cautious;
    Profile.TrustLevel = 0.7f;

    // Warning about predator tracks
    FNarr_DialogueSequence PredatorWarning;
    PredatorWarning.SequenceID = TEXT("PredatorTracks");
    PredatorWarning.bIsRepeatable = false;
    PredatorWarning.Priority = 10;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Elder Hunter");
    Line1.DialogueText = FText::FromString(TEXT("The tracks are fresh. Three-toed, deep impressions in the mud. A large predator passed this way not long ago."));
    Line1.Duration = 4.0f;
    Line1.RequiredMood = ENarr_NPCMood::Cautious;

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Elder Hunter");
    Line2.DialogueText = FText::FromString(TEXT("We should move camp before nightfall. Carnotaurus hunts in the darkness, and its bite can crush bone."));
    Line2.Duration = 4.5f;
    Line2.RequiredMood = ENarr_NPCMood::Cautious;

    PredatorWarning.DialogueLines.Add(Line1);
    PredatorWarning.DialogueLines.Add(Line2);
    Profile.AvailableDialogues.Add(PredatorWarning);

    return Profile;
}

FNarr_NPCProfile UNarrativeManager::CreateScoutRunnerProfile()
{
    FNarr_NPCProfile Profile;
    Profile.NPCName = TEXT("Scout Runner");
    Profile.NPCRole = TEXT("Young Scout");
    Profile.CurrentMood = ENarr_NPCMood::Neutral;
    Profile.TrustLevel = 0.6f;

    // Herd movement report
    FNarr_DialogueSequence HerdReport;
    HerdReport.SequenceID = TEXT("HerdMovement");
    HerdReport.bIsRepeatable = true;
    HerdReport.Priority = 8;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Scout Runner");
    Line1.DialogueText = FText::FromString(TEXT("Chief! The herds are moving south toward the river valley. Hundreds of them - good hunting ahead."));
    Line1.Duration = 3.5f;
    Line1.RequiredMood = ENarr_NPCMood::Neutral;

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Scout Runner");
    Line2.DialogueText = FText::FromString(TEXT("But something's driving them. Something big is hunting behind the migration."));
    Line2.Duration = 3.0f;
    Line2.RequiredMood = ENarr_NPCMood::Fearful;

    HerdReport.DialogueLines.Add(Line1);
    HerdReport.DialogueLines.Add(Line2);
    Profile.AvailableDialogues.Add(HerdReport);

    return Profile;
}

FNarr_NPCProfile UNarrativeManager::CreateTribeChiefProfile()
{
    FNarr_NPCProfile Profile;
    Profile.NPCName = TEXT("Tribe Chief");
    Profile.NPCRole = TEXT("Tribal Leader");
    Profile.CurrentMood = ENarr_NPCMood::Friendly;
    Profile.TrustLevel = 0.8f;

    // Leadership wisdom
    FNarr_DialogueSequence LeadershipWisdom;
    LeadershipWisdom.SequenceID = TEXT("SurvivalWisdom");
    LeadershipWisdom.bIsRepeatable = false;
    LeadershipWisdom.Priority = 9;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Tribe Chief");
    Line1.DialogueText = FText::FromString(TEXT("This land has fed our people for generations, but it demands respect."));
    Line1.Duration = 3.0f;
    Line1.RequiredMood = ENarr_NPCMood::Friendly;

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Tribe Chief");
    Line2.DialogueText = FText::FromString(TEXT("Every dawn brings new dangers. Only the strong and the wise will see another sunrise."));
    Line2.Duration = 4.0f;
    Line2.RequiredMood = ENarr_NPCMood::Neutral;

    LeadershipWisdom.DialogueLines.Add(Line1);
    LeadershipWisdom.DialogueLines.Add(Line2);
    Profile.AvailableDialogues.Add(LeadershipWisdom);

    return Profile;
}