#include "Narr_TribalCommunicationSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UNarr_TribalCommunicationSystem::UNarr_TribalCommunicationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    MaxCommunicationRange = 2000.0f;
    GestureDetectionRange = 500.0f;
    VocalSoundRange = 1500.0f;
    bCanSendMessages = true;
    bCanReceiveMessages = true;
    MessageCooldownTime = 2.0f;
    LastMessageTime = 0.0f;
}

void UNarr_TribalCommunicationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    PendingMessages.Empty();
    ReceivedMessages.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Tribal Communication System initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNarr_TribalCommunicationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ProcessMessageQueue();
    ProcessIncomingSignals();
    UpdateCommunicationRange();
}

void UNarr_TribalCommunicationSystem::SendTribalMessage(const FNarr_TribalMessage& Message)
{
    if (!bCanSendMessages || !ValidateMessage(Message))
    {
        return;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastMessageTime < MessageCooldownTime)
    {
        return;
    }
    
    // Add to pending messages for processing
    PendingMessages.Add(Message);
    LastMessageTime = CurrentTime;
    
    // Trigger visual/audio feedback
    TriggerCommunicationAnimation(Message.MessageType);
    PlayCommunicationSound(Message.MessageType, Message.UrgencyLevel);
    
    UE_LOG(LogTemp, Log, TEXT("Tribal message sent: %s (Type: %d)"), 
           *Message.MessageContent, (int32)Message.MessageType);
}

void UNarr_TribalCommunicationSystem::ReceiveTribalMessage(const FNarr_TribalMessage& Message)
{
    if (!bCanReceiveMessages)
    {
        return;
    }
    
    // Check if we can hear this message
    if (!CanHearCommunication(Message.TargetLocation, Message.CommunicationRange))
    {
        return;
    }
    
    ReceivedMessages.Add(Message);
    
    // Auto-acknowledge urgent messages
    if (Message.UrgencyLevel > 0.8f && Message.bRequiresResponse)
    {
        AcknowledgeMessage(Message);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal message received: %s"), *Message.MessageContent);
}

void UNarr_TribalCommunicationSystem::BroadcastEmergencySignal(ENarr_CommunicationType SignalType, const FVector& DangerLocation)
{
    FNarr_TribalMessage EmergencyMessage;
    EmergencyMessage.MessageType = SignalType;
    EmergencyMessage.TargetLocation = DangerLocation;
    EmergencyMessage.UrgencyLevel = 1.0f;
    EmergencyMessage.CommunicationRange = MaxCommunicationRange;
    EmergencyMessage.bRequiresResponse = true;
    
    switch (SignalType)
    {
        case ENarr_CommunicationType::Danger:
            EmergencyMessage.MessageContent = TEXT("DANGER! Predator spotted!");
            break;
        case ENarr_CommunicationType::Fire:
            EmergencyMessage.MessageContent = TEXT("FIRE! Evacuate immediately!");
            break;
        default:
            EmergencyMessage.MessageContent = TEXT("Emergency signal!");
            break;
    }
    
    SendTribalMessage(EmergencyMessage);
}

void UNarr_TribalCommunicationSystem::SendHuntingCoordination(const FVector& PreyLocation, float PreySize)
{
    FNarr_TribalMessage HuntMessage;
    HuntMessage.MessageType = ENarr_CommunicationType::Hunting;
    HuntMessage.TargetLocation = PreyLocation;
    HuntMessage.UrgencyLevel = 0.7f;
    HuntMessage.CommunicationRange = VocalSoundRange;
    HuntMessage.bRequiresResponse = true;
    
    if (PreySize > 1000.0f)
    {
        HuntMessage.MessageContent = TEXT("Large prey spotted - coordinate attack!");
    }
    else
    {
        HuntMessage.MessageContent = TEXT("Small prey - quick strike needed!");
    }
    
    SendTribalMessage(HuntMessage);
}

void UNarr_TribalCommunicationSystem::CallForGathering(const FVector& ResourceLocation, EResourceType ResourceType)
{
    FNarr_TribalMessage GatherMessage;
    GatherMessage.MessageType = ENarr_CommunicationType::Gathering;
    GatherMessage.TargetLocation = ResourceLocation;
    GatherMessage.UrgencyLevel = 0.5f;
    GatherMessage.CommunicationRange = GestureDetectionRange;
    GatherMessage.bRequiresResponse = false;
    
    switch (ResourceType)
    {
        case EResourceType::Food:
            GatherMessage.MessageContent = TEXT("Food source found - gather here!");
            break;
        case EResourceType::Water:
            GatherMessage.MessageContent = TEXT("Fresh water discovered!");
            GatherMessage.MessageType = ENarr_CommunicationType::Water;
            break;
        case EResourceType::Stone:
            GatherMessage.MessageContent = TEXT("Good stones for tools!");
            break;
        default:
            GatherMessage.MessageContent = TEXT("Resources available!");
            break;
    }
    
    SendTribalMessage(GatherMessage);
}

void UNarr_TribalCommunicationSystem::AcknowledgeMessage(const FNarr_TribalMessage& OriginalMessage)
{
    FNarr_TribalMessage AckMessage;
    AckMessage.MessageType = ENarr_CommunicationType::Gesture;
    AckMessage.MessageContent = TEXT("Message acknowledged");
    AckMessage.TargetLocation = OriginalMessage.TargetLocation;
    AckMessage.UrgencyLevel = 0.3f;
    AckMessage.CommunicationRange = GestureDetectionRange;
    AckMessage.bRequiresResponse = false;
    
    SendTribalMessage(AckMessage);
}

bool UNarr_TribalCommunicationSystem::CanHearCommunication(const FVector& SourceLocation, float MessageRange) const
{
    if (!GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), SourceLocation);
    return Distance <= MessageRange;
}

void UNarr_TribalCommunicationSystem::ProcessIncomingSignals()
{
    // Process received messages for appropriate responses
    for (int32 i = ReceivedMessages.Num() - 1; i >= 0; i--)
    {
        const FNarr_TribalMessage& Message = ReceivedMessages[i];
        
        // Remove old messages (older than 10 seconds)
        float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        if (CurrentTime - LastMessageTime > 10.0f)
        {
            ReceivedMessages.RemoveAt(i);
            continue;
        }
        
        // Process urgent messages immediately
        if (Message.UrgencyLevel > 0.8f)
        {
            switch (Message.MessageType)
            {
                case ENarr_CommunicationType::Danger:
                    // Trigger flee behavior or defensive stance
                    UE_LOG(LogTemp, Warning, TEXT("Responding to danger signal!"));
                    break;
                case ENarr_CommunicationType::Fire:
                    // Trigger evacuation behavior
                    UE_LOG(LogTemp, Warning, TEXT("Responding to fire signal!"));
                    break;
                default:
                    break;
            }
        }
    }
}

void UNarr_TribalCommunicationSystem::PerformGesture(ENarr_CommunicationType GestureType, const FVector& TargetDirection)
{
    FNarr_TribalMessage GestureMessage;
    GestureMessage.MessageType = GestureType;
    GestureMessage.TargetLocation = GetOwner()->GetActorLocation() + TargetDirection;
    GestureMessage.UrgencyLevel = 0.4f;
    GestureMessage.CommunicationRange = GestureDetectionRange;
    GestureMessage.MessageContent = TEXT("Gesture performed");
    
    SendTribalMessage(GestureMessage);
    TriggerCommunicationAnimation(GestureType);
}

void UNarr_TribalCommunicationSystem::MakeVocalSound(ENarr_CommunicationType SoundType, float Intensity)
{
    FNarr_TribalMessage VocalMessage;
    VocalMessage.MessageType = SoundType;
    VocalMessage.TargetLocation = GetOwner()->GetActorLocation();
    VocalMessage.UrgencyLevel = Intensity;
    VocalMessage.CommunicationRange = VocalSoundRange * Intensity;
    VocalMessage.MessageContent = TEXT("Vocal sound made");
    
    SendTribalMessage(VocalMessage);
    PlayCommunicationSound(SoundType, Intensity);
}

void UNarr_TribalCommunicationSystem::UpdateCommunicationRange()
{
    // Adjust communication range based on environmental factors
    // Could be affected by weather, terrain, etc.
}

void UNarr_TribalCommunicationSystem::ProcessMessageQueue()
{
    // Process pending outgoing messages
    for (int32 i = PendingMessages.Num() - 1; i >= 0; i--)
    {
        const FNarr_TribalMessage& Message = PendingMessages[i];
        
        // Broadcast to nearby tribal communication systems
        // This would typically involve finding other actors with this component
        // and calling their ReceiveTribalMessage function
        
        // For now, just log and remove
        UE_LOG(LogTemp, Log, TEXT("Processing tribal message: %s"), *Message.MessageContent);
        PendingMessages.RemoveAt(i);
    }
}

bool UNarr_TribalCommunicationSystem::ValidateMessage(const FNarr_TribalMessage& Message) const
{
    // Basic validation
    if (Message.MessageContent.IsEmpty())
    {
        return false;
    }
    
    if (Message.CommunicationRange <= 0.0f)
    {
        return false;
    }
    
    if (Message.UrgencyLevel < 0.0f || Message.UrgencyLevel > 1.0f)
    {
        return false;
    }
    
    return true;
}

void UNarr_TribalCommunicationSystem::TriggerCommunicationAnimation(ENarr_CommunicationType Type)
{
    // Trigger appropriate animation based on communication type
    // This would interface with the animation system
    UE_LOG(LogTemp, Log, TEXT("Triggering communication animation for type: %d"), (int32)Type);
}

void UNarr_TribalCommunicationSystem::PlayCommunicationSound(ENarr_CommunicationType Type, float Intensity)
{
    // Play appropriate sound effect based on communication type and intensity
    // This would interface with the audio system
    UE_LOG(LogTemp, Log, TEXT("Playing communication sound - Type: %d, Intensity: %f"), (int32)Type, Intensity);
}