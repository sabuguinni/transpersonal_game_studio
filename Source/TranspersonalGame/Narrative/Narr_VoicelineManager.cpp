#include "Narr_VoicelineManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNarr_VoicelineManager::UNarr_VoicelineManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsPlayingVoiceline = false;
    CurrentPlaybackTime = 0.0f;
    LastVoicelineTime = 0.0f;

    // Initialize default voicelines with generated audio URLs
    FNarr_VoicelineData SurvivalNarratorLine;
    SurvivalNarratorLine.VoicelineText = TEXT("The massive footprints in the mud tell a story of terror. A T-Rex passed here just hours ago, heading toward the watering hole where you planned to drink. Change your route, survivor. Death stalks these ancient lands.");
    SurvivalNarratorLine.VoicelineType = ENarr_VoicelineType::Narrator;
    SurvivalNarratorLine.TriggerCondition = ENarr_VoicelineTrigger::OnDinosaurSighting;
    SurvivalNarratorLine.Duration = 15.0f;
    SurvivalNarratorLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777683648776_SurvivalNarrator.mp3");
    SurvivalNarratorLine.CooldownTime = 45.0f;
    VoicelineDatabase.Add(SurvivalNarratorLine);

    FNarr_VoicelineData PlayerThoughtLine;
    PlayerThoughtLine.VoicelineText = TEXT("My heart pounds as I hear the distant roar echoing through the valley. The crude spear in my hands feels pathetic against such prehistoric power. I must find shelter before nightfall brings even greater horrors.");
    PlayerThoughtLine.VoicelineType = ENarr_VoicelineType::PlayerThought;
    PlayerThoughtLine.TriggerCondition = ENarr_VoicelineTrigger::OnDanger;
    PlayerThoughtLine.Duration = 15.0f;
    PlayerThoughtLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777683651253_PlayerCharacter.mp3");
    PlayerThoughtLine.CooldownTime = 30.0f;
    VoicelineDatabase.Add(PlayerThoughtLine);

    FNarr_VoicelineData TribalElderLine;
    TribalElderLine.VoicelineText = TEXT("Watch the herbivores, young one. When the Triceratops herd suddenly stops grazing and lifts their heads, predators are near. Their ancient instincts have kept them alive for millions of years.");
    TribalElderLine.VoicelineType = ENarr_VoicelineType::TribalElder;
    TribalElderLine.TriggerCondition = ENarr_VoicelineTrigger::OnSurvival;
    TribalElderLine.Duration = 13.0f;
    TribalElderLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777683653390_TribalElder.mp3");
    TribalElderLine.CooldownTime = 60.0f;
    VoicelineDatabase.Add(TribalElderLine);

    FNarr_VoicelineData WildernessGuideLine;
    WildernessGuideLine.VoicelineText = TEXT("The ground trembles with each step of the approaching Brachiosaurus. These gentle giants fear nothing except the apex predators. If they're running, you should be too.");
    WildernessGuideLine.VoicelineType = ENarr_VoicelineType::WildernessGuide;
    WildernessGuideLine.TriggerCondition = ENarr_VoicelineTrigger::OnDanger;
    WildernessGuideLine.Duration = 12.0f;
    WildernessGuideLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777683655355_WildernessGuide.mp3");
    WildernessGuideLine.CooldownTime = 40.0f;
    VoicelineDatabase.Add(WildernessGuideLine);
}

void UNarr_VoicelineManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize trigger cooldowns
    TriggerCooldowns.Add(ENarr_VoicelineTrigger::OnPlayerSpawn, 0.0f);
    TriggerCooldowns.Add(ENarr_VoicelineTrigger::OnDinosaurSighting, 0.0f);
    TriggerCooldowns.Add(ENarr_VoicelineTrigger::OnLowHealth, 0.0f);
    TriggerCooldowns.Add(ENarr_VoicelineTrigger::OnCrafting, 0.0f);
    TriggerCooldowns.Add(ENarr_VoicelineTrigger::OnNightfall, 0.0f);
    TriggerCooldowns.Add(ENarr_VoicelineTrigger::OnDanger, 0.0f);
    TriggerCooldowns.Add(ENarr_VoicelineTrigger::OnDiscovery, 0.0f);
    TriggerCooldowns.Add(ENarr_VoicelineTrigger::OnSurvival, 0.0f);

    // Trigger welcome voiceline on spawn
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this]() { TriggerVoiceline(ENarr_VoicelineTrigger::OnPlayerSpawn); },
        2.0f,
        false
    );
}

void UNarr_VoicelineManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateCooldowns(DeltaTime);

    if (bIsPlayingVoiceline)
    {
        CurrentPlaybackTime += DeltaTime;
        if (CurrentPlaybackTime >= CurrentVoiceline.Duration)
        {
            StopCurrentVoiceline();
        }
    }
}

void UNarr_VoicelineManager::TriggerVoiceline(ENarr_VoicelineTrigger TriggerType)
{
    if (bIsPlayingVoiceline)
    {
        return; // Don't interrupt current voiceline
    }

    // Check cooldown
    if (TriggerCooldowns.Contains(TriggerType))
    {
        if (TriggerCooldowns[TriggerType] > 0.0f)
        {
            return; // Still on cooldown
        }
    }

    // Find voicelines for this trigger
    TArray<FNarr_VoicelineData> AvailableVoicelines = GetVoicelinesByTrigger(TriggerType);
    
    // Filter out recently played voicelines
    TArray<FNarr_VoicelineData> PlayableVoicelines;
    for (const FNarr_VoicelineData& Voiceline : AvailableVoicelines)
    {
        if (CanPlayVoiceline(Voiceline))
        {
            PlayableVoicelines.Add(Voiceline);
        }
    }

    if (PlayableVoicelines.Num() > 0)
    {
        FNarr_VoicelineData SelectedVoiceline = SelectRandomVoiceline(PlayableVoicelines);
        PlayVoiceline(SelectedVoiceline);
        
        // Set cooldown for this trigger type
        TriggerCooldowns[TriggerType] = SelectedVoiceline.CooldownTime;
    }
}

void UNarr_VoicelineManager::PlayVoiceline(const FNarr_VoicelineData& VoicelineData)
{
    if (bIsPlayingVoiceline)
    {
        StopCurrentVoiceline();
    }

    CurrentVoiceline = VoicelineData;
    bIsPlayingVoiceline = true;
    CurrentPlaybackTime = 0.0f;
    LastVoicelineTime = GetWorld()->GetTimeSeconds();

    // Log the voiceline for debugging
    UE_LOG(LogTemp, Warning, TEXT("Playing Voiceline: %s"), *VoicelineData.VoicelineText);
    
    // Display subtitle on screen
    if (GEngine)
    {
        FString VoicelineTypeStr;
        switch (VoicelineData.VoicelineType)
        {
            case ENarr_VoicelineType::Narrator:
                VoicelineTypeStr = TEXT("Narrator");
                break;
            case ENarr_VoicelineType::PlayerThought:
                VoicelineTypeStr = TEXT("Player");
                break;
            case ENarr_VoicelineType::TribalElder:
                VoicelineTypeStr = TEXT("Elder");
                break;
            case ENarr_VoicelineType::WildernessGuide:
                VoicelineTypeStr = TEXT("Guide");
                break;
            default:
                VoicelineTypeStr = TEXT("Voice");
                break;
        }

        FString DisplayText = FString::Printf(TEXT("[%s]: %s"), *VoicelineTypeStr, *VoicelineData.VoicelineText);
        GEngine->AddOnScreenDebugMessage(-1, VoicelineData.Duration, FColor::Yellow, DisplayText);
    }

    // Mark as played in database
    for (FNarr_VoicelineData& Voiceline : VoicelineDatabase)
    {
        if (Voiceline.VoicelineText == VoicelineData.VoicelineText)
        {
            Voiceline.bIsPlayed = true;
            break;
        }
    }
}

void UNarr_VoicelineManager::StopCurrentVoiceline()
{
    bIsPlayingVoiceline = false;
    CurrentPlaybackTime = 0.0f;
    CurrentVoiceline = FNarr_VoicelineData();
}

void UNarr_VoicelineManager::AddVoiceline(const FNarr_VoicelineData& NewVoiceline)
{
    VoicelineDatabase.Add(NewVoiceline);
}

TArray<FNarr_VoicelineData> UNarr_VoicelineManager::GetVoicelinesByType(ENarr_VoicelineType VoicelineType)
{
    TArray<FNarr_VoicelineData> FilteredVoicelines;
    for (const FNarr_VoicelineData& Voiceline : VoicelineDatabase)
    {
        if (Voiceline.VoicelineType == VoicelineType)
        {
            FilteredVoicelines.Add(Voiceline);
        }
    }
    return FilteredVoicelines;
}

TArray<FNarr_VoicelineData> UNarr_VoicelineManager::GetVoicelinesByTrigger(ENarr_VoicelineTrigger TriggerType)
{
    TArray<FNarr_VoicelineData> FilteredVoicelines;
    for (const FNarr_VoicelineData& Voiceline : VoicelineDatabase)
    {
        if (Voiceline.TriggerCondition == TriggerType)
        {
            FilteredVoicelines.Add(Voiceline);
        }
    }
    return FilteredVoicelines;
}

void UNarr_VoicelineManager::OnPlayerHealthLow()
{
    TriggerVoiceline(ENarr_VoicelineTrigger::OnLowHealth);
}

void UNarr_VoicelineManager::OnDinosaurDetected(const FString& DinosaurType)
{
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur detected: %s"), *DinosaurType);
    TriggerVoiceline(ENarr_VoicelineTrigger::OnDinosaurSighting);
}

void UNarr_VoicelineManager::OnCraftingStarted(const FString& ItemType)
{
    UE_LOG(LogTemp, Warning, TEXT("Crafting started: %s"), *ItemType);
    TriggerVoiceline(ENarr_VoicelineTrigger::OnCrafting);
}

void UNarr_VoicelineManager::OnNightfallApproaching()
{
    TriggerVoiceline(ENarr_VoicelineTrigger::OnNightfall);
}

void UNarr_VoicelineManager::OnDiscoveryMade(const FString& DiscoveryType)
{
    UE_LOG(LogTemp, Warning, TEXT("Discovery made: %s"), *DiscoveryType);
    TriggerVoiceline(ENarr_VoicelineTrigger::OnDiscovery);
}

bool UNarr_VoicelineManager::CanPlayVoiceline(const FNarr_VoicelineData& VoicelineData)
{
    // Check if enough time has passed since last voiceline
    float TimeSinceLastVoiceline = GetWorld()->GetTimeSeconds() - LastVoicelineTime;
    if (TimeSinceLastVoiceline < 5.0f) // Minimum 5 seconds between voicelines
    {
        return false;
    }

    return true;
}

void UNarr_VoicelineManager::UpdateCooldowns(float DeltaTime)
{
    for (auto& CooldownPair : TriggerCooldowns)
    {
        if (CooldownPair.Value > 0.0f)
        {
            CooldownPair.Value -= DeltaTime;
            if (CooldownPair.Value < 0.0f)
            {
                CooldownPair.Value = 0.0f;
            }
        }
    }
}

FNarr_VoicelineData UNarr_VoicelineManager::SelectRandomVoiceline(const TArray<FNarr_VoicelineData>& Voicelines)
{
    if (Voicelines.Num() == 0)
    {
        return FNarr_VoicelineData();
    }

    int32 RandomIndex = FMath::RandRange(0, Voicelines.Num() - 1);
    return Voicelines[RandomIndex];
}