#include "Narr_SurvivalStorySystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/Core/TranspersonalCharacter.h"
#include "TranspersonalGame/Shared/SharedTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogSurvivalStory, Log, All);

UNarr_SurvivalStorySystem::UNarr_SurvivalStorySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize survival story states
    CurrentStoryPhase = ENarr_StoryPhase::Introduction;
    DaysInWilderness = 0;
    MajorEventsTriggered = 0;
    PlayerSurvivalScore = 0.0f;
    LastNarrativeUpdate = 0.0f;
    
    // Initialize story triggers
    bVolcanicEventTriggered = false;
    bFirstDinosaurEncounter = false;
    bFirstNightSurvived = false;
    bFirstToolCrafted = false;
    bFirstShelterBuilt = false;
    bFirstHuntSuccessful = false;
    bPackDynamicsDiscovered = false;
    bMigrationEventWitnessed = false;
    bTerritorialConflictSeen = false;
    bSeasonalChangeExperienced = false;
    
    // Initialize contextual triggers
    CurrentBiome = ENarr_BiomeType::Savana;
    CurrentTimeOfDay = ENarr_TimeOfDay::Dawn;
    CurrentWeather = ENarr_WeatherType::Clear;
    CurrentThreatLevel = ENarr_ThreatLevel::Low;
    
    // Audio settings
    NarrativeAudioRange = 2000.0f;
    VoicelineVolume = 0.8f;
    bUsePositionalAudio = true;
}

void UNarr_SurvivalStorySystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize story progression
    InitializeStoryProgression();
    
    // Start introduction sequence
    TriggerIntroductionSequence();
    
    UE_LOG(LogSurvivalStory, Log, TEXT("Survival Story System initialized"));
}

void UNarr_SurvivalStorySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update story progression
    UpdateStoryProgression(DeltaTime);
    
    // Check for contextual narrative triggers
    CheckContextualTriggers();
    
    // Update survival metrics
    UpdateSurvivalMetrics();
}

void UNarr_SurvivalStorySystem::InitializeStoryProgression()
{
    // Set up story phases
    StoryPhases.Empty();
    
    FNarr_StoryPhaseData IntroPhase;
    IntroPhase.PhaseName = TEXT("Awakening");
    IntroPhase.PhaseDescription = TEXT("First moments in the prehistoric wilderness");
    IntroPhase.RequiredEvents.Add(TEXT("FirstMovement"));
    IntroPhase.RequiredEvents.Add(TEXT("FirstLook"));
    StoryPhases.Add(ENarr_StoryPhase::Introduction, IntroPhase);
    
    FNarr_StoryPhaseData SurvivalPhase;
    SurvivalPhase.PhaseName = TEXT("Survival Basics");
    SurvivalPhase.PhaseDescription = TEXT("Learning to survive in the dangerous world");
    SurvivalPhase.RequiredEvents.Add(TEXT("FirstNight"));
    SurvivalPhase.RequiredEvents.Add(TEXT("FirstTool"));
    SurvivalPhase.RequiredEvents.Add(TEXT("FirstShelter"));
    StoryPhases.Add(ENarr_StoryPhase::EarlyGame, SurvivalPhase);
    
    FNarr_StoryPhaseData ExplorationPhase;
    ExplorationPhase.PhaseName = TEXT("World Discovery");
    ExplorationPhase.PhaseDescription = TEXT("Exploring biomes and discovering dinosaur behavior");
    ExplorationPhase.RequiredEvents.Add(TEXT("BiomeDiscovery"));
    ExplorationPhase.RequiredEvents.Add(TEXT("DinosaurEncounter"));
    ExplorationPhase.RequiredEvents.Add(TEXT("PackBehavior"));
    StoryPhases.Add(ENarr_StoryPhase::MidGame, ExplorationPhase);
    
    FNarr_StoryPhaseData MasteryPhase;
    MasteryPhase.PhaseName = TEXT("Apex Survivor");
    MasteryPhase.PhaseDescription = TEXT("Mastering the prehistoric ecosystem");
    MasteryPhase.RequiredEvents.Add(TEXT("TerritoryEstablished"));
    MasteryPhase.RequiredEvents.Add(TEXT("AlphaEncounter"));
    MasteryPhase.RequiredEvents.Add(TEXT("EcosystemMastery"));
    StoryPhases.Add(ENarr_StoryPhase::LateGame, MasteryPhase);
}

void UNarr_SurvivalStorySystem::TriggerIntroductionSequence()
{
    // Play opening narrative
    FNarr_VoicelineData IntroVoiceline;
    IntroVoiceline.VoicelineText = TEXT("Day 1 in the wilderness. I wake to the sounds of a world that time forgot. Massive creatures roam these lands, and I am nothing but prey. I must learn quickly, adapt faster, or become another fossil in this ancient graveyard.");
    IntroVoiceline.CharacterName = TEXT("Player_Journal");
    IntroVoiceline.EmotionalState = ENarr_EmotionalState::Fearful;
    IntroVoiceline.Priority = ENarr_VoicelinePriority::Critical;
    IntroVoiceline.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778394097042_Player_Journal.mp3");
    
    PlayVoiceline(IntroVoiceline);
    
    // Set initial story state
    CurrentStoryPhase = ENarr_StoryPhase::Introduction;
    DaysInWilderness = 1;
    
    UE_LOG(LogSurvivalStory, Log, TEXT("Introduction sequence triggered"));
}

void UNarr_SurvivalStorySystem::UpdateStoryProgression(float DeltaTime)
{
    LastNarrativeUpdate += DeltaTime;
    
    // Update every 5 seconds
    if (LastNarrativeUpdate >= 5.0f)
    {
        LastNarrativeUpdate = 0.0f;
        
        // Check for story phase progression
        CheckStoryPhaseProgression();
        
        // Update day counter (simplified - in real game would be based on day/night cycle)
        static float DayTimer = 0.0f;
        DayTimer += DeltaTime;
        if (DayTimer >= 300.0f) // 5 minutes = 1 day for testing
        {
            DayTimer = 0.0f;
            DaysInWilderness++;
            OnDayProgressed();
        }
    }
}

void UNarr_SurvivalStorySystem::CheckContextualTriggers()
{
    // Check biome-based triggers
    CheckBiomeTriggers();
    
    // Check time-based triggers
    CheckTimeTriggers();
    
    // Check threat-based triggers
    CheckThreatTriggers();
    
    // Check survival state triggers
    CheckSurvivalTriggers();
}

void UNarr_SurvivalStorySystem::CheckBiomeTriggers()
{
    // Get player location and determine biome
    if (ATranspersonalCharacter* Player = GetPlayerCharacter())
    {
        FVector PlayerLocation = Player->GetActorLocation();
        ENarr_BiomeType NewBiome = DetermineBiomeFromLocation(PlayerLocation);
        
        if (NewBiome != CurrentBiome)
        {
            OnBiomeChanged(CurrentBiome, NewBiome);
            CurrentBiome = NewBiome;
        }
    }
}

void UNarr_SurvivalStorySystem::CheckTimeTriggers()
{
    // In a full implementation, this would check actual time of day
    // For now, we simulate time progression
    static float TimeCounter = 0.0f;
    TimeCounter += GetWorld()->GetDeltaSeconds();
    
    if (TimeCounter >= 60.0f) // Change time every minute for testing
    {
        TimeCounter = 0.0f;
        
        // Cycle through times of day
        int32 CurrentTimeIndex = static_cast<int32>(CurrentTimeOfDay);
        CurrentTimeIndex = (CurrentTimeIndex + 1) % 4;
        CurrentTimeOfDay = static_cast<ENarr_TimeOfDay>(CurrentTimeIndex);
        
        OnTimeOfDayChanged(CurrentTimeOfDay);
    }
}

void UNarr_SurvivalStorySystem::CheckThreatTriggers()
{
    // Check for nearby dinosaurs or threats
    if (ATranspersonalCharacter* Player = GetPlayerCharacter())
    {
        // In full implementation, would scan for dinosaur actors
        // For now, simulate threat level changes
        static float ThreatTimer = 0.0f;
        ThreatTimer += GetWorld()->GetDeltaSeconds();
        
        if (ThreatTimer >= 30.0f) // Change threat level every 30 seconds
        {
            ThreatTimer = 0.0f;
            
            // Randomly change threat level
            int32 RandomThreat = FMath::RandRange(0, 3);
            ENarr_ThreatLevel NewThreatLevel = static_cast<ENarr_ThreatLevel>(RandomThreat);
            
            if (NewThreatLevel != CurrentThreatLevel)
            {
                OnThreatLevelChanged(CurrentThreatLevel, NewThreatLevel);
                CurrentThreatLevel = NewThreatLevel;
            }
        }
    }
}

void UNarr_SurvivalStorySystem::CheckSurvivalTriggers()
{
    if (ATranspersonalCharacter* Player = GetPlayerCharacter())
    {
        // Check survival stats and trigger appropriate narratives
        float Health = Player->GetHealth();
        float Hunger = Player->GetHunger();
        float Thirst = Player->GetThirst();
        float Fear = Player->GetFear();
        
        // Low health warning
        if (Health < 30.0f && !bLowHealthWarningGiven)
        {
            TriggerLowHealthNarrative();
            bLowHealthWarningGiven = true;
        }
        else if (Health > 60.0f)
        {
            bLowHealthWarningGiven = false;
        }
        
        // High fear state
        if (Fear > 80.0f && !bHighFearStateActive)
        {
            TriggerHighFearNarrative();
            bHighFearStateActive = true;
        }
        else if (Fear < 40.0f)
        {
            bHighFearStateActive = false;
        }
    }
}

void UNarr_SurvivalStorySystem::OnBiomeChanged(ENarr_BiomeType OldBiome, ENarr_BiomeType NewBiome)
{
    FNarr_VoicelineData BiomeVoiceline;
    
    switch (NewBiome)
    {
        case ENarr_BiomeType::Forest:
            BiomeVoiceline.VoicelineText = TEXT("The forest canopy blocks out the sun. Ancient trees tower overhead like cathedral pillars. Every shadow could hide a predator. The air is thick with the scent of decay and new growth.");
            BiomeVoiceline.CharacterName = TEXT("Kael_Tracker");
            BiomeVoiceline.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778394088072_Kael_Tracker.mp3");
            break;
            
        case ENarr_BiomeType::Desert:
            BiomeVoiceline.VoicelineText = TEXT("The desert stretches endlessly before me. The heat is merciless, and water is more precious than gold. But the open terrain means I can see threats coming from miles away.");
            BiomeVoiceline.CharacterName = TEXT("Player_Journal");
            break;
            
        case ENarr_BiomeType::Mountain:
            BiomeVoiceline.VoicelineText = TEXT("The mountain peaks pierce the sky like ancient spears. The air is thin and cold, but the elevation provides a strategic advantage. I can see the entire valley from here.");
            BiomeVoiceline.CharacterName = TEXT("Player_Journal");
            break;
            
        case ENarr_BiomeType::Swamp:
            BiomeVoiceline.VoicelineText = TEXT("The swamp bubbles with primordial life. Every step could be my last in this treacherous bog. But where there's water, there's life - and where there's life, there's hope.");
            BiomeVoiceline.CharacterName = TEXT("Player_Journal");
            break;
            
        default:
            BiomeVoiceline.VoicelineText = TEXT("The savanna stretches out in all directions. Tall grasses sway in the wind, providing cover but also concealing danger. This is where my journey truly begins.");
            BiomeVoiceline.CharacterName = TEXT("Player_Journal");
            break;
    }
    
    BiomeVoiceline.EmotionalState = ENarr_EmotionalState::Cautious;
    BiomeVoiceline.Priority = ENarr_VoicelinePriority::Normal;
    
    PlayVoiceline(BiomeVoiceline);
    
    UE_LOG(LogSurvivalStory, Log, TEXT("Biome changed from %d to %d"), static_cast<int32>(OldBiome), static_cast<int32>(NewBiome));
}

void UNarr_SurvivalStorySystem::OnTimeOfDayChanged(ENarr_TimeOfDay NewTimeOfDay)
{
    FNarr_VoicelineData TimeVoiceline;
    
    switch (NewTimeOfDay)
    {
        case ENarr_TimeOfDay::Dawn:
            TimeVoiceline.VoicelineText = TEXT("Dawn breaks over the prehistoric landscape. The predators of the night retreat to their lairs, but new dangers emerge with the light. I must make the most of these precious daylight hours.");
            break;
            
        case ENarr_TimeOfDay::Day:
            TimeVoiceline.VoicelineText = TEXT("The sun climbs high overhead. Herbivores graze in the distance while I search for resources. Midday brings both opportunity and exposure - I am visible to everything that hunts.");
            break;
            
        case ENarr_TimeOfDay::Dusk:
            TimeVoiceline.VoicelineText = TEXT("Dusk approaches, painting the sky in shades of amber and crimson. The nocturnal hunters begin to stir. I need to find shelter before darkness falls completely.");
            break;
            
        case ENarr_TimeOfDay::Night:
            TimeVoiceline.VoicelineText = TEXT("Night has fallen like a predator's shadow. Every sound could signal death approaching. I huddle in my shelter, counting the hours until dawn returns.");
            break;
    }
    
    TimeVoiceline.CharacterName = TEXT("Player_Journal");
    TimeVoiceline.EmotionalState = ENarr_EmotionalState::Cautious;
    TimeVoiceline.Priority = ENarr_VoicelinePriority::Low;
    
    PlayVoiceline(TimeVoiceline);
}

void UNarr_SurvivalStorySystem::OnThreatLevelChanged(ENarr_ThreatLevel OldLevel, ENarr_ThreatLevel NewLevel)
{
    if (NewLevel > OldLevel)
    {
        // Threat increased
        FNarr_VoicelineData ThreatVoiceline;
        ThreatVoiceline.VoicelineText = TEXT("Danger approaches. My instincts scream warnings as the prehistoric wilderness reveals its true nature. Every muscle tenses, every sense sharpens. Survival depends on my next decision.");
        ThreatVoiceline.CharacterName = TEXT("Emergency_Alert");
        ThreatVoiceline.EmotionalState = ENarr_EmotionalState::Fearful;
        ThreatVoiceline.Priority = ENarr_VoicelinePriority::High;
        ThreatVoiceline.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778394091111_Emergency_Alert.mp3");
        
        PlayVoiceline(ThreatVoiceline);
    }
}

void UNarr_SurvivalStorySystem::PlayVoiceline(const FNarr_VoicelineData& VoicelineData)
{
    // Log the voiceline
    UE_LOG(LogSurvivalStory, Log, TEXT("Playing voiceline: %s - %s"), *VoicelineData.CharacterName, *VoicelineData.VoicelineText);
    
    // In a full implementation, this would:
    // 1. Load the audio file from the URL
    // 2. Create an audio component
    // 3. Play the audio with appropriate settings
    // 4. Handle positional audio if needed
    // 5. Display subtitles
    
    // For now, we'll just broadcast the event
    OnVoicelineTriggered.Broadcast(VoicelineData);
}

ENarr_BiomeType UNarr_SurvivalStorySystem::DetermineBiomeFromLocation(const FVector& Location) const
{
    // Determine biome based on world coordinates
    // Using the biome coordinates from brain memories
    
    if (Location.X >= -77500 && Location.X <= -25000 && Location.Y >= -76500 && Location.Y <= -15000)
    {
        return ENarr_BiomeType::Swamp; // Southwest
    }
    else if (Location.X >= -77500 && Location.X <= -15000 && Location.Y >= 15000 && Location.Y <= 76500)
    {
        return ENarr_BiomeType::Forest; // Northwest
    }
    else if (Location.X >= 25000 && Location.X <= 79500 && Location.Y >= -30000 && Location.Y <= 30000)
    {
        return ENarr_BiomeType::Desert; // East
    }
    else if (Location.X >= 15000 && Location.X <= 79500 && Location.Y >= 20000 && Location.Y <= 76500)
    {
        return ENarr_BiomeType::Mountain; // Northeast
    }
    else
    {
        return ENarr_BiomeType::Savana; // Center/default
    }
}

ATranspersonalCharacter* UNarr_SurvivalStorySystem::GetPlayerCharacter() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            return Cast<ATranspersonalCharacter>(PC->GetPawn());
        }
    }
    return nullptr;
}

void UNarr_SurvivalStorySystem::CheckStoryPhaseProgression()
{
    // Check if current phase requirements are met
    if (StoryPhases.Contains(CurrentStoryPhase))
    {
        const FNarr_StoryPhaseData& CurrentPhaseData = StoryPhases[CurrentStoryPhase];
        bool bAllRequirementsMet = true;
        
        for (const FString& RequiredEvent : CurrentPhaseData.RequiredEvents)
        {
            if (!CompletedEvents.Contains(RequiredEvent))
            {
                bAllRequirementsMet = false;
                break;
            }
        }
        
        if (bAllRequirementsMet)
        {
            AdvanceStoryPhase();
        }
    }
}

void UNarr_SurvivalStorySystem::AdvanceStoryPhase()
{
    ENarr_StoryPhase NextPhase = CurrentStoryPhase;
    
    switch (CurrentStoryPhase)
    {
        case ENarr_StoryPhase::Introduction:
            NextPhase = ENarr_StoryPhase::EarlyGame;
            break;
        case ENarr_StoryPhase::EarlyGame:
            NextPhase = ENarr_StoryPhase::MidGame;
            break;
        case ENarr_StoryPhase::MidGame:
            NextPhase = ENarr_StoryPhase::LateGame;
            break;
        case ENarr_StoryPhase::LateGame:
            NextPhase = ENarr_StoryPhase::Endgame;
            break;
        default:
            return; // Already at max phase
    }
    
    CurrentStoryPhase = NextPhase;
    OnStoryPhaseChanged.Broadcast(CurrentStoryPhase);
    
    UE_LOG(LogSurvivalStory, Log, TEXT("Advanced to story phase: %d"), static_cast<int32>(CurrentStoryPhase));
}

void UNarr_SurvivalStorySystem::OnDayProgressed()
{
    // Trigger daily narrative updates
    FNarr_VoicelineData DayVoiceline;
    DayVoiceline.VoicelineText = FString::Printf(TEXT("Day %d in the wilderness. Each sunrise brings new challenges and new opportunities to prove I belong in this ancient world."), DaysInWilderness);
    DayVoiceline.CharacterName = TEXT("Player_Journal");
    DayVoiceline.EmotionalState = ENarr_EmotionalState::Determined;
    DayVoiceline.Priority = ENarr_VoicelinePriority::Low;
    
    PlayVoiceline(DayVoiceline);
    
    OnDayChanged.Broadcast(DaysInWilderness);
}

void UNarr_SurvivalStorySystem::TriggerLowHealthNarrative()
{
    FNarr_VoicelineData HealthVoiceline;
    HealthVoiceline.VoicelineText = TEXT("My body screams in agony. Blood loss and exhaustion threaten to claim me. I must find healing herbs or risk becoming carrion for the scavengers.");
    HealthVoiceline.CharacterName = TEXT("Player_Journal");
    HealthVoiceline.EmotionalState = ENarr_EmotionalState::Desperate;
    HealthVoiceline.Priority = ENarr_VoicelinePriority::High;
    
    PlayVoiceline(HealthVoiceline);
}

void UNarr_SurvivalStorySystem::TriggerHighFearNarrative()
{
    FNarr_VoicelineData FearVoiceline;
    FearVoiceline.VoicelineText = TEXT("Terror grips my heart like ice. Every shadow harbors death, every sound signals doom. I must master this fear or it will master me.");
    FearVoiceline.CharacterName = TEXT("Player_Journal");
    FearVoiceline.EmotionalState = ENarr_EmotionalState::Fearful;
    FearVoiceline.Priority = ENarr_VoicelinePriority::High;
    
    PlayVoiceline(FearVoiceline);
}

void UNarr_SurvivalStorySystem::UpdateSurvivalMetrics()
{
    if (ATranspersonalCharacter* Player = GetPlayerCharacter())
    {
        // Calculate survival score based on various factors
        float HealthScore = Player->GetHealth() / 100.0f;
        float HungerScore = (100.0f - Player->GetHunger()) / 100.0f;
        float ThirstScore = (100.0f - Player->GetThirst()) / 100.0f;
        float FearScore = (100.0f - Player->GetFear()) / 100.0f;
        float DayScore = FMath::Min(DaysInWilderness / 100.0f, 1.0f);
        
        PlayerSurvivalScore = (HealthScore + HungerScore + ThirstScore + FearScore + DayScore) / 5.0f;
        PlayerSurvivalScore = FMath::Clamp(PlayerSurvivalScore, 0.0f, 1.0f);
    }
}

void UNarr_SurvivalStorySystem::TriggerStoryEvent(const FString& EventName)
{
    if (!CompletedEvents.Contains(EventName))
    {
        CompletedEvents.Add(EventName);
        MajorEventsTriggered++;
        
        OnStoryEventTriggered.Broadcast(EventName);
        
        UE_LOG(LogSurvivalStory, Log, TEXT("Story event triggered: %s"), *EventName);
    }
}