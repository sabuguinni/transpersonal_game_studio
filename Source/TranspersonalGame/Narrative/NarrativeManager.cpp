#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

UNarrativeManager::UNarrativeManager()
{
    NarrationVolume = 0.8f;
    ProximityCheckInterval = 2.0f;
    bEnableContextualNarration = true;
    NarrationAudioComponent = nullptr;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initializing..."));
    
    // Initialize narrative database
    InitializeNarrativeDatabase();
    
    // Initialize points of interest
    InitializePointsOfInterest();
    
    // Create audio component for narration
    if (GetWorld())
    {
        AActor* WorldActor = GetWorld()->GetFirstPlayerController();
        if (WorldActor)
        {
            NarrationAudioComponent = NewObject<UAudioComponent>(WorldActor);
            if (NarrationAudioComponent)
            {
                NarrationAudioComponent->SetVolumeMultiplier(NarrationVolume);
                NarrationAudioComponent->bAutoActivate = false;
                UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Audio component created"));
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initialization complete"));
}

void UNarrativeManager::Deinitialize()
{
    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ProximityCheckTimer);
    }
    
    // Clean up audio component
    if (NarrationAudioComponent)
    {
        NarrationAudioComponent->Stop();
        NarrationAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UNarrativeManager::InitializeNarrativeDatabase()
{
    NarrativeEvents.Empty();
    
    // Threat Detection Events
    FNarr_NarrativeEvent ThreatEvent;
    ThreatEvent.EventType = ENarr_EventType::ThreatDetection;
    ThreatEvent.NarratorType = ENarr_NarratorType::TacticalNarrator;
    ThreatEvent.EventText = "Sobrevivente, detectei movimento de predador na zona norte. Protocolo de segurança activado.";
    ThreatEvent.AudioAssetPath = "/Game/Audio/TacticalNarrator.mp3";
    ThreatEvent.Priority = 10.0f;
    ThreatEvent.Cooldown = 45.0f;
    ThreatEvent.TriggerDinosaur = EEng_DinosaurSpecies::TRex;
    NarrativeEvents.Add(ThreatEvent);
    
    // Research Update Events
    FNarr_NarrativeEvent ResearchEvent;
    ResearchEvent.EventType = ENarr_EventType::ResearchUpdate;
    ResearchEvent.NarratorType = ENarr_NarratorType::FieldResearcher;
    ResearchEvent.EventText = "Registo de campo, dia 203. Observei comportamento de caça coordenada entre três Velociraptors.";
    ResearchEvent.AudioAssetPath = "/Game/Audio/FieldResearcher.mp3";
    ResearchEvent.Priority = 7.0f;
    ResearchEvent.Cooldown = 60.0f;
    ResearchEvent.TriggerDinosaur = EEng_DinosaurSpecies::Raptor;
    NarrativeEvents.Add(ResearchEvent);
    
    // Safety Warning Events
    FNarr_NarrativeEvent SafetyEvent;
    SafetyEvent.EventType = ENarr_EventType::SafetyWarning;
    SafetyEvent.NarratorType = ENarr_NarratorType::SafetyGuide;
    SafetyEvent.EventText = "Atenção, paleontólogo. O Brachiosaurus à sua frente está em estado de alerta.";
    SafetyEvent.AudioAssetPath = "/Game/Audio/SafetyGuide.mp3";
    SafetyEvent.Priority = 8.0f;
    SafetyEvent.Cooldown = 30.0f;
    SafetyEvent.TriggerDinosaur = EEng_DinosaurSpecies::Brachiosaurus;
    NarrativeEvents.Add(SafetyEvent);
    
    // Fossil Discovery Events
    FNarr_NarrativeEvent FossilEvent;
    FossilEvent.EventType = ENarr_EventType::FossilDiscovery;
    FossilEvent.NarratorType = ENarr_NarratorType::PaleontologyNarrator;
    FossilEvent.EventText = "Descoberta extraordinária! Este fóssil de Triceratops mostra marcas de combate.";
    FossilEvent.AudioAssetPath = "/Game/Audio/PaleontologyNarrator.mp3";
    FossilEvent.Priority = 9.0f;
    FossilEvent.Cooldown = 90.0f;
    FossilEvent.TriggerDinosaur = EEng_DinosaurSpecies::Triceratops;
    NarrativeEvents.Add(FossilEvent);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Loaded %d narrative events"), NarrativeEvents.Num());
}

void UNarrativeManager::InitializePointsOfInterest()
{
    PointsOfInterest.Empty();
    
    // Fossil Discovery Site (Savanna)
    FNarr_PointOfInterest FossilSite;
    FossilSite.Name = "FossilDiscoverySite";
    FossilSite.Location = FVector(0, 0, 100);
    FossilSite.Biome = EEng_BiomeType::Savanna;
    FossilSite.InteractionRadius = 1500.0f;
    FossilSite.bIsDiscovered = false;
    PointsOfInterest.Add(FossilSite);
    
    // Ancient Boneyard (Savanna)
    FNarr_PointOfInterest Boneyard;
    Boneyard.Name = "AncientBoneyard";
    Boneyard.Location = FVector(2000, -1500, 80);
    Boneyard.Biome = EEng_BiomeType::Savanna;
    Boneyard.InteractionRadius = 2000.0f;
    Boneyard.bIsDiscovered = false;
    PointsOfInterest.Add(Boneyard);
    
    // Raptor Hunting Grounds (Forest)
    FNarr_PointOfInterest RaptorGrounds;
    RaptorGrounds.Name = "RaptorHuntingGrounds";
    RaptorGrounds.Location = FVector(-45000, 40000, 200);
    RaptorGrounds.Biome = EEng_BiomeType::Forest;
    RaptorGrounds.InteractionRadius = 3000.0f;
    RaptorGrounds.bIsDiscovered = false;
    PointsOfInterest.Add(RaptorGrounds);
    
    // T-Rex Territory (Swamp)
    FNarr_PointOfInterest TRexTerritory;
    TRexTerritory.Name = "TRexTerritory";
    TRexTerritory.Location = FVector(-50000, -45000, 50);
    TRexTerritory.Biome = EEng_BiomeType::Swamp;
    TRexTerritory.InteractionRadius = 5000.0f;
    TRexTerritory.bIsDiscovered = false;
    PointsOfInterest.Add(TRexTerritory);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initialized %d points of interest"), PointsOfInterest.Num());
}

void UNarrativeManager::TriggerNarrativeEvent(ENarr_EventType EventType, EEng_DinosaurSpecies DinosaurSpecies)
{
    if (!bEnableContextualNarration)
    {
        return;
    }
    
    if (!CanTriggerEvent(EventType))
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Event %d is on cooldown"), (int32)EventType);
        return;
    }
    
    // Find appropriate narrative event
    FNarr_NarrativeEvent* EventToPlay = nullptr;
    for (FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.EventType == EventType && Event.TriggerDinosaur == DinosaurSpecies)
        {
            EventToPlay = &Event;
            break;
        }
    }
    
    // Fallback to any event of the same type
    if (!EventToPlay)
    {
        for (FNarr_NarrativeEvent& Event : NarrativeEvents)
        {
            if (Event.EventType == EventType)
            {
                EventToPlay = &Event;
                break;
            }
        }
    }
    
    if (EventToPlay)
    {
        PlayNarration(*EventToPlay);
        UpdateEventCooldown(EventType, EventToPlay->Cooldown);
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Triggered event %d for dinosaur %d"), (int32)EventType, (int32)DinosaurSpecies);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: No suitable event found for type %d"), (int32)EventType);
    }
}

void UNarrativeManager::PlayNarration(const FNarr_NarrativeEvent& Event)
{
    if (!NarrationAudioComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("NarrativeManager: No audio component available"));
        return;
    }
    
    // Stop current narration if playing
    if (NarrationAudioComponent->IsPlaying())
    {
        NarrationAudioComponent->Stop();
    }
    
    // Try to load audio asset
    if (!Event.AudioAssetPath.IsEmpty())
    {
        USoundBase* AudioAsset = LoadObject<USoundBase>(nullptr, *Event.AudioAssetPath);
        if (AudioAsset)
        {
            NarrationAudioComponent->SetSound(AudioAsset);
            NarrationAudioComponent->Play();
            UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Playing narration: %s"), *Event.EventText);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("NarrativeManager: Failed to load audio: %s"), *Event.AudioAssetPath);
        }
    }
    
    // Display text (for debugging/subtitles)
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, 
            FString::Printf(TEXT("[%s] %s"), 
                *UEnum::GetValueAsString(Event.NarratorType), 
                *Event.EventText));
    }
}

void UNarrativeManager::RegisterPointOfInterest(const FNarr_PointOfInterest& POI)
{
    PointsOfInterest.Add(POI);
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Registered POI: %s"), *POI.Name);
}

void UNarrativeManager::CheckProximityToPoints(const FVector& PlayerLocation)
{
    for (FNarr_PointOfInterest& POI : PointsOfInterest)
    {
        if (!POI.bIsDiscovered)
        {
            float Distance = FVector::Dist(PlayerLocation, POI.Location);
            if (Distance <= POI.InteractionRadius)
            {
                POI.bIsDiscovered = true;
                
                // Trigger discovery event based on POI type
                if (POI.Name.Contains("Fossil") || POI.Name.Contains("Bone"))
                {
                    TriggerNarrativeEvent(ENarr_EventType::FossilDiscovery, EEng_DinosaurSpecies::Triceratops);
                }
                else if (POI.Name.Contains("Raptor"))
                {
                    TriggerNarrativeEvent(ENarr_EventType::ThreatDetection, EEng_DinosaurSpecies::Raptor);
                }
                else if (POI.Name.Contains("TRex"))
                {
                    TriggerNarrativeEvent(ENarr_EventType::ThreatDetection, EEng_DinosaurSpecies::TRex);
                }
                
                UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Discovered POI: %s"), *POI.Name);
            }
        }
    }
}

void UNarrativeManager::OnBiomeTransition(EEng_BiomeType FromBiome, EEng_BiomeType ToBiome)
{
    TriggerNarrativeEvent(ENarr_EventType::BiomeTransition);
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Biome transition from %d to %d"), (int32)FromBiome, (int32)ToBiome);
}

void UNarrativeManager::OnDinosaurEncounter(EEng_DinosaurSpecies Species, float Distance, bool bIsHostile)
{
    if (bIsHostile || Distance < 2000.0f)
    {
        TriggerNarrativeEvent(ENarr_EventType::ThreatDetection, Species);
    }
    else
    {
        TriggerNarrativeEvent(ENarr_EventType::DinosaurSighting, Species);
    }
}

void UNarrativeManager::OnFossilDiscovery(EEng_DinosaurSpecies Species, const FVector& Location)
{
    TriggerNarrativeEvent(ENarr_EventType::FossilDiscovery, Species);
}

void UNarrativeManager::LoadNarrativeEvents()
{
    InitializeNarrativeDatabase();
}

void UNarrativeManager::SetNarrationVolume(float Volume)
{
    NarrationVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (NarrationAudioComponent)
    {
        NarrationAudioComponent->SetVolumeMultiplier(NarrationVolume);
    }
}

bool UNarrativeManager::IsNarrationPlaying() const
{
    return NarrationAudioComponent && NarrationAudioComponent->IsPlaying();
}

bool UNarrativeManager::CanTriggerEvent(ENarr_EventType EventType) const
{
    if (const float* CooldownTime = EventCooldowns.Find(EventType))
    {
        return GetWorld()->GetTimeSeconds() >= *CooldownTime;
    }
    return true;
}

void UNarrativeManager::UpdateEventCooldown(ENarr_EventType EventType, float CooldownTime)
{
    if (GetWorld())
    {
        EventCooldowns.Add(EventType, GetWorld()->GetTimeSeconds() + CooldownTime);
    }
}

FNarr_NarrativeEvent* UNarrativeManager::FindEventByType(ENarr_EventType EventType, ENarr_NarratorType PreferredNarrator)
{
    // First try to find event with preferred narrator
    for (FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.EventType == EventType && Event.NarratorType == PreferredNarrator)
        {
            return &Event;
        }
    }
    
    // Fallback to any event of the same type
    for (FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.EventType == EventType)
        {
            return &Event;
        }
    }
    
    return nullptr;
}