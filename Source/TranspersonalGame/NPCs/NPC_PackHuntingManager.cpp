#include "NPC_PackHuntingManager.h"
#include "DinosaurBase.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

UNPC_PackHuntingManager::UNPC_PackHuntingManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Pack hunting parameters
    PackCoordinationRadius = 1500.0f;
    FlankingDistance = 800.0f;
    PackAttackCooldown = 3.0f;
    MaxPackSize = 5;
    MinPackSize = 2;
    
    // Hunting states
    CurrentHuntingState = ENPC_PackHuntingState::Patrolling;
    bIsPackLeader = false;
    bHasTarget = false;
    
    // Pack coordination
    PackFormationSpacing = 300.0f;
    PackMovementSpeed = 600.0f;
    LastPackAttackTime = 0.0f;
    
    // Initialize arrays
    PackMembers.Empty();
    FlankingPositions.Empty();
}

void UNPC_PackHuntingManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find nearby pack members on startup
    FindNearbyPackMembers();
    
    // Determine pack leadership
    DeterminePackLeadership();
    
    // Initialize hunting behavior
    InitializePackHunting();
}

void UNPC_PackHuntingManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
        return;
    
    // Update pack hunting behavior
    UpdatePackHuntingBehavior(DeltaTime);
    
    // Coordinate with pack members
    CoordinatePackMovement(DeltaTime);
    
    // Check for targets
    UpdateTargetDetection(DeltaTime);
    
    // Execute hunting strategy
    ExecuteHuntingStrategy(DeltaTime);
}

void UNPC_PackHuntingManager::FindNearbyPackMembers()
{
    if (!GetOwner())
        return;
    
    UWorld* World = GetOwner()->GetWorld();
    if (!World)
        return;
    
    // Clear existing pack
    PackMembers.Empty();
    
    // Find all dinosaurs in range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADinosaurBase::StaticClass(), FoundActors);
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner())
            continue;
        
        ADinosaurBase* Dinosaur = Cast<ADinosaurBase>(Actor);
        if (!Dinosaur)
            continue;
        
        // Check if it's the same species (simplified check)
        if (Dinosaur->GetClass() != GetOwner()->GetClass())
            continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= PackCoordinationRadius)
        {
            PackMembers.Add(Dinosaur);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Pack hunting: Found %d pack members"), PackMembers.Num());
}

void UNPC_PackHuntingManager::DeterminePackLeadership()
{
    if (PackMembers.Num() < MinPackSize)
    {
        bIsPackLeader = false;
        return;
    }
    
    // Simple leadership determination: largest pack member becomes leader
    ADinosaurBase* OwnerDinosaur = Cast<ADinosaurBase>(GetOwner());
    if (!OwnerDinosaur)
        return;
    
    float OwnerHealth = OwnerDinosaur->GetHealth();
    bool bShouldBeLeader = true;
    
    for (ADinosaurBase* Member : PackMembers)
    {
        if (Member && Member->GetHealth() > OwnerHealth)
        {
            bShouldBeLeader = false;
            break;
        }
    }
    
    bIsPackLeader = bShouldBeLeader;
    
    if (bIsPackLeader)
    {
        UE_LOG(LogTemp, Warning, TEXT("Pack hunting: %s is now pack leader"), *GetOwner()->GetName());
    }
}

void UNPC_PackHuntingManager::InitializePackHunting()
{
    if (PackMembers.Num() >= MinPackSize)
    {
        CurrentHuntingState = ENPC_PackHuntingState::Patrolling;
        
        if (bIsPackLeader)
        {
            // Set up flanking positions
            CalculateFlankingPositions();
        }
    }
    else
    {
        CurrentHuntingState = ENPC_PackHuntingState::Patrolling;
    }
}

void UNPC_PackHuntingManager::UpdatePackHuntingBehavior(float DeltaTime)
{
    switch (CurrentHuntingState)
    {
        case ENPC_PackHuntingState::Patrolling:
            ExecutePatrolBehavior(DeltaTime);
            break;
            
        case ENPC_PackHuntingState::Stalking:
            ExecuteStalkingBehavior(DeltaTime);
            break;
            
        case ENPC_PackHuntingState::Flanking:
            ExecuteFlankingBehavior(DeltaTime);
            break;
            
        case ENPC_PackHuntingState::Attacking:
            ExecuteAttackBehavior(DeltaTime);
            break;
            
        case ENPC_PackHuntingState::Retreating:
            ExecuteRetreatBehavior(DeltaTime);
            break;
    }
}

void UNPC_PackHuntingManager::ExecutePatrolBehavior(float DeltaTime)
{
    // Simple patrol behavior - move in formation
    if (bIsPackLeader && PackMembers.Num() > 0)
    {
        // Lead the pack in a patrol pattern
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        FVector PatrolDirection = GetOwner()->GetActorForwardVector();
        
        // Broadcast patrol direction to pack members
        for (ADinosaurBase* Member : PackMembers)
        {
            if (Member)
            {
                UNPC_PackHuntingManager* MemberPackManager = Member->FindComponentByClass<UNPC_PackHuntingManager>();
                if (MemberPackManager)
                {
                    MemberPackManager->FollowPackLeader(CurrentLocation, PatrolDirection);
                }
            }
        }
    }
}

void UNPC_PackHuntingManager::ExecuteStalkingBehavior(float DeltaTime)
{
    if (!CurrentTarget)
    {
        CurrentHuntingState = ENPC_PackHuntingState::Patrolling;
        return;
    }
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    float DistanceToTarget = FVector::Dist(OwnerLocation, TargetLocation);
    
    // If close enough, switch to flanking
    if (DistanceToTarget <= FlankingDistance * 1.5f && PackMembers.Num() >= MinPackSize)
    {
        CurrentHuntingState = ENPC_PackHuntingState::Flanking;
        
        if (bIsPackLeader)
        {
            InitiatePackFlanking();
        }
    }
}

void UNPC_PackHuntingManager::ExecuteFlankingBehavior(float DeltaTime)
{
    if (!CurrentTarget)
    {
        CurrentHuntingState = ENPC_PackHuntingState::Patrolling;
        return;
    }
    
    // Move to assigned flanking position
    if (AssignedFlankingPosition != FVector::ZeroVector)
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        FVector DirectionToFlanking = (AssignedFlankingPosition - OwnerLocation).GetSafeNormal();
        
        // Move towards flanking position
        GetOwner()->SetActorLocation(OwnerLocation + DirectionToFlanking * PackMovementSpeed * DeltaTime);
        
        // Check if in position for attack
        float DistanceToFlankingPos = FVector::Dist(OwnerLocation, AssignedFlankingPosition);
        if (DistanceToFlankingPos <= 200.0f)
        {
            // Ready to attack
            CurrentHuntingState = ENPC_PackHuntingState::Attacking;
        }
    }
}

void UNPC_PackHuntingManager::ExecuteAttackBehavior(float DeltaTime)
{
    if (!CurrentTarget)
    {
        CurrentHuntingState = ENPC_PackHuntingState::Patrolling;
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastPackAttackTime >= PackAttackCooldown)
    {
        // Execute coordinated attack
        PerformPackAttack();
        LastPackAttackTime = CurrentTime;
    }
}

void UNPC_PackHuntingManager::ExecuteRetreatBehavior(float DeltaTime)
{
    // Move away from target
    if (CurrentTarget)
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        FVector TargetLocation = CurrentTarget->GetActorLocation();
        FVector RetreatDirection = (OwnerLocation - TargetLocation).GetSafeNormal();
        
        GetOwner()->SetActorLocation(OwnerLocation + RetreatDirection * PackMovementSpeed * DeltaTime);
        
        // Check if far enough to resume patrolling
        float DistanceToTarget = FVector::Dist(OwnerLocation, TargetLocation);
        if (DistanceToTarget >= PackCoordinationRadius)
        {
            CurrentHuntingState = ENPC_PackHuntingState::Patrolling;
            CurrentTarget = nullptr;
            bHasTarget = false;
        }
    }
}

void UNPC_PackHuntingManager::CoordinatePackMovement(float DeltaTime)
{
    if (!bIsPackLeader || PackMembers.Num() == 0)
        return;
    
    // Maintain pack formation
    FVector LeaderLocation = GetOwner()->GetActorLocation();
    
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (!PackMembers[i])
            continue;
        
        UNPC_PackHuntingManager* MemberPackManager = PackMembers[i]->FindComponentByClass<UNPC_PackHuntingManager>();
        if (MemberPackManager)
        {
            FVector FormationPosition = CalculateFormationPosition(i, LeaderLocation);
            MemberPackManager->MoveToFormationPosition(FormationPosition);
        }
    }
}

void UNPC_PackHuntingManager::UpdateTargetDetection(float DeltaTime)
{
    if (!GetOwner())
        return;
    
    UWorld* World = GetOwner()->GetWorld();
    if (!World)
        return;
    
    // Look for player character
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerCharacter)
        return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    float DistanceToPlayer = FVector::Dist(OwnerLocation, PlayerLocation);
    
    ADinosaurBase* OwnerDinosaur = Cast<ADinosaurBase>(GetOwner());
    if (!OwnerDinosaur)
        return;
    
    float DetectionRadius = OwnerDinosaur->GetDetectionRadius();
    
    if (DistanceToPlayer <= DetectionRadius)
    {
        if (!bHasTarget)
        {
            // New target detected
            CurrentTarget = PlayerCharacter;
            bHasTarget = true;
            CurrentHuntingState = ENPC_PackHuntingState::Stalking;
            
            // Alert pack members
            AlertPackMembers(PlayerCharacter);
        }
    }
    else if (bHasTarget && DistanceToPlayer > DetectionRadius * 1.5f)
    {
        // Lost target
        CurrentTarget = nullptr;
        bHasTarget = false;
        CurrentHuntingState = ENPC_PackHuntingState::Patrolling;
    }
}

void UNPC_PackHuntingManager::ExecuteHuntingStrategy(float DeltaTime)
{
    if (!bHasTarget || !CurrentTarget)
        return;
    
    // Implement pack hunting strategy based on pack size
    if (PackMembers.Num() >= 3 && bIsPackLeader)
    {
        // Large pack - use coordinated flanking
        if (CurrentHuntingState == ENPC_PackHuntingState::Stalking)
        {
            InitiatePackFlanking();
        }
    }
    else if (PackMembers.Num() >= 1)
    {
        // Small pack - use simple coordination
        CoordinateSimpleAttack();
    }
}

void UNPC_PackHuntingManager::CalculateFlankingPositions()
{
    if (!CurrentTarget || PackMembers.Num() == 0)
        return;
    
    FlankingPositions.Empty();
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // Calculate flanking positions around the target
    float AngleStep = 360.0f / (PackMembers.Num() + 1); // +1 for the leader
    
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        float Angle = AngleStep * (i + 1);
        FVector FlankingDirection = DirectionToTarget.RotateAngleAxis(Angle, FVector::UpVector);
        FVector FlankingPosition = TargetLocation + FlankingDirection * FlankingDistance;
        
        FlankingPositions.Add(FlankingPosition);
    }
}

void UNPC_PackHuntingManager::InitiatePackFlanking()
{
    if (!bIsPackLeader || PackMembers.Num() == 0)
        return;
    
    CalculateFlankingPositions();
    
    // Assign flanking positions to pack members
    for (int32 i = 0; i < PackMembers.Num() && i < FlankingPositions.Num(); i++)
    {
        if (PackMembers[i])
        {
            UNPC_PackHuntingManager* MemberPackManager = PackMembers[i]->FindComponentByClass<UNPC_PackHuntingManager>();
            if (MemberPackManager)
            {
                MemberPackManager->AssignFlankingPosition(FlankingPositions[i]);
                MemberPackManager->SetHuntingState(ENPC_PackHuntingState::Flanking);
            }
        }
    }
    
    // Leader takes direct approach
    CurrentHuntingState = ENPC_PackHuntingState::Flanking;
    AssignedFlankingPosition = CurrentTarget->GetActorLocation() + GetOwner()->GetActorForwardVector() * FlankingDistance * 0.5f;
}

void UNPC_PackHuntingManager::PerformPackAttack()
{
    if (!CurrentTarget)
        return;
    
    // Execute attack on target
    ADinosaurBase* OwnerDinosaur = Cast<ADinosaurBase>(GetOwner());
    if (OwnerDinosaur)
    {
        // Perform attack logic here
        float Damage = OwnerDinosaur->GetAttackDamage();
        
        // Apply damage to target if it's a character
        ACharacter* TargetCharacter = Cast<ACharacter>(CurrentTarget);
        if (TargetCharacter)
        {
            // Damage application would go here
            UE_LOG(LogTemp, Warning, TEXT("Pack attack: %s attacks %s for %f damage"), 
                   *GetOwner()->GetName(), *CurrentTarget->GetName(), Damage);
        }
    }
}

void UNPC_PackHuntingManager::AlertPackMembers(AActor* Target)
{
    for (ADinosaurBase* Member : PackMembers)
    {
        if (Member)
        {
            UNPC_PackHuntingManager* MemberPackManager = Member->FindComponentByClass<UNPC_PackHuntingManager>();
            if (MemberPackManager)
            {
                MemberPackManager->SetTarget(Target);
                MemberPackManager->SetHuntingState(ENPC_PackHuntingState::Stalking);
            }
        }
    }
}

void UNPC_PackHuntingManager::FollowPackLeader(const FVector& LeaderLocation, const FVector& Direction)
{
    if (bIsPackLeader)
        return;
    
    // Follow the pack leader
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector TargetPosition = LeaderLocation - Direction * PackFormationSpacing;
    FVector MoveDirection = (TargetPosition - OwnerLocation).GetSafeNormal();
    
    GetOwner()->SetActorLocation(OwnerLocation + MoveDirection * PackMovementSpeed * GetWorld()->GetDeltaSeconds());
}

FVector UNPC_PackHuntingManager::CalculateFormationPosition(int32 MemberIndex, const FVector& LeaderLocation)
{
    // Calculate formation position relative to leader
    float Angle = (360.0f / PackMembers.Num()) * MemberIndex;
    FVector Offset = FVector(FMath::Cos(FMath::DegreesToRadians(Angle)), FMath::Sin(FMath::DegreesToRadians(Angle)), 0.0f);
    Offset *= PackFormationSpacing;
    
    return LeaderLocation + Offset;
}

void UNPC_PackHuntingManager::MoveToFormationPosition(const FVector& FormationPosition)
{
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector Direction = (FormationPosition - OwnerLocation).GetSafeNormal();
    
    GetOwner()->SetActorLocation(OwnerLocation + Direction * PackMovementSpeed * GetWorld()->GetDeltaSeconds());
}

void UNPC_PackHuntingManager::CoordinateSimpleAttack()
{
    // Simple attack coordination for small packs
    if (PackMembers.Num() > 0 && bIsPackLeader)
    {
        for (ADinosaurBase* Member : PackMembers)
        {
            if (Member)
            {
                UNPC_PackHuntingManager* MemberPackManager = Member->FindComponentByClass<UNPC_PackHuntingManager>();
                if (MemberPackManager)
                {
                    MemberPackManager->SetHuntingState(ENPC_PackHuntingState::Attacking);
                }
            }
        }
    }
}

void UNPC_PackHuntingManager::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    bHasTarget = (NewTarget != nullptr);
}

void UNPC_PackHuntingManager::SetHuntingState(ENPC_PackHuntingState NewState)
{
    CurrentHuntingState = NewState;
}

void UNPC_PackHuntingManager::AssignFlankingPosition(const FVector& Position)
{
    AssignedFlankingPosition = Position;
}