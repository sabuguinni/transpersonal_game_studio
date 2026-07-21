#include "NPC_DinosaurVocalSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/Engine.h"

UNPC_DinosaurVocalSystem::UNPC_DinosaurVocalSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    CommunicationRange = 3000.0f;
    VocalFrequency = 2.0f;
    CurrentVocalization = ENPC_VocalizationType::TerritorialRoar;
    CurrentEmotionalState = ENPC_EmotionalState::Neutral;
    LastVocalizationTime = 0.0f;
    VocalizationCooldown = 5.0f;

    // Create audio component
    VocalAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VocalAudioComponent"));
    if (VocalAudioComponent)
    {
        VocalAudioComponent->bAutoActivate = false;
        VocalAudioComponent->SetVolumeMultiplier(1.0f);
    }
}

void UNPC_DinosaurVocalSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVocalPatterns();
    
    // Set up species-specific patterns based on owner
    if (AActor* Owner = GetOwner())
    {
        FString OwnerName = Owner->GetName();
        if (OwnerName.Contains(TEXT("TRex")))
        {
            SetSpeciesVocalPatterns(ENPC_DinosaurSpecies::TRex);
        }
        else if (OwnerName.Contains(TEXT("Raptor")) || OwnerName.Contains(TEXT("Veloci")))
        {
            SetSpeciesVocalPatterns(ENPC_DinosaurSpecies::Velociraptor);
        }
        else if (OwnerName.Contains(TEXT("Brachio")))
        {
            SetSpeciesVocalPatterns(ENPC_DinosaurSpecies::Brachiosaurus);
        }
        else
        {
            SetSpeciesVocalPatterns(ENPC_DinosaurSpecies::GenericHerbivore);
        }
    }
}

void UNPC_DinosaurVocalSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update vocalization timing
    LastVocalizationTime += DeltaTime;
    
    // Process incoming vocalizations from other dinosaurs
    ProcessIncomingVocalizations();
    
    // Automatic territorial calls based on frequency
    if (LastVocalizationTime >= (60.0f / VocalFrequency))
    {
        if (CurrentEmotionalState == ENPC_EmotionalState::Aggressive || 
            CurrentEmotionalState == ENPC_EmotionalState::Territorial)
        {
            BroadcastTerritorialClaim();
        }
    }
}

void UNPC_DinosaurVocalSystem::PlayVocalization(ENPC_VocalizationType VocalizationType)
{
    if (!VocalAudioComponent)
    {
        return;
    }
    
    // Check cooldown
    if (LastVocalizationTime < VocalizationCooldown)
    {
        return;
    }
    
    CurrentVocalization = VocalizationType;
    
    // Get vocal pattern for current species
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    ENPC_DinosaurSpecies OwnerSpecies = ENPC_DinosaurSpecies::GenericHerbivore;
    FString OwnerName = Owner->GetName();
    if (OwnerName.Contains(TEXT("TRex")))
    {
        OwnerSpecies = ENPC_DinosaurSpecies::TRex;
    }
    else if (OwnerName.Contains(TEXT("Raptor")))
    {
        OwnerSpecies = ENPC_DinosaurSpecies::Velociraptor;
    }
    
    if (SpeciesVocalPatterns.Contains(OwnerSpecies))
    {
        TArray<FNPC_VocalPattern>& Patterns = SpeciesVocalPatterns[OwnerSpecies];
        for (FNPC_VocalPattern& Pattern : Patterns)
        {
            if (Pattern.VocalizationType == VocalizationType)
            {
                // Modify pattern based on emotional state
                ModifyVocalizationForEmotionalState(Pattern);
                
                // Play the vocalization
                if (VocalAudioComponent)
                {
                    VocalAudioComponent->SetVolumeMultiplier(Pattern.Intensity);
                    VocalAudioComponent->Play();
                }
                
                // Broadcast to nearby dinosaurs
                TArray<AActor*> NearbyActors = GetActorsInVocalRange();
                for (AActor* NearbyActor : NearbyActors)
                {
                    if (UNPC_DinosaurVocalSystem* OtherVocalSystem = NearbyActor->FindComponentByClass<UNPC_DinosaurVocalSystem>())
                    {
                        if (OtherVocalSystem->CanHearVocalization(Owner, Pattern.Range))
                        {
                            OtherVocalSystem->RespondToVocalization(VocalizationType, Owner);
                        }
                    }
                }
                
                LastVocalizationTime = 0.0f;
                VocalizationCooldown = Pattern.Duration + 1.0f;
                break;
            }
        }
    }
    
    // Log vocalization for debugging
    UE_LOG(LogTemp, Log, TEXT("%s played vocalization: %s"), 
           *Owner->GetName(), 
           *UEnum::GetValueAsString(VocalizationType));
}

void UNPC_DinosaurVocalSystem::RespondToVocalization(ENPC_VocalizationType HeardVocalization, AActor* Caller)
{
    if (!Caller)
    {
        return;
    }
    
    // Determine appropriate response
    ENPC_VocalizationType ResponseType = DetermineVocalResponse(HeardVocalization, Caller);
    
    // Check if we should respond based on vocal response patterns
    for (const FNPC_VocalResponse& Response : VocalResponses)
    {
        if (Response.TriggerCall == HeardVocalization)
        {
            if (FMath::RandRange(0.0f, 1.0f) <= Response.ResponseProbability)
            {
                // Delay the response
                GetWorld()->GetTimerManager().SetTimer(
                    FTimerHandle(),
                    [this, ResponseType]()
                    {
                        PlayVocalization(ResponseType);
                    },
                    Response.ResponseDelay,
                    false
                );
            }
            break;
        }
    }
}

void UNPC_DinosaurVocalSystem::BroadcastTerritorialClaim()
{
    PlayVocalization(ENPC_VocalizationType::TerritorialRoar);
}

void UNPC_DinosaurVocalSystem::CallForPackAssistance()
{
    PlayVocalization(ENPC_VocalizationType::PackCall);
}

void UNPC_DinosaurVocalSystem::SoundAlarmCall()
{
    PlayVocalization(ENPC_VocalizationType::AlarmCall);
}

void UNPC_DinosaurVocalSystem::InitiateHuntingCall()
{
    PlayVocalization(ENPC_VocalizationType::HuntingCall);
}

TArray<AActor*> UNPC_DinosaurVocalSystem::GetActorsInVocalRange()
{
    TArray<AActor*> NearbyActors;
    
    if (UWorld* World = GetWorld())
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        
        // Get all actors with vocal systems
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor != GetOwner() && Actor->FindComponentByClass<UNPC_DinosaurVocalSystem>())
            {
                float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
                if (Distance <= CommunicationRange)
                {
                    NearbyActors.Add(Actor);
                }
            }
        }
    }
    
    return NearbyActors;
}

bool UNPC_DinosaurVocalSystem::CanHearVocalization(AActor* Caller, float CallRange)
{
    if (!Caller || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Caller->GetActorLocation());
    return Distance <= CallRange;
}

void UNPC_DinosaurVocalSystem::SetSpeciesVocalPatterns(ENPC_DinosaurSpecies Species)
{
    if (!SpeciesVocalPatterns.Contains(Species))
    {
        return;
    }
    
    // Update communication range based on species
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            CommunicationRange = 5000.0f;
            VocalFrequency = 1.5f;
            break;
        case ENPC_DinosaurSpecies::Velociraptor:
            CommunicationRange = 2000.0f;
            VocalFrequency = 3.0f;
            break;
        case ENPC_DinosaurSpecies::Brachiosaurus:
            CommunicationRange = 8000.0f;
            VocalFrequency = 0.5f;
            break;
        default:
            CommunicationRange = 3000.0f;
            VocalFrequency = 2.0f;
            break;
    }
}

void UNPC_DinosaurVocalSystem::InitializeVocalPatterns()
{
    // Initialize T-Rex vocal patterns
    TArray<FNPC_VocalPattern> TRexPatterns;
    
    FNPC_VocalPattern TRexRoar;
    TRexRoar.VocalizationType = ENPC_VocalizationType::TerritorialRoar;
    TRexRoar.Range = 5000.0f;
    TRexRoar.Frequency = 1.0f;
    TRexRoar.Duration = 4.0f;
    TRexRoar.Intensity = 1.5f;
    TRexRoar.bCanInterruptOtherCalls = true;
    TRexPatterns.Add(TRexRoar);
    
    FNPC_VocalPattern TRexGrowl;
    TRexGrowl.VocalizationType = ENPC_VocalizationType::AggressionGrowl;
    TRexGrowl.Range = 2000.0f;
    TRexGrowl.Frequency = 2.0f;
    TRexGrowl.Duration = 2.0f;
    TRexGrowl.Intensity = 1.2f;
    TRexGrowl.bCanInterruptOtherCalls = false;
    TRexPatterns.Add(TRexGrowl);
    
    SpeciesVocalPatterns.Add(ENPC_DinosaurSpecies::TRex, TRexPatterns);
    
    // Initialize Velociraptor vocal patterns
    TArray<FNPC_VocalPattern> RaptorPatterns;
    
    FNPC_VocalPattern RaptorPackCall;
    RaptorPackCall.VocalizationType = ENPC_VocalizationType::PackCall;
    RaptorPackCall.Range = 2000.0f;
    RaptorPackCall.Frequency = 3.0f;
    RaptorPackCall.Duration = 1.5f;
    RaptorPackCall.Intensity = 1.0f;
    RaptorPackCall.bCanInterruptOtherCalls = false;
    RaptorPatterns.Add(RaptorPackCall);
    
    FNPC_VocalPattern RaptorHuntCall;
    RaptorHuntCall.VocalizationType = ENPC_VocalizationType::HuntingCall;
    RaptorHuntCall.Range = 1500.0f;
    RaptorHuntCall.Frequency = 4.0f;
    RaptorHuntCall.Duration = 1.0f;
    RaptorHuntCall.Intensity = 0.8f;
    RaptorHuntCall.bCanInterruptOtherCalls = true;
    RaptorPatterns.Add(RaptorHuntCall);
    
    SpeciesVocalPatterns.Add(ENPC_DinosaurSpecies::Velociraptor, RaptorPatterns);
    
    // Initialize vocal responses
    FNPC_VocalResponse PackResponse;
    PackResponse.TriggerCall = ENPC_VocalizationType::PackCall;
    PackResponse.ResponseCall = ENPC_VocalizationType::PackCall;
    PackResponse.ResponseDelay = 1.0f;
    PackResponse.ResponseProbability = 0.9f;
    VocalResponses.Add(PackResponse);
    
    FNPC_VocalResponse AlarmResponse;
    AlarmResponse.TriggerCall = ENPC_VocalizationType::AlarmCall;
    AlarmResponse.ResponseCall = ENPC_VocalizationType::AlarmCall;
    AlarmResponse.ResponseDelay = 0.5f;
    AlarmResponse.ResponseProbability = 0.7f;
    VocalResponses.Add(AlarmResponse);
}

void UNPC_DinosaurVocalSystem::ProcessIncomingVocalizations()
{
    // This would be called by the audio system when vocalizations are detected
    // For now, it's a placeholder for future audio detection implementation
}

ENPC_VocalizationType UNPC_DinosaurVocalSystem::DetermineVocalResponse(ENPC_VocalizationType HeardCall, AActor* Caller)
{
    // Default response logic
    switch (HeardCall)
    {
        case ENPC_VocalizationType::PackCall:
            return ENPC_VocalizationType::PackCall;
        case ENPC_VocalizationType::AlarmCall:
            return ENPC_VocalizationType::AlarmCall;
        case ENPC_VocalizationType::TerritorialRoar:
            return ENPC_VocalizationType::AggressionGrowl;
        case ENPC_VocalizationType::HuntingCall:
            return ENPC_VocalizationType::HuntingCall;
        default:
            return ENPC_VocalizationType::TerritorialRoar;
    }
}

void UNPC_DinosaurVocalSystem::ModifyVocalizationForEmotionalState(FNPC_VocalPattern& Pattern)
{
    switch (CurrentEmotionalState)
    {
        case ENPC_EmotionalState::Aggressive:
            Pattern.Intensity *= 1.3f;
            Pattern.Duration *= 1.2f;
            break;
        case ENPC_EmotionalState::Fearful:
            Pattern.Intensity *= 0.7f;
            Pattern.Duration *= 0.8f;
            break;
        case ENPC_EmotionalState::Excited:
            Pattern.Frequency *= 1.5f;
            Pattern.Intensity *= 1.1f;
            break;
        case ENPC_EmotionalState::Calm:
            Pattern.Intensity *= 0.9f;
            Pattern.Frequency *= 0.8f;
            break;
        default:
            // No modification for neutral state
            break;
    }
}