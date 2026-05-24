#include "Narr_VoiceAssetManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UNarr_VoiceAssetManager::UNarr_VoiceAssetManager()
{
    bIsVoicePlaying = false;
    CurrentVoiceStartTime = 0.0f;
    VoiceAudioComponent = nullptr;
    CurrentlyPlayingAssetID = TEXT("");
}

void UNarr_VoiceAssetManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("VoiceAssetManager: Initializing narrative voice system"));
    
    // Initialize default voice assets from this production cycle
    InitializeDefaultVoiceAssets();
    
    UE_LOG(LogTemp, Log, TEXT("VoiceAssetManager: Initialized with %d voice assets"), VoiceAssets.Num());
}

void UNarr_VoiceAssetManager::Deinitialize()
{
    StopAllVoices();
    VoiceAssets.Empty();
    VoiceQueue.Empty();
    
    if (VoiceAudioComponent && IsValid(VoiceAudioComponent))
    {
        VoiceAudioComponent->DestroyComponent();
        VoiceAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UNarr_VoiceAssetManager::RegisterVoiceAsset(const FNarr_VoiceAssetData& AssetData)
{
    if (AssetData.AssetID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("VoiceAssetManager: Cannot register voice asset with empty AssetID"));
        return;
    }
    
    VoiceAssets.Add(AssetData.AssetID, AssetData);
    UE_LOG(LogTemp, Log, TEXT("VoiceAssetManager: Registered voice asset '%s' for character '%s'"), 
           *AssetData.AssetID, *AssetData.CharacterName);
}

bool UNarr_VoiceAssetManager::GetVoiceAsset(const FString& AssetID, FNarr_VoiceAssetData& OutAssetData)
{
    if (FNarr_VoiceAssetData* FoundAsset = VoiceAssets.Find(AssetID))
    {
        OutAssetData = *FoundAsset;
        return true;
    }
    return false;
}

TArray<FNarr_VoiceAssetData> UNarr_VoiceAssetManager::GetVoiceAssetsByCategory(ENarr_VoiceCategory Category)
{
    TArray<FNarr_VoiceAssetData> FilteredAssets;
    
    for (const auto& AssetPair : VoiceAssets)
    {
        if (AssetPair.Value.Category == Category)
        {
            FilteredAssets.Add(AssetPair.Value);
        }
    }
    
    return FilteredAssets;
}

TArray<FNarr_VoiceAssetData> UNarr_VoiceAssetManager::GetVoiceAssetsByCharacter(const FString& CharacterName)
{
    TArray<FNarr_VoiceAssetData> FilteredAssets;
    
    for (const auto& AssetPair : VoiceAssets)
    {
        if (AssetPair.Value.CharacterName.Equals(CharacterName, ESearchCase::IgnoreCase))
        {
            FilteredAssets.Add(AssetPair.Value);
        }
    }
    
    return FilteredAssets;
}

bool UNarr_VoiceAssetManager::PlayVoiceAsset(const FNarr_VoicePlaybackRequest& Request)
{
    FNarr_VoiceAssetData AssetData;
    if (!GetVoiceAsset(Request.AssetID, AssetData))
    {
        UE_LOG(LogTemp, Warning, TEXT("VoiceAssetManager: Voice asset '%s' not found"), *Request.AssetID);
        return false;
    }
    
    // Check if we should interrupt current playback
    if (bIsVoicePlaying && !Request.bInterruptCurrent)
    {
        // Queue the request instead
        QueueVoiceAsset(Request);
        return true;
    }
    
    // Stop current voice if playing
    if (bIsVoicePlaying)
    {
        StopCurrentVoice();
    }
    
    // Load and play the voice asset
    return LoadAndPlayVoiceAsset(AssetData, Request.VolumeMultiplier);
}

void UNarr_VoiceAssetManager::StopCurrentVoice()
{
    if (VoiceAudioComponent && IsValid(VoiceAudioComponent))
    {
        VoiceAudioComponent->Stop();
    }
    
    bIsVoicePlaying = false;
    CurrentlyPlayingAssetID = TEXT("");
    CurrentVoiceStartTime = 0.0f;
}

void UNarr_VoiceAssetManager::StopAllVoices()
{
    StopCurrentVoice();
    ClearVoiceQueue();
}

bool UNarr_VoiceAssetManager::IsVoicePlaying() const
{
    return bIsVoicePlaying;
}

FString UNarr_VoiceAssetManager::GetCurrentlyPlayingAssetID() const
{
    return CurrentlyPlayingAssetID;
}

void UNarr_VoiceAssetManager::QueueVoiceAsset(const FNarr_VoicePlaybackRequest& Request)
{
    // Insert based on priority
    int32 InsertIndex = VoiceQueue.Num();
    for (int32 i = 0; i < VoiceQueue.Num(); i++)
    {
        if (Request.Priority > VoiceQueue[i].Priority)
        {
            InsertIndex = i;
            break;
        }
    }
    
    VoiceQueue.Insert(Request, InsertIndex);
    UE_LOG(LogTemp, Log, TEXT("VoiceAssetManager: Queued voice asset '%s' at position %d"), 
           *Request.AssetID, InsertIndex);
}

void UNarr_VoiceAssetManager::ClearVoiceQueue()
{
    VoiceQueue.Empty();
}

int32 UNarr_VoiceAssetManager::GetQueueLength() const
{
    return VoiceQueue.Num();
}

FString UNarr_VoiceAssetManager::SelectContextualVoice(const TArray<FString>& Keywords, ENarr_VoiceCategory PreferredCategory)
{
    TArray<FNarr_VoiceAssetData> CategoryAssets = GetVoiceAssetsByCategory(PreferredCategory);
    
    // Score assets based on keyword matches
    TMap<FString, int32> AssetScores;
    
    for (const FNarr_VoiceAssetData& Asset : CategoryAssets)
    {
        int32 Score = 0;
        
        for (const FString& Keyword : Keywords)
        {
            for (const FString& AssetKeyword : Asset.TriggerKeywords)
            {
                if (AssetKeyword.Contains(Keyword) || Keyword.Contains(AssetKeyword))
                {
                    Score += 10;
                }
            }
            
            // Check transcript for keyword matches
            if (Asset.TranscriptText.Contains(Keyword))
            {
                Score += 5;
            }
        }
        
        if (Score > 0)
        {
            AssetScores.Add(Asset.AssetID, Score);
        }
    }
    
    // Find highest scoring asset
    FString BestAssetID;
    int32 HighestScore = 0;
    
    for (const auto& ScorePair : AssetScores)
    {
        if (ScorePair.Value > HighestScore)
        {
            HighestScore = ScorePair.Value;
            BestAssetID = ScorePair.Key;
        }
    }
    
    return BestAssetID;
}

void UNarr_VoiceAssetManager::PlayContextualVoice(const TArray<FString>& Keywords, ENarr_VoiceCategory Category, ENarr_VoicePriority Priority)
{
    FString SelectedAssetID = SelectContextualVoice(Keywords, Category);
    
    if (!SelectedAssetID.IsEmpty())
    {
        FNarr_VoicePlaybackRequest Request;
        Request.AssetID = SelectedAssetID;
        Request.Priority = Priority;
        Request.bInterruptCurrent = (Priority >= ENarr_VoicePriority::High);
        Request.VolumeMultiplier = 1.0f;
        
        PlayVoiceAsset(Request);
    }
}

void UNarr_VoiceAssetManager::ProcessVoiceQueue()
{
    if (!bIsVoicePlaying && VoiceQueue.Num() > 0)
    {
        FNarr_VoicePlaybackRequest NextRequest = VoiceQueue[0];
        VoiceQueue.RemoveAt(0);
        
        PlayVoiceAsset(NextRequest);
    }
}

void UNarr_VoiceAssetManager::OnVoicePlaybackFinished()
{
    bIsVoicePlaying = false;
    CurrentlyPlayingAssetID = TEXT("");
    CurrentVoiceStartTime = 0.0f;
    
    // Process next item in queue
    ProcessVoiceQueue();
}

bool UNarr_VoiceAssetManager::LoadAndPlayVoiceAsset(const FNarr_VoiceAssetData& AssetData, float VolumeMultiplier)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("VoiceAssetManager: No valid world for voice playback"));
        return false;
    }
    
    // For now, log the voice asset details (actual audio loading would require additional implementation)
    UE_LOG(LogTemp, Log, TEXT("VoiceAssetManager: Playing voice asset '%s' by %s"), 
           *AssetData.AssetID, *AssetData.CharacterName);
    UE_LOG(LogTemp, Log, TEXT("VoiceAssetManager: Audio URL: %s"), *AssetData.AudioURL);
    UE_LOG(LogTemp, Log, TEXT("VoiceAssetManager: Duration: %.1f seconds"), AssetData.DurationSeconds);
    
    // Set playback state
    bIsVoicePlaying = true;
    CurrentlyPlayingAssetID = AssetData.AssetID;
    CurrentVoiceStartTime = World->GetTimeSeconds();
    
    // Schedule playback completion (simplified for now)
    FTimerHandle PlaybackTimer;
    World->GetTimerManager().SetTimer(PlaybackTimer, 
        FTimerDelegate::CreateUObject(this, &UNarr_VoiceAssetManager::OnVoicePlaybackFinished),
        AssetData.DurationSeconds, false);
    
    return true;
}

void UNarr_VoiceAssetManager::InitializeDefaultVoiceAssets()
{
    // Register voice assets generated in this production cycle
    
    // Tactical Survivor - Carnotaurus warning
    FNarr_VoiceAssetData TacticalAsset;
    TacticalAsset.AssetID = TEXT("tactical_carnotaurus_warning");
    TacticalAsset.CharacterName = TEXT("TacticalSurvivor");
    TacticalAsset.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778558500645_TacticalSurvivor.mp3");
    TacticalAsset.DurationSeconds = 28.0f;
    TacticalAsset.Category = ENarr_VoiceCategory::TacticalWarning;
    TacticalAsset.Priority = ENarr_VoicePriority::High;
    TacticalAsset.TranscriptText = TEXT("Listen carefully, survivor. The Carnotaurus pack has been tracked moving through sector seven. Three adults, one juvenile. They hunt in coordinated formation - the alpha leads from the front while the others flank from both sides. If you encounter them, do not run in a straight line. Use the rocky outcrops for cover and wait for them to pass. Your survival depends on understanding their hunting patterns.");
    TacticalAsset.TriggerKeywords = {TEXT("Carnotaurus"), TEXT("pack"), TEXT("hunting"), TEXT("tactical"), TEXT("survival"), TEXT("warning")};
    RegisterVoiceAsset(TacticalAsset);
    
    // Field Researcher - Daily log
    FNarr_VoiceAssetData ResearchAsset;
    ResearchAsset.AssetID = TEXT("field_research_day241");
    ResearchAsset.CharacterName = TEXT("FieldResearcher");
    ResearchAsset.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778558511526_FieldResearcher.mp3");
    ResearchAsset.DurationSeconds = 29.0f;
    ResearchAsset.Category = ENarr_VoiceCategory::FieldResearch;
    ResearchAsset.Priority = ENarr_VoicePriority::Medium;
    ResearchAsset.TranscriptText = TEXT("Day 241 in the Cretaceous wilderness. The morning brings troubling discoveries. Fresh Allosaurus tracks near the water source suggest a territorial dispute is brewing. The herbivore herds have moved south earlier than expected - a clear sign they sense danger approaching. Weather patterns indicate a storm system moving in from the northwest. Recommend immediate shelter preparation and water collection before the rains arrive.");
    ResearchAsset.TriggerKeywords = {TEXT("Allosaurus"), TEXT("research"), TEXT("tracks"), TEXT("weather"), TEXT("storm"), TEXT("shelter")};
    RegisterVoiceAsset(ResearchAsset);
    
    // Emergency Coordinator - Giganotosaurus alert
    FNarr_VoiceAssetData EmergencyAsset;
    EmergencyAsset.AssetID = TEXT("emergency_giganotosaurus_alert");
    EmergencyAsset.CharacterName = TEXT("EmergencyCoordinator");
    EmergencyAsset.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778558520998_EmergencyCoordinator.mp3");
    EmergencyAsset.DurationSeconds = 28.0f;
    EmergencyAsset.Category = ENarr_VoiceCategory::EmergencyAlert;
    EmergencyAsset.Priority = ENarr_VoicePriority::Critical;
    EmergencyAsset.TranscriptText = TEXT("Emergency protocol activated! Massive Giganotosaurus detected approaching from the eastern ridge. This is not a drill. All personnel must evacuate to designated safe zones immediately. The creature measures approximately fourteen meters in length with active hunting behavior. Do not attempt to engage or observe. Repeat - immediate evacuation required. Use emergency route Charlie through the canyon passage.");
    EmergencyAsset.TriggerKeywords = {TEXT("emergency"), TEXT("Giganotosaurus"), TEXT("evacuation"), TEXT("protocol"), TEXT("alert"), TEXT("danger")};
    RegisterVoiceAsset(EmergencyAsset);
    
    // Wise Tracker - Ancient forest wisdom
    FNarr_VoiceAssetData WisdomAsset;
    WisdomAsset.AssetID = TEXT("wisdom_ancient_forest");
    WisdomAsset.CharacterName = TEXT("WiseTracker");
    WisdomAsset.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778558531047_WiseTracker.mp3");
    WisdomAsset.DurationSeconds = 30.0f;
    WisdomAsset.Category = ENarr_VoiceCategory::WisdomNarration;
    WisdomAsset.Priority = ENarr_VoicePriority::Low;
    WisdomAsset.TranscriptText = TEXT("The ancient forest holds many secrets, young tracker. These massive fern groves have stood for millions of years, witnessing the rise and fall of countless species. The Parasaurolophus you seek follows the old migration routes carved into the very landscape by their ancestors. Listen for their haunting calls echoing through the valleys - they communicate across vast distances, warning of dangers and sharing the location of fresh feeding grounds.");
    WisdomAsset.TriggerKeywords = {TEXT("forest"), TEXT("ancient"), TEXT("Parasaurolophus"), TEXT("migration"), TEXT("wisdom"), TEXT("tracker")};
    RegisterVoiceAsset(WisdomAsset);
}