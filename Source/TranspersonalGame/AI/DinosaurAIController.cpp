#include "DinosaurAIController.h"
#include "DinosaurMemoryComponent.h"
#include "DinosaurSocialComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create core components
    MemoryComponent = CreateDefaultSubobject<UDinosaurMemoryComponent>(TEXT("MemoryComponent"));
    SocialComponent = CreateDefaultSubobject<UDinosaurSocialComponent>(TEXT("SocialComponent"));
    
    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);
    
    // Initialize default values
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    Personality = EDinosaurPersonality::Curious;
    StateTimer = 0.0f;
    
    // Set default need decay rates based on dinosaur type
    HungerDecayRate = 5.0f;
    ThirstDecayRate = 8.0f;
    EnergyDecayRate = 3.0f;
    SafetyRecoveryRate = 2.0f;
}

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();
    
    CachedWorld = GetWorld();
    InitializePerception();
    InitializeBehaviorTree();
    
    // Set up initial routine
    UpdateDailyRoutine();
    
    UE_LOG(LogTemp, Warning, TEXT("DinosaurAIController initialized for %s"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"));
}

void ADinosaurAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update needs over time
    NeedUpdateTimer += DeltaTime;
    if (NeedUpdateTimer >= 1.0f) // Update needs every second
    {
        UpdateNeeds(NeedUpdateTimer);
        NeedUpdateTimer = 0.0f;
    }
    
    // Update routine periodically
    RoutineUpdateTimer += DeltaTime;
    if (RoutineUpdateTimer >= 30.0f) // Update routine every 30 seconds
    {
        UpdateDailyRoutine();
        RoutineUpdateTimer = 0.0f;
    }
    
    // Update blackboard with current state
    UpdateBlackboard();
    
    // Process emergent behaviors
    ProcessEmergentBehavior();
    
    // Update state timer
    StateTimer += DeltaTime;
}

void ADinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    DinosaurPawn = Cast<ADinosaurCharacter>(InPawn);
    if (!DinosaurPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("DinosaurAIController can only possess DinosaurCharacter pawns"));
        return;
    }
    
    // Start behavior tree if available
    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void ADinosaurAIController::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        EDinosaurBehaviorState PreviousState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;
        StateTimer = 0.0f;
        
        // Update blackboard
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
        }
        
        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("%s changed behavior state from %d to %d"), 
               GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"),
               static_cast<int32>(PreviousState),
               static_cast<int32>(NewState));
        
        // Notify memory component of state change
        if (MemoryComponent)
        {
            MemoryComponent->RecordBehaviorChange(PreviousState, NewState);
        }
    }
}

void ADinosaurAIController::UpdateNeeds(float DeltaTime)
{
    // Decay needs over time
    CurrentNeeds.Hunger = FMath::Max(0.0f, CurrentNeeds.Hunger - (HungerDecayRate * DeltaTime));
    CurrentNeeds.Thirst = FMath::Max(0.0f, CurrentNeeds.Thirst - (ThirstDecayRate * DeltaTime));
    CurrentNeeds.Energy = FMath::Max(0.0f, CurrentNeeds.Energy - (EnergyDecayRate * DeltaTime));
    
    // Safety recovers slowly when not in danger
    if (CurrentBehaviorState != EDinosaurBehaviorState::Fleeing)
    {
        CurrentNeeds.Safety = FMath::Min(100.0f, CurrentNeeds.Safety + (SafetyRecoveryRate * DeltaTime));
    }
    
    // Social need depends on personality
    if (Personality == EDinosaurPersonality::Social)
    {
        CurrentNeeds.Social = FMath::Max(0.0f, CurrentNeeds.Social - (2.0f * DeltaTime));
    }
    else if (Personality == EDinosaurPersonality::Solitary)
    {
        CurrentNeeds.Social = FMath::Min(100.0f, CurrentNeeds.Social + (1.0f * DeltaTime));
    }
    
    // Update blackboard with current needs
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsFloat(TEXT("Hunger"), CurrentNeeds.Hunger);
        GetBlackboardComponent()->SetValueAsFloat(TEXT("Thirst"), CurrentNeeds.Thirst);
        GetBlackboardComponent()->SetValueAsFloat(TEXT("Energy"), CurrentNeeds.Energy);
        GetBlackboardComponent()->SetValueAsFloat(TEXT("Safety"), CurrentNeeds.Safety);
        GetBlackboardComponent()->SetValueAsFloat(TEXT("Social"), CurrentNeeds.Social);
    }
}

float ADinosaurAIController::GetHighestNeed() const
{
    float HighestNeed = 0.0f;
    
    // Invert needs (lower values = higher priority)
    HighestNeed = FMath::Max(HighestNeed, 100.0f - CurrentNeeds.Hunger);
    HighestNeed = FMath::Max(HighestNeed, 100.0f - CurrentNeeds.Thirst);
    HighestNeed = FMath::Max(HighestNeed, 100.0f - CurrentNeeds.Energy);
    HighestNeed = FMath::Max(HighestNeed, 100.0f - CurrentNeeds.Safety);
    
    return HighestNeed;
}

bool ADinosaurAIController::IsNeedCritical(float Threshold) const
{
    return CurrentNeeds.Hunger < Threshold || 
           CurrentNeeds.Thirst < Threshold || 
           CurrentNeeds.Energy < Threshold ||
           CurrentNeeds.Safety < Threshold;
}

FVector ADinosaurAIController::GetNearestLocationOfType(const TArray<FVector>& Locations) const
{
    if (Locations.Num() == 0 || !GetPawn())
    {
        return FVector::ZeroVector;
    }
    
    FVector PawnLocation = GetPawn()->GetActorLocation();
    FVector NearestLocation = Locations[0];
    float NearestDistance = FVector::Dist(PawnLocation, NearestLocation);
    
    for (const FVector& Location : Locations)
    {
        float Distance = FVector::Dist(PawnLocation, Location);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestLocation = Location;
        }
    }
    
    return NearestLocation;
}

void ADinosaurAIController::UpdateDailyRoutine()
{
    if (!CachedWorld)
        return;
    
    // Get current time of day (simplified - you might want to use a proper day/night system)
    float CurrentTime = FMath::Fmod(CachedWorld->GetTimeSeconds() / 60.0f, 24.0f); // Convert to hours
    
    // Update blackboard with time-based information
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsFloat(TEXT("CurrentTimeOfDay"), CurrentTime);
        GetBlackboardComponent()->SetValueAsBool(TEXT("IsNight"), 
            CurrentTime < DailyRoutine.MorningStartTime || CurrentTime > DailyRoutine.EveningStartTime);
    }
}

bool ADinosaurAIController::IsTimeForActivity(float StartTime, float EndTime) const
{
    if (!CachedWorld)
        return false;
    
    float CurrentTime = FMath::Fmod(CachedWorld->GetTimeSeconds() / 60.0f, 24.0f);
    
    if (StartTime < EndTime)
    {
        return CurrentTime >= StartTime && CurrentTime <= EndTime;
    }
    else // Activity spans midnight
    {
        return CurrentTime >= StartTime || CurrentTime <= EndTime;
    }
}

void ADinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
            continue;
        
        // Record in memory
        if (MemoryComponent)
        {
            MemoryComponent->RecordSighting(Actor, Actor->GetActorLocation());
        }
        
        // Update blackboard with perceived actors
        if (GetBlackboardComponent())
        {
            // Check if this is a player or threat
            if (Actor->IsA<APawn>() && !Actor->IsA<ADinosaurCharacter>())
            {
                GetBlackboardComponent()->SetValueAsObject(TEXT("PlayerTarget"), Actor);
                GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownPlayerLocation"), Actor->GetActorLocation());
            }
        }
    }
}

void ADinosaurAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;
    
    if (Stimulus.WasSuccessfullySensed())
    {
        // Actor was detected
        if (MemoryComponent)
        {
            MemoryComponent->RecordSighting(Actor, Stimulus.StimulusLocation);
        }
        
        // React based on personality and actor type
        if (Actor->IsA<APawn>() && !Actor->IsA<ADinosaurCharacter>())
        {
            // Player detected - react based on personality
            switch (Personality)
            {
                case EDinosaurPersonality::Aggressive:
                    if (FearLevel < 0.3f)
                    {
                        SetBehaviorState(EDinosaurBehaviorState::Territorial);
                    }
                    break;
                    
                case EDinosaurPersonality::Timid:
                    SetBehaviorState(EDinosaurBehaviorState::Fleeing);
                    CurrentNeeds.Safety = FMath::Max(0.0f, CurrentNeeds.Safety - 30.0f);
                    break;
                    
                case EDinosaurPersonality::Curious:
                    if (CuriosityLevel > 0.6f && CurrentNeeds.Safety > 50.0f)
                    {
                        // Approach cautiously
                        CurrentTarget = Actor;
                    }
                    break;
            }
        }
    }
    else
    {
        // Actor was lost from perception
        if (CurrentTarget == Actor)
        {
            CurrentTarget = nullptr;
        }
    }
}

void ADinosaurAIController::InitiateSocialInteraction(AActor* OtherDinosaur)
{
    if (!CanInteractWith(OtherDinosaur))
        return;
    
    if (SocialComponent)
    {
        SocialComponent->BeginInteraction(OtherDinosaur);
    }
    
    SetBehaviorState(EDinosaurBehaviorState::Socializing);
    CurrentTarget = OtherDinosaur;
}

bool ADinosaurAIController::CanInteractWith(AActor* OtherActor) const
{
    if (!OtherActor || !OtherActor->IsA<ADinosaurCharacter>())
        return false;
    
    // Check distance
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), OtherActor->GetActorLocation());
    if (Distance > 500.0f) // Interaction range
        return false;
    
    // Check social needs and personality
    if (Personality == EDinosaurPersonality::Solitary && CurrentNeeds.Social > 80.0f)
        return false;
    
    return true;
}

bool ADinosaurAIController::HasMemoryOf(AActor* Actor) const
{
    return MemoryComponent ? MemoryComponent->HasMemoryOf(Actor) : false;
}

void ADinosaurAIController::RememberLocation(FVector Location, const FString& LocationType)
{
    if (MemoryComponent)
    {
        MemoryComponent->RecordLocation(Location, LocationType);
    }
}

FVector ADinosaurAIController::RecallLocationOfType(const FString& LocationType) const
{
    return MemoryComponent ? MemoryComponent->RecallLocationOfType(LocationType) : FVector::ZeroVector;
}

void ADinosaurAIController::DebugDrawBehaviorInfo() const
{
    if (!GetPawn() || !CachedWorld)
        return;
    
    FVector PawnLocation = GetPawn()->GetActorLocation();
    
    // Draw behavior state
    FString StateText = FString::Printf(TEXT("State: %d"), static_cast<int32>(CurrentBehaviorState));
    DrawDebugString(CachedWorld, PawnLocation + FVector(0, 0, 200), StateText, nullptr, FColor::White, 0.0f);
    
    // Draw needs
    FString NeedsText = FString::Printf(TEXT("H:%.0f T:%.0f E:%.0f S:%.0f"), 
        CurrentNeeds.Hunger, CurrentNeeds.Thirst, CurrentNeeds.Energy, CurrentNeeds.Safety);
    DrawDebugString(CachedWorld, PawnLocation + FVector(0, 0, 180), NeedsText, nullptr, FColor::Yellow, 0.0f);
    
    // Draw perception radius
    DrawDebugCircle(CachedWorld, PawnLocation, SightRadius, 32, FColor::Green, false, -1.0f, 0, 2.0f, FVector(0, 1, 0), FVector(1, 0, 0));
}

void ADinosaurAIController::InitializePerception()
{
    if (!AIPerceptionComponent)
        return;
    
    // Configure sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure damage sensing
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(2.0f);
    
    // Add configurations to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnTargetPerceptionUpdated);
}

void ADinosaurAIController::InitializeBehaviorTree()
{
    if (BehaviorTree && BlackboardAsset)
    {
        UseBlackboard(BlackboardAsset);
        RunBehaviorTree(BehaviorTree);
    }
}

void ADinosaurAIController::UpdateBlackboard()
{
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp)
        return;
    
    // Update current state
    BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
    BlackboardComp->SetValueAsFloat(TEXT("StateTimer"), StateTimer);
    
    // Update personality traits
    BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
    BlackboardComp->SetValueAsFloat(TEXT("CuriosityLevel"), CuriosityLevel);
    BlackboardComp->SetValueAsFloat(TEXT("FearLevel"), FearLevel);
    
    // Update current target
    BlackboardComp->SetValueAsObject(TEXT("CurrentTarget"), CurrentTarget);
    
    // Update destination
    if (CurrentDestination != FVector::ZeroVector)
    {
        BlackboardComp->SetValueAsVector(TEXT("CurrentDestination"), CurrentDestination);
    }
    
    // Update routine locations
    if (DailyRoutine.FeedingAreas.Num() > 0)
    {
        FVector NearestFeeding = GetNearestLocationOfType(DailyRoutine.FeedingAreas);
        BlackboardComp->SetValueAsVector(TEXT("NearestFeedingArea"), NearestFeeding);
    }
    
    if (DailyRoutine.WaterSources.Num() > 0)
    {
        FVector NearestWater = GetNearestLocationOfType(DailyRoutine.WaterSources);
        BlackboardComp->SetValueAsVector(TEXT("NearestWaterSource"), NearestWater);
    }
}

void ADinosaurAIController::ProcessEmergentBehavior()
{
    // Check for critical needs that override current behavior
    if (IsNeedCritical(15.0f))
    {
        if (CurrentNeeds.Thirst < 15.0f && CurrentBehaviorState != EDinosaurBehaviorState::Drinking)
        {
            SetBehaviorState(EDinosaurBehaviorState::Drinking);
            return;
        }
        
        if (CurrentNeeds.Hunger < 15.0f && CurrentBehaviorState != EDinosaurBehaviorState::Foraging)
        {
            SetBehaviorState(EDinosaurBehaviorState::Foraging);
            return;
        }
        
        if (CurrentNeeds.Energy < 10.0f && CurrentBehaviorState != EDinosaurBehaviorState::Resting)
        {
            SetBehaviorState(EDinosaurBehaviorState::Resting);
            return;
        }
    }
    
    // Check for social opportunities
    if (Personality == EDinosaurPersonality::Social && CurrentNeeds.Social < 30.0f)
    {
        // Look for nearby dinosaurs to socialize with
        if (SocialComponent)
        {
            AActor* NearbyDinosaur = SocialComponent->FindNearbyDinosaur();
            if (NearbyDinosaur && CanInteractWith(NearbyDinosaur))
            {
                InitiateSocialInteraction(NearbyDinosaur);
            }
        }
    }
}