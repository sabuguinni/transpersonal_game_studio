#include "NarrativeManager.h"
#include "NarrativeTrigger.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

UNarrativeManager::UNarrativeManager()
{
    CurrentBiome = EEng_BiomeType::Forest;
    CurrentWeather = EEng_WeatherType::Clear;
    CurrentThreatLevel = EEng_ThreatLevel::Safe;

    // Initialize audio URLs from generated TTS samples
    PaleontologistFieldNotesURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778073748639_PaleontologistFieldNotes.mp3";
    EmergencyAlertURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778073761060_EmergencyAlert.mp3";
    DiscoveryNarrationURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778073771201_DiscoveryNarration.mp3";
    WeatherWarningURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778073780906_WeatherWarning.mp3";
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeNarrativeLibrary();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager initialized with %d narrative types"), NarrativeLibrary.Num());
}

void UNarrativeManager::Deinitialize()
{
    NarrativeTriggers.Empty();
    NarrativeLibrary.Empty();
    
    Super::Deinitialize();
}

void UNarrativeManager::InitializeNarrativeLibrary()
{
    // Discovery narratives
    TArray<FNarr_NarrativeData> DiscoveryNarratives;
    DiscoveryNarratives.Add(CreateNarrativeData(
        ENarr_NarrativeEvent::Discovery,
        "Descoberta extraordinária! Encontrei um ninho de Triceratops abandonado com três ovos intactos.",
        DiscoveryNarrationURL,
        26.0f
    ));
    DiscoveryNarratives.Add(CreateNarrativeData(
        ENarr_NarrativeEvent::Discovery,
        "Registo de campo: Observei três Raptors a desenvolver estratégias de caça coordenada.",
        PaleontologistFieldNotesURL,
        26.0f
    ));
    NarrativeLibrary.Add(ENarr_NarrativeEvent::Discovery, DiscoveryNarratives);

    // Danger narratives
    TArray<FNarr_NarrativeData> DangerNarratives;
    DangerNarratives.Add(CreateNarrativeData(
        ENarr_NarrativeEvent::Danger,
        "Alerta de emergência! Detectei pegadas frescas de Tyrannosaurus Rex a menos de 500 metros!",
        EmergencyAlertURL,
        25.0f,
        true
    ));
    NarrativeLibrary.Add(ENarr_NarrativeEvent::Danger, DangerNarratives);

    // Weather narratives
    TArray<FNarr_NarrativeData> WeatherNarratives;
    WeatherNarratives.Add(CreateNarrativeData(
        ENarr_NarrativeEvent::Weather,
        "Atenção sobrevivente! Tempestade tropical severa aproxima-se. Procurai abrigo imediatamente!",
        WeatherWarningURL,
        26.0f,
        true
    ));
    NarrativeLibrary.Add(ENarr_NarrativeEvent::Weather, WeatherNarratives);

    // Dinosaur encounter narratives
    TArray<FNarr_NarrativeData> DinosaurNarratives;
    DinosaurNarratives.Add(CreateNarrativeData(
        ENarr_NarrativeEvent::DinosaurEncounter,
        "Movimento detectado! Um grande predador aproxima-se da vossa posição.",
        "",
        8.0f,
        true
    ));
    DinosaurNarratives.Add(CreateNarrativeData(
        ENarr_NarrativeEvent::DinosaurEncounter,
        "Herbívoro pacífico avistado. Mantende distância respeitosa para observação.",
        "",
        6.0f
    ));
    NarrativeLibrary.Add(ENarr_NarrativeEvent::DinosaurEncounter, DinosaurNarratives);
}

FNarr_NarrativeData UNarrativeManager::CreateNarrativeData(ENarr_NarrativeEvent EventType, const FString& Text, const FString& AudioURL, float Duration, bool bUrgent)
{
    FNarr_NarrativeData Data;
    Data.EventType = EventType;
    Data.NarrativeText = Text;
    Data.AudioPath = AudioURL;
    Data.Duration = Duration;
    Data.bIsUrgent = bUrgent;
    return Data;
}

void UNarrativeManager::TriggerNarrativeEvent(ENarr_NarrativeEvent EventType, const FString& Context)
{
    FNarr_NarrativeData NarrativeData = GetContextualNarrative(EventType);
    
    if (!Context.IsEmpty())
    {
        NarrativeData.NarrativeText = FString::Printf(TEXT("%s %s"), *NarrativeData.NarrativeText, *Context);
    }
    
    PlayNarration(NarrativeData);
    LogNarrativeEvent(NarrativeData);
}

void UNarrativeManager::PlayNarration(const FNarr_NarrativeData& NarrativeData)
{
    // Broadcast to UI and audio systems
    OnNarrativeEvent.Broadcast(NarrativeData);
    
    // Log for debugging
    UE_LOG(LogTemp, Log, TEXT("Playing narration: %s"), *NarrativeData.NarrativeText);
    
    // TODO: Integrate with audio system to play actual audio files
    if (!NarrativeData.AudioPath.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Audio URL: %s"), *NarrativeData.AudioPath);
    }
}

void UNarrativeManager::RegisterNarrativeTrigger(ANarr_NarrativeTrigger* Trigger)
{
    if (Trigger && !NarrativeTriggers.Contains(Trigger))
    {
        NarrativeTriggers.Add(Trigger);
        UE_LOG(LogTemp, Log, TEXT("Registered narrative trigger: %s"), *Trigger->GetName());
    }
}

void UNarrativeManager::UnregisterNarrativeTrigger(ANarr_NarrativeTrigger* Trigger)
{
    if (Trigger)
    {
        NarrativeTriggers.Remove(Trigger);
        UE_LOG(LogTemp, Log, TEXT("Unregistered narrative trigger: %s"), *Trigger->GetName());
    }
}

void UNarrativeManager::UpdateNarrativeContext(EEng_BiomeType CurrentBiome, EEng_WeatherType Weather, EEng_ThreatLevel ThreatLevel)
{
    this->CurrentBiome = CurrentBiome;
    this->CurrentWeather = Weather;
    this->CurrentThreatLevel = ThreatLevel;
    
    UE_LOG(LogTemp, Log, TEXT("Narrative context updated - Biome: %d, Weather: %d, Threat: %d"), 
           (int32)CurrentBiome, (int32)Weather, (int32)ThreatLevel);
}

FNarr_NarrativeData UNarrativeManager::GetContextualNarrative(ENarr_NarrativeEvent EventType)
{
    if (NarrativeLibrary.Contains(EventType))
    {
        const TArray<FNarr_NarrativeData>& Narratives = NarrativeLibrary[EventType];
        if (Narratives.Num() > 0)
        {
            // Select narrative based on context or randomly
            int32 Index = FMath::RandRange(0, Narratives.Num() - 1);
            return Narratives[Index];
        }
    }
    
    // Fallback narrative
    FNarr_NarrativeData Fallback;
    Fallback.EventType = EventType;
    Fallback.NarrativeText = "Algo interessante aconteceu nesta região pré-histórica.";
    Fallback.Duration = 3.0f;
    return Fallback;
}

void UNarrativeManager::TriggerDinosaurEncounter(EEng_DinosaurSpecies Species, float Distance, bool bIsHostile)
{
    FString SpeciesName;
    switch (Species)
    {
        case EEng_DinosaurSpecies::TRex:
            SpeciesName = "Tyrannosaurus Rex";
            break;
        case EEng_DinosaurSpecies::Raptor:
            SpeciesName = "Velociraptor";
            break;
        case EEng_DinosaurSpecies::Triceratops:
            SpeciesName = "Triceratops";
            break;
        case EEng_DinosaurSpecies::Brachiosaurus:
            SpeciesName = "Brachiosaurus";
            break;
        default:
            SpeciesName = "Dinossauro desconhecido";
            break;
    }
    
    FString Context = FString::Printf(TEXT("%s detectado a %.0f metros. %s"), 
                                     *SpeciesName, 
                                     Distance,
                                     bIsHostile ? TEXT("Comportamento hostil!") : TEXT("Comportamento neutro."));
    
    TriggerNarrativeEvent(ENarr_NarrativeEvent::DinosaurEncounter, Context);
}

void UNarrativeManager::TriggerWeatherWarning(EEng_WeatherType IncomingWeather, float TimeToArrival)
{
    FString WeatherName;
    switch (IncomingWeather)
    {
        case EEng_WeatherType::Storm:
            WeatherName = "tempestade";
            break;
        case EEng_WeatherType::Rain:
            WeatherName = "chuva intensa";
            break;
        case EEng_WeatherType::Snow:
            WeatherName = "nevasca";
            break;
        default:
            WeatherName = "mudança meteorológica";
            break;
    }
    
    FString Context = FString::Printf(TEXT("Aproxima-se %s em %.1f minutos."), *WeatherName, TimeToArrival);
    TriggerNarrativeEvent(ENarr_NarrativeEvent::Weather, Context);
}

void UNarrativeManager::LogNarrativeEvent(const FNarr_NarrativeData& Data)
{
    FString UrgencyText = Data.bIsUrgent ? TEXT("[URGENT]") : TEXT("[INFO]");
    UE_LOG(LogTemp, Warning, TEXT("%s Narrative Event: %s"), *UrgencyText, *Data.NarrativeText);
}