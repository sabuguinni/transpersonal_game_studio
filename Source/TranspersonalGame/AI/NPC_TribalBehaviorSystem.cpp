#include "NPC_TribalBehaviorSystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "AI/BehaviorTree/BlackboardComponent.h"
#include "AI/BehaviorTree/BehaviorTreeComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"

UNPC_TribalBehaviorSystem::UNPC_TribalBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default values
    TribalRole = ENPC_TribalRole::Gatherer;
    CurrentActivity = ENPC_TribalActivity::Idle;
    
    PatrolRadius = 1000.0f;
    DangerDetectionRadius = 1500.0f;
    SocialInteractionRadius = 300.0f;
    ResourceSearchRadius = 800.0f;
    
    LastDecisionTime = 0.0f;
    DecisionInterval = 2.0f;
    
    TribalSettlementCenter = FVector(2000.0f, 2000.0f, 200.0f);
}

void UNPC_TribalBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeTribalBehavior();
}

void UNPC_TribalBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateTribalStats(DeltaTime);
    UpdateMemory(DeltaTime);
    
    // Process decisions at intervals
    if (GetWorld()->GetTimeSeconds() - LastDecisionTime >= DecisionInterval)
    {
        ProcessTribalDecisions();
        LastDecisionTime = GetWorld()->GetTimeSeconds();
    }
    
    ProcessRoleSpecificBehavior();
    UpdateBlackboardValues();
}

void UNPC_TribalBehaviorSystem::InitializeTribalBehavior()
{
    // Find tribal chief if not set
    if (!TribalChief)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (Actor->GetName().Contains("Chief"))
            {
                TribalChief = Actor;
                break;
            }
        }
    }
    
    // Find other tribe members
    TArray<AActor*> AllCharacters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), AllCharacters);
    
    for (AActor* Character : AllCharacters)
    {
        if (Character != GetOwner() && Character->GetName().Contains("Tribal"))
        {
            TribeMembers.AddUnique(Character);
            TribalMemory.KnownTribeMembers.AddUnique(Character);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Tribal NPC initialized: Role=%d, TribeMembers=%d"), 
           (int32)TribalRole, TribeMembers.Num());
}

void UNPC_TribalBehaviorSystem::SetTribalRole(ENPC_TribalRole NewRole)
{
    TribalRole = NewRole;
    
    // Adjust stats based on role
    switch (TribalRole)
    {
        case ENPC_TribalRole::Chief:
            TribalStats.Loyalty = 100.0f;
            PatrolRadius = 1500.0f;
            break;
        case ENPC_TribalRole::Hunter:
            TribalStats.Energy = 90.0f;
            DangerDetectionRadius = 2000.0f;
            break;
        case ENPC_TribalRole::Gatherer:
            ResourceSearchRadius = 1200.0f;
            break;
        case ENPC_TribalRole::Scout:
            PatrolRadius = 2000.0f;
            DangerDetectionRadius = 2500.0f;
            break;
        default:
            break;
    }
}

void UNPC_TribalBehaviorSystem::SetCurrentActivity(ENPC_TribalActivity NewActivity)
{
    if (CurrentActivity != NewActivity)
    {
        CurrentActivity = NewActivity;
        UE_LOG(LogTemp, Log, TEXT("NPC %s changed activity to %d"), 
               *GetOwner()->GetName(), (int32)CurrentActivity);
    }
}

void UNPC_TribalBehaviorSystem::UpdateTribalStats(float DeltaTime)
{
    // Gradual stat changes over time
    TribalStats.Hunger = FMath::Clamp(TribalStats.Hunger + (DeltaTime * 2.0f), 0.0f, 100.0f);
    TribalStats.Thirst = FMath::Clamp(TribalStats.Thirst + (DeltaTime * 3.0f), 0.0f, 100.0f);
    
    // Energy decreases with activity
    float EnergyDrain = 1.0f;
    if (CurrentActivity == ENPC_TribalActivity::Hunting || CurrentActivity == ENPC_TribalActivity::Fleeing)
    {
        EnergyDrain = 4.0f;
    }
    else if (CurrentActivity == ENPC_TribalActivity::Patrolling || CurrentActivity == ENPC_TribalActivity::Gathering)
    {
        EnergyDrain = 2.0f;
    }
    
    TribalStats.Energy = FMath::Clamp(TribalStats.Energy - (DeltaTime * EnergyDrain), 0.0f, 100.0f);
    
    // Fear decreases over time when safe
    if (CurrentActivity != ENPC_TribalActivity::Fleeing && CurrentActivity != ENPC_TribalActivity::Defending)
    {
        TribalStats.Fear = FMath::Clamp(TribalStats.Fear - (DeltaTime * 5.0f), 0.0f, 100.0f);
    }
}

void UNPC_TribalBehaviorSystem::ProcessTribalDecisions()
{
    ENPC_TribalActivity NewActivity = DetermineNextActivity();
    SetCurrentActivity(NewActivity);
}

ENPC_TribalActivity UNPC_TribalBehaviorSystem::DetermineNextActivity()
{
    // Priority-based decision making
    
    // Highest priority: Flee from danger
    if (TribalStats.Fear > 70.0f)
    {
        return ENPC_TribalActivity::Fleeing;
    }
    
    // High priority: Seek resources when needed
    if (TribalStats.Hunger > 80.0f || TribalStats.Thirst > 85.0f)
    {
        return ENPC_TribalActivity::Gathering;
    }
    
    // Role-specific activities
    switch (TribalRole)
    {
        case ENPC_TribalRole::Chief:
            if (TribalStats.Energy > 50.0f)
                return ENPC_TribalActivity::Socializing;
            break;
            
        case ENPC_TribalRole::Hunter:
            if (TribalStats.Energy > 60.0f && TribalStats.Fear < 30.0f)
                return ENPC_TribalActivity::Hunting;
            break;
            
        case ENPC_TribalRole::Scout:
            if (TribalStats.Energy > 40.0f)
                return ENPC_TribalActivity::Patrolling;
            break;
            
        case ENPC_TribalRole::Gatherer:
            if (TribalStats.Energy > 30.0f)
                return ENPC_TribalActivity::Gathering;
            break;
            
        default:
            break;
    }
    
    // Default to idle if energy is low
    if (TribalStats.Energy < 30.0f)
    {
        return ENPC_TribalActivity::Idle;
    }
    
    return ENPC_TribalActivity::Patrolling;
}

void UNPC_TribalBehaviorSystem::HandleDangerResponse(FVector DangerLocation)
{
    TribalStats.Fear = FMath::Clamp(TribalStats.Fear + 30.0f, 0.0f, 100.0f);
    TribalMemory.KnownDangerLocations.AddUnique(DangerLocation);
    TribalMemory.LastDangerTime = GetWorld()->GetTimeSeconds();
    
    SetCurrentActivity(ENPC_TribalActivity::Fleeing);
    
    UE_LOG(LogTemp, Warning, TEXT("NPC %s detected danger at %s"), 
           *GetOwner()->GetName(), *DangerLocation.ToString());
}

void UNPC_TribalBehaviorSystem::HandleResourceFound(FVector ResourceLocation)
{
    TribalMemory.KnownResourceLocations.AddUnique(ResourceLocation);
    
    // Reduce hunger/thirst when finding resources
    TribalStats.Hunger = FMath::Clamp(TribalStats.Hunger - 20.0f, 0.0f, 100.0f);
    TribalStats.Thirst = FMath::Clamp(TribalStats.Thirst - 25.0f, 0.0f, 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s found resource at %s"), 
           *GetOwner()->GetName(), *ResourceLocation.ToString());
}

void UNPC_TribalBehaviorSystem::HandleSocialInteraction(AActor* OtherTribeMember)
{
    if (OtherTribeMember && TribeMembers.Contains(OtherTribeMember))
    {
        TribalStats.Loyalty = FMath::Clamp(TribalStats.Loyalty + 5.0f, 0.0f, 100.0f);
        TribalStats.Fear = FMath::Clamp(TribalStats.Fear - 10.0f, 0.0f, 100.0f);
        
        UE_LOG(LogTemp, Log, TEXT("NPC %s interacted with %s"), 
               *GetOwner()->GetName(), *OtherTribeMember->GetName());
    }
}

FVector UNPC_TribalBehaviorSystem::FindBestPatrolLocation()
{
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector PatrolTarget = TribalSettlementCenter;
    
    // Add random offset within patrol radius
    float RandomAngle = FMath::RandRange(0.0f, 360.0f);
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    
    PatrolTarget.X += FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance;
    PatrolTarget.Y += FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance;
    
    return PatrolTarget;
}

AActor* UNPC_TribalBehaviorSystem::FindNearestTribeMember()
{
    AActor* NearestMember = nullptr;
    float NearestDistance = SocialInteractionRadius;
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Member : TribeMembers)
    {
        if (Member && Member != GetOwner())
        {
            float Distance = FVector::Dist(OwnerLocation, Member->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestMember = Member;
            }
        }
    }
    
    return NearestMember;
}

bool UNPC_TribalBehaviorSystem::ShouldFleeFromDanger()
{
    return TribalStats.Fear > 60.0f || TribalStats.Energy < 20.0f;
}

bool UNPC_TribalBehaviorSystem::ShouldSeekResources()
{
    return TribalStats.Hunger > 70.0f || TribalStats.Thirst > 75.0f;
}

void UNPC_TribalBehaviorSystem::UpdateMemory(float DeltaTime)
{
    // Clean old danger locations
    float CurrentTime = GetWorld()->GetTimeSeconds();
    TribalMemory.KnownDangerLocations.RemoveAll([CurrentTime, this](const FVector& DangerLoc)
    {
        return (CurrentTime - TribalMemory.LastDangerTime) > 300.0f; // 5 minutes
    });
    
    // Update last known safe location if currently safe
    if (TribalStats.Fear < 20.0f)
    {
        TribalMemory.LastKnownSafeLocation = GetOwner()->GetActorLocation();
    }
}

void UNPC_TribalBehaviorSystem::ProcessRoleSpecificBehavior()
{
    switch (TribalRole)
    {
        case ENPC_TribalRole::Chief:
            // Chiefs should stay near settlement center
            if (FVector::Dist(GetOwner()->GetActorLocation(), TribalSettlementCenter) > PatrolRadius)
            {
                SetCurrentActivity(ENPC_TribalActivity::Patrolling);
            }
            break;
            
        case ENPC_TribalRole::Scout:
            // Scouts should patrol wider areas and detect dangers
            if (CurrentActivity == ENPC_TribalActivity::Patrolling)
            {
                // Extended patrol behavior for scouts
            }
            break;
            
        default:
            break;
    }
}

void UNPC_TribalBehaviorSystem::UpdateBlackboardValues()
{
    if (TribalBlackboard)
    {
        TribalBlackboard->SetValueAsEnum("CurrentActivity", (uint8)CurrentActivity);
        TribalBlackboard->SetValueAsEnum("TribalRole", (uint8)TribalRole);
        TribalBlackboard->SetValueAsFloat("Fear", TribalStats.Fear);
        TribalBlackboard->SetValueAsFloat("Energy", TribalStats.Energy);
        TribalBlackboard->SetValueAsVector("SettlementCenter", TribalSettlementCenter);
        
        if (TribalChief)
        {
            TribalBlackboard->SetValueAsObject("TribalChief", TribalChief);
        }
    }
}