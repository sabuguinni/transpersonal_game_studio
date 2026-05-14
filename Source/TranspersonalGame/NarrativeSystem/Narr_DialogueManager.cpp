#include "Narr_DialogueManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component for voice playback
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    RootComponent = AudioComponent;

    // Initialize default values
    DefaultDialogueCooldown = 3.0f;
    bIsDialoguePlaying = false;
    CurrentDialogueIndex = 0;
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultDialogues();
    
    if (AudioComponent)
    {
        AudioComponent->SetVolumeMultiplier(0.8f);
    }
}

void ANarr_DialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Handle dialogue progression
    if (bIsDialoguePlaying && AudioComponent && !AudioComponent->IsPlaying())
    {
        OnDialogueFinished();
    }
}

void ANarr_DialogueManager::PlayDialogueSequence(const FString& SequenceName)
{
    if (bIsDialoguePlaying)
    {
        return; // Don't interrupt current dialogue
    }

    FNarr_DialogueSequence* Sequence = FindDialogueSequence(SequenceName);
    if (!Sequence || Sequence->DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceName);
        return;
    }

    // Check if this dialogue has been played and is not repeatable
    if (!Sequence->bIsRepeatable && PlayedDialogues.Contains(SequenceName))
    {
        return;
    }

    bIsDialoguePlaying = true;
    CurrentDialogueIndex = 0;
    PlayedDialogues.Add(SequenceName, true);
    
    PlayNextDialogueLine();
}

void ANarr_DialogueManager::StopCurrentDialogue()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
    
    bIsDialoguePlaying = false;
    CurrentDialogueIndex = 0;
}

bool ANarr_DialogueManager::IsDialoguePlaying() const
{
    return bIsDialoguePlaying;
}

void ANarr_DialogueManager::RegisterDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    DialogueSequences.Add(NewSequence);
}

void ANarr_DialogueManager::TriggerContextualDialogue(ENarr_DialogueType DialogueType, const FVector& PlayerLocation)
{
    // Find appropriate dialogue based on context
    FString SequenceName;
    
    switch (DialogueType)
    {
        case ENarr_DialogueType::Warning:
            SequenceName = TEXT("ScoutWarning");
            break;
        case ENarr_DialogueType::Information:
            SequenceName = TEXT("ElderWisdom");
            break;
        case ENarr_DialogueType::Combat:
            SequenceName = TEXT("WarriorBattle");
            break;
        case ENarr_DialogueType::Exploration:
            SequenceName = TEXT("ShamanHerd");
            break;
        default:
            SequenceName = TEXT("ElderWisdom");
            break;
    }
    
    PlayDialogueSequence(SequenceName);
}

void ANarr_DialogueManager::SetDialogueVolume(float Volume)
{
    if (AudioComponent)
    {
        AudioComponent->SetVolumeMultiplier(FMath::Clamp(Volume, 0.0f, 1.0f));
    }
}

void ANarr_DialogueManager::OnDialogueFinished()
{
    if (!bIsDialoguePlaying)
    {
        return;
    }

    // Move to next dialogue line or finish sequence
    CurrentDialogueIndex++;
    
    if (CurrentDialogueIndex < DialogueSequences.Num() && 
        DialogueSequences.IsValidIndex(0) && 
        CurrentDialogueIndex < DialogueSequences[0].DialogueLines.Num())
    {
        // Wait for cooldown before next line
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &ANarr_DialogueManager::PlayNextDialogueLine,
            DefaultDialogueCooldown,
            false
        );
    }
    else
    {
        bIsDialoguePlaying = false;
        CurrentDialogueIndex = 0;
    }
}

void ANarr_DialogueManager::PlayNextDialogueLine()
{
    if (!bIsDialoguePlaying || DialogueSequences.Num() == 0)
    {
        return;
    }

    FNarr_DialogueSequence& CurrentSequence = DialogueSequences[0];
    if (!CurrentSequence.DialogueLines.IsValidIndex(CurrentDialogueIndex))
    {
        bIsDialoguePlaying = false;
        return;
    }

    FNarr_DialogueLine& CurrentLine = CurrentSequence.DialogueLines[CurrentDialogueIndex];
    
    // Display dialogue text (this would connect to UI system)
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), 
            *CurrentLine.CharacterName, 
            *CurrentLine.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, CurrentLine.Duration, FColor::Yellow, DisplayText);
    }

    // Play voice clip if available
    if (AudioComponent && CurrentLine.VoiceClip)
    {
        AudioComponent->SetSound(CurrentLine.VoiceClip);
        AudioComponent->Play();
    }
}

void ANarr_DialogueManager::InitializeDefaultDialogues()
{
    // Elder Wisdom dialogue
    FNarr_DialogueSequence ElderSequence;
    ElderSequence.SequenceName = TEXT("ElderWisdom");
    ElderSequence.bIsRepeatable = true;
    ElderSequence.Priority = 1;

    FNarr_DialogueLine ElderLine;
    ElderLine.CharacterName = TEXT("Tribal Elder");
    ElderLine.DialogueText = TEXT("The ancient valley holds many secrets, hunter. Beyond the great river lies the territory of the Thunder Lizards.");
    ElderLine.Duration = 8.0f;
    ElderLine.DialogueType = ENarr_DialogueType::Information;
    ElderSequence.DialogueLines.Add(ElderLine);

    // Scout Warning dialogue
    FNarr_DialogueSequence ScoutSequence;
    ScoutSequence.SequenceName = TEXT("ScoutWarning");
    ScoutSequence.bIsRepeatable = true;
    ScoutSequence.Priority = 3;

    FNarr_DialogueLine ScoutLine;
    ScoutLine.CharacterName = TEXT("Tribal Scout");
    ScoutLine.DialogueText = TEXT("Danger approaches from the north! The pack hunters move like shadows through the fern groves.");
    ScoutLine.Duration = 7.0f;
    ScoutLine.DialogueType = ENarr_DialogueType::Warning;
    ScoutSequence.DialogueLines.Add(ScoutLine);

    // Shaman Herd dialogue
    FNarr_DialogueSequence ShamanSequence;
    ShamanSequence.SequenceName = TEXT("ShamanHerd");
    ShamanSequence.bIsRepeatable = true;
    ShamanSequence.Priority = 2;

    FNarr_DialogueLine ShamanLine;
    ShamanLine.CharacterName = TEXT("Tribal Shaman");
    ShamanLine.DialogueText = TEXT("The great herds are moving again. This is the season of plenty - when the long-necks follow the green paths.");
    ShamanLine.Duration = 9.0f;
    ShamanLine.DialogueType = ENarr_DialogueType::Exploration;
    ShamanSequence.DialogueLines.Add(ShamanLine);

    // Warrior Battle dialogue
    FNarr_DialogueSequence WarriorSequence;
    WarriorSequence.SequenceName = TEXT("WarriorBattle");
    WarriorSequence.bIsRepeatable = false;
    WarriorSequence.Priority = 5;

    FNarr_DialogueLine WarriorLine;
    WarriorLine.CharacterName = TEXT("Tribal Warrior");
    WarriorLine.DialogueText = TEXT("Hold your ground, warriors! The great predator circles our camp, testing our resolve.");
    WarriorLine.Duration = 6.0f;
    WarriorLine.DialogueType = ENarr_DialogueType::Combat;
    WarriorSequence.DialogueLines.Add(WarriorLine);

    // Register all sequences
    DialogueSequences.Add(ElderSequence);
    DialogueSequences.Add(ScoutSequence);
    DialogueSequences.Add(ShamanSequence);
    DialogueSequences.Add(WarriorSequence);
}

FNarr_DialogueSequence* ANarr_DialogueManager::FindDialogueSequence(const FString& SequenceName)
{
    for (FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceName == SequenceName)
        {
            return &Sequence;
        }
    }
    return nullptr;
}