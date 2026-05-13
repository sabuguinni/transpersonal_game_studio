#include "NPC_DinosaurBehaviorTree.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

ANPC_DinosaurBehaviorTree::ANPC_DinosaurBehaviorTree()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create mesh component
    DinosaurMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DinosaurMesh"));
    RootComponent = DinosaurMesh;
    DinosaurMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DinosaurMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create detection sphere
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(1500.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create behavior tree component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    
    // Create blackboard component
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Initialize default stats
    DinosaurStats.Health = 100.0f;
    DinosaurStats.MaxHealth = 100.0f;
    DinosaurStats.Hunger = 30.0f;
    DinosaurStats.Thirst = 30.0f;
    DinosaurStats.Stamina = 100.0f;
    DinosaurStats.Fear = 0.0f;
    DinosaurStats.Aggression = 50.0f;
    DinosaurStats.TerritorialRadius = 2000.0f;
    DinosaurStats.DetectionRadius = 1500.0f;
    DinosaurStats.AttackRange = 300.0f;

    // Initialize pack behavior
    PackBehavior.bIsPackAnimal = false;
    PackBehavior.PackSize = 1;
    PackBehavior.PackCohesionRadius = 1000.0f;
    PackBehavior.bIsPackLeader = false;

    // Set default territory center to spawn location
    TerritoryCenter = GetActorLocation();
}

void ANPC_DinosaurBehaviorTree::BeginPlay()
{
    Super::BeginPlay();

    // Initialize territory center
    TerritoryCenter = GetActorLocation();

    // Initialize species-specific behavior
    InitializeSpeciesBehavior();

    // Initialize behavior tree
    InitializeBehaviorTree();

    // Set up detection sphere radius based on species
    if (DetectionSphere)
    {
        DetectionSphere->SetSphereRadius(DinosaurStats.DetectionRadius);
    }

    // Generate initial patrol points around territory
    for (int32 i = 0; i < 4; i++)
    {
        float Angle = (i * 90.0f) * PI / 180.0f;
        FVector PatrolPoint = TerritoryCenter + FVector(
            FMath::Cos(Angle) * DinosaurStats.TerritorialRadius * 0.7f,
            FMath::Sin(Angle) * DinosaurStats.TerritorialRadius * 0.7f,
            0.0f
        );
        PatrolPoints.Add(PatrolPoint);
    }

    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s initialized with species: %d"), 
           *GetName(), (int32)Species);
}

void ANPC_DinosaurBehaviorTree::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive())
    {
        return;
    }

    // Update survival stats
    UpdateHunger(DeltaTime);
    UpdateThirst(DeltaTime);

    // Update behavior state
    UpdateBehaviorState(DeltaTime);

    // Handle state transitions
    HandleStateTransitions();

    // Update behavior tree blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("Health"), DinosaurStats.Health);
        BlackboardComponent->SetValueAsFloat(TEXT("Hunger"), DinosaurStats.Hunger);
        BlackboardComponent->SetValueAsFloat(TEXT("Thirst"), DinosaurStats.Thirst);
        BlackboardComponent->SetValueAsFloat(TEXT("Fear"), DinosaurStats.Fear);
        BlackboardComponent->SetValueAsInt(TEXT("CurrentState"), (int32)CurrentState);
        
        if (CurrentTarget)
        {
            BlackboardComponent->SetValueAsObject(TEXT("Target"), CurrentTarget);
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
        }
    }

    // Debug visualization
    if (GEngine && GEngine->bEnableOnScreenDebugMessages)
    {
        FString StateString = UEnum::GetValueAsString(CurrentState);
        FString DebugString = FString::Printf(TEXT("%s - %s (H:%.1f T:%.1f F:%.1f)"), 
                                            *GetName(), *StateString, 
                                            DinosaurStats.Hunger, DinosaurStats.Thirst, DinosaurStats.Fear);
        
        DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 200), DebugString, 
                       nullptr, FColor::White, 0.0f);
    }
}

void ANPC_DinosaurBehaviorTree::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ANPC_DinosaurBehaviorTree::InitializeSpeciesBehavior()
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            SetupTRexBehavior();
            break;
        case ENPC_DinosaurSpecies::Velociraptor:
            SetupVelociraptorBehavior();
            break;
        case ENPC_DinosaurSpecies::Triceratops:
        case ENPC_DinosaurSpecies::Brachiosaurus:
        case ENPC_DinosaurSpecies::Stegosaurus:
        case ENPC_DinosaurSpecies::Parasaurolophus:
        case ENPC_DinosaurSpecies::Ankylosaurus:
            SetupHerbivoreBehavior();
            break;
        default:
            SetupTRexBehavior(); // Default to predator behavior
            break;
    }
}

void ANPC_DinosaurBehaviorTree::SetupTRexBehavior()
{
    // T-Rex: Apex predator, territorial, solitary
    DinosaurStats.MaxHealth = 200.0f;
    DinosaurStats.Health = 200.0f;
    DinosaurStats.Aggression = 80.0f;
    DinosaurStats.TerritorialRadius = 3000.0f;
    DinosaurStats.DetectionRadius = 2000.0f;
    DinosaurStats.AttackRange = 400.0f;
    
    PackBehavior.bIsPackAnimal = false;
    PackBehavior.PackSize = 1;
    
    CurrentState = ENPC_DinosaurState::Patrolling;
    
    UE_LOG(LogTemp, Warning, TEXT("T-Rex behavior initialized for %s"), *GetName());
}

void ANPC_DinosaurBehaviorTree::SetupVelociraptorBehavior()
{
    // Velociraptor: Pack hunter, intelligent, coordinated
    DinosaurStats.MaxHealth = 80.0f;
    DinosaurStats.Health = 80.0f;
    DinosaurStats.Aggression = 70.0f;
    DinosaurStats.TerritorialRadius = 2000.0f;
    DinosaurStats.DetectionRadius = 1800.0f;
    DinosaurStats.AttackRange = 250.0f;
    
    PackBehavior.bIsPackAnimal = true;
    PackBehavior.PackSize = 3;
    PackBehavior.PackCohesionRadius = 1500.0f;
    
    CurrentState = ENPC_DinosaurState::Patrolling;
    
    UE_LOG(LogTemp, Warning, TEXT("Velociraptor behavior initialized for %s"), *GetName());
}

void ANPC_DinosaurBehaviorTree::SetupHerbivoreBehavior()
{
    // Herbivores: Peaceful, herd animals, flee from threats
    DinosaurStats.MaxHealth = 150.0f;
    DinosaurStats.Health = 150.0f;
    DinosaurStats.Aggression = 20.0f;
    DinosaurStats.TerritorialRadius = 1500.0f;
    DinosaurStats.DetectionRadius = 1200.0f;
    DinosaurStats.AttackRange = 200.0f;
    
    PackBehavior.bIsPackAnimal = true;
    PackBehavior.PackSize = 5;
    PackBehavior.PackCohesionRadius = 800.0f;
    
    CurrentState = ENPC_DinosaurState::Feeding;
    
    UE_LOG(LogTemp, Warning, TEXT("Herbivore behavior initialized for %s"), *GetName());
}

void ANPC_DinosaurBehaviorTree::InitializeBehaviorTree()
{
    if (BehaviorTreeComponent && BlackboardComponent)
    {
        // Initialize blackboard values
        BlackboardComponent->SetValueAsFloat(TEXT("Health"), DinosaurStats.Health);
        BlackboardComponent->SetValueAsFloat(TEXT("MaxHealth"), DinosaurStats.MaxHealth);
        BlackboardComponent->SetValueAsFloat(TEXT("Hunger"), DinosaurStats.Hunger);
        BlackboardComponent->SetValueAsFloat(TEXT("Thirst"), DinosaurStats.Thirst);
        BlackboardComponent->SetValueAsFloat(TEXT("Fear"), DinosaurStats.Fear);
        BlackboardComponent->SetValueAsFloat(TEXT("Aggression"), DinosaurStats.Aggression);
        BlackboardComponent->SetValueAsVector(TEXT("TerritoryCenter"), TerritoryCenter);
        BlackboardComponent->SetValueAsFloat(TEXT("TerritorialRadius"), DinosaurStats.TerritorialRadius);
        BlackboardComponent->SetValueAsInt(TEXT("CurrentState"), (int32)CurrentState);
        
        if (DinosaurBehaviorTree)
        {
            BehaviorTreeComponent->StartTree(*DinosaurBehaviorTree);
        }
    }
}

void ANPC_DinosaurBehaviorTree::UpdateBehaviorState(float DeltaTime)
{
    BehaviorUpdateTimer += DeltaTime;
    
    if (BehaviorUpdateTimer >= 1.0f) // Update behavior every second
    {
        BehaviorUpdateTimer = 0.0f;
        
        switch (CurrentState)
        {
            case ENPC_DinosaurState::Idle:
                ProcessIdleState(DeltaTime);
                break;
            case ENPC_DinosaurState::Patrolling:
                ProcessPatrollingState(DeltaTime);
                break;
            case ENPC_DinosaurState::Hunting:
                ProcessHuntingState(DeltaTime);
                break;
            case ENPC_DinosaurState::Fleeing:
                ProcessFleeingState(DeltaTime);
                break;
            case ENPC_DinosaurState::Feeding:
                ProcessFeedingState(DeltaTime);
                break;
            case ENPC_DinosaurState::Territorial:
                ProcessTerritorialState(DeltaTime);
                break;
        }
    }
}

void ANPC_DinosaurBehaviorTree::ProcessIdleState(float DeltaTime)
{
    // Look for threats or prey
    AActor* Threat = FindNearestThreat();
    AActor* Prey = FindNearestPrey();
    
    if (Threat)
    {
        if (DinosaurStats.Aggression > 60.0f)
        {
            StartHunting(Threat);
        }
        else
        {
            StartFleeing(Threat);
        }
    }
    else if (Prey && IsHungry())
    {
        StartHunting(Prey);
    }
    else if (GetWorld()->GetTimeSeconds() - LastStateChangeTime > 5.0f)
    {
        StartPatrolling();
    }
}

void ANPC_DinosaurBehaviorTree::ProcessPatrollingState(float DeltaTime)
{
    // Check for threats and prey while patrolling
    AActor* Threat = FindNearestThreat();
    AActor* Prey = FindNearestPrey();
    
    if (Threat)
    {
        if (DinosaurStats.Aggression > 60.0f && !IsInTerritory(Threat->GetActorLocation()))
        {
            SetDinosaurState(ENPC_DinosaurState::Territorial);
        }
        else if (DinosaurStats.Aggression <= 40.0f)
        {
            StartFleeing(Threat);
        }
    }
    else if (Prey && IsHungry())
    {
        StartHunting(Prey);
    }
}

void ANPC_DinosaurBehaviorTree::ProcessHuntingState(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        SetDinosaurState(ENPC_DinosaurState::Patrolling);
        return;
    }
    
    float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistanceToTarget <= DinosaurStats.AttackRange)
    {
        AttackTarget(CurrentTarget);
    }
    else if (DistanceToTarget > DinosaurStats.DetectionRadius * 2.0f)
    {
        // Lost target
        CurrentTarget = nullptr;
        SetDinosaurState(ENPC_DinosaurState::Patrolling);
    }
}

void ANPC_DinosaurBehaviorTree::ProcessFleeingState(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        SetDinosaurState(ENPC_DinosaurState::Idle);
        return;
    }
    
    float DistanceToThreat = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistanceToThreat > DinosaurStats.DetectionRadius * 1.5f)
    {
        // Safe distance reached
        CurrentTarget = nullptr;
        DinosaurStats.Fear = FMath::Max(0.0f, DinosaurStats.Fear - 20.0f);
        SetDinosaurState(ENPC_DinosaurState::Idle);
    }
}

void ANPC_DinosaurBehaviorTree::ProcessFeedingState(float DeltaTime)
{
    // Reduce hunger while feeding
    DinosaurStats.Hunger = FMath::Max(0.0f, DinosaurStats.Hunger - 15.0f * DeltaTime);
    
    if (!IsHungry())
    {
        SetDinosaurState(ENPC_DinosaurState::Idle);
    }
    
    // Still check for threats while feeding
    AActor* Threat = FindNearestThreat();
    if (Threat)
    {
        StartFleeing(Threat);
    }
}

void ANPC_DinosaurBehaviorTree::ProcessTerritorialState(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        SetDinosaurState(ENPC_DinosaurState::Patrolling);
        return;
    }
    
    // Chase intruder out of territory
    if (!IsInTerritory(CurrentTarget->GetActorLocation()))
    {
        // Intruder left territory
        CurrentTarget = nullptr;
        SetDinosaurState(ENPC_DinosaurState::Patrolling);
    }
    else
    {
        float DistanceToIntruder = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
        if (DistanceToIntruder <= DinosaurStats.AttackRange)
        {
            AttackTarget(CurrentTarget);
        }
    }
}

void ANPC_DinosaurBehaviorTree::HandleStateTransitions()
{
    // Automatic state transitions based on needs
    if (DinosaurStats.Health <= 20.0f && CurrentState != ENPC_DinosaurState::Fleeing)
    {
        AActor* Threat = FindNearestThreat();
        if (Threat)
        {
            StartFleeing(Threat);
        }
    }
    
    if (IsHungry() && CurrentState == ENPC_DinosaurState::Idle)
    {
        if (Species == ENPC_DinosaurSpecies::TRex || Species == ENPC_DinosaurSpecies::Velociraptor)
        {
            AActor* Prey = FindNearestPrey();
            if (Prey)
            {
                StartHunting(Prey);
            }
        }
        else
        {
            SetDinosaurState(ENPC_DinosaurState::Feeding);
        }
    }
}

void ANPC_DinosaurBehaviorTree::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsInt(TEXT("CurrentState"), (int32)CurrentState);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed state to: %s"), 
               *GetName(), *UEnum::GetValueAsString(CurrentState));
    }
}

void ANPC_DinosaurBehaviorTree::StartHunting(AActor* Target)
{
    if (Target && IsValid(Target))
    {
        CurrentTarget = Target;
        SetDinosaurState(ENPC_DinosaurState::Hunting);
        
        if (PackBehavior.bIsPackAnimal && PackBehavior.bIsPackLeader)
        {
            CoordinatePackHunt(Target);
        }
    }
}

void ANPC_DinosaurBehaviorTree::StartFleeing(AActor* ThreatSource)
{
    if (ThreatSource && IsValid(ThreatSource))
    {
        CurrentTarget = ThreatSource;
        DinosaurStats.Fear = FMath::Min(100.0f, DinosaurStats.Fear + 30.0f);
        SetDinosaurState(ENPC_DinosaurState::Fleeing);
    }
}

void ANPC_DinosaurBehaviorTree::StartPatrolling()
{
    CurrentTarget = nullptr;
    SetDinosaurState(ENPC_DinosaurState::Patrolling);
}

void ANPC_DinosaurBehaviorTree::AttackTarget(AActor* Target)
{
    if (!Target || !IsValid(Target))
    {
        return;
    }
    
    // Calculate damage based on species
    float Damage = 25.0f;
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            Damage = 50.0f;
            break;
        case ENPC_DinosaurSpecies::Velociraptor:
            Damage = 30.0f;
            break;
        default:
            Damage = 20.0f;
            break;
    }
    
    // Apply damage if target is another dinosaur
    if (ANPC_DinosaurBehaviorTree* TargetDinosaur = Cast<ANPC_DinosaurBehaviorTree>(Target))
    {
        TargetDinosaur->TakeDamage(Damage, this);
    }
    
    // Reduce hunger slightly when attacking (energy expenditure)
    DinosaurStats.Hunger = FMath::Min(100.0f, DinosaurStats.Hunger + 5.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s attacked %s for %.1f damage"), 
           *GetName(), *Target->GetName(), Damage);
}

bool ANPC_DinosaurBehaviorTree::IsInTerritory(FVector Location) const
{
    float DistanceFromCenter = FVector::Dist(Location, TerritoryCenter);
    return DistanceFromCenter <= DinosaurStats.TerritorialRadius;
}

AActor* ANPC_DinosaurBehaviorTree::FindNearestThreat() const
{
    AActor* NearestThreat = nullptr;
    float NearestDistance = DinosaurStats.DetectionRadius;
    
    // Find player as potential threat
    if (UWorld* World = GetWorld())
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            float Distance = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestThreat = PC->GetPawn();
                NearestDistance = Distance;
            }
        }
        
        // Find other dinosaurs as potential threats
        TArray<AActor*> FoundDinosaurs;
        UGameplayStatics::GetAllActorsOfClass(World, ANPC_DinosaurBehaviorTree::StaticClass(), FoundDinosaurs);
        
        for (AActor* Actor : FoundDinosaurs)
        {
            if (Actor != this)
            {
                if (ANPC_DinosaurBehaviorTree* OtherDinosaur = Cast<ANPC_DinosaurBehaviorTree>(Actor))
                {
                    // Consider predators as threats to herbivores
                    bool bIsThreat = false;
                    if (Species != ENPC_DinosaurSpecies::TRex && Species != ENPC_DinosaurSpecies::Velociraptor)
                    {
                        if (OtherDinosaur->Species == ENPC_DinosaurSpecies::TRex || 
                            OtherDinosaur->Species == ENPC_DinosaurSpecies::Velociraptor)
                        {
                            bIsThreat = true;
                        }
                    }
                    
                    if (bIsThreat)
                    {
                        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
                        if (Distance < NearestDistance)
                        {
                            NearestThreat = Actor;
                            NearestDistance = Distance;
                        }
                    }
                }
            }
        }
    }
    
    return NearestThreat;
}

AActor* ANPC_DinosaurBehaviorTree::FindNearestPrey() const
{
    if (Species != ENPC_DinosaurSpecies::TRex && Species != ENPC_DinosaurSpecies::Velociraptor)
    {
        return nullptr; // Only predators hunt
    }
    
    AActor* NearestPrey = nullptr;
    float NearestDistance = DinosaurStats.DetectionRadius;
    
    if (UWorld* World = GetWorld())
    {
        // Consider player as prey
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            float Distance = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestPrey = PC->GetPawn();
                NearestDistance = Distance;
            }
        }
        
        // Find herbivore dinosaurs as prey
        TArray<AActor*> FoundDinosaurs;
        UGameplayStatics::GetAllActorsOfClass(World, ANPC_DinosaurBehaviorTree::StaticClass(), FoundDinosaurs);
        
        for (AActor* Actor : FoundDinosaurs)
        {
            if (Actor != this)
            {
                if (ANPC_DinosaurBehaviorTree* OtherDinosaur = Cast<ANPC_DinosaurBehaviorTree>(Actor))
                {
                    // Herbivores are prey for carnivores
                    if (OtherDinosaur->Species != ENPC_DinosaurSpecies::TRex && 
                        OtherDinosaur->Species != ENPC_DinosaurSpecies::Velociraptor)
                    {
                        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
                        if (Distance < NearestDistance)
                        {
                            NearestPrey = Actor;
                            NearestDistance = Distance;
                        }
                    }
                }
            }
        }
    }
    
    return NearestPrey;
}

void ANPC_DinosaurBehaviorTree::JoinPack(ANPC_DinosaurBehaviorTree* PackLeader)
{
    if (PackLeader && PackLeader != this && PackBehavior.bIsPackAnimal)
    {
        PackBehavior.PackLeader = PackLeader;
        PackBehavior.bIsPackLeader = false;
        
        if (PackLeader->PackBehavior.PackMembers.Num() < PackLeader->PackBehavior.PackSize)
        {
            PackLeader->PackBehavior.PackMembers.AddUnique(this);
            UE_LOG(LogTemp, Log, TEXT("Dinosaur %s joined pack led by %s"), 
                   *GetName(), *PackLeader->GetName());
        }
    }
}

void ANPC_DinosaurBehaviorTree::LeavePack()
{
    if (PackBehavior.PackLeader)
    {
        PackBehavior.PackLeader->PackBehavior.PackMembers.Remove(this);
        PackBehavior.PackLeader = nullptr;
    }
    
    PackBehavior.PackMembers.Empty();
    PackBehavior.bIsPackLeader = true;
}

void ANPC_DinosaurBehaviorTree::CoordinatePackHunt(AActor* Target)
{
    if (!PackBehavior.bIsPackLeader || !Target)
    {
        return;
    }
    
    for (ANPC_DinosaurBehaviorTree* PackMember : PackBehavior.PackMembers)
    {
        if (PackMember && IsValid(PackMember))
        {
            PackMember->StartHunting(Target);
        }
    }
}

FVector ANPC_DinosaurBehaviorTree::GetPackCenterLocation() const
{
    if (!PackBehavior.bIsPackAnimal || PackBehavior.PackMembers.Num() == 0)
    {
        return GetActorLocation();
    }
    
    FVector CenterLocation = GetActorLocation();
    int32 ValidMembers = 1;
    
    for (ANPC_DinosaurBehaviorTree* PackMember : PackBehavior.PackMembers)
    {
        if (PackMember && IsValid(PackMember))
        {
            CenterLocation += PackMember->GetActorLocation();
            ValidMembers++;
        }
    }
    
    return CenterLocation / ValidMembers;
}

void ANPC_DinosaurBehaviorTree::UpdateHunger(float DeltaTime)
{
    HungerUpdateTimer += DeltaTime;
    
    if (HungerUpdateTimer >= 10.0f) // Update hunger every 10 seconds
    {
        HungerUpdateTimer = 0.0f;
        
        // Increase hunger based on activity
        float HungerIncrease = 2.0f;
        if (CurrentState == ENPC_DinosaurState::Hunting || CurrentState == ENPC_DinosaurState::Fleeing)
        {
            HungerIncrease = 5.0f; // More energy expenditure
        }
        
        DinosaurStats.Hunger = FMath::Min(100.0f, DinosaurStats.Hunger + HungerIncrease);
    }
}

void ANPC_DinosaurBehaviorTree::UpdateThirst(float DeltaTime)
{
    ThirstUpdateTimer += DeltaTime;
    
    if (ThirstUpdateTimer >= 15.0f) // Update thirst every 15 seconds
    {
        ThirstUpdateTimer = 0.0f;
        DinosaurStats.Thirst = FMath::Min(100.0f, DinosaurStats.Thirst + 3.0f);
    }
}

void ANPC_DinosaurBehaviorTree::TakeDamage(float Damage, AActor* DamageSource)
{
    DinosaurStats.Health = FMath::Max(0.0f, DinosaurStats.Health - Damage);
    DinosaurStats.Fear = FMath::Min(100.0f, DinosaurStats.Fear + Damage * 0.5f);
    
    if (DamageSource && DinosaurStats.Health > 0.0f)
    {
        if (DinosaurStats.Aggression > 50.0f)
        {
            StartHunting(DamageSource);
        }
        else
        {
            StartFleeing(DamageSource);
        }
    }
    
    if (DinosaurStats.Health <= 0.0f)
    {
        SetDinosaurState(ENPC_DinosaurState::Idle);
        SetActorEnableCollision(false);
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s has died"), *GetName());
    }
}

void ANPC_DinosaurBehaviorTree::RestoreHealth(float Amount)
{
    DinosaurStats.Health = FMath::Min(DinosaurStats.MaxHealth, DinosaurStats.Health + Amount);
}