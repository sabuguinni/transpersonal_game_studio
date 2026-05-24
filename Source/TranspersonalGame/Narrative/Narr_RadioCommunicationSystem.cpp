#include "Narr_RadioCommunicationSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UNarr_RadioCommunicationSystem::UNarr_RadioCommunicationSystem()
{
    DefaultTransmissionRange = 5000.0f;
    MessagePlaybackDelay = 2.0f;
    bRadioSystemEnabled = true;
    RadioAudioComponent = nullptr;
}

void UNarr_RadioCommunicationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Radio Communication System initialized"));
    
    // Initialize default expedition stations
    InitializeDefaultStations();
    
    // Clear active messages
    ActiveMessages.Empty();
}

void UNarr_RadioCommunicationSystem::Deinitialize()
{
    if (RadioAudioComponent && IsValid(RadioAudioComponent))
    {
        RadioAudioComponent->Stop();
        RadioAudioComponent = nullptr;
    }
    
    RadioStations.Empty();
    ActiveMessages.Empty();
    
    Super::Deinitialize();
}

void UNarr_RadioCommunicationSystem::BroadcastMessage(const FNarr_RadioMessage& Message, const FString& StationCallsign)
{
    if (!bRadioSystemEnabled)
    {
        return;
    }
    
    // Check if station exists and is active
    if (!RadioStations.Contains(StationCallsign))
    {
        UE_LOG(LogTemp, Warning, TEXT("Radio station not found: %s"), *StationCallsign);
        return;
    }
    
    FNarr_RadioStation& Station = RadioStations[StationCallsign];
    if (!Station.bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Radio station inactive: %s"), *StationCallsign);
        return;
    }
    
    // Check if player is in range
    if (!IsPlayerInRange(StationCallsign))
    {
        UE_LOG(LogTemp, Log, TEXT("Player out of range for station: %s"), *StationCallsign);
        return;
    }
    
    // Add to active messages
    ActiveMessages.Add(Message);
    
    // Play immediately if emergency
    if (Message.bIsEmergency)
    {
        PlayRadioMessage(Message);
    }
    else
    {
        // Queue for normal playback
        QueueMessage(Message, StationCallsign);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Broadcasting message from %s: %s"), *Message.SenderName, *Message.MessageContent);
}

void UNarr_RadioCommunicationSystem::BroadcastEmergencyAlert(const FString& SenderName, const FString& AlertMessage, const FString& AudioURL)
{
    FNarr_RadioMessage EmergencyMessage;
    EmergencyMessage.SenderName = SenderName;
    EmergencyMessage.MessageContent = AlertMessage;
    EmergencyMessage.AudioURL = AudioURL;
    EmergencyMessage.Priority = ENarr_MessagePriority::Critical;
    EmergencyMessage.Category = ENarr_MessageCategory::Emergency;
    EmergencyMessage.bIsEmergency = true;
    EmergencyMessage.MessageDuration = 35.0f;
    
    // Broadcast from all active stations
    for (auto& StationPair : RadioStations)
    {
        if (StationPair.Value.bIsActive)
        {
            BroadcastMessage(EmergencyMessage, StationPair.Key);
        }
    }
}

void UNarr_RadioCommunicationSystem::QueueMessage(const FNarr_RadioMessage& Message, const FString& StationCallsign)
{
    if (RadioStations.Contains(StationCallsign))
    {
        RadioStations[StationCallsign].MessageQueue.Add(Message);
        UE_LOG(LogTemp, Log, TEXT("Message queued for station %s"), *StationCallsign);
    }
}

bool UNarr_RadioCommunicationSystem::IsPlayerInRange(const FString& StationCallsign) const
{
    if (!RadioStations.Contains(StationCallsign))
    {
        return false;
    }
    
    const FNarr_RadioStation& Station = RadioStations[StationCallsign];
    FVector PlayerLocation = GetPlayerLocation();
    
    if (PlayerLocation == FVector::ZeroVector)
    {
        return false;
    }
    
    float Distance = CalculateDistance(PlayerLocation, Station.StationLocation);
    return Distance <= Station.TransmissionRange;
}

void UNarr_RadioCommunicationSystem::RegisterRadioStation(const FNarr_RadioStation& Station)
{
    RadioStations.Add(Station.StationCallsign, Station);
    UE_LOG(LogTemp, Log, TEXT("Radio station registered: %s at %s"), *Station.StationName, *Station.StationLocation.ToString());
}

void UNarr_RadioCommunicationSystem::SetStationActive(const FString& StationCallsign, bool bActive)
{
    if (RadioStations.Contains(StationCallsign))
    {
        RadioStations[StationCallsign].bIsActive = bActive;
        UE_LOG(LogTemp, Log, TEXT("Station %s set to %s"), *StationCallsign, bActive ? TEXT("active") : TEXT("inactive"));
    }
}

FNarr_RadioStation UNarr_RadioCommunicationSystem::GetStation(const FString& StationCallsign) const
{
    if (RadioStations.Contains(StationCallsign))
    {
        return RadioStations[StationCallsign];
    }
    return FNarr_RadioStation();
}

void UNarr_RadioCommunicationSystem::PlayRadioMessage(const FNarr_RadioMessage& Message)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Stop current transmission
    StopCurrentTransmission();
    
    // Create audio component if needed
    if (!RadioAudioComponent)
    {
        AActor* WorldActor = UGameplayStatics::GetPlayerController(World, 0);
        if (WorldActor)
        {
            RadioAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
        }
    }
    
    // Log the message playback
    UE_LOG(LogTemp, Log, TEXT("Playing radio message: %s - %s"), *Message.SenderName, *Message.MessageContent);
    
    // In a real implementation, we would load and play the audio from AudioURL
    // For now, we'll just log the message details
    if (!Message.AudioURL.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Audio URL: %s"), *Message.AudioURL);
    }
}

void UNarr_RadioCommunicationSystem::StopCurrentTransmission()
{
    if (RadioAudioComponent && IsValid(RadioAudioComponent))
    {
        RadioAudioComponent->Stop();
    }
}

TArray<FNarr_RadioMessage> UNarr_RadioCommunicationSystem::GetMessagesInRange() const
{
    TArray<FNarr_RadioMessage> MessagesInRange;
    
    for (const auto& StationPair : RadioStations)
    {
        if (StationPair.Value.bIsActive && IsPlayerInRange(StationPair.Key))
        {
            MessagesInRange.Append(StationPair.Value.MessageQueue);
        }
    }
    
    return MessagesInRange;
}

TArray<FNarr_RadioMessage> UNarr_RadioCommunicationSystem::GetEmergencyMessages() const
{
    TArray<FNarr_RadioMessage> EmergencyMessages;
    
    for (const FNarr_RadioMessage& Message : ActiveMessages)
    {
        if (Message.bIsEmergency)
        {
            EmergencyMessages.Add(Message);
        }
    }
    
    return EmergencyMessages;
}

void UNarr_RadioCommunicationSystem::ProcessMessageQueue()
{
    for (auto& StationPair : RadioStations)
    {
        FNarr_RadioStation& Station = StationPair.Value;
        
        if (!Station.bIsActive || Station.MessageQueue.Num() == 0)
        {
            continue;
        }
        
        if (IsPlayerInRange(StationPair.Key))
        {
            // Process next message in queue
            FNarr_RadioMessage NextMessage = Station.MessageQueue[0];
            Station.MessageQueue.RemoveAt(0);
            
            PlayRadioMessage(NextMessage);
        }
    }
}

void UNarr_RadioCommunicationSystem::UpdateActiveMessages()
{
    // Remove expired messages
    ActiveMessages.RemoveAll([](const FNarr_RadioMessage& Message)
    {
        // In a real implementation, we would track message timestamps
        // For now, we'll keep all messages
        return false;
    });
}

FVector UNarr_RadioCommunicationSystem::GetPlayerLocation() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FVector::ZeroVector;
    }
    
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
    if (!PlayerController)
    {
        return FVector::ZeroVector;
    }
    
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        return FVector::ZeroVector;
    }
    
    return PlayerPawn->GetActorLocation();
}

float UNarr_RadioCommunicationSystem::CalculateDistance(const FVector& Location1, const FVector& Location2) const
{
    return FVector::Dist(Location1, Location2);
}

void UNarr_RadioCommunicationSystem::InitializeDefaultStations()
{
    // Northern Ridge Research Station
    FNarr_RadioStation NorthernRidge;
    NorthernRidge.StationName = TEXT("Northern Ridge Research Station");
    NorthernRidge.StationCallsign = TEXT("NRRS");
    NorthernRidge.StationLocation = FVector(2000.0f, 8000.0f, 500.0f);
    NorthernRidge.TransmissionRange = 6000.0f;
    NorthernRidge.bIsActive = true;
    RegisterRadioStation(NorthernRidge);
    
    // Eastern Valley Expedition Base
    FNarr_RadioStation EasternValley;
    EasternValley.StationName = TEXT("Eastern Valley Expedition Base");
    EasternValley.StationCallsign = TEXT("EVEB");
    EasternValley.StationLocation = FVector(8000.0f, 2000.0f, 200.0f);
    EasternValley.TransmissionRange = 5500.0f;
    EasternValley.bIsActive = true;
    RegisterRadioStation(EasternValley);
    
    // Southern Valley Research Outpost
    FNarr_RadioStation SouthernValley;
    SouthernValley.StationName = TEXT("Southern Valley Research Outpost");
    SouthernValley.StationCallsign = TEXT("SVRO");
    SouthernValley.StationLocation = FVector(3000.0f, -6000.0f, 150.0f);
    SouthernValley.TransmissionRange = 5000.0f;
    SouthernValley.bIsActive = true;
    RegisterRadioStation(SouthernValley);
    
    // Western Cliffs Security Station
    FNarr_RadioStation WesternCliffs;
    WesternCliffs.StationName = TEXT("Western Cliffs Security Station");
    WesternCliffs.StationCallsign = TEXT("WCSS");
    WesternCliffs.StationLocation = FVector(-5000.0f, 1000.0f, 800.0f);
    WesternCliffs.TransmissionRange = 7000.0f;
    WesternCliffs.bIsActive = true;
    RegisterRadioStation(WesternCliffs);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d default radio stations"), RadioStations.Num());
}