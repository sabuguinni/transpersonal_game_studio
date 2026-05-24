#include "NPC_DinosaurBehaviorManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/Anim_BlendSpaceController.h"
#include "DrawDebugHelpers.h"

UNPC_DinosaurBehaviorManager::UNPC_DinosaurBehaviorManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for behavior updates
    
    // Initialize default stats
    DinosaurStats.Health = 100.0f;
    DinosaurStats.Hunger = 50.0f;
    DinosaurStats.Aggression = 30.0f;
    DinosaurStats.Fear = 10.0f;
    DinosaurStats.MovementSpeed = 400.0f;
    DinosaurStats.DetectionRange = 3000.0f;
    DinosaurStats.AttackRange = 500.0f;
    
    // Initialize memory
    DinosaurMemory.LastPlayerLocation = FVector::ZeroVector;
    DinosaurMemory.LastPlayerSeen = 0.0f;
    DinosaurMemory.HomeLocation = FVector::ZeroVector;
    DinosaurMemory.CurrentPatrolIndex = 0;
    
    // Behavior config
    StateUpdateInterval = 1.0f;
    HungerDecayRate = 5.0f;
    MemoryDuration = 30.0f;
    bIsPackHunter = false;
    
    // Player tracking
    PlayerPawn = nullptr;
    DistanceToPlayer = 0.0f;
    bPlayerInSight = false;
    
    // Animation
    AnimationInstance = nullptr;
    BlendSpaceController = nullptr;
    
    CurrentState = ENPC_DinosaurState::Idle;
    Species = ENPC_DinosaurSpecies::TRex;
}

void UNPC_DinosaurBehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize home location
    if (GetOwner())
    {
        DinosaurMemory.HomeLocation = GetOwner()->GetActorLocation();
    }
    
    // Configure species-specific behavior
    InitializeSpeciesDefaults();
    
    // Initialize patrol points
    InitializePatrolPoints();
    
    // Find player pawn
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Setup animation references
    if (GetOwner())
    {
        if (USkeletalMeshComponent* MeshComp = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
        {
            AnimationInstance = MeshComp->GetAnimInstance();
            if (AnimationInstance)
            {
                BlendSpaceController = Cast<UAnim_BlendSpaceController>(AnimationInstance);
            }
        }
    }
    
    // Start behavior update timer
    GetWorld()->GetTimerManager().SetTimer(BehaviorUpdateTimer, this, &UNPC_DinosaurBehaviorManager::UpdateBehaviorLogic, StateUpdateInterval, true);
    
    // Start hunger update timer
    GetWorld()->GetTimerManager().SetTimer(HungerUpdateTimer, [this]()
    {
        UpdateHungerSystem(HungerDecayRate);
    }, 1.0f, true);
    
    UE_LOG(LogTemp, Warning, TEXT("DinosaurBehaviorManager initialized for %s"), *GetOwner()->GetName());
}

void UNPC_DinosaurBehaviorManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update player tracking
    CheckPlayerProximity();
    
    // Update memory
    UpdateMemory();
    
    // Update animation state
    UpdateAnimationState();
    
    LastBehaviorUpdate += DeltaTime;
}

void UNPC_DinosaurBehaviorManager::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        ENPC_DinosaurState PreviousState = CurrentState;
        CurrentState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("%s state changed from %d to %d"), 
               *GetOwner()->GetName(), 
               (int32)PreviousState, 
               (int32)CurrentState);
        
        // Trigger state-specific initialization
        switch (CurrentState)
        {
            case ENPC_DinosaurState::Patrolling:
                InitializePatrolPoints();
                break;
            case ENPC_DinosaurState::Hunting:
                DinosaurStats.Aggression = FMath::Min(DinosaurStats.Aggression + 20.0f, 100.0f);
                break;
            case ENPC_DinosaurState::Fleeing:
                DinosaurStats.Fear = FMath::Min(DinosaurStats.Fear + 30.0f, 100.0f);
                break;
            case ENPC_DinosaurState::Feeding:
                DinosaurStats.Hunger = FMath::Max(DinosaurStats.Hunger - 25.0f, 0.0f);
                break;
        }
    }
}

void UNPC_DinosaurBehaviorManager::UpdateBehaviorLogic()
{
    if (!GetOwner() || !PlayerPawn)
        return;
    
    // Species-specific behavior updates
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            UpdateTRexBehavior();
            break;
        case ENPC_DinosaurSpecies::Velociraptor:
            UpdateVelociraptorBehavior();
            break;
        case ENPC_DinosaurSpecies::Triceratops:
        case ENPC_DinosaurSpecies::Brachiosaurus:
        case ENPC_DinosaurSpecies::Ankylosaurus:
        case ENPC_DinosaurSpecies::Parasaurolophus:
            UpdateHerbivoreBehavior();
            break;
    }
    
    // State-specific behavior updates
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            UpdateIdleBehavior();
            break;
        case ENPC_DinosaurState::Patrolling:
            UpdatePatrolBehavior();
            break;
        case ENPC_DinosaurState::Hunting:
            UpdateHuntingBehavior();
            break;
    }
}

void UNPC_DinosaurBehaviorManager::CheckPlayerProximity()
{
    if (!PlayerPawn || !GetOwner())
    {
        bPlayerInSight = false;
        DistanceToPlayer = 0.0f;
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    DistanceToPlayer = FVector::Dist(OwnerLocation, PlayerLocation);
    
    // Check if player is within detection range
    if (DistanceToPlayer <= DinosaurStats.DetectionRange)
    {
        // Check line of sight
        bPlayerInSight = LineOfSightToPlayer();
        
        if (bPlayerInSight)
        {
            DinosaurMemory.LastPlayerLocation = PlayerLocation;
            DinosaurMemory.LastPlayerSeen = GetWorld()->GetTimeSeconds();
        }
    }
    else
    {
        bPlayerInSight = false;
    }
}

void UNPC_DinosaurBehaviorManager::UpdatePatrolBehavior()
{
    if (DinosaurMemory.PatrolPoints.Num() == 0)
        return;
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector TargetPoint = DinosaurMemory.PatrolPoints[DinosaurMemory.CurrentPatrolIndex];
    
    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetPoint);
    
    // If close to patrol point, move to next one
    if (DistanceToTarget < 500.0f)
    {
        DinosaurMemory.CurrentPatrolIndex = (DinosaurMemory.CurrentPatrolIndex + 1) % DinosaurMemory.PatrolPoints.Num();
    }
    
    // Check if player detected during patrol
    if (bPlayerInSight && DistanceToPlayer < DinosaurStats.DetectionRange)
    {
        if (Species == ENPC_DinosaurSpecies::TRex || Species == ENPC_DinosaurSpecies::Velociraptor)
        {
            SetDinosaurState(ENPC_DinosaurState::Hunting);
        }
        else
        {
            SetDinosaurState(ENPC_DinosaurState::Fleeing);
        }
    }
}

void UNPC_DinosaurBehaviorManager::UpdateHuntingBehavior()
{
    if (!bPlayerInSight && !IsPlayerRemembered())
    {
        SetDinosaurState(ENPC_DinosaurState::Patrolling);
        return;
    }
    
    FVector TargetLocation = bPlayerInSight ? PlayerPawn->GetActorLocation() : DinosaurMemory.LastPlayerLocation;
    
    // If close enough to attack
    if (DistanceToPlayer < DinosaurStats.AttackRange && bPlayerInSight)
    {
        SetDinosaurState(ENPC_DinosaurState::Aggressive);
        TriggerAnimationEvent("Attack");
    }
    
    // Update aggression based on hunt duration
    DinosaurStats.Aggression = FMath::Min(DinosaurStats.Aggression + 1.0f, 100.0f);
}

void UNPC_DinosaurBehaviorManager::UpdateIdleBehavior()
{
    // Random chance to start patrolling
    if (FMath::RandRange(0.0f, 1.0f) < 0.1f)
    {
        SetDinosaurState(ENPC_DinosaurState::Patrolling);
    }
    
    // Check for player
    if (bPlayerInSight)
    {
        if (Species == ENPC_DinosaurSpecies::TRex || Species == ENPC_DinosaurSpecies::Velociraptor)
        {
            SetDinosaurState(ENPC_DinosaurState::Hunting);
        }
        else if (DistanceToPlayer < 1500.0f)
        {
            SetDinosaurState(ENPC_DinosaurState::Fleeing);
        }
    }
    
    // Check hunger
    if (DinosaurStats.Hunger > 80.0f)
    {
        SetDinosaurState(ENPC_DinosaurState::Feeding);
    }
}

void UNPC_DinosaurBehaviorManager::InitializePatrolPoints()
{
    DinosaurMemory.PatrolPoints.Empty();
    FVector HomeLocation = DinosaurMemory.HomeLocation;
    
    // Create patrol points in a circle around home
    int32 NumPoints = FMath::RandRange(3, 6);
    float PatrolRadius = DinosaurStats.DetectionRange * 0.5f;
    
    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = (2.0f * PI * i) / NumPoints;
        FVector Offset = FVector(FMath::Cos(Angle) * PatrolRadius, FMath::Sin(Angle) * PatrolRadius, 0.0f);
        DinosaurMemory.PatrolPoints.Add(HomeLocation + Offset);
    }
    
    DinosaurMemory.CurrentPatrolIndex = 0;
}

FVector UNPC_DinosaurBehaviorManager::GetNextPatrolPoint()
{
    if (DinosaurMemory.PatrolPoints.Num() == 0)
        return DinosaurMemory.HomeLocation;
    
    return DinosaurMemory.PatrolPoints[DinosaurMemory.CurrentPatrolIndex];
}

void UNPC_DinosaurBehaviorManager::UpdateMemory()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Forget player if not seen for too long
    if (CurrentTime - DinosaurMemory.LastPlayerSeen > MemoryDuration)
    {
        DinosaurMemory.LastPlayerLocation = FVector::ZeroVector;
    }
}

bool UNPC_DinosaurBehaviorManager::IsPlayerRemembered() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - DinosaurMemory.LastPlayerSeen) < MemoryDuration;
}

void UNPC_DinosaurBehaviorManager::ConfigureForSpecies(ENPC_DinosaurSpecies NewSpecies)
{
    Species = NewSpecies;
    InitializeSpeciesDefaults();
}

void UNPC_DinosaurBehaviorManager::UpdateTRexBehavior()
{
    // T-Rex is aggressive and territorial
    if (bPlayerInSight && DistanceToPlayer < DinosaurStats.DetectionRange)
    {
        if (CurrentState != ENPC_DinosaurState::Hunting && CurrentState != ENPC_DinosaurState::Aggressive)
        {
            SetDinosaurState(ENPC_DinosaurState::Hunting);
        }
    }
    
    // T-Rex doesn't flee easily
    DinosaurStats.Fear = FMath::Max(DinosaurStats.Fear - 1.0f, 0.0f);
}

void UNPC_DinosaurBehaviorManager::UpdateVelociraptorBehavior()
{
    // Velociraptors are pack hunters - more aggressive in groups
    if (bIsPackHunter)
    {
        DinosaurStats.Aggression = FMath::Min(DinosaurStats.Aggression + 2.0f, 100.0f);
    }
    
    // Fast and cunning - larger detection range
    DinosaurStats.DetectionRange = 4000.0f;
    DinosaurStats.MovementSpeed = 600.0f;
}

void UNPC_DinosaurBehaviorManager::UpdateHerbivoreBehavior()
{
    // Herbivores are generally peaceful but defensive
    if (bPlayerInSight && DistanceToPlayer < 1000.0f)
    {
        if (DinosaurStats.Fear > 50.0f)
        {
            SetDinosaurState(ENPC_DinosaurState::Fleeing);
        }
        else if (DistanceToPlayer < 300.0f)
        {
            // Defensive behavior - stand ground
            SetDinosaurState(ENPC_DinosaurState::Aggressive);
        }
    }
    
    // Herbivores spend more time feeding
    if (DinosaurStats.Hunger > 60.0f && CurrentState == ENPC_DinosaurState::Idle)
    {
        SetDinosaurState(ENPC_DinosaurState::Feeding);
    }
}

void UNPC_DinosaurBehaviorManager::UpdateAnimationState()
{
    if (!BlendSpaceController)
        return;
    
    // Set animation speed based on current state
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            SetAnimationSpeed(0.0f);
            break;
        case ENPC_DinosaurState::Patrolling:
            SetAnimationSpeed(DinosaurStats.MovementSpeed * 0.3f);
            break;
        case ENPC_DinosaurState::Hunting:
            SetAnimationSpeed(DinosaurStats.MovementSpeed * 0.8f);
            break;
        case ENPC_DinosaurState::Fleeing:
            SetAnimationSpeed(DinosaurStats.MovementSpeed);
            break;
        case ENPC_DinosaurState::Feeding:
            SetAnimationSpeed(0.0f);
            TriggerAnimationEvent("Feed");
            break;
        case ENPC_DinosaurState::Aggressive:
            SetAnimationSpeed(0.0f);
            TriggerAnimationEvent("Roar");
            break;
    }
}

void UNPC_DinosaurBehaviorManager::SetAnimationSpeed(float Speed)
{
    if (BlendSpaceController)
    {
        // Integration with blend space controller from Animation Agent
        // This would set the speed parameter in the blend space
    }
}

void UNPC_DinosaurBehaviorManager::TriggerAnimationEvent(const FString& EventName)
{
    UE_LOG(LogTemp, Log, TEXT("%s triggered animation event: %s"), *GetOwner()->GetName(), *EventName);
}

void UNPC_DinosaurBehaviorManager::InitializeSpeciesDefaults()
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            DinosaurStats.Health = 200.0f;
            DinosaurStats.Aggression = 80.0f;
            DinosaurStats.Fear = 5.0f;
            DinosaurStats.MovementSpeed = 500.0f;
            DinosaurStats.DetectionRange = 5000.0f;
            DinosaurStats.AttackRange = 800.0f;
            bIsPackHunter = false;
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            DinosaurStats.Health = 80.0f;
            DinosaurStats.Aggression = 70.0f;
            DinosaurStats.Fear = 20.0f;
            DinosaurStats.MovementSpeed = 700.0f;
            DinosaurStats.DetectionRange = 4000.0f;
            DinosaurStats.AttackRange = 400.0f;
            bIsPackHunter = true;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            DinosaurStats.Health = 180.0f;
            DinosaurStats.Aggression = 30.0f;
            DinosaurStats.Fear = 40.0f;
            DinosaurStats.MovementSpeed = 300.0f;
            DinosaurStats.DetectionRange = 2500.0f;
            DinosaurStats.AttackRange = 600.0f;
            bIsPackHunter = false;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            DinosaurStats.Health = 300.0f;
            DinosaurStats.Aggression = 10.0f;
            DinosaurStats.Fear = 60.0f;
            DinosaurStats.MovementSpeed = 200.0f;
            DinosaurStats.DetectionRange = 3000.0f;
            DinosaurStats.AttackRange = 1000.0f;
            bIsPackHunter = false;
            break;
            
        default:
            // Default herbivore stats
            DinosaurStats.Health = 120.0f;
            DinosaurStats.Aggression = 25.0f;
            DinosaurStats.Fear = 50.0f;
            DinosaurStats.MovementSpeed = 350.0f;
            DinosaurStats.DetectionRange = 2000.0f;
            DinosaurStats.AttackRange = 500.0f;
            bIsPackHunter = false;
            break;
    }
}

void UNPC_DinosaurBehaviorManager::UpdateHungerSystem(float DeltaTime)
{
    DinosaurStats.Hunger = FMath::Min(DinosaurStats.Hunger + DeltaTime, 100.0f);
    
    // Hunger affects behavior
    if (DinosaurStats.Hunger > 80.0f)
    {
        DinosaurStats.Aggression = FMath::Min(DinosaurStats.Aggression + 5.0f, 100.0f);
    }
}

bool UNPC_DinosaurBehaviorManager::LineOfSightToPlayer() const
{
    if (!PlayerPawn || !GetOwner())
        return false;
    
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = PlayerPawn->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(PlayerPawn);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit;
}

FVector UNPC_DinosaurBehaviorManager::CalculateFleeDirection() const
{
    if (!PlayerPawn || !GetOwner())
        return FVector::ZeroVector;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Flee in opposite direction from player
    FVector FleeDirection = (OwnerLocation - PlayerLocation).GetSafeNormal();
    
    // Add some randomness
    FleeDirection += FVector(FMath::RandRange(-0.3f, 0.3f), FMath::RandRange(-0.3f, 0.3f), 0.0f);
    
    return FleeDirection.GetSafeNormal();
}