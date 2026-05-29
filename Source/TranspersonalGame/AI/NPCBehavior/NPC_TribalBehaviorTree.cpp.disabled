#include "NPC_TribalBehaviorTree.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UNPC_TribalBehaviorTree::UNPC_TribalBehaviorTree()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    TribalRole = ENPC_TribalRole::Gatherer;
    CurrentActivity = ENPC_TribalActivity::Idle;
    ActivityChangeInterval = 30.0f;
    DangerDetectionRadius = 1500.0f;
    SocialInteractionRadius = 800.0f;
    NeedDecayRate = 1.0f;
    
    TimeSinceLastActivityChange = 0.0f;
    TimeSinceLastNeedUpdate = 0.0f;
    bIsInitialized = false;
    LastDangerCheckTime = 0.0f;
    CurrentSocialTarget = nullptr;
}

void UNPC_TribalBehaviorTree::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBehaviorTree();
    
    // Initialize needs based on role
    switch (TribalRole)
    {
        case ENPC_TribalRole::Hunter:
            TribalNeeds.Energy = 80.0f;
            TribalNeeds.Safety = 70.0f;
            break;
        case ENPC_TribalRole::Elder:
            TribalNeeds.Social = 80.0f;
            TribalNeeds.Safety = 90.0f;
            break;
        case ENPC_TribalRole::Scout:
            TribalNeeds.Energy = 90.0f;
            TribalNeeds.Safety = 60.0f;
            break;
        default:
            break;
    }
    
    bIsInitialized = true;
}

void UNPC_TribalBehaviorTree::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized)
        return;
    
    UpdateBehaviorState(DeltaTime);
    UpdateNeeds(DeltaTime);
    
    // Check for danger periodically
    LastDangerCheckTime += DeltaTime;
    if (LastDangerCheckTime >= 2.0f)
    {
        if (ShouldFleeFromDanger())
        {
            SetCurrentActivity(ENPC_TribalActivity::FleeingDanger);
        }
        LastDangerCheckTime = 0.0f;
    }
}

void UNPC_TribalBehaviorTree::InitializeBehaviorTree()
{
    AActor* Owner = GetOwner();
    if (!Owner)
        return;
    
    // Try to get or create blackboard component
    BlackboardComponent = Owner->FindComponentByClass<UBlackboardComponent>();
    if (!BlackboardComponent)
    {
        BlackboardComponent = NewObject<UBlackboardComponent>(Owner);
        if (BlackboardComponent)
        {
            Owner->AddInstanceComponent(BlackboardComponent);
        }
    }
    
    if (BlackboardComponent)
    {
        // Initialize blackboard values
        BlackboardComponent->SetValueAsEnum(TEXT("CurrentActivity"), static_cast<uint8>(CurrentActivity));
        BlackboardComponent->SetValueAsEnum(TEXT("TribalRole"), static_cast<uint8>(TribalRole));
        BlackboardComponent->SetValueAsFloat(TEXT("Hunger"), TribalNeeds.Hunger);
        BlackboardComponent->SetValueAsFloat(TEXT("Energy"), TribalNeeds.Energy);
        BlackboardComponent->SetValueAsFloat(TEXT("Safety"), TribalNeeds.Safety);
    }
}

void UNPC_TribalBehaviorTree::UpdateBehaviorState(float DeltaTime)
{
    TimeSinceLastActivityChange += DeltaTime;
    
    // Check if we should change activity
    if (TimeSinceLastActivityChange >= ActivityChangeInterval)
    {
        ENPC_TribalActivity NewActivity = DetermineNextActivity();
        if (NewActivity != CurrentActivity)
        {
            SetCurrentActivity(NewActivity);
        }
        TimeSinceLastActivityChange = 0.0f;
    }
    
    // Update blackboard with current state
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("CurrentActivity"), static_cast<uint8>(CurrentActivity));
        BlackboardComponent->SetValueAsFloat(TEXT("Hunger"), TribalNeeds.Hunger);
        BlackboardComponent->SetValueAsFloat(TEXT("Energy"), TribalNeeds.Energy);
        BlackboardComponent->SetValueAsFloat(TEXT("Safety"), TribalNeeds.Safety);
        BlackboardComponent->SetValueAsFloat(TEXT("Social"), TribalNeeds.Social);
    }
}

void UNPC_TribalBehaviorTree::SetCurrentActivity(ENPC_TribalActivity NewActivity)
{
    if (CurrentActivity == NewActivity)
        return;
    
    CurrentActivity = NewActivity;
    TimeSinceLastActivityChange = 0.0f;
    
    // Log activity change for debugging
    FString ActivityName = TEXT("Unknown");
    switch (NewActivity)
    {
        case ENPC_TribalActivity::Gathering: ActivityName = TEXT("Gathering"); break;
        case ENPC_TribalActivity::Hunting: ActivityName = TEXT("Hunting"); break;
        case ENPC_TribalActivity::Patrolling: ActivityName = TEXT("Patrolling"); break;
        case ENPC_TribalActivity::SocialInteraction: ActivityName = TEXT("Social"); break;
        case ENPC_TribalActivity::Sleeping: ActivityName = TEXT("Sleeping"); break;
        case ENPC_TribalActivity::FleeingDanger: ActivityName = TEXT("Fleeing"); break;
        default: ActivityName = TEXT("Idle"); break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal NPC changed activity to: %s"), *ActivityName);
}

ENPC_TribalActivity UNPC_TribalBehaviorTree::DetermineNextActivity()
{
    // Check for immediate danger first
    if (ShouldFleeFromDanger())
    {
        return ENPC_TribalActivity::FleeingDanger;
    }
    
    // Check if it's night time
    if (IsNightTime())
    {
        if (TribalNeeds.Energy < 30.0f)
        {
            return ENPC_TribalActivity::Sleeping;
        }
        else
        {
            return ENPC_TribalActivity::GuardDuty;
        }
    }
    
    // Determine activity based on most urgent need
    ENPC_TribalActivity UrgentNeed = GetMostUrgentNeed();
    if (UrgentNeed != ENPC_TribalActivity::Idle)
    {
        return UrgentNeed;
    }
    
    // Role-based default activities
    switch (TribalRole)
    {
        case ENPC_TribalRole::Hunter:
            return ENPC_TribalActivity::Hunting;
        case ENPC_TribalRole::Gatherer:
            return ENPC_TribalActivity::Gathering;
        case ENPC_TribalRole::Scout:
            return ENPC_TribalActivity::Patrolling;
        case ENPC_TribalRole::Elder:
            return ENPC_TribalActivity::SocialInteraction;
        case ENPC_TribalRole::Guard:
            return ENPC_TribalActivity::GuardDuty;
        default:
            return ENPC_TribalActivity::Idle;
    }
}

bool UNPC_TribalBehaviorTree::ShouldFleeFromDanger()
{
    AActor* Owner = GetOwner();
    if (!Owner)
        return false;
    
    // Check for dangerous actors nearby
    UWorld* World = Owner->GetWorld();
    if (!World)
        return false;
    
    FVector OwnerLocation = Owner->GetActorLocation();
    
    // Look for dinosaurs or other threats
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == Owner)
            continue;
        
        // Check if this is a known threat
        if (TribalMemory.KnownThreats.Contains(Actor))
        {
            float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
            if (Distance < DangerDetectionRadius)
            {
                RememberDangerLocation(Actor->GetActorLocation());
                return true;
            }
        }
        
        // Check for dinosaur actors (basic name check)
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Raptor")))
        {
            float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
            if (Distance < DangerDetectionRadius)
            {
                RememberThreat(Actor);
                RememberDangerLocation(Actor->GetActorLocation());
                return true;
            }
        }
    }
    
    return false;
}

bool UNPC_TribalBehaviorTree::IsNightTime()
{
    UWorld* World = GetWorld();
    if (!World)
        return false;
    
    // Simple time check - could be enhanced with proper day/night cycle
    float TimeSeconds = World->GetTimeSeconds();
    float DayLength = 1200.0f; // 20 minutes per day
    float TimeOfDay = FMath::Fmod(TimeSeconds, DayLength) / DayLength;
    
    // Night time is between 0.75 and 0.25 (6 PM to 6 AM equivalent)
    return (TimeOfDay > 0.75f || TimeOfDay < 0.25f);
}

void UNPC_TribalBehaviorTree::RememberDangerLocation(FVector Location)
{
    TribalMemory.KnownDangerLocations.AddUnique(Location);
    TribalMemory.LastDangerTime = GetWorld()->GetTimeSeconds();
    
    // Limit memory size
    if (TribalMemory.KnownDangerLocations.Num() > 10)
    {
        TribalMemory.KnownDangerLocations.RemoveAt(0);
    }
}

void UNPC_TribalBehaviorTree::RememberResourceLocation(FVector Location)
{
    TribalMemory.KnownResourceLocations.AddUnique(Location);
    
    // Limit memory size
    if (TribalMemory.KnownResourceLocations.Num() > 15)
    {
        TribalMemory.KnownResourceLocations.RemoveAt(0);
    }
}

void UNPC_TribalBehaviorTree::RememberThreat(AActor* ThreatActor)
{
    if (ThreatActor && !TribalMemory.KnownThreats.Contains(ThreatActor))
    {
        TribalMemory.KnownThreats.Add(ThreatActor);
        
        // Limit memory size
        if (TribalMemory.KnownThreats.Num() > 8)
        {
            TribalMemory.KnownThreats.RemoveAt(0);
        }
    }
}

bool UNPC_TribalBehaviorTree::IsLocationDangerous(FVector Location)
{
    for (const FVector& DangerLoc : TribalMemory.KnownDangerLocations)
    {
        if (FVector::Dist(Location, DangerLoc) < 500.0f)
        {
            return true;
        }
    }
    return false;
}

void UNPC_TribalBehaviorTree::UpdateNeeds(float DeltaTime)
{
    TimeSinceLastNeedUpdate += DeltaTime;
    
    if (TimeSinceLastNeedUpdate < 1.0f)
        return;
    
    // Decay needs over time
    TribalNeeds.Hunger = FMath::Max(0.0f, TribalNeeds.Hunger - (NeedDecayRate * TimeSinceLastNeedUpdate));
    TribalNeeds.Thirst = FMath::Max(0.0f, TribalNeeds.Thirst - (NeedDecayRate * 1.5f * TimeSinceLastNeedUpdate));
    TribalNeeds.Energy = FMath::Max(0.0f, TribalNeeds.Energy - (NeedDecayRate * 0.8f * TimeSinceLastNeedUpdate));
    
    // Safety decreases if in danger
    if (ShouldFleeFromDanger())
    {
        TribalNeeds.Safety = FMath::Max(0.0f, TribalNeeds.Safety - (NeedDecayRate * 5.0f * TimeSinceLastNeedUpdate));
    }
    else
    {
        TribalNeeds.Safety = FMath::Min(100.0f, TribalNeeds.Safety + (NeedDecayRate * 2.0f * TimeSinceLastNeedUpdate));
    }
    
    // Social need decreases when alone
    TArray<AActor*> NearbyTribalMembers = FindNearbyTribalMembers();
    if (NearbyTribalMembers.Num() == 0)
    {
        TribalNeeds.Social = FMath::Max(0.0f, TribalNeeds.Social - (NeedDecayRate * 0.5f * TimeSinceLastNeedUpdate));
    }
    else
    {
        TribalNeeds.Social = FMath::Min(100.0f, TribalNeeds.Social + (NeedDecayRate * 1.0f * TimeSinceLastNeedUpdate));
    }
    
    TimeSinceLastNeedUpdate = 0.0f;
}

ENPC_TribalActivity UNPC_TribalBehaviorTree::GetMostUrgentNeed()
{
    // Critical thresholds
    if (TribalNeeds.Safety < 20.0f)
        return ENPC_TribalActivity::FleeingDanger;
    
    if (TribalNeeds.Energy < 15.0f)
        return ENPC_TribalActivity::Sleeping;
    
    if (TribalNeeds.Thirst < 25.0f)
        return ENPC_TribalActivity::Gathering; // Assume gathering includes water
    
    if (TribalNeeds.Hunger < 30.0f)
        return ENPC_TribalActivity::Gathering;
    
    if (TribalNeeds.Social < 20.0f && ShouldInteractSocially())
        return ENPC_TribalActivity::SocialInteraction;
    
    return ENPC_TribalActivity::Idle;
}

void UNPC_TribalBehaviorTree::SatisfyNeed(ENPC_TribalActivity NeedType, float Amount)
{
    switch (NeedType)
    {
        case ENPC_TribalActivity::Gathering:
            TribalNeeds.Hunger = FMath::Min(100.0f, TribalNeeds.Hunger + Amount);
            TribalNeeds.Thirst = FMath::Min(100.0f, TribalNeeds.Thirst + Amount * 0.8f);
            break;
        case ENPC_TribalActivity::Sleeping:
            TribalNeeds.Energy = FMath::Min(100.0f, TribalNeeds.Energy + Amount);
            break;
        case ENPC_TribalActivity::SocialInteraction:
            TribalNeeds.Social = FMath::Min(100.0f, TribalNeeds.Social + Amount);
            break;
        default:
            break;
    }
}

TArray<AActor*> UNPC_TribalBehaviorTree::FindNearbyTribalMembers(float SearchRadius)
{
    TArray<AActor*> NearbyMembers;
    
    AActor* Owner = GetOwner();
    if (!Owner)
        return NearbyMembers;
    
    UWorld* World = Owner->GetWorld();
    if (!World)
        return NearbyMembers;
    
    FVector OwnerLocation = Owner->GetActorLocation();
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == Owner)
            continue;
        
        // Check if this is another tribal NPC
        UNPC_TribalBehaviorTree* OtherTribalBehavior = Actor->FindComponentByClass<UNPC_TribalBehaviorTree>();
        if (OtherTribalBehavior)
        {
            float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                NearbyMembers.Add(Actor);
            }
        }
    }
    
    return NearbyMembers;
}

bool UNPC_TribalBehaviorTree::ShouldInteractSocially()
{
    return (TribalNeeds.Social < 60.0f && !IsNightTime() && TribalNeeds.Safety > 50.0f);
}

AActor* UNPC_TribalBehaviorTree::FindBestSocialTarget()
{
    TArray<AActor*> NearbyMembers = FindNearbyTribalMembers(SocialInteractionRadius);
    
    if (NearbyMembers.Num() == 0)
        return nullptr;
    
    // Prefer elders for social interaction
    for (AActor* Member : NearbyMembers)
    {
        UNPC_TribalBehaviorTree* OtherBehavior = Member->FindComponentByClass<UNPC_TribalBehaviorTree>();
        if (OtherBehavior && OtherBehavior->TribalRole == ENPC_TribalRole::Elder)
        {
            return Member;
        }
    }
    
    // Return first available member
    return NearbyMembers.Num() > 0 ? NearbyMembers[0] : nullptr;
}