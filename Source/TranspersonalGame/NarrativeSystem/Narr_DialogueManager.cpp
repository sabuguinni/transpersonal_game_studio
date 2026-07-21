#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    bIsDialogueActive = false;
    CurrentDialogueIndex = 0;
    CurrentNPCActor = nullptr;
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative Dialogue Manager initialized"));
    
    InitializeDefaultDialogues();
    InitializeCharacterVoices();
}

void UNarr_DialogueManager::Deinitialize()
{
    EndDialogue();
    DialogueSequences.Empty();
    CharacterVoiceMap.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueManager::StartDialogue(const FString& SequenceID, AActor* NPCActor)
{
    if (!HasDialogueSequence(SequenceID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
        return;
    }

    if (bIsDialogueActive)
    {
        EndDialogue();
    }

    CurrentSequenceID = SequenceID;
    CurrentNPCActor = NPCActor;
    CurrentDialogueIndex = 0;
    bIsDialogueActive = true;

    UE_LOG(LogTemp, Warning, TEXT("Started dialogue sequence: %s"), *SequenceID);
    
    // Pause game for dialogue
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SetGamePaused(World, true);
    }
}

void UNarr_DialogueManager::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }

    bIsDialogueActive = false;
    CurrentSequenceID = "";
    CurrentDialogueIndex = 0;
    CurrentNPCActor = nullptr;

    // Unpause game
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SetGamePaused(World, false);
    }

    UE_LOG(LogTemp, Warning, TEXT("Ended dialogue"));
}

void UNarr_DialogueManager::AdvanceDialogue()
{
    if (!bIsDialogueActive || !HasDialogueSequence(CurrentSequenceID))
    {
        return;
    }

    FNarr_DialogueSequence& Sequence = DialogueSequences[CurrentSequenceID];
    
    CurrentDialogueIndex++;
    
    if (CurrentDialogueIndex >= Sequence.DialogueLines.Num())
    {
        EndDialogue();
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Advanced to dialogue line %d"), CurrentDialogueIndex);
}

void UNarr_DialogueManager::SelectPlayerResponse(int32 ResponseIndex)
{
    if (!bIsDialogueActive)
    {
        return;
    }

    FNarr_DialogueLine CurrentLine = GetCurrentDialogueLine();
    
    if (ResponseIndex >= 0 && ResponseIndex < CurrentLine.PlayerResponses.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Player selected response %d: %s"), 
               ResponseIndex, *CurrentLine.PlayerResponses[ResponseIndex].ToString());
        
        AdvanceDialogue();
    }
}

void UNarr_DialogueManager::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    DialogueSequences.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Warning, TEXT("Registered dialogue sequence: %s"), *Sequence.SequenceID);
}

FNarr_DialogueSequence UNarr_DialogueManager::GetDialogueSequence(const FString& SequenceID)
{
    if (HasDialogueSequence(SequenceID))
    {
        return DialogueSequences[SequenceID];
    }
    
    return FNarr_DialogueSequence();
}

bool UNarr_DialogueManager::HasDialogueSequence(const FString& SequenceID)
{
    return DialogueSequences.Contains(SequenceID);
}

FString UNarr_DialogueManager::GetCharacterVoiceURL(ENarr_CharacterType CharacterType, const FString& DialogueKey)
{
    FString VoiceKey = GenerateVoiceKey(CharacterType, DialogueKey);
    
    if (CharacterVoiceMap.Contains(VoiceKey))
    {
        return CharacterVoiceMap[VoiceKey];
    }
    
    return "";
}

void UNarr_DialogueManager::RegisterCharacterVoice(ENarr_CharacterType CharacterType, const FString& DialogueKey, const FString& AudioURL)
{
    FString VoiceKey = GenerateVoiceKey(CharacterType, DialogueKey);
    CharacterVoiceMap.Add(VoiceKey, AudioURL);
    
    UE_LOG(LogTemp, Warning, TEXT("Registered voice for %s: %s"), *VoiceKey, *AudioURL);
}

FNarr_DialogueLine UNarr_DialogueManager::GetCurrentDialogueLine() const
{
    if (!bIsDialogueActive || !DialogueSequences.Contains(CurrentSequenceID))
    {
        return FNarr_DialogueLine();
    }

    const FNarr_DialogueSequence& Sequence = DialogueSequences[CurrentSequenceID];
    
    if (CurrentDialogueIndex >= 0 && CurrentDialogueIndex < Sequence.DialogueLines.Num())
    {
        return Sequence.DialogueLines[CurrentDialogueIndex];
    }
    
    return FNarr_DialogueLine();
}

void UNarr_DialogueManager::InitializeDefaultDialogues()
{
    // Tribal Elder Welcome Sequence
    FNarr_DialogueSequence ElderWelcome;
    ElderWelcome.SequenceID = "TribalElder_Welcome";
    ElderWelcome.bIsRepeatable = false;
    
    FNarr_DialogueLine ElderLine1;
    ElderLine1.DialogueText = FText::FromString("Welcome, survivor. The tribal camp needs your help. Our hunters have not returned from the eastern plains.");
    ElderLine1.SpeakerType = ENarr_CharacterType::TribalElder;
    ElderLine1.Context = ENarr_DialogueContext::FirstMeeting;
    ElderLine1.DisplayDuration = 6.0f;
    ElderLine1.PlayerResponses.Add(FText::FromString("What happened to the hunters?"));
    ElderLine1.PlayerResponses.Add(FText::FromString("I'll help if I can."));
    ElderWelcome.DialogueLines.Add(ElderLine1);
    
    FNarr_DialogueLine ElderLine2;
    ElderLine2.DialogueText = FText::FromString("The gathering parties report strange sounds from the forest. Will you join us in our time of need?");
    ElderLine2.SpeakerType = ENarr_CharacterType::TribalElder;
    ElderLine2.Context = ENarr_DialogueContext::QuestGiving;
    ElderLine2.DisplayDuration = 5.0f;
    ElderLine2.PlayerResponses.Add(FText::FromString("Yes, I'll investigate."));
    ElderLine2.PlayerResponses.Add(FText::FromString("I need to prepare first."));
    ElderWelcome.DialogueLines.Add(ElderLine2);
    
    RegisterDialogueSequence(ElderWelcome);

    // Scout Warning Sequence
    FNarr_DialogueSequence ScoutWarning;
    ScoutWarning.SequenceID = "TribalScout_Warning";
    ScoutWarning.bIsRepeatable = true;
    
    FNarr_DialogueLine ScoutLine1;
    ScoutLine1.DialogueText = FText::FromString("Pack hunters spotted near the eastern cliffs! Three, maybe four raptors moving in formation.");
    ScoutLine1.SpeakerType = ENarr_CharacterType::TribalScout;
    ScoutLine1.Context = ENarr_DialogueContext::Warning;
    ScoutLine1.DisplayDuration = 5.0f;
    ScoutLine1.PlayerResponses.Add(FText::FromString("How dangerous are they?"));
    ScoutLine1.PlayerResponses.Add(FText::FromString("Thanks for the warning."));
    ScoutWarning.DialogueLines.Add(ScoutLine1);
    
    FNarr_DialogueLine ScoutLine2;
    ScoutLine2.DialogueText = FText::FromString("They hunt as one mind, one deadly purpose. Stay low, move quiet, and never turn your back on them.");
    ScoutLine2.SpeakerType = ENarr_CharacterType::TribalScout;
    ScoutLine2.Context = ENarr_DialogueContext::Teaching;
    ScoutLine2.DisplayDuration = 5.0f;
    ScoutWarning.DialogueLines.Add(ScoutLine2);
    
    RegisterDialogueSequence(ScoutWarning);

    // Shaman Lore Sequence
    FNarr_DialogueSequence ShamanLore;
    ShamanLore.SequenceID = "TribalShaman_Lore";
    ShamanLore.bIsRepeatable = true;
    
    FNarr_DialogueLine ShamanLine1;
    ShamanLine1.DialogueText = FText::FromString("The bones tell stories. This skull belonged to a young Thunder Walker, killed by claw marks deep in the bone.");
    ShamanLine1.SpeakerType = ENarr_CharacterType::TribalShaman;
    ShamanLine1.Context = ENarr_DialogueContext::Teaching;
    ShamanLine1.DisplayDuration = 6.0f;
    ShamanLine1.PlayerResponses.Add(FText::FromString("What killed it?"));
    ShamanLine1.PlayerResponses.Add(FText::FromString("How can you tell?"));
    ShamanLore.DialogueLines.Add(ShamanLine1);
    
    FNarr_DialogueLine ShamanLine2;
    ShamanLine2.DialogueText = FText::FromString("The pack that did this... they are still out there, still hunting. We must be ready.");
    ShamanLine2.SpeakerType = ENarr_CharacterType::TribalShaman;
    ShamanLine2.Context = ENarr_DialogueContext::Warning;
    ShamanLine2.DisplayDuration = 4.0f;
    ShamanLore.DialogueLines.Add(ShamanLine2);
    
    RegisterDialogueSequence(ShamanLore);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d default dialogue sequences"), DialogueSequences.Num());
}

void UNarr_DialogueManager::InitializeCharacterVoices()
{
    // Register voice URLs from generated audio
    RegisterCharacterVoice(ENarr_CharacterType::TribalElder, "Welcome", 
        "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778766459096_TribalElder.mp3");
    
    RegisterCharacterVoice(ENarr_CharacterType::TribalScout, "Warning", 
        "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778766538780_TribalScout.mp3");
    
    RegisterCharacterVoice(ENarr_CharacterType::TribalShaman, "Lore", 
        "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778766544296_TribalShaman.mp3");
    
    RegisterCharacterVoice(ENarr_CharacterType::TribalMentor, "Teaching", 
        "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778766549466_TribalMentor.mp3");

    UE_LOG(LogTemp, Warning, TEXT("Initialized character voice mappings"));
}

FString UNarr_DialogueManager::GenerateVoiceKey(ENarr_CharacterType CharacterType, const FString& DialogueKey)
{
    FString CharacterName;
    switch (CharacterType)
    {
        case ENarr_CharacterType::TribalElder:
            CharacterName = "TribalElder";
            break;
        case ENarr_CharacterType::TribalScout:
            CharacterName = "TribalScout";
            break;
        case ENarr_CharacterType::TribalShaman:
            CharacterName = "TribalShaman";
            break;
        case ENarr_CharacterType::TribalMentor:
            CharacterName = "TribalMentor";
            break;
        case ENarr_CharacterType::TribalHunter:
            CharacterName = "TribalHunter";
            break;
        case ENarr_CharacterType::TribalCrafter:
            CharacterName = "TribalCrafter";
            break;
        default:
            CharacterName = "Unknown";
            break;
    }
    
    return FString::Printf(TEXT("%s_%s"), *CharacterName, *DialogueKey);
}