#include "Combat_DinosaurAI.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

ACombat_DinosaurAI::ACombat_DinosaurAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create mesh component
    DinosaurMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DinosaurMesh"));
    DinosaurMesh->SetupAttachment(RootComponent);
    DinosaurMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DinosaurMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create detection sphere
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(800.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create attack sphere
    AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphere"));
    AttackSphere->SetupAttachment(RootComponent);
    AttackSphere->SetSphereRadius(200.0f);
    AttackSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AttackSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    AttackSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize default stats based on species
    Stats = FCombat_DinosaurStats();
    CurrentState = ECombat_DinosaurState::Idle;
    Species = ECombat_DinosaurSpecies::Raptor;
    DinosaurName = "Raptor_01";

    // Set default voice URLs
    CombatVoiceURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777707850720_AlphaRaptor.mp3";
}

void ACombat_DinosaurAI::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACombat_DinosaurAI::OnDetectionSphereBeginOverlap);
    AttackSphere->OnComponentBeginOverlap.AddDynamic(this, &ACombat_DinosaurAI::OnAttackSphereBeginOverlap);

    // Configure stats based on species
    switch (Species)
    {
        case ECombat_DinosaurSpecies::TRex:
            Stats.Health = 500.0f;
            Stats.MaxHealth = 500.0f;
            Stats.AttackDamage = 75.0f;
            Stats.AttackRange = 300.0f;
            Stats.DetectionRange = 1200.0f;
            Stats.MovementSpeed = 400.0f;
            Stats.Aggression = 0.9f;
            Stats.bIsPackHunter = false;
            CombatVoiceURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777707855068_TRexAlpha.mp3";
            break;

        case ECombat_DinosaurSpecies::Raptor:
            Stats.Health = 120.0f;
            Stats.MaxHealth = 120.0f;
            Stats.AttackDamage = 35.0f;
            Stats.AttackRange = 150.0f;
            Stats.DetectionRange = 900.0f;
            Stats.MovementSpeed = 500.0f;
            Stats.Aggression = 0.8f;
            Stats.bIsPackHunter = true;
            break;

        case ECombat_DinosaurSpecies::Brachio:
            Stats.Health = 800.0f;
            Stats.MaxHealth = 800.0f;
            Stats.AttackDamage = 50.0f;
            Stats.AttackRange = 400.0f;
            Stats.DetectionRange = 600.0f;
            Stats.MovementSpeed = 200.0f;
            Stats.Aggression = 0.3f;
            Stats.bIsPackHunter = false;
            break;

        case ECombat_DinosaurSpecies::Triceratops:
            Stats.Health = 350.0f;
            Stats.MaxHealth = 350.0f;
            Stats.AttackDamage = 60.0f;
            Stats.AttackRange = 250.0f;
            Stats.DetectionRange = 700.0f;
            Stats.MovementSpeed = 300.0f;
            Stats.Aggression = 0.6f;
            Stats.bIsPackHunter = false;
            break;

        case ECombat_DinosaurSpecies::Pteranodon:
            Stats.Health = 80.0f;
            Stats.MaxHealth = 80.0f;
            Stats.AttackDamage = 20.0f;
            Stats.AttackRange = 200.0f;
            Stats.DetectionRange = 1500.0f;
            Stats.MovementSpeed = 600.0f;
            Stats.Aggression = 0.4f;
            Stats.bIsPackHunter = true;
            break;
    }

    // Update sphere radii based on stats
    DetectionSphere->SetSphereRadius(Stats.DetectionRange);
    AttackSphere->SetSphereRadius(Stats.AttackRange);

    // Set patrol origin
    PatrolOrigin = GetActorLocation();
    PatrolTarget = GetRandomPatrolPoint();

    // Start patrolling
    StartPatrolling();

    UE_LOG(LogTemp, Warning, TEXT("DinosaurAI %s initialized with species %d"), *DinosaurName, (int32)Species);
}

void ACombat_DinosaurAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState != ECombat_DinosaurState::Dead)
    {
        UpdateAI(DeltaTime);
        
        if (Stats.bIsPackHunter)
        {
            UpdatePackBehavior(DeltaTime);
        }
    }

    TimeInCurrentState += DeltaTime;
}

void ACombat_DinosaurAI::UpdateAI(float DeltaTime)
{
    switch (CurrentState)
    {
        case ECombat_DinosaurState::Idle:
            UpdateIdleBehavior(DeltaTime);
            break;
        case ECombat_DinosaurState::Patrolling:
            UpdatePatrolBehavior(DeltaTime);
            break;
        case ECombat_DinosaurState::Hunting:
            UpdateHuntingBehavior(DeltaTime);
            break;
        case ECombat_DinosaurState::Attacking:
            UpdateAttackingBehavior(DeltaTime);
            break;
        case ECombat_DinosaurState::Fleeing:
            UpdateFleeingBehavior(DeltaTime);
            break;
        default:
            break;
    }
}

void ACombat_DinosaurAI::UpdateIdleBehavior(float DeltaTime)
{
    // After 3 seconds of idle, start patrolling
    if (TimeInCurrentState > 3.0f)
    {
        StartPatrolling();
    }
}

void ACombat_DinosaurAI::UpdatePatrolBehavior(float DeltaTime)
{
    if (PatrolTarget.IsZero())
    {
        PatrolTarget = GetRandomPatrolPoint();
    }

    MoveTowards(PatrolTarget, DeltaTime);

    // Check if reached patrol target
    float DistanceToTarget = FVector::Dist(GetActorLocation(), PatrolTarget);
    if (DistanceToTarget < 100.0f)
    {
        PatrolTarget = GetRandomPatrolPoint();
    }
}

void ACombat_DinosaurAI::UpdateHuntingBehavior(float DeltaTime)
{
    if (!CurrentTarget || !IsValidTarget(CurrentTarget))
    {
        CurrentTarget = nullptr;
        StartPatrolling();
        return;
    }

    FVector TargetLocation = CurrentTarget->GetActorLocation();
    MoveTowards(TargetLocation, DeltaTime);

    // Check if close enough to attack
    float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetLocation);
    if (DistanceToTarget <= Stats.AttackRange)
    {
        SetDinosaurState(ECombat_DinosaurState::Attacking);
    }
}

void ACombat_DinosaurAI::UpdateAttackingBehavior(float DeltaTime)
{
    if (!CurrentTarget || !IsValidTarget(CurrentTarget))
    {
        CurrentTarget = nullptr;
        StartPatrolling();
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime >= AttackCooldown)
    {
        AttackTarget(CurrentTarget);
        LastAttackTime = CurrentTime;
    }

    // Check if target is still in range
    float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistanceToTarget > Stats.AttackRange * 1.2f)
    {
        SetDinosaurState(ECombat_DinosaurState::Hunting);
    }
}

void ACombat_DinosaurAI::UpdateFleeingBehavior(float DeltaTime)
{
    // Move away from current target
    if (CurrentTarget)
    {
        FVector FleeDirection = (GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
        FVector FleeTarget = GetActorLocation() + FleeDirection * 1000.0f;
        MoveTowards(FleeTarget, DeltaTime);
    }

    // After fleeing for 5 seconds, return to patrolling
    if (TimeInCurrentState > 5.0f)
    {
        CurrentTarget = nullptr;
        StartPatrolling();
    }
}

void ACombat_DinosaurAI::MoveTowards(FVector TargetLocation, float DeltaTime)
{
    FVector CurrentLocation = GetActorLocation();
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    
    FVector NewLocation = CurrentLocation + Direction * Stats.MovementSpeed * DeltaTime;
    SetActorLocation(NewLocation);

    // Rotate to face movement direction
    if (!Direction.IsZero())
    {
        FRotator NewRotation = Direction.Rotation();
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), NewRotation, DeltaTime, 5.0f));
    }
}

FVector ACombat_DinosaurAI::GetRandomPatrolPoint()
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    return PatrolOrigin + RandomDirection * FMath::RandRange(200.0f, PatrolRadius);
}

bool ACombat_DinosaurAI::IsValidTarget(AActor* Target)
{
    if (!Target)
        return false;

    // Check if target is a character (player or NPC)
    if (Target->IsA<ACharacter>())
        return true;

    // Check if target is another dinosaur (for territorial behavior)
    if (Target->IsA<ACombat_DinosaurAI>())
    {
        ACombat_DinosaurAI* OtherDino = Cast<ACombat_DinosaurAI>(Target);
        if (OtherDino && OtherDino->Species != Species)
            return true;
    }

    return false;
}

void ACombat_DinosaurAI::SetDinosaurState(ECombat_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        ECombat_DinosaurState OldState = CurrentState;
        CurrentState = NewState;
        TimeInCurrentState = 0.0f;

        UE_LOG(LogTemp, Warning, TEXT("DinosaurAI %s state changed from %d to %d"), 
               *DinosaurName, (int32)OldState, (int32)NewState);

        // Play voice on state changes
        if (NewState == ECombat_DinosaurState::Hunting || NewState == ECombat_DinosaurState::Attacking)
        {
            PlayCombatVoice();
        }
    }
}

void ACombat_DinosaurAI::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    Stats.Health = FMath::Clamp(Stats.Health - DamageAmount, 0.0f, Stats.MaxHealth);

    UE_LOG(LogTemp, Warning, TEXT("DinosaurAI %s took %f damage, health: %f"), 
           *DinosaurName, DamageAmount, Stats.Health);

    if (Stats.Health <= 0.0f)
    {
        SetDinosaurState(ECombat_DinosaurState::Dead);
        UE_LOG(LogTemp, Warning, TEXT("DinosaurAI %s died"), *DinosaurName);
    }
    else if (DamageSource && Stats.Health < Stats.MaxHealth * 0.3f)
    {
        // Flee if health is low
        CurrentTarget = DamageSource;
        SetDinosaurState(ECombat_DinosaurState::Fleeing);
    }
    else if (DamageSource)
    {
        // Fight back
        CurrentTarget = DamageSource;
        SetDinosaurState(ECombat_DinosaurState::Hunting);
    }
}

void ACombat_DinosaurAI::AttackTarget(AActor* Target)
{
    if (!Target)
        return;

    UE_LOG(LogTemp, Warning, TEXT("DinosaurAI %s attacks %s for %f damage"), 
           *DinosaurName, *Target->GetName(), Stats.AttackDamage);

    // If target is another dinosaur, apply damage
    if (ACombat_DinosaurAI* TargetDino = Cast<ACombat_DinosaurAI>(Target))
    {
        TargetDino->TakeDamage(Stats.AttackDamage, this);
    }
    // If target is a character, apply damage (would need damage interface)
    else if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
    {
        // Apply damage to character (placeholder)
        UE_LOG(LogTemp, Warning, TEXT("Attacking character %s"), *TargetCharacter->GetName());
    }
}

void ACombat_DinosaurAI::StartHunting(AActor* Target)
{
    if (Target && IsValidTarget(Target))
    {
        CurrentTarget = Target;
        SetDinosaurState(ECombat_DinosaurState::Hunting);
    }
}

void ACombat_DinosaurAI::StartPatrolling()
{
    CurrentTarget = nullptr;
    PatrolTarget = GetRandomPatrolPoint();
    SetDinosaurState(ECombat_DinosaurState::Patrolling);
}

void ACombat_DinosaurAI::JoinPack(ACombat_DinosaurAI* Leader)
{
    if (Leader && Leader != this)
    {
        PackLeader = Leader;
        bIsPackLeader = false;
        
        if (!Leader->PackMembers.Contains(this))
        {
            Leader->PackMembers.Add(this);
        }

        UE_LOG(LogTemp, Warning, TEXT("DinosaurAI %s joined pack led by %s"), 
               *DinosaurName, *Leader->DinosaurName);
    }
}

void ACombat_DinosaurAI::PlayCombatVoice()
{
    if (!CombatVoiceURL.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurAI %s plays combat voice: %s"), 
               *DinosaurName, *CombatVoiceURL);
        // Voice playback would be implemented with audio component
    }
}

void ACombat_DinosaurAI::UpdatePackBehavior(float DeltaTime)
{
    if (bIsPackLeader)
    {
        // Leader coordinates pack attacks
        if (CurrentTarget && CurrentState == ECombat_DinosaurState::Hunting)
        {
            CoordinatePackAttack();
        }
    }
    else if (PackLeader)
    {
        // Follow pack leader's target
        if (PackLeader->CurrentTarget && !CurrentTarget)
        {
            StartHunting(PackLeader->CurrentTarget);
        }
    }
}

void ACombat_DinosaurAI::CoordinatePackAttack()
{
    if (!CurrentTarget)
        return;

    // Command pack members to attack
    for (ACombat_DinosaurAI* Member : PackMembers)
    {
        if (Member && Member->CurrentState != ECombat_DinosaurState::Dead)
        {
            Member->StartHunting(CurrentTarget);
        }
    }
}

void ACombat_DinosaurAI::FormPackFormation()
{
    if (!bIsPackLeader || PackMembers.Num() == 0)
        return;

    // Simple formation: spread pack members around leader
    float AngleStep = 360.0f / PackMembers.Num();
    float FormationRadius = 300.0f;

    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i])
        {
            float Angle = i * AngleStep;
            FVector FormationOffset = FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * FormationRadius,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * FormationRadius,
                0.0f
            );
            
            FVector TargetLocation = GetActorLocation() + FormationOffset;
            PackMembers[i]->PatrolTarget = TargetLocation;
        }
    }
}

void ACombat_DinosaurAI::OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (CurrentState == ECombat_DinosaurState::Dead)
        return;

    if (IsValidTarget(OtherActor))
    {
        float AggressionRoll = FMath::RandRange(0.0f, 1.0f);
        if (AggressionRoll <= Stats.Aggression)
        {
            StartHunting(OtherActor);
            UE_LOG(LogTemp, Warning, TEXT("DinosaurAI %s detected target %s"), 
                   *DinosaurName, *OtherActor->GetName());
        }
    }
}

void ACombat_DinosaurAI::OnAttackSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (CurrentState == ECombat_DinosaurState::Dead)
        return;

    if (IsValidTarget(OtherActor) && CurrentState == ECombat_DinosaurState::Hunting)
    {
        SetDinosaurState(ECombat_DinosaurState::Attacking);
    }
}