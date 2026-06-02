#include "Narr_DialogueSystem.h"
#include "Narr_TribalNPC.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    bIsDialogueActive = false;
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;
    CurrentNPC = nullptr;
    DialogueAudioComponent = nullptr;
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDialogueDatabase();
    LoadDialogueDatabase();

    // Create audio component for dialogue playback
    DialogueAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudioComponent"));
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->bAutoActivate = false;
        DialogueAudioComponent->SetVolumeMultiplier(0.8f);
    }
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDialogueActive)
    {
        DialogueTimer += DeltaTime;
        ProcessCurrentDialogueLine();
    }
}

bool UNarr_DialogueSystem::StartDialogue(const FString& DialogueID, ANarr_TribalNPC* NPC)
{
    if (!NPC || bIsDialogueActive)
    {
        return false;
    }

    FNarr_DialogueNode DialogueNode = GetDialogueNode(DialogueID);
    if (DialogueNode.DialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue node not found: %s"), *DialogueID);
        return false;
    }

    CurrentDialogueNode = DialogueNode;
    CurrentNPC = NPC;
    bIsDialogueActive = true;
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s with NPC: %s"), *DialogueID, *NPC->GetName());
    return true;
}

void UNarr_DialogueSystem::SelectPlayerChoice(int32 ChoiceIndex)
{
    if (!bIsDialogueActive || ChoiceIndex < 0 || ChoiceIndex >= CurrentDialogueNode.PlayerChoices.Num())
    {
        return;
    }

    FNarr_DialogueChoice SelectedChoice = CurrentDialogueNode.PlayerChoices[ChoiceIndex];
    
    if (!ValidateDialogueChoice(SelectedChoice))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player choice validation failed"));
        return;
    }

    if (SelectedChoice.bEndsConversation || SelectedChoice.NextDialogueID.IsEmpty())
    {
        EndDialogue();
    }
    else
    {
        // Transition to next dialogue node
        StartDialogue(SelectedChoice.NextDialogueID, CurrentNPC);
    }
}

void UNarr_DialogueSystem::EndDialogue()
{
    bIsDialogueActive = false;
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;
    CurrentNPC = nullptr;
    StopDialogueAudio();

    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

void UNarr_DialogueSystem::LoadDialogueDatabase()
{
    // This would typically load from a data table or JSON file
    // For now, we'll populate with hardcoded dialogue for testing
    InitializeDialogueDatabase();
}

FNarr_DialogueNode UNarr_DialogueSystem::GetDialogueNode(const FString& DialogueID) const
{
    if (DialogueDatabase.Contains(DialogueID))
    {
        return DialogueDatabase[DialogueID];
    }
    
    return FNarr_DialogueNode();
}

void UNarr_DialogueSystem::PlayDialogueAudio(const FString& AudioPath)
{
    if (!DialogueAudioComponent || AudioPath.IsEmpty())
    {
        return;
    }

    // Load and play audio file
    // In a real implementation, this would load the audio asset from the path
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue audio: %s"), *AudioPath);
    
    // For now, just log that we would play the audio
    // DialogueAudioComponent->SetSound(LoadedSoundWave);
    // DialogueAudioComponent->Play();
}

void UNarr_DialogueSystem::StopDialogueAudio()
{
    if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
    {
        DialogueAudioComponent->Stop();
    }
}

void UNarr_DialogueSystem::InitializeDialogueDatabase()
{
    // Elder Greeting Dialogue
    FNarr_DialogueNode ElderGreeting;
    ElderGreeting.DialogueID = TEXT("elder_greeting");
    ElderGreeting.NPCRole = ENPCRole::Elder;
    ElderGreeting.bIsQuestDialogue = false;

    FNarr_DialogueLine ElderLine1;
    ElderLine1.SpeakerName = TEXT("Village Elder");
    ElderLine1.DialogueText = TEXT("Welcome, young hunter. I sense great potential in you.");
    ElderLine1.Duration = 4.0f;
    ElderLine1.RequiredMood = ENPCMoodState::Friendly;
    ElderGreeting.DialogueLines.Add(ElderLine1);

    FNarr_DialogueChoice ElderChoice1;
    ElderChoice1.ChoiceText = TEXT("I seek wisdom about surviving in these lands.");
    ElderChoice1.NextDialogueID = TEXT("elder_wisdom");
    ElderGreeting.PlayerChoices.Add(ElderChoice1);

    FNarr_DialogueChoice ElderChoice2;
    ElderChoice2.ChoiceText = TEXT("Do you have any tasks for me?");
    ElderChoice2.NextDialogueID = TEXT("elder_quest");
    ElderGreeting.PlayerChoices.Add(ElderChoice2);

    DialogueDatabase.Add(ElderGreeting.DialogueID, ElderGreeting);

    // Hunter Guide Dialogue
    FNarr_DialogueNode HunterDialogue;
    HunterDialogue.DialogueID = TEXT("hunter_training");
    HunterDialogue.NPCRole = ENPCRole::Hunter;
    HunterDialogue.bIsQuestDialogue = true;

    FNarr_DialogueLine HunterLine1;
    HunterLine1.SpeakerName = TEXT("Hunter Guide");
    HunterLine1.DialogueText = TEXT("The great beasts require respect and strategy. Are you ready to learn?");
    HunterLine1.Duration = 5.0f;
    HunterLine1.RequiredMood = ENPCMoodState::Confident;
    HunterDialogue.DialogueLines.Add(HunterLine1);

    FNarr_DialogueChoice HunterChoice1;
    HunterChoice1.ChoiceText = TEXT("Teach me to hunt the thunder lizards.");
    HunterChoice1.NextDialogueID = TEXT("hunting_lesson");
    HunterDialogue.PlayerChoices.Add(HunterChoice1);

    DialogueDatabase.Add(HunterDialogue.DialogueID, HunterDialogue);

    // Scout Warning Dialogue
    FNarr_DialogueNode ScoutWarning;
    ScoutWarning.DialogueID = TEXT("scout_warning");
    ScoutWarning.NPCRole = ENPCRole::Scout;
    ScoutWarning.bIsQuestDialogue = false;

    FNarr_DialogueLine ScoutLine1;
    ScoutLine1.SpeakerName = TEXT("Tribal Scout");
    ScoutLine1.DialogueText = TEXT("Danger approaches from the northern canyon. Massive footprints in the mud.");
    ScoutLine1.Duration = 4.5f;
    ScoutLine1.RequiredMood = ENPCMoodState::Anxious;
    ScoutWarning.DialogueLines.Add(ScoutLine1);

    FNarr_DialogueChoice ScoutChoice1;
    ScoutChoice1.ChoiceText = TEXT("I'll investigate the threat.");
    ScoutChoice1.NextDialogueID = TEXT("scout_mission");
    ScoutWarning.PlayerChoices.Add(ScoutChoice1);

    DialogueDatabase.Add(ScoutWarning.DialogueID, ScoutWarning);

    UE_LOG(LogTemp, Log, TEXT("Dialogue database initialized with %d entries"), DialogueDatabase.Num());
}

void UNarr_DialogueSystem::ProcessCurrentDialogueLine()
{
    if (CurrentLineIndex >= CurrentDialogueNode.DialogueLines.Num())
    {
        return;
    }

    FNarr_DialogueLine CurrentLine = CurrentDialogueNode.DialogueLines[CurrentLineIndex];
    
    // Check if enough time has passed for this line
    if (DialogueTimer >= CurrentLine.Duration)
    {
        AdvanceToNextLine();
    }
}

void UNarr_DialogueSystem::AdvanceToNextLine()
{
    CurrentLineIndex++;
    DialogueTimer = 0.0f;

    if (CurrentLineIndex >= CurrentDialogueNode.DialogueLines.Num())
    {
        // All lines have been played, show player choices or end dialogue
        if (CurrentDialogueNode.PlayerChoices.Num() == 0)
        {
            EndDialogue();
        }
        // Player choices would be handled by UI system
    }
    else
    {
        // Play next line
        FNarr_DialogueLine NextLine = CurrentDialogueNode.DialogueLines[CurrentLineIndex];
        PlayDialogueAudio(NextLine.AudioPath);
    }
}

bool UNarr_DialogueSystem::ValidateDialogueChoice(const FNarr_DialogueChoice& Choice) const
{
    // Check if player meets requirements for this choice
    if (Choice.RequiredSurvivalLevel > 0)
    {
        // In a real implementation, check player's survival level
        // For now, always return true
    }

    return true;
}