#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

UNarrativeManager::UNarrativeManager()
{
    DialogueAudioComponent = nullptr;
    NarrationAudioComponent = nullptr;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initializing narrative system"));
    
    LoadStoryData();
    InitializeDefaultCharacters();
    InitializeDefaultNarrations();
    
    // Create audio components for dialogue playback
    if (UWorld* World = GetWorld())
    {
        if (AActor* WorldActor = World->GetFirstPlayerController())
        {
            DialogueAudioComponent = WorldActor->CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudio"));
            NarrationAudioComponent = WorldActor->CreateDefaultSubobject<UAudioComponent>(TEXT("NarrationAudio"));
        }
    }
}

void UNarrativeManager::Deinitialize()
{
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->Stop();
        DialogueAudioComponent = nullptr;
    }
    
    if (NarrationAudioComponent)
    {
        NarrationAudioComponent->Stop();
        NarrationAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UNarrativeManager::PlayDialogueLine(const FNarr_DialogueLine& DialogueLine)
{
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Playing dialogue from %s"), *DialogueLine.SpeakerName);
    
    if (DialogueLine.bIsNarration && NarrationAudioComponent)
    {
        if (USoundCue* SoundCue = DialogueLine.VoiceAudio.LoadSynchronous())
        {
            NarrationAudioComponent->SetSound(SoundCue);
            NarrationAudioComponent->Play();
        }
    }
    else if (DialogueAudioComponent)
    {
        if (USoundCue* SoundCue = DialogueLine.VoiceAudio.LoadSynchronous())
        {
            DialogueAudioComponent->SetSound(SoundCue);
            DialogueAudioComponent->Play();
        }
    }
    
    // Display dialogue text (would integrate with UI system)
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), 
            *DialogueLine.SpeakerName, 
            *DialogueLine.DialogueText.ToString());
        GEngine->AddOnScreenDebugMessage(-1, DialogueLine.Duration, FColor::White, DisplayText);
    }
}

void UNarrativeManager::TriggerStoryBeat(const FString& BeatID)
{
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Triggering story beat %s"), *BeatID);
    
    for (FNarr_StoryBeat& StoryBeat : StoryBeats)
    {
        if (StoryBeat.BeatID == BeatID && !StoryBeat.bIsCompleted)
        {
            if (CheckStoryBeatConditions(StoryBeat))
            {
                // Play all dialogue lines in sequence
                for (const FNarr_DialogueLine& Line : StoryBeat.DialogueLines)
                {
                    PlayDialogueLine(Line);
                }
                
                CompleteStoryBeat(BeatID);
                return;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Story beat %s conditions not met"), *BeatID);
            }
        }
    }
}

bool UNarrativeManager::IsStoryBeatCompleted(const FString& BeatID) const
{
    for (const FNarr_StoryBeat& StoryBeat : StoryBeats)
    {
        if (StoryBeat.BeatID == BeatID)
        {
            return StoryBeat.bIsCompleted;
        }
    }
    return false;
}

void UNarrativeManager::AddCharacter(const FNarr_CharacterProfile& Character)
{
    // Check if character already exists
    for (FNarr_CharacterProfile& ExistingChar : Characters)
    {
        if (ExistingChar.CharacterID == Character.CharacterID)
        {
            ExistingChar = Character; // Update existing
            return;
        }
    }
    
    Characters.Add(Character);
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Added character %s"), *Character.CharacterID);
}

FNarr_CharacterProfile UNarrativeManager::GetCharacter(const FString& CharacterID) const
{
    for (const FNarr_CharacterProfile& Character : Characters)
    {
        if (Character.CharacterID == CharacterID)
        {
            return Character;
        }
    }
    
    // Return default character if not found
    FNarr_CharacterProfile DefaultChar;
    DefaultChar.CharacterID = CharacterID;
    DefaultChar.CharacterName = FText::FromString(TEXT("Unknown"));
    return DefaultChar;
}

void UNarrativeManager::UpdateCharacterTrust(const FString& CharacterID, float TrustDelta)
{
    for (FNarr_CharacterProfile& Character : Characters)
    {
        if (Character.CharacterID == CharacterID)
        {
            Character.TrustLevel = FMath::Clamp(Character.TrustLevel + TrustDelta, 0.0f, 1.0f);
            UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: %s trust now %.2f"), *CharacterID, Character.TrustLevel);
            return;
        }
    }
}

void UNarrativeManager::PlayNarration(const FString& NarrationKey)
{
    if (NarrationLibrary.Contains(NarrationKey))
    {
        const FNarr_DialogueLine& Narration = NarrationLibrary[NarrationKey];
        PlayDialogueLine(Narration);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Narration key %s not found"), *NarrationKey);
    }
}

void UNarrativeManager::SetStoryFlag(const FString& FlagName, bool bValue)
{
    StoryFlags.Add(FlagName, bValue);
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Set flag %s to %s"), *FlagName, bValue ? TEXT("true") : TEXT("false"));
}

bool UNarrativeManager::GetStoryFlag(const FString& FlagName) const
{
    if (StoryFlags.Contains(FlagName))
    {
        return StoryFlags[FlagName];
    }
    return false;
}

TArray<FString> UNarrativeManager::GetAvailableStoryBeats() const
{
    TArray<FString> AvailableBeats;
    
    for (const FNarr_StoryBeat& StoryBeat : StoryBeats)
    {
        if (!StoryBeat.bIsCompleted && CheckStoryBeatConditions(StoryBeat))
        {
            AvailableBeats.Add(StoryBeat.BeatID);
        }
    }
    
    return AvailableBeats;
}

void UNarrativeManager::RegisterNarrationTrigger(const FString& TriggerID, const FVector& Location, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Registered trigger %s at location %s"), *TriggerID, *Location.ToString());
    // This would integrate with a trigger system to play narration when player enters areas
}

void UNarrativeManager::LoadStoryData()
{
    // Initialize default story beats for the prehistoric survival game
    FNarr_StoryBeat IntroStoryBeat;
    IntroStoryBeat.BeatID = TEXT("Intro_Awakening");
    IntroStoryBeat.BeatTitle = FText::FromString(TEXT("First Dawn"));
    
    FNarr_DialogueLine IntroLine;
    IntroLine.SpeakerName = TEXT("Narrator");
    IntroLine.DialogueText = FText::FromString(TEXT("You awaken in a world ruled by giants. Every shadow could hide death, every sound could be your last warning."));
    IntroLine.bIsNarration = true;
    IntroLine.Duration = 5.0f;
    
    IntroStoryBeat.DialogueLines.Add(IntroLine);
    StoryBeats.Add(IntroStoryBeat);
    
    // Add more story beats
    FNarr_StoryBeat FirstDinosaurEncounter;
    FirstDinosaurEncounter.BeatID = TEXT("Encounter_FirstDinosaur");
    FirstDinosaurEncounter.BeatTitle = FText::FromString(TEXT("First Contact"));
    FirstDinosaurEncounter.RequiredConditions.Add(TEXT("PlayerNearDinosaur"));
    
    FNarr_DialogueLine EncounterLine;
    EncounterLine.SpeakerName = TEXT("Narrator");
    EncounterLine.DialogueText = FText::FromString(TEXT("A massive predator emerges from the undergrowth. Your heart pounds as ancient instincts scream: freeze, flee, or fight."));
    EncounterLine.bIsNarration = true;
    EncounterLine.Duration = 6.0f;
    
    FirstDinosaurEncounter.DialogueLines.Add(EncounterLine);
    StoryBeats.Add(FirstDinosaurEncounter);
}

void UNarrativeManager::InitializeDefaultCharacters()
{
    // Tribe Elder - experienced survivor
    FNarr_CharacterProfile TribeElder;
    TribeElder.CharacterID = TEXT("TribeElder_Kael");
    TribeElder.CharacterName = FText::FromString(TEXT("Kael"));
    TribeElder.CharacterDescription = FText::FromString(TEXT("A weathered survivor who has lived through countless seasons. His scars tell stories of battles with both beasts and nature."));
    TribeElder.Role = ENarr_CharacterRole::TribeElder;
    TribeElder.TrustLevel = 0.7f;
    AddCharacter(TribeElder);
    
    // Young Hunter - ambitious but inexperienced
    FNarr_CharacterProfile YoungHunter;
    YoungHunter.CharacterID = TEXT("Hunter_Zara");
    YoungHunter.CharacterName = FText::FromString(TEXT("Zara"));
    YoungHunter.CharacterDescription = FText::FromString(TEXT("A skilled tracker eager to prove herself. Quick with a spear but sometimes too bold for her own good."));
    YoungHunter.Role = ENarr_CharacterRole::Hunter;
    YoungHunter.TrustLevel = 0.5f;
    AddCharacter(YoungHunter);
    
    // Craftsman - tool maker and builder
    FNarr_CharacterProfile Craftsman;
    Craftsman.CharacterID = TEXT("Craftsman_Thorne");
    Craftsman.CharacterName = FText::FromString(TEXT("Thorne"));
    Craftsman.CharacterDescription = FText::FromString(TEXT("Master of stone and bone. His tools have saved more lives than any weapon. Speaks little but his work speaks volumes."));
    Craftsman.Role = ENarr_CharacterRole::Craftsman;
    Craftsman.TrustLevel = 0.8f;
    AddCharacter(Craftsman);
}

void UNarrativeManager::InitializeDefaultNarrations()
{
    // Danger warnings
    FNarr_DialogueLine DangerNarration;
    DangerNarration.SpeakerName = TEXT("Narrator");
    DangerNarration.DialogueText = FText::FromString(TEXT("The air grows thick with the scent of predator. Every instinct screams danger."));
    DangerNarration.bIsNarration = true;
    DangerNarration.Duration = 4.0f;
    NarrationLibrary.Add(TEXT("Danger_General"), DangerNarration);
    
    // Discovery narration
    FNarr_DialogueLine DiscoveryNarration;
    DiscoveryNarration.SpeakerName = TEXT("Narrator");
    DiscoveryNarration.DialogueText = FText::FromString(TEXT("Ancient bones scattered across the ground tell tales of battles long past."));
    DiscoveryNarration.bIsNarration = true;
    DiscoveryNarration.Duration = 4.0f;
    NarrationLibrary.Add(TEXT("Discovery_Bones"), DiscoveryNarration);
    
    // Survival tips
    FNarr_DialogueLine SurvivalNarration;
    SurvivalNarration.SpeakerName = TEXT("Narrator");
    SurvivalNarration.DialogueText = FText::FromString(TEXT("The river runs clear here. Fresh water means life, but it also draws the hungry."));
    SurvivalNarration.bIsNarration = true;
    SurvivalNarration.Duration = 5.0f;
    NarrationLibrary.Add(TEXT("Survival_Water"), SurvivalNarration);
}

bool UNarrativeManager::CheckStoryBeatConditions(const FNarr_StoryBeat& StoryBeat) const
{
    for (const FString& Condition : StoryBeat.RequiredConditions)
    {
        if (!GetStoryFlag(Condition))
        {
            return false;
        }
    }
    return true;
}

void UNarrativeManager::CompleteStoryBeat(const FString& BeatID)
{
    for (FNarr_StoryBeat& StoryBeat : StoryBeats)
    {
        if (StoryBeat.BeatID == BeatID)
        {
            StoryBeat.bIsCompleted = true;
            
            // Set completion flags
            for (const FString& Flag : StoryBeat.CompletionFlags)
            {
                SetStoryFlag(Flag, true);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Completed story beat %s"), *BeatID);
            return;
        }
    }
}