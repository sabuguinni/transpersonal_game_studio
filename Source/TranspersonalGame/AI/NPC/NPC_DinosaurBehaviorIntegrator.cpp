#include "NPC_DinosaurBehaviorIntegrator.h"
#include "NPC_DinosaurAIController.h"
#include "NPC_DinosaurPersonalitySystem.h"
#include "NPC_DinosaurSensorySystem.h"
#include "NPC_DinosaurVocalSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"

UNPC_DinosaurBehaviorIntegrator::UNPC_DinosaurBehaviorIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for behavior updates

    // Initialize behavior state
    CurrentBehaviorMode = ENPCBehaviorMode::Idle;
    CurrentEmotionalState = ENPCEmotionalState::Calm;
    BehaviorIntensity = 0.5f;
    StressLevel = 0.0f;

    // Initialize behavior parameters
    BehaviorUpdateInterval = 0.5f;
    ThreatDetectionRadius = 2000.0f;
    SocialInteractionRadius = 1500.0f;
    StressDecayRate = 0.1f;

    // Initialize timers
    LastBehaviorUpdate = 0.0f;
    LastEnvironmentalScan = 0.0f;
    LastSocialUpdate = 0.0f;

    // Initialize system references
    DinosaurAIController = nullptr;
    PersonalitySystem = nullptr;
    SensorySystem = nullptr;
    VocalSystem = nullptr;
}

void UNPC_DinosaurBehaviorIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize behavior systems after a short delay to ensure all components are ready
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        this,
        &UNPC_DinosaurBehaviorIntegrator::InitializeBehaviorSystems,
        0.5f,
        false
    );
}

void UNPC_DinosaurBehaviorIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateBehaviorState(DeltaTime);
}

void UNPC_DinosaurBehaviorIntegrator::InitializeBehaviorSystems()
{
    UpdateSystemReferences();
    
    if (DinosaurAIController)
    {
        UE_LOG(LogTemp, Log, TEXT("NPC Behavior Integrator: AI Controller found and linked"));
    }
    
    if (PersonalitySystem)
    {
        UE_LOG(LogTemp, Log, TEXT("NPC Behavior Integrator: Personality System found and linked"));
    }
    
    if (SensorySystem)
    {
        UE_LOG(LogTemp, Log, TEXT("NPC Behavior Integrator: Sensory System found and linked"));
    }
    
    if (VocalSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("NPC Behavior Integrator: Vocal System found and linked"));
    }
    
    RegisterWithPackManager();
}

void UNPC_DinosaurBehaviorIntegrator::UpdateBehaviorState(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update behavior systems at intervals
    if (CurrentTime - LastBehaviorUpdate >= BehaviorUpdateInterval)
    {
        ProcessBehaviorTransitions();
        UpdateBehaviorIntensity();
        LastBehaviorUpdate = CurrentTime;
    }
    
    // Environmental scanning
    if (CurrentTime - LastEnvironmentalScan >= 1.0f)
    {
        ProcessEnvironmentalStimuli();
        LastEnvironmentalScan = CurrentTime;
    }
    
    // Social updates
    if (CurrentTime - LastSocialUpdate >= 2.0f)
    {
        UpdateTerritorialBehavior();
        LastSocialUpdate = CurrentTime;
    }
    
    // Continuous stress management
    HandleStressManagement(DeltaTime);
}

void UNPC_DinosaurBehaviorIntegrator::HandleThreatDetection(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor)
    {
        return;
    }
    
    // Increase stress based on threat level
    StressLevel = FMath::Clamp(StressLevel + (ThreatLevel * 0.3f), 0.0f, 1.0f);
    
    // Determine appropriate behavioral response
    if (ThreatLevel > 0.8f)
    {
        SetBehaviorMode(ENPCBehaviorMode::Fleeing);
        SetEmotionalState(ENPCEmotionalState::Terrified);
    }
    else if (ThreatLevel > 0.5f)
    {
        SetBehaviorMode(ENPCBehaviorMode::Aggressive);
        SetEmotionalState(ENPCEmotionalState::Angry);
    }
    else
    {
        SetBehaviorMode(ENPCBehaviorMode::Alert);
        SetEmotionalState(ENPCEmotionalState::Nervous);
    }
    
    // Notify vocal system for appropriate sounds
    if (VocalSystem)
    {
        if (ThreatLevel > 0.8f)
        {
            VocalSystem->PlayVocalization(ENPCVocalizationType::Alarm);
        }
        else if (ThreatLevel > 0.5f)
        {
            VocalSystem->PlayVocalization(ENPCVocalizationType::Threat);
        }
        else
        {
            VocalSystem->PlayVocalization(ENPCVocalizationType::Warning);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NPC Behavior: Threat detected - Level: %f, Response: %s"), 
           ThreatLevel, *UEnum::GetValueAsString(CurrentBehaviorMode));
}

void UNPC_DinosaurBehaviorIntegrator::HandleSocialInteraction(AActor* OtherActor, ENPCInteractionType InteractionType)
{
    if (!OtherActor)
    {
        return;
    }
    
    switch (InteractionType)
    {
        case ENPCInteractionType::Friendly:
            if (CurrentEmotionalState != ENPCEmotionalState::Angry)
            {
                SetEmotionalState(ENPCEmotionalState::Happy);
                StressLevel = FMath::Max(0.0f, StressLevel - 0.1f);
            }
            break;
            
        case ENPCInteractionType::Aggressive:
            SetBehaviorMode(ENPCBehaviorMode::Aggressive);
            SetEmotionalState(ENPCEmotionalState::Angry);
            StressLevel = FMath::Min(1.0f, StressLevel + 0.2f);
            break;
            
        case ENPCInteractionType::Territorial:
            SetBehaviorMode(ENPCBehaviorMode::Territorial);
            SetEmotionalState(ENPCEmotionalState::Aggressive);
            break;
            
        case ENPCInteractionType::Mating:
            if (PersonalitySystem && PersonalitySystem->GetPersonalityTrait(ENPCPersonalityTrait::Sociability) > 0.5f)
            {
                SetBehaviorMode(ENPCBehaviorMode::Social);
                SetEmotionalState(ENPCEmotionalState::Excited);
            }
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("NPC Behavior: Social interaction - Type: %s"), 
           *UEnum::GetValueAsString(InteractionType));
}

void UNPC_DinosaurBehaviorIntegrator::SetBehaviorMode(ENPCBehaviorMode NewMode)
{
    if (CurrentBehaviorMode != NewMode)
    {
        ENPCBehaviorMode PreviousMode = CurrentBehaviorMode;
        CurrentBehaviorMode = NewMode;
        
        // Update behavior intensity based on new mode
        switch (NewMode)
        {
            case ENPCBehaviorMode::Idle:
                BehaviorIntensity = 0.2f;
                break;
            case ENPCBehaviorMode::Foraging:
                BehaviorIntensity = 0.4f;
                break;
            case ENPCBehaviorMode::Patrolling:
                BehaviorIntensity = 0.5f;
                break;
            case ENPCBehaviorMode::Alert:
                BehaviorIntensity = 0.7f;
                break;
            case ENPCBehaviorMode::Aggressive:
            case ENPCBehaviorMode::Fleeing:
                BehaviorIntensity = 1.0f;
                break;
            default:
                BehaviorIntensity = 0.5f;
                break;
        }
        
        NotifyBehaviorChange();
        
        UE_LOG(LogTemp, Log, TEXT("NPC Behavior: Mode changed from %s to %s"), 
               *UEnum::GetValueAsString(PreviousMode), *UEnum::GetValueAsString(NewMode));
    }
}

void UNPC_DinosaurBehaviorIntegrator::SetEmotionalState(ENPCEmotionalState NewState)
{
    if (CurrentEmotionalState != NewState)
    {
        CurrentEmotionalState = NewState;
        
        // Notify vocal system of emotional change
        if (VocalSystem)
        {
            switch (NewState)
            {
                case ENPCEmotionalState::Happy:
                    VocalSystem->PlayVocalization(ENPCVocalizationType::Social);
                    break;
                case ENPCEmotionalState::Angry:
                    VocalSystem->PlayVocalization(ENPCVocalizationType::Threat);
                    break;
                case ENPCEmotionalState::Terrified:
                    VocalSystem->PlayVocalization(ENPCVocalizationType::Alarm);
                    break;
                default:
                    break;
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("NPC Behavior: Emotional state changed to %s"), 
               *UEnum::GetValueAsString(NewState));
    }
}

void UNPC_DinosaurBehaviorIntegrator::RegisterWithPackManager()
{
    // Find and register with pack manager if this dinosaur is part of a pack
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor->GetName().Contains(TEXT("PackManager")))
        {
            UE_LOG(LogTemp, Log, TEXT("NPC Behavior: Registered with Pack Manager"));
            break;
        }
    }
}

void UNPC_DinosaurBehaviorIntegrator::UpdateTerritorialBehavior()
{
    if (CurrentBehaviorMode == ENPCBehaviorMode::Territorial)
    {
        // Check for intruders in territory
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);
        
        FVector MyLocation = GetOwner()->GetActorLocation();
        
        for (AActor* Actor : NearbyActors)
        {
            if (Actor != GetOwner())
            {
                float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
                if (Distance < ThreatDetectionRadius)
                {
                    // Potential territorial threat
                    HandleThreatDetection(Actor, 0.6f);
                    break;
                }
            }
        }
    }
}

void UNPC_DinosaurBehaviorIntegrator::ProcessEnvironmentalStimuli()
{
    // Process environmental factors that might affect behavior
    if (SensorySystem)
    {
        // Environmental awareness updates would go here
        // For now, just log that we're processing
        UE_LOG(LogTemp, VeryVerbose, TEXT("NPC Behavior: Processing environmental stimuli"));
    }
}

void UNPC_DinosaurBehaviorIntegrator::UpdateSystemReferences()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        DinosaurAIController = Cast<ANPC_DinosaurAIController>(OwnerPawn->GetController());
        PersonalitySystem = OwnerPawn->FindComponentByClass<UNPC_DinosaurPersonalitySystem>();
        SensorySystem = OwnerPawn->FindComponentByClass<UNPC_DinosaurSensorySystem>();
        VocalSystem = OwnerPawn->FindComponentByClass<UNPC_DinosaurVocalSystem>();
    }
}

void UNPC_DinosaurBehaviorIntegrator::ProcessBehaviorTransitions()
{
    if (ShouldTransitionBehavior())
    {
        // Determine next behavior based on current state and environmental factors
        if (StressLevel > 0.8f)
        {
            if (CurrentBehaviorMode != ENPCBehaviorMode::Fleeing)
            {
                SetBehaviorMode(ENPCBehaviorMode::Fleeing);
            }
        }
        else if (StressLevel > 0.5f)
        {
            if (CurrentBehaviorMode != ENPCBehaviorMode::Alert)
            {
                SetBehaviorMode(ENPCBehaviorMode::Alert);
            }
        }
        else if (StressLevel < 0.2f)
        {
            if (CurrentBehaviorMode == ENPCBehaviorMode::Alert || CurrentBehaviorMode == ENPCBehaviorMode::Fleeing)
            {
                SetBehaviorMode(ENPCBehaviorMode::Idle);
                SetEmotionalState(ENPCEmotionalState::Calm);
            }
        }
    }
}

void UNPC_DinosaurBehaviorIntegrator::HandleStressManagement(float DeltaTime)
{
    // Gradually reduce stress over time
    if (StressLevel > 0.0f)
    {
        StressLevel = FMath::Max(0.0f, StressLevel - (StressDecayRate * DeltaTime));
    }
}

void UNPC_DinosaurBehaviorIntegrator::UpdateBehaviorIntensity()
{
    // Adjust behavior intensity based on current state
    float TargetIntensity = 0.5f;
    
    switch (CurrentBehaviorMode)
    {
        case ENPCBehaviorMode::Idle:
            TargetIntensity = 0.2f;
            break;
        case ENPCBehaviorMode::Alert:
            TargetIntensity = 0.7f + (StressLevel * 0.3f);
            break;
        case ENPCBehaviorMode::Aggressive:
        case ENPCBehaviorMode::Fleeing:
            TargetIntensity = 0.9f + (StressLevel * 0.1f);
            break;
        default:
            TargetIntensity = 0.5f;
            break;
    }
    
    BehaviorIntensity = FMath::FInterpTo(BehaviorIntensity, TargetIntensity, GetWorld()->GetDeltaSeconds(), 2.0f);
}

bool UNPC_DinosaurBehaviorIntegrator::ShouldTransitionBehavior() const
{
    // Simple transition logic based on stress level changes
    return (GetWorld()->GetTimeSeconds() - LastBehaviorUpdate) >= BehaviorUpdateInterval;
}

void UNPC_DinosaurBehaviorIntegrator::NotifyBehaviorChange()
{
    // Notify other systems of behavior change
    if (DinosaurAIController)
    {
        // AI Controller can respond to behavior changes
        UE_LOG(LogTemp, VeryVerbose, TEXT("NPC Behavior: Notified AI Controller of behavior change"));
    }
}