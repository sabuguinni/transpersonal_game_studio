#include "NPC_DinosaurCommunicationSystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UNPC_DinosaurCommunicationSystem::UNPC_DinosaurCommunicationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 0.5 seconds
    
    // Initialize communication parameters
    MaxCommunicationRange = 5000.0f;
    CommunicationCooldown = 2.0f;
    LastCommunicationTime = 0.0f;
    
    // Initialize call types
    CallTypes.Add(ENPC_DinosaurCallType::Alert);
    CallTypes.Add(ENPC_DinosaurCallType::Territorial);
    CallTypes.Add(ENPC_DinosaurCallType::Mating);
    CallTypes.Add(ENPC_DinosaurCallType::Distress);
    CallTypes.Add(ENPC_DinosaurCallType::Hunting);
    CallTypes.Add(ENPC_DinosaurCallType::Social);
    
    // Initialize response probabilities
    ResponseProbabilities.Add(ENPC_DinosaurCallType::Alert, 0.9f);
    ResponseProbabilities.Add(ENPC_DinosaurCallType::Territorial, 0.7f);
    ResponseProbabilities.Add(ENPC_DinosaurCallType::Mating, 0.8f);
    ResponseProbabilities.Add(ENPC_DinosaurCallType::Distress, 0.95f);
    ResponseProbabilities.Add(ENPC_DinosaurCallType::Hunting, 0.6f);
    ResponseProbabilities.Add(ENPC_DinosaurCallType::Social, 0.4f);
}

void UNPC_DinosaurCommunicationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        OwnerController = Cast<AAIController>(OwnerPawn->GetController());
        if (OwnerController && OwnerController->GetBlackboardComponent())
        {
            BlackboardComponent = OwnerController->GetBlackboardComponent();
        }
    }
    
    // Initialize audio component for vocalizations
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VocalizationAudio"));
    if (AudioComponent)
    {
        AudioComponent->SetupAttachment(GetOwner()->GetRootComponent());
        AudioComponent->bAutoActivate = false;
    }
}

void UNPC_DinosaurCommunicationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Process incoming communications
    ProcessIncomingCommunications();
    
    // Update communication cooldowns
    UpdateCommunicationCooldowns(DeltaTime);
    
    // Check for automatic communication triggers
    CheckAutomaticCommunicationTriggers();
}

void UNPC_DinosaurCommunicationSystem::SendCommunication(ENPC_DinosaurCallType CallType, const FVector& TargetLocation, float Range)
{
    if (!CanCommunicate())
    {
        return;
    }
    
    FNPC_CommunicationMessage Message;
    Message.CallType = CallType;
    Message.SenderLocation = GetOwner()->GetActorLocation();
    Message.TargetLocation = TargetLocation;
    Message.Range = FMath::Min(Range, MaxCommunicationRange);
    Message.Timestamp = GetWorld()->GetTimeSeconds();
    Message.SenderPawn = OwnerPawn;
    Message.Intensity = CalculateCallIntensity(CallType);
    
    // Broadcast to nearby dinosaurs
    BroadcastCommunication(Message);
    
    // Play vocalization sound
    PlayVocalizationSound(CallType, Message.Intensity);
    
    // Update communication state
    LastCommunicationTime = GetWorld()->GetTimeSeconds();
    CommunicationHistory.Add(Message);
    
    // Limit history size
    if (CommunicationHistory.Num() > 50)
    {
        CommunicationHistory.RemoveAt(0);
    }
    
    // Update blackboard if available
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("LastCommunicationTime"), LastCommunicationTime);
        BlackboardComponent->SetValueAsEnum(TEXT("LastCallType"), static_cast<uint8>(CallType));
    }
}

void UNPC_DinosaurCommunicationSystem::ReceiveCommunication(const FNPC_CommunicationMessage& Message)
{
    if (!IsValidCommunication(Message))
    {
        return;
    }
    
    // Add to incoming queue
    IncomingCommunications.Add(Message);
    
    // Immediate response for critical calls
    if (Message.CallType == ENPC_DinosaurCallType::Distress || 
        Message.CallType == ENPC_DinosaurCallType::Alert)
    {
        ProcessCommunicationImmediate(Message);
    }
}

void UNPC_DinosaurCommunicationSystem::BroadcastCommunication(const FNPC_CommunicationMessage& Message)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Find all dinosaurs within range
    TArray<AActor*> NearbyActors;
    UWorld* World = GetWorld();
    
    for (TActorIterator<APawn> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        APawn* OtherPawn = *ActorIterator;
        if (!OtherPawn || OtherPawn == OwnerPawn)
        {
            continue;
        }
        
        // Check if it's a dinosaur with communication system
        UNPC_DinosaurCommunicationSystem* OtherCommSystem = OtherPawn->FindComponentByClass<UNPC_DinosaurCommunicationSystem>();
        if (!OtherCommSystem)
        {
            continue;
        }
        
        // Check range
        float Distance = FVector::Dist(Message.SenderLocation, OtherPawn->GetActorLocation());
        if (Distance <= Message.Range)
        {
            // Apply distance-based intensity falloff
            FNPC_CommunicationMessage ModifiedMessage = Message;
            ModifiedMessage.Intensity *= FMath::Clamp(1.0f - (Distance / Message.Range), 0.1f, 1.0f);
            
            OtherCommSystem->ReceiveCommunication(ModifiedMessage);
        }
    }
}

void UNPC_DinosaurCommunicationSystem::ProcessIncomingCommunications()
{
    for (int32 i = IncomingCommunications.Num() - 1; i >= 0; i--)
    {
        const FNPC_CommunicationMessage& Message = IncomingCommunications[i];
        
        // Check if message is still valid (not too old)
        float MessageAge = GetWorld()->GetTimeSeconds() - Message.Timestamp;
        if (MessageAge > 10.0f) // Messages expire after 10 seconds
        {
            IncomingCommunications.RemoveAt(i);
            continue;
        }
        
        // Process the message
        ProcessCommunicationMessage(Message);
        
        // Remove processed message
        IncomingCommunications.RemoveAt(i);
    }
}

void UNPC_DinosaurCommunicationSystem::ProcessCommunicationMessage(const FNPC_CommunicationMessage& Message)
{
    // Determine if we should respond
    float ResponseChance = ResponseProbabilities.FindRef(Message.CallType);
    if (FMath::RandRange(0.0f, 1.0f) > ResponseChance)
    {
        return;
    }
    
    // Update blackboard with communication info
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("LastCommunicationSource"), Message.SenderLocation);
        BlackboardComponent->SetValueAsEnum(TEXT("LastReceivedCallType"), static_cast<uint8>(Message.CallType));
        BlackboardComponent->SetValueAsFloat(TEXT("CommunicationIntensity"), Message.Intensity);
    }
    
    // Trigger appropriate behavioral response
    switch (Message.CallType)
    {
        case ENPC_DinosaurCallType::Alert:
            HandleAlertCall(Message);
            break;
        case ENPC_DinosaurCallType::Territorial:
            HandleTerritorialCall(Message);
            break;
        case ENPC_DinosaurCallType::Mating:
            HandleMatingCall(Message);
            break;
        case ENPC_DinosaurCallType::Distress:
            HandleDistressCall(Message);
            break;
        case ENPC_DinosaurCallType::Hunting:
            HandleHuntingCall(Message);
            break;
        case ENPC_DinosaurCallType::Social:
            HandleSocialCall(Message);
            break;
    }
}

void UNPC_DinosaurCommunicationSystem::ProcessCommunicationImmediate(const FNPC_CommunicationMessage& Message)
{
    // Immediate processing for critical communications
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("EmergencyLocation"), Message.SenderLocation);
        BlackboardComponent->SetValueAsBool(TEXT("EmergencyResponse"), true);
    }
    
    // Trigger immediate behavioral change
    if (Message.CallType == ENPC_DinosaurCallType::Distress)
    {
        // Move towards distress location or flee depending on species/personality
        TriggerEmergencyResponse(Message);
    }
    else if (Message.CallType == ENPC_DinosaurCallType::Alert)
    {
        // Increase alertness and scan for threats
        TriggerAlertResponse(Message);
    }
}

void UNPC_DinosaurCommunicationSystem::HandleAlertCall(const FNPC_CommunicationMessage& Message)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("IsAlert"), true);
        BlackboardComponent->SetValueAsVector(TEXT("AlertLocation"), Message.SenderLocation);
        BlackboardComponent->SetValueAsFloat(TEXT("AlertLevel"), Message.Intensity);
    }
    
    // Possible response call
    if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
    {
        FTimerHandle ResponseTimer;
        GetWorld()->GetTimerManager().SetTimer(ResponseTimer, [this]()
        {
            SendCommunication(ENPC_DinosaurCallType::Alert, GetOwner()->GetActorLocation(), MaxCommunicationRange * 0.8f);
        }, FMath::RandRange(1.0f, 3.0f), false);
    }
}

void UNPC_DinosaurCommunicationSystem::HandleTerritorialCall(const FNPC_CommunicationMessage& Message)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("TerritorialThreat"), Message.SenderLocation);
        BlackboardComponent->SetValueAsBool(TEXT("TerritorialChallenge"), true);
    }
    
    // Response depends on species and personality
    float AggressionLevel = BlackboardComponent ? BlackboardComponent->GetValueAsFloat(TEXT("AggressionLevel")) : 0.5f;
    
    if (AggressionLevel > 0.6f)
    {
        // Aggressive response
        FTimerHandle ResponseTimer;
        GetWorld()->GetTimerManager().SetTimer(ResponseTimer, [this, Message]()
        {
            SendCommunication(ENPC_DinosaurCallType::Territorial, Message.SenderLocation, MaxCommunicationRange);
        }, FMath::RandRange(0.5f, 2.0f), false);
    }
}

void UNPC_DinosaurCommunicationSystem::HandleMatingCall(const FNPC_CommunicationMessage& Message)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("MatingCallSource"), Message.SenderLocation);
        BlackboardComponent->SetValueAsBool(TEXT("MatingOpportunity"), true);
    }
    
    // Response based on mating season and readiness
    bool IsMatingReady = BlackboardComponent ? BlackboardComponent->GetValueAsBool(TEXT("IsMatingReady")) : false;
    
    if (IsMatingReady)
    {
        FTimerHandle ResponseTimer;
        GetWorld()->GetTimerManager().SetTimer(ResponseTimer, [this, Message]()
        {
            SendCommunication(ENPC_DinosaurCallType::Mating, Message.SenderLocation, MaxCommunicationRange * 0.6f);
        }, FMath::RandRange(2.0f, 5.0f), false);
    }
}

void UNPC_DinosaurCommunicationSystem::HandleDistressCall(const FNPC_CommunicationMessage& Message)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("DistressLocation"), Message.SenderLocation);
        BlackboardComponent->SetValueAsBool(TEXT("PackMemberInDanger"), true);
        BlackboardComponent->SetValueAsFloat(TEXT("DistressUrgency"), Message.Intensity);
    }
    
    // High probability of response for pack members
    TriggerEmergencyResponse(Message);
}

void UNPC_DinosaurCommunicationSystem::HandleHuntingCall(const FNPC_CommunicationMessage& Message)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("HuntingLocation"), Message.SenderLocation);
        BlackboardComponent->SetValueAsBool(TEXT("HuntingOpportunity"), true);
    }
    
    // Join hunting if hungry and pack member
    bool IsHungry = BlackboardComponent ? BlackboardComponent->GetValueAsFloat(TEXT("HungerLevel")) > 0.6f : false;
    bool IsPackMember = BlackboardComponent ? BlackboardComponent->GetValueAsBool(TEXT("IsPackMember")) : false;
    
    if (IsHungry && IsPackMember)
    {
        FTimerHandle ResponseTimer;
        GetWorld()->GetTimerManager().SetTimer(ResponseTimer, [this]()
        {
            SendCommunication(ENPC_DinosaurCallType::Hunting, GetOwner()->GetActorLocation(), MaxCommunicationRange * 0.7f);
        }, FMath::RandRange(1.0f, 4.0f), false);
    }
}

void UNPC_DinosaurCommunicationSystem::HandleSocialCall(const FNPC_CommunicationMessage& Message)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("SocialCallSource"), Message.SenderLocation);
        BlackboardComponent->SetValueAsBool(TEXT("SocialInteraction"), true);
    }
    
    // Random social response
    if (FMath::RandRange(0.0f, 1.0f) < 0.2f)
    {
        FTimerHandle ResponseTimer;
        GetWorld()->GetTimerManager().SetTimer(ResponseTimer, [this]()
        {
            SendCommunication(ENPC_DinosaurCallType::Social, GetOwner()->GetActorLocation(), MaxCommunicationRange * 0.5f);
        }, FMath::RandRange(3.0f, 8.0f), false);
    }
}

void UNPC_DinosaurCommunicationSystem::TriggerEmergencyResponse(const FNPC_CommunicationMessage& Message)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("EmergencyMode"), true);
        BlackboardComponent->SetValueAsVector(TEXT("EmergencyTarget"), Message.SenderLocation);
        BlackboardComponent->SetValueAsFloat(TEXT("EmergencyTimer"), 30.0f); // 30 second emergency mode
    }
}

void UNPC_DinosaurCommunicationSystem::TriggerAlertResponse(const FNPC_CommunicationMessage& Message)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("HighAlert"), true);
        BlackboardComponent->SetValueAsFloat(TEXT("AlertTimer"), 15.0f); // 15 second alert mode
        BlackboardComponent->SetValueAsVector(TEXT("ThreatDirection"), (Message.SenderLocation - GetOwner()->GetActorLocation()).GetSafeNormal());
    }
}

bool UNPC_DinosaurCommunicationSystem::CanCommunicate() const
{
    if (!GetWorld())
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastCommunicationTime) >= CommunicationCooldown;
}

bool UNPC_DinosaurCommunicationSystem::IsValidCommunication(const FNPC_CommunicationMessage& Message) const
{
    if (!Message.SenderPawn || Message.SenderPawn == OwnerPawn)
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Message.SenderLocation);
    return Distance <= Message.Range;
}

float UNPC_DinosaurCommunicationSystem::CalculateCallIntensity(ENPC_DinosaurCallType CallType) const
{
    float BaseIntensity = 1.0f;
    
    switch (CallType)
    {
        case ENPC_DinosaurCallType::Distress:
            BaseIntensity = 1.0f;
            break;
        case ENPC_DinosaurCallType::Alert:
            BaseIntensity = 0.9f;
            break;
        case ENPC_DinosaurCallType::Territorial:
            BaseIntensity = 0.8f;
            break;
        case ENPC_DinosaurCallType::Mating:
            BaseIntensity = 0.7f;
            break;
        case ENPC_DinosaurCallType::Hunting:
            BaseIntensity = 0.6f;
            break;
        case ENPC_DinosaurCallType::Social:
            BaseIntensity = 0.4f;
            break;
    }
    
    // Modify based on emotional state if available
    if (BlackboardComponent)
    {
        float StressLevel = BlackboardComponent->GetValueAsFloat(TEXT("StressLevel"));
        float AggressionLevel = BlackboardComponent->GetValueAsFloat(TEXT("AggressionLevel"));
        
        BaseIntensity *= (1.0f + StressLevel * 0.3f + AggressionLevel * 0.2f);
    }
    
    return FMath::Clamp(BaseIntensity, 0.1f, 2.0f);
}

void UNPC_DinosaurCommunicationSystem::PlayVocalizationSound(ENPC_DinosaurCallType CallType, float Intensity)
{
    if (!AudioComponent)
    {
        return;
    }
    
    // This would be connected to actual sound assets in a full implementation
    // For now, we just set volume based on intensity
    AudioComponent->SetVolumeMultiplier(FMath::Clamp(Intensity, 0.1f, 1.0f));
    
    // Different sounds for different call types would be loaded here
    // AudioComponent->SetSound(GetSoundForCallType(CallType));
    // AudioComponent->Play();
}

void UNPC_DinosaurCommunicationSystem::UpdateCommunicationCooldowns(float DeltaTime)
{
    // This method can be used to update various communication-related timers
    // Currently handled by the CanCommunicate() method
}

void UNPC_DinosaurCommunicationSystem::CheckAutomaticCommunicationTriggers()
{
    if (!BlackboardComponent || !CanCommunicate())
    {
        return;
    }
    
    // Check for automatic communication triggers based on blackboard state
    bool IsInDanger = BlackboardComponent->GetValueAsBool(TEXT("IsInDanger"));
    bool SawPlayer = BlackboardComponent->GetValueAsBool(TEXT("SawPlayer"));
    float HungerLevel = BlackboardComponent->GetValueAsFloat(TEXT("HungerLevel"));
    
    // Automatic distress call when in danger
    if (IsInDanger)
    {
        SendCommunication(ENPC_DinosaurCallType::Distress, GetOwner()->GetActorLocation(), MaxCommunicationRange);
    }
    // Automatic alert call when spotting player
    else if (SawPlayer)
    {
        SendCommunication(ENPC_DinosaurCallType::Alert, GetOwner()->GetActorLocation(), MaxCommunicationRange * 0.8f);
    }
    // Automatic hunting call when very hungry
    else if (HungerLevel > 0.8f && FMath::RandRange(0.0f, 1.0f) < 0.1f)
    {
        SendCommunication(ENPC_DinosaurCallType::Hunting, GetOwner()->GetActorLocation(), MaxCommunicationRange * 0.6f);
    }
}