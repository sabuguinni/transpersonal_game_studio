#include "Combat_DinosaurAI.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

UCombat_DinosaurAI::UCombat_DinosaurAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize dinosaur behavior parameters
    DinosaurType = ECombat_DinosaurType::Raptor;
    BehaviorState = ECombat_DinosaurBehavior::Idle;
    AggressionLevel = 0.5f;
    HuntingRange = 1000.0f;
    PackRadius = 500.0f;
    AttackRange = 150.0f;
    FleeThreshold = 0.2f;
    
    // Initialize behavior flags
    bIsHunting = false;
    bIsInPack = false;
    bCanAttack = true;
    bIsTerritory = false;
    
    // Initialize arrays
    PackMembers.Empty();
    PreyTargets.Empty();
    ThreatTargets.Empty();
    
    // Set update intervals
    BehaviorUpdateInterval = 1.5f;
    PackUpdateInterval = 3.0f;
    
    // Initialize stats based on type
    InitializeDinosaurStats();
}

void UCombat_DinosaurAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Start behavior update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            BehaviorUpdateTimer,
            this,
            &UCombat_DinosaurAI::UpdateBehavior,
            BehaviorUpdateInterval,
            true
        );
        
        GetWorld()->GetTimerManager().SetTimer(
            PackUpdateTimer,
            this,
            &UCombat_DinosaurAI::UpdatePackBehavior,
            PackUpdateInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurAI initialized for %s - Type: %d"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), (int32)DinosaurType);
}

void UCombat_DinosaurAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Execute current behavior
    ExecuteBehavior(DeltaTime);
    
    // Update aggression based on circumstances
    UpdateAggressionLevel(DeltaTime);
}

void UCombat_DinosaurAI::InitializeDinosaurStats()
{
    switch (DinosaurType)
    {
        case ECombat_DinosaurType::TRex:
            AggressionLevel = 0.8f;
            HuntingRange = 2000.0f;
            AttackRange = 300.0f;
            PackRadius = 0.0f; // Solitary
            bIsTerritory = true;
            break;
            
        case ECombat_DinosaurType::Raptor:
            AggressionLevel = 0.7f;
            HuntingRange = 1200.0f;
            AttackRange = 150.0f;
            PackRadius = 600.0f;
            bIsInPack = true;
            break;
            
        case ECombat_DinosaurType::Triceratops:
            AggressionLevel = 0.3f;
            HuntingRange = 500.0f;
            AttackRange = 200.0f;
            PackRadius = 300.0f;
            bIsTerritory = true;
            break;
            
        case ECombat_DinosaurType::Brachiosaurus:
            AggressionLevel = 0.1f;
            HuntingRange = 300.0f;
            AttackRange = 250.0f;
            PackRadius = 800.0f;
            FleeThreshold = 0.8f; // Flees easily
            break;
            
        case ECombat_DinosaurType::Ankylosaurus:
            AggressionLevel = 0.4f;
            HuntingRange = 400.0f;
            AttackRange = 180.0f;
            PackRadius = 200.0f;
            bIsTerritory = true;
            break;
    }
}

void UCombat_DinosaurAI::UpdateBehavior()
{
    if (!GetOwner()) return;
    
    // Scan for targets and threats
    ScanEnvironment();
    
    // Determine new behavior state
    DetermineBehaviorState();
    
    // Update pack coordination if applicable
    if (bIsInPack && PackMembers.Num() > 0)
    {
        CoordinatePackBehavior();
    }
}

void UCombat_DinosaurAI::ScanEnvironment()
{
    if (!GetWorld()) return;
    
    PreyTargets.Empty();
    ThreatTargets.Empty();
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Get all pawns in hunting range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == GetOwner()) continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        if (Distance <= HuntingRange)
        {
            FString ActorName = Actor->GetName().ToLower();
            
            // Classify targets based on dinosaur type and actor type
            if (ActorName.Contains("player") || ActorName.Contains("character"))
            {
                if (ShouldTargetPlayer())
                {
                    PreyTargets.Add(Actor);
                }
                else
                {
                    ThreatTargets.Add(Actor);
                }
            }
            else if (ActorName.Contains("dino"))
            {
                // Other dinosaurs - check if threat or potential pack member
                if (ShouldConsiderThreat(Actor))
                {
                    ThreatTargets.Add(Actor);
                }
            }
        }
    }
}

bool UCombat_DinosaurAI::ShouldTargetPlayer() const
{
    switch (DinosaurType)
    {
        case ECombat_DinosaurType::TRex:
        case ECombat_DinosaurType::Raptor:
            return AggressionLevel > 0.5f;
            
        case ECombat_DinosaurType::Triceratops:
        case ECombat_DinosaurType::Ankylosaurus:
            return AggressionLevel > 0.7f; // Only when very aggressive
            
        case ECombat_DinosaurType::Brachiosaurus:
            return false; // Herbivore, doesn't hunt players
    }
    return false;
}

bool UCombat_DinosaurAI::ShouldConsiderThreat(AActor* OtherActor) const
{
    if (!OtherActor) return false;
    
    FString ActorName = OtherActor->GetName().ToLower();
    
    // T-Rex is threat to most dinosaurs
    if (ActorName.Contains("trex"))
    {
        return DinosaurType != ECombat_DinosaurType::TRex;
    }
    
    // Large herbivores can be threats when defending
    if (ActorName.Contains("tricera") || ActorName.Contains("ankylo"))
    {
        return DinosaurType == ECombat_DinosaurType::Raptor;
    }
    
    return false;
}

void UCombat_DinosaurAI::DetermineBehaviorState()
{
    // Priority: Flee > Attack > Hunt > Pack > Idle
    
    if (ShouldFlee())
    {
        BehaviorState = ECombat_DinosaurBehavior::Fleeing;
        bIsHunting = false;
    }
    else if (CanAttackTarget())
    {
        BehaviorState = ECombat_DinosaurBehavior::Attacking;
        bIsHunting = true;
    }
    else if (PreyTargets.Num() > 0)
    {
        BehaviorState = ECombat_DinosaurBehavior::Hunting;
        bIsHunting = true;
    }
    else if (bIsInPack && PackMembers.Num() > 0)
    {
        BehaviorState = ECombat_DinosaurBehavior::PackBehavior;
        bIsHunting = false;
    }
    else
    {
        BehaviorState = ECombat_DinosaurBehavior::Idle;
        bIsHunting = false;
    }
}

bool UCombat_DinosaurAI::ShouldFlee() const
{
    if (ThreatTargets.Num() == 0) return false;
    
    // Check if overwhelmed or health is low
    float ThreatLevel = (float)ThreatTargets.Num() / FMath::Max(1.0f, (float)PackMembers.Num());
    
    return ThreatLevel > FleeThreshold || AggressionLevel < 0.2f;
}

bool UCombat_DinosaurAI::CanAttackTarget() const
{
    if (!bCanAttack || PreyTargets.Num() == 0) return false;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Target : PreyTargets)
    {
        float Distance = FVector::Dist(OwnerLocation, Target->GetActorLocation());
        if (Distance <= AttackRange)
        {
            return true;
        }
    }
    
    return false;
}

void UCombat_DinosaurAI::ExecuteBehavior(float DeltaTime)
{
    switch (BehaviorState)
    {
        case ECombat_DinosaurBehavior::Idle:
            ExecuteIdleBehavior(DeltaTime);
            break;
            
        case ECombat_DinosaurBehavior::Hunting:
            ExecuteHuntingBehavior(DeltaTime);
            break;
            
        case ECombat_DinosaurBehavior::Attacking:
            ExecuteAttackingBehavior(DeltaTime);
            break;
            
        case ECombat_DinosaurBehavior::Fleeing:
            ExecuteFleeingBehavior(DeltaTime);
            break;
            
        case ECombat_DinosaurBehavior::PackBehavior:
            ExecutePackBehavior(DeltaTime);
            break;
            
        case ECombat_DinosaurBehavior::Territorial:
            ExecuteTerritorialBehavior(DeltaTime);
            break;
    }
}

void UCombat_DinosaurAI::ExecuteIdleBehavior(float DeltaTime)
{
    if (!GetOwner()) return;
    
    // Random wandering
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    // Occasional random movement
    if (FMath::RandRange(0.0f, 1.0f) < 0.1f * DeltaTime)
    {
        FVector RandomDirection = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ).GetSafeNormal();
        
        float MoveSpeed = GetMoveSpeedForType() * 0.3f; // Slow idle movement
        GetOwner()->SetActorLocation(CurrentLocation + RandomDirection * MoveSpeed * DeltaTime);
    }
}

void UCombat_DinosaurAI::ExecuteHuntingBehavior(float DeltaTime)
{
    if (PreyTargets.Num() == 0 || !GetOwner()) return;
    
    // Move towards nearest prey
    AActor* NearestPrey = PreyTargets[0];
    FVector PreyLocation = NearestPrey->GetActorLocation();
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    FVector Direction = (PreyLocation - CurrentLocation).GetSafeNormal();
    float MoveSpeed = GetMoveSpeedForType();
    
    // Apply pack hunting coordination if in pack
    if (bIsInPack && PackMembers.Num() > 1)
    {
        Direction = CalculatePackHuntingDirection(PreyLocation);
    }
    
    GetOwner()->SetActorLocation(CurrentLocation + Direction * MoveSpeed * DeltaTime);
    
    // Face the target
    FRotator LookRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
    GetOwner()->SetActorRotation(LookRotation);
}

void UCombat_DinosaurAI::ExecuteAttackingBehavior(float DeltaTime)
{
    if (PreyTargets.Num() == 0 || !GetOwner()) return;
    
    AActor* Target = PreyTargets[0];
    FVector TargetLocation = Target->GetActorLocation();
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    // Face target and perform attack animation
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    FRotator LookRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
    GetOwner()->SetActorRotation(LookRotation);
    
    // Simple attack cooldown
    static float AttackCooldown = 0.0f;
    AttackCooldown -= DeltaTime;
    
    if (AttackCooldown <= 0.0f)
    {
        PerformAttack(Target);
        AttackCooldown = GetAttackCooldownForType();
    }
}

void UCombat_DinosaurAI::ExecuteFleeingBehavior(float DeltaTime)
{
    if (ThreatTargets.Num() == 0 || !GetOwner()) return;
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector FleeDirection = FVector::ZeroVector;
    
    // Calculate direction away from all threats
    for (AActor* Threat : ThreatTargets)
    {
        FVector ThreatLocation = Threat->GetActorLocation();
        FVector AwayFromThreat = (CurrentLocation - ThreatLocation).GetSafeNormal();
        FleeDirection += AwayFromThreat;
    }
    
    FleeDirection = FleeDirection.GetSafeNormal();
    float FleeSpeed = GetMoveSpeedForType() * 1.5f; // Faster when fleeing
    
    GetOwner()->SetActorLocation(CurrentLocation + FleeDirection * FleeSpeed * DeltaTime);
    
    // Face flee direction
    FRotator FleeRotation = FRotationMatrix::MakeFromX(FleeDirection).Rotator();
    GetOwner()->SetActorRotation(FleeRotation);
}

void UCombat_DinosaurAI::ExecutePackBehavior(float DeltaTime)
{
    if (PackMembers.Num() == 0 || !GetOwner()) return;
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector PackCenter = CalculatePackCenter();
    
    // Maintain formation within pack
    float DistanceToCenter = FVector::Dist(CurrentLocation, PackCenter);
    
    if (DistanceToCenter > PackRadius * 0.8f)
    {
        // Move towards pack center
        FVector Direction = (PackCenter - CurrentLocation).GetSafeNormal();
        float MoveSpeed = GetMoveSpeedForType() * 0.7f;
        GetOwner()->SetActorLocation(CurrentLocation + Direction * MoveSpeed * DeltaTime);
    }
    
    // Maintain spacing from other pack members
    for (AActor* PackMember : PackMembers)
    {
        if (!PackMember) continue;
        
        float Distance = FVector::Dist(CurrentLocation, PackMember->GetActorLocation());
        if (Distance < 100.0f) // Too close
        {
            FVector AwayDirection = (CurrentLocation - PackMember->GetActorLocation()).GetSafeNormal();
            GetOwner()->SetActorLocation(CurrentLocation + AwayDirection * 30.0f * DeltaTime);
        }
    }
}

void UCombat_DinosaurAI::ExecuteTerritorialBehavior(float DeltaTime)
{
    // Patrol territory and challenge intruders
    if (!GetOwner()) return;
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    // Check for intruders in territory
    for (AActor* Intruder : PreyTargets)
    {
        float Distance = FVector::Dist(CurrentLocation, Intruder->GetActorLocation());
        if (Distance <= PackRadius)
        {
            // Challenge intruder
            BehaviorState = ECombat_DinosaurBehavior::Hunting;
            return;
        }
    }
    
    // Patrol territory perimeter
    FVector PatrolDirection = FVector(
        FMath::Cos(GetWorld()->GetTimeSeconds() * 0.5f),
        FMath::Sin(GetWorld()->GetTimeSeconds() * 0.5f),
        0.0f
    );
    
    float MoveSpeed = GetMoveSpeedForType() * 0.4f;
    GetOwner()->SetActorLocation(CurrentLocation + PatrolDirection * MoveSpeed * DeltaTime);
}

void UCombat_DinosaurAI::UpdatePackBehavior()
{
    if (!bIsInPack) return;
    
    // Find nearby pack members
    PackMembers.Empty();
    
    if (GetWorld())
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (!Actor || Actor == GetOwner()) continue;
            
            // Check if same type of dinosaur
            FString ActorName = Actor->GetName().ToLower();
            if (IsSameSpecies(ActorName))
            {
                float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
                if (Distance <= PackRadius)
                {
                    PackMembers.Add(Actor);
                }
            }
        }
    }
}

void UCombat_DinosaurAI::CoordinatePackBehavior()
{
    // Implement pack coordination logic
    if (PackMembers.Num() < 2) return;
    
    // Simple pack coordination - share target information
    if (PreyTargets.Num() > 0)
    {
        // All pack members should focus on the same target
        AActor* PrimaryTarget = PreyTargets[0];
        
        // Broadcast target to pack (simplified - in real implementation would use proper communication)
        for (AActor* PackMember : PackMembers)
        {
            // In a full implementation, would send target info to other AI components
        }
    }
}

FVector UCombat_DinosaurAI::CalculatePackCenter() const
{
    if (PackMembers.Num() == 0) return GetOwner()->GetActorLocation();
    
    FVector Center = FVector::ZeroVector;
    for (AActor* Member : PackMembers)
    {
        Center += Member->GetActorLocation();
    }
    Center /= PackMembers.Num();
    
    return Center;
}

FVector UCombat_DinosaurAI::CalculatePackHuntingDirection(const FVector& PreyLocation) const
{
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector DirectDirection = (PreyLocation - CurrentLocation).GetSafeNormal();
    
    // Add flanking behavior for pack hunting
    if (PackMembers.Num() >= 2)
    {
        // Calculate flanking angle based on pack member index
        float FlankAngle = FMath::RandRange(-45.0f, 45.0f);
        FVector FlankDirection = DirectDirection.RotateAngleAxis(FlankAngle, FVector::UpVector);
        return FlankDirection;
    }
    
    return DirectDirection;
}

float UCombat_DinosaurAI::GetMoveSpeedForType() const
{
    switch (DinosaurType)
    {
        case ECombat_DinosaurType::TRex:
            return 200.0f;
        case ECombat_DinosaurType::Raptor:
            return 300.0f;
        case ECombat_DinosaurType::Triceratops:
            return 150.0f;
        case ECombat_DinosaurType::Brachiosaurus:
            return 100.0f;
        case ECombat_DinosaurType::Ankylosaurus:
            return 120.0f;
    }
    return 150.0f;
}

float UCombat_DinosaurAI::GetAttackCooldownForType() const
{
    switch (DinosaurType)
    {
        case ECombat_DinosaurType::TRex:
            return 3.0f;
        case ECombat_DinosaurType::Raptor:
            return 1.5f;
        case ECombat_DinosaurType::Triceratops:
            return 4.0f;
        case ECombat_DinosaurType::Brachiosaurus:
            return 5.0f;
        case ECombat_DinosaurType::Ankylosaurus:
            return 3.5f;
    }
    return 2.0f;
}

bool UCombat_DinosaurAI::IsSameSpecies(const FString& ActorName) const
{
    switch (DinosaurType)
    {
        case ECombat_DinosaurType::TRex:
            return ActorName.Contains("trex");
        case ECombat_DinosaurType::Raptor:
            return ActorName.Contains("raptor") || ActorName.Contains("veloci");
        case ECombat_DinosaurType::Triceratops:
            return ActorName.Contains("tricera");
        case ECombat_DinosaurType::Brachiosaurus:
            return ActorName.Contains("brachi");
        case ECombat_DinosaurType::Ankylosaurus:
            return ActorName.Contains("ankylo");
    }
    return false;
}

void UCombat_DinosaurAI::PerformAttack(AActor* Target)
{
    if (!Target) return;
    
    UE_LOG(LogTemp, Log, TEXT("%s attacks %s!"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           *Target->GetName());
    
    // In a full implementation, this would:
    // - Play attack animation
    // - Deal damage to target
    // - Apply knockback/effects
    // - Trigger sound effects
}

void UCombat_DinosaurAI::UpdateAggressionLevel(float DeltaTime)
{
    // Aggression increases when threatened, decreases when safe
    if (ThreatTargets.Num() > 0)
    {
        AggressionLevel = FMath::Clamp(AggressionLevel + DeltaTime * 0.2f, 0.0f, 1.0f);
    }
    else
    {
        AggressionLevel = FMath::Clamp(AggressionLevel - DeltaTime * 0.1f, 0.0f, 1.0f);
    }
}

void UCombat_DinosaurAI::SetDinosaurType(ECombat_DinosaurType NewType)
{
    DinosaurType = NewType;
    InitializeDinosaurStats();
    UE_LOG(LogTemp, Log, TEXT("Dinosaur type changed to: %d"), (int32)NewType);
}

void UCombat_DinosaurAI::SetAggressionLevel(float NewAggression)
{
    AggressionLevel = FMath::Clamp(NewAggression, 0.0f, 1.0f);
}

ECombat_DinosaurType UCombat_DinosaurAI::GetDinosaurType() const
{
    return DinosaurType;
}

ECombat_DinosaurBehavior UCombat_DinosaurAI::GetCurrentBehavior() const
{
    return BehaviorState;
}

float UCombat_DinosaurAI::GetAggressionLevel() const
{
    return AggressionLevel;
}

bool UCombat_DinosaurAI::IsHunting() const
{
    return bIsHunting;
}

int32 UCombat_DinosaurAI::GetPackSize() const
{
    return PackMembers.Num();
}