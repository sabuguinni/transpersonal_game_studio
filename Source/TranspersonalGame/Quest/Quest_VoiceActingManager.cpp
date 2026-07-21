#include "Quest_VoiceActingManager.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

AQuest_VoiceActingManager::AQuest_VoiceActingManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    RootComponent = AudioComponent;

    // Initialize default values
    bAutoPlayOnTrigger = true;
    VoiceLineDelay = 1.0f;
    bIsPlaying = false;
    CurrentPlayTime = 0.0f;
}

void AQuest_VoiceActingManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVoiceLines();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Voice Acting Manager initialized with %d voice lines"), VoiceLines.Num());
}

void AQuest_VoiceActingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsPlaying)
    {
        CurrentPlayTime += DeltaTime;
        
        // Check if current voice line has finished
        if (CurrentPlayTime >= CurrentVoiceLine.Duration)
        {
            bIsPlaying = false;
            CurrentPlayTime = 0.0f;
            OnVoiceLineCompleted(CurrentVoiceLine);
        }
    }
}

void AQuest_VoiceActingManager::PlayVoiceLine(const FString& CharacterName, const FString& QuestID)
{
    if (bIsPlaying)
    {
        StopCurrentVoiceLine();
    }

    // Find voice line by character name
    FQuest_VoiceLineData* FoundVoiceLine = nullptr;
    
    for (FQuest_VoiceLineData& VoiceLine : VoiceLines)
    {
        if (VoiceLine.CharacterName == CharacterName)
        {
            // If QuestID is specified, match it too
            if (!QuestID.IsEmpty() && VoiceLine.QuestID != QuestID)
            {
                continue;
            }
            
            FoundVoiceLine = &VoiceLine;
            break;
        }
    }

    if (FoundVoiceLine)
    {
        CurrentVoiceLine = *FoundVoiceLine;
        bIsPlaying = true;
        CurrentPlayTime = 0.0f;

        // Log the voice line being played
        UE_LOG(LogTemp, Warning, TEXT("Playing voice line for %s: %s"), 
               *CharacterName, *CurrentVoiceLine.VoiceLineText);

        // Load and play audio from URL (placeholder implementation)
        LoadAudioFromURL(CurrentVoiceLine.AudioURL);

        // Trigger Blueprint event
        OnVoiceLineStarted(CurrentVoiceLine);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Voice line not found for character: %s"), *CharacterName);
    }
}

void AQuest_VoiceActingManager::StopCurrentVoiceLine()
{
    if (bIsPlaying && AudioComponent)
    {
        AudioComponent->Stop();
        bIsPlaying = false;
        CurrentPlayTime = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("Stopped current voice line for %s"), *CurrentVoiceLine.CharacterName);
    }
}

void AQuest_VoiceActingManager::AddVoiceLine(const FQuest_VoiceLineData& NewVoiceLine)
{
    VoiceLines.Add(NewVoiceLine);
    UE_LOG(LogTemp, Warning, TEXT("Added voice line for character: %s"), *NewVoiceLine.CharacterName);
}

bool AQuest_VoiceActingManager::IsVoiceLinePlaying() const
{
    return bIsPlaying;
}

FQuest_VoiceLineData AQuest_VoiceActingManager::GetVoiceLineByCharacter(const FString& CharacterName) const
{
    for (const FQuest_VoiceLineData& VoiceLine : VoiceLines)
    {
        if (VoiceLine.CharacterName == CharacterName)
        {
            return VoiceLine;
        }
    }
    
    return FQuest_VoiceLineData(); // Return empty struct if not found
}

TArray<FQuest_VoiceLineData> AQuest_VoiceActingManager::GetVoiceLinesByQuest(const FString& QuestID) const
{
    TArray<FQuest_VoiceLineData> QuestVoiceLines;
    
    for (const FQuest_VoiceLineData& VoiceLine : VoiceLines)
    {
        if (VoiceLine.QuestID == QuestID)
        {
            QuestVoiceLines.Add(VoiceLine);
        }
    }
    
    return QuestVoiceLines;
}

void AQuest_VoiceActingManager::InitializeVoiceLines()
{
    // Initialize with the generated voice lines
    FQuest_VoiceLineData DrElenaLine;
    DrElenaLine.CharacterName = TEXT("Dr_Elena_Vasquez");
    DrElenaLine.VoiceLineText = TEXT("Welcome, fellow survivor. I am Dr. Elena Vasquez, a paleontologist who has been studying these ancient creatures for months. The key to survival in this Cretaceous world is understanding the behavior patterns of the dinosaurs around us. Stay close, observe carefully, and remember - knowledge is your greatest weapon against extinction.");
    DrElenaLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778406574060_Dr_Elena_Vasquez.mp3");
    DrElenaLine.Duration = 23.0f;
    DrElenaLine.bIsQuestCritical = true;
    DrElenaLine.QuestID = TEXT("INTRO_TUTORIAL");
    VoiceLines.Add(DrElenaLine);

    FQuest_VoiceLineData ForestGuideLine;
    ForestGuideLine.CharacterName = TEXT("Forest_Guide_Marcus");
    ForestGuideLine.VoiceLineText = TEXT("Listen carefully, newcomer. The forest holds many secrets, but also many dangers. The Velociraptors hunt in packs here, especially near the fallen logs. If you need shelter materials, look for the dead trees - but never venture alone. The pack hunters can smell fear, and they never forget a scent.");
    ForestGuideLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778406582754_Forest_Guide_Marcus.mp3");
    ForestGuideLine.Duration = 20.0f;
    ForestGuideLine.bIsQuestCritical = true;
    ForestGuideLine.QuestID = TEXT("FOREST_EXPLORATION");
    VoiceLines.Add(ForestGuideLine);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d voice lines for quest system"), VoiceLines.Num());
}

void AQuest_VoiceActingManager::LoadAudioFromURL(const FString& AudioURL)
{
    // Placeholder implementation for loading audio from URL
    // In a real implementation, this would download and load the audio file
    UE_LOG(LogTemp, Warning, TEXT("Loading audio from URL: %s"), *AudioURL);
    
    // For now, just simulate playing audio
    if (AudioComponent)
    {
        // In a real implementation, you would:
        // 1. Download the audio file from the URL
        // 2. Create a USoundWave from the downloaded data
        // 3. Set it as the sound for the AudioComponent
        // 4. Play the audio
        
        UE_LOG(LogTemp, Warning, TEXT("Audio playback simulated for URL: %s"), *AudioURL);
    }
}