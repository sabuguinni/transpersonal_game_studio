#include "CombatBehaviorComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UCombatBehaviorComponent::UCombatBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize default combat stats
    CombatStats.MaxHealth = 100.0f;
    CombatStats.CurrentHealth = 100.0f;
    CombatStats.MovementSpeed = 600.0f;
    CombatStats.AttackSpeed = 1.0f;
    CombatStats.DefenseRating = 0.1f;
    CombatStats.StaminaMax = 100.0f;
    CombatStats.CurrentStamina = 100.0f;
    CombatStats.StaminaRegenRate = 10.0f;
    
    // Initialize default attack patterns
    FAttackPattern BiteAttack;
    BiteAttack.AttackType = EAttackType::Bite;
    BiteAttack.Damage = 35.0f;
    BiteAttack.Range = 150.0f;
    BiteAttack.Cooldown = 2.0f;
    BiteAttack.WindupTime = 0.8f;
    BiteAttack.RecoveryTime = 0.3f;
    BiteAttack.bRequiresLineOfSight = true;
    AttackPatterns.Add(BiteAttack);
    
    FAttackPattern ClawAttack;
    ClawAttack.AttackType = EAttackType::Claw;
    ClawAttack.Damage = 25.0f;
    ClawAttack.Range = 120.0f;
    ClawAttack.Cooldown = 1.5f;
    ClawAttack.WindupTime = 0.5f;
    ClawAttack.RecoveryTime = 0.2f;
    ClawAttack.bRequiresLineOfSight = true;
    AttackPatterns.Add(ClawAttack);
    
    // Initialize tactical parameters
    CircleDistance = 400.0f;
    FlankingAngle = 45.0f;
    AmbushWaitTime = 5.0f;
    IntimidationRadius = 800.0f;
    RoarCooldown = 15.0f;
}

void UCombatBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize health and stamina to max
    CombatStats.CurrentHealth = CombatStats.MaxHealth;
    CombatStats.CurrentStamina = CombatStats.StaminaMax;
    
    // Reset timers
    LastAttackTime = 0.0f;
    LastRoarTime = 0.0f;
    AmbushStartTime = 0.0f;
}

void UCombatBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateStamina(DeltaTime);
    UpdateCombatState(DeltaTime);
}

void UCombatBehaviorComponent::UpdateBehavior(float DeltaTime, ECombatState CombatState, AActor* Target)
{
    if (!GetOwner())
        return;
    
    // Update combat state based on current situation
    bIsInCombat = (CombatState == ECombatState::Fighting || CombatState == ECombatState::Engaging);
    
    // Update last known target location
    if (Target)
    {
        LastKnownTargetLocation = Target->GetActorLocation();
    }
    
    // Handle state-specific behavior
    switch (CombatState)
    {
        case ECombatState::Stalking:
            // Reduce movement speed for stealth
            if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
            {
                OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = CombatStats.MovementSpeed * 0.5f;
            }
            break;
            
        case ECombatState::Fighting:
            // Full speed combat
            if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
            {
                OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = CombatStats.MovementSpeed;
            }
            break;
            
        case ECombatState::Retreating:
            // Increase speed when fleeing
            if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
            {
                OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = CombatStats.MovementSpeed * 1.2f;
            }
            break;
            
        default:
            // Normal movement speed
            if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
            {
                OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = CombatStats.MovementSpeed;
            }
            break;
    }
}

bool UCombatBehaviorComponent::TryExecuteAttack(AActor* Target)
{
    if (!CanAttack() || !Target || AttackPatterns.Num() == 0)
        return false;
    
    // Select optimal attack based on distance and situation
    SelectOptimalAttack(Target);
    
    FAttackPattern& CurrentAttack = AttackPatterns[CurrentAttackIndex];
    
    // Check if target is in range
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    if (DistanceToTarget > CurrentAttack.Range)
        return false;
    
    // Check line of sight if required
    if (CurrentAttack.bRequiresLineOfSight && !HasLineOfSight(Target))
        return false;
    
    // Check stamina cost (attacks cost 20 stamina)
    if (CombatStats.CurrentStamina < 20.0f)
        return false;
    
    // Execute attack
    bIsAttacking = true;
    LastAttackTime = GetWorld()->GetTimeSeconds();
    CombatStats.CurrentStamina -= 20.0f;
    
    // Broadcast attack event
    OnAttackExecuted.Broadcast(CurrentAttack.AttackType, Target, CurrentAttack.Damage);
    
    // Set timer to end attack state
    FTimerHandle AttackTimer;
    GetWorld()->GetTimerManager().SetTimer(AttackTimer, [this]()
    {
        bIsAttacking = false;
    }, CurrentAttack.WindupTime + CurrentAttack.RecoveryTime, false);
    
    return true;
}

FVector UCombatBehaviorComponent::GetTacticalPosition(AActor* Target, ECombatState CombatState)
{
    if (!Target)
        return GetOwner()->GetActorLocation();
    
    switch (CombatState)
    {
        case ECombatState::Stalking:
            return CalculateAmbushPosition(Target);
            
        case ECombatState::Fighting:
            return CalculateFlankingPosition(Target);
            
        case ECombatState::Hunting:
            return CalculateCirclingPosition(Target);
            
        default:
            return GetOwner()->GetActorLocation();
    }
}

void UCombatBehaviorComponent::TakeDamage(float Damage, AActor* DamageSource)
{
    float ActualDamage = Damage * (1.0f - CombatStats.DefenseRating);
    CombatStats.CurrentHealth = FMath::Max(0.0f, CombatStats.CurrentHealth - ActualDamage);
    
    // Broadcast health change
    OnHealthChanged.Broadcast(CombatStats.CurrentHealth, CombatStats.MaxHealth);
    
    // Update combat state if health is low
    if (GetHealthPercentage() < 0.3f && !bIsInCombat)
    {
        bIsInCombat = true;
        OnCombatStateChanged.Broadcast(true);
    }
}

void UCombatBehaviorComponent::OnStateChanged(ECombatState PreviousState, ECombatState NewState)
{
    // Handle state transition logic
    if (PreviousState != NewState)
    {
        // Reset ambush timer when entering stalking
        if (NewState == ECombatState::Stalking)
        {
            AmbushStartTime = GetWorld()->GetTimeSeconds();
        }
        
        // Update combat flag
        bool bWasInCombat = bIsInCombat;
        bIsInCombat = (NewState == ECombatState::Fighting || NewState == ECombatState::Engaging);
        
        if (bWasInCombat != bIsInCombat)
        {
            OnCombatStateChanged.Broadcast(bIsInCombat);
        }
    }
}

bool UCombatBehaviorComponent::CanAttack() const
{
    if (bIsAttacking)
        return false;
    
    if (CombatStats.CurrentStamina < 20.0f)
        return false;
    
    if (AttackPatterns.Num() == 0)
        return false;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    FAttackPattern CurrentAttack = GetCurrentAttackPattern();
    
    return (CurrentTime - LastAttackTime) >= CurrentAttack.Cooldown;
}

FAttackPattern UCombatBehaviorComponent::GetCurrentAttackPattern() const
{
    if (AttackPatterns.Num() == 0)
    {
        return FAttackPattern(); // Return default pattern
    }
    
    int32 SafeIndex = FMath::Clamp(CurrentAttackIndex, 0, AttackPatterns.Num() - 1);
    return AttackPatterns[SafeIndex];
}

void UCombatBehaviorComponent::ExecuteRoar()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if ((CurrentTime - LastRoarTime) < RoarCooldown)
        return;
    
    LastRoarTime = CurrentTime;
    
    // Intimidate nearby actors
    IntimidateNearbyActors();
    
    // Broadcast roar attack
    OnAttackExecuted.Broadcast(EAttackType::Roar, nullptr, 0.0f);
}

void UCombatBehaviorComponent::IntimidateNearbyActors()
{
    if (!GetOwner())
        return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Find all actors in intimidation radius
    TArray<AActor*> OverlappingActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        OwnerLocation,
        IntimidationRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        APawn::StaticClass(),
        TArray<AActor*>(),
        OverlappingActors
    );
    
    // Apply intimidation effect to each actor
    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor != GetOwner())
        {
            // Try to get their combat component and apply fear
            if (UCombatBehaviorComponent* OtherCombat = Actor->FindComponentByClass<UCombatBehaviorComponent>())
            {
                // Smaller creatures should be more affected
                float IntimidationEffect = 1.0f;
                if (APawn* OtherPawn = Cast<APawn>(Actor))
                {
                    if (UCapsuleComponent* OtherCapsule = OtherPawn->FindComponentByClass<UCapsuleComponent>())
                    {
                        if (UCapsuleComponent* MyCapsule = GetOwner()->FindComponentByClass<UCapsuleComponent>())
                        {
                            float SizeRatio = MyCapsule->GetScaledCapsuleRadius() / OtherCapsule->GetScaledCapsuleRadius();
                            IntimidationEffect = FMath::Clamp(SizeRatio, 0.5f, 2.0f);
                        }
                    }
                }
                
                // Apply intimidation (could trigger flee behavior)
                BroadcastIntimidation(IntimidationEffect);
            }
        }
    }
}

void UCombatBehaviorComponent::UpdateStamina(float DeltaTime)
{
    if (CombatStats.CurrentStamina < CombatStats.StaminaMax)
    {
        // Regenerate stamina slower during combat
        float RegenMultiplier = bIsInCombat ? 0.5f : 1.0f;
        CombatStats.CurrentStamina = FMath::Min(
            CombatStats.StaminaMax,
            CombatStats.CurrentStamina + (CombatStats.StaminaRegenRate * DeltaTime * RegenMultiplier)
        );
    }
}

void UCombatBehaviorComponent::UpdateCombatState(float DeltaTime)
{
    // Update combat state based on health and situation
    if (GetHealthPercentage() < 0.2f && bIsInCombat)
    {
        // Consider retreating when health is very low
        // This would be handled by the AI controller
    }
}

void UCombatBehaviorComponent::SelectOptimalAttack(AActor* Target)
{
    if (!Target || AttackPatterns.Num() == 0)
        return;
    
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    
    // Find the best attack pattern for current distance
    int32 BestAttackIndex = 0;
    float BestScore = 0.0f;
    
    for (int32 i = 0; i < AttackPatterns.Num(); ++i)
    {
        const FAttackPattern& Attack = AttackPatterns[i];
        
        // Skip if target is out of range
        if (DistanceToTarget > Attack.Range)
            continue;
        
        // Calculate score based on damage and availability
        float Score = Attack.Damage;
        
        // Prefer attacks that are off cooldown
        float TimeSinceLastAttack = GetWorld()->GetTimeSeconds() - LastAttackTime;
        if (TimeSinceLastAttack >= Attack.Cooldown)
        {
            Score *= 1.5f;
        }
        
        // Prefer shorter range attacks when close
        if (DistanceToTarget < Attack.Range * 0.7f)
        {
            Score *= 1.2f;
        }
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestAttackIndex = i;
        }
    }
    
    CurrentAttackIndex = BestAttackIndex;
}

FVector UCombatBehaviorComponent::CalculateFlankingPosition(AActor* Target)
{
    if (!Target)
        return GetOwner()->GetActorLocation();
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Calculate flanking angle
    FVector ToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    // Choose left or right flank randomly
    float FlankDirection = FMath::RandBool() ? 1.0f : -1.0f;
    FVector FlankVector = RightVector * FlankDirection;
    
    // Rotate by flanking angle
    FVector FlankPosition = TargetLocation + (FlankVector * CircleDistance);
    
    // Project to navigation mesh
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(FlankPosition, NavLocation))
        {
            return NavLocation.Location;
        }
    }
    
    return FlankPosition;
}

FVector UCombatBehaviorComponent::CalculateAmbushPosition(AActor* Target)
{
    if (!Target)
        return GetOwner()->GetActorLocation();
    
    // Find a position behind cover near the target's predicted path
    FVector TargetLocation = Target->GetActorLocation();
    FVector PredictedLocation = TargetLocation;
    
    // If target is moving, predict where they'll be
    if (APawn* TargetPawn = Cast<APawn>(Target))
    {
        FVector Velocity = TargetPawn->GetVelocity();
        if (Velocity.Size() > 10.0f)
        {
            PredictedLocation = TargetLocation + (Velocity.GetSafeNormal() * 500.0f);
        }
    }
    
    // Find position perpendicular to target's movement
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector ToTarget = (PredictedLocation - OwnerLocation).GetSafeNormal();
    FVector AmbushDirection = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    FVector AmbushPosition = PredictedLocation + (AmbushDirection * 300.0f);
    
    // Project to navigation mesh
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(AmbushPosition, NavLocation))
        {
            return NavLocation.Location;
        }
    }
    
    return AmbushPosition;
}

FVector UCombatBehaviorComponent::CalculateCirclingPosition(AActor* Target)
{
    if (!Target)
        return GetOwner()->GetActorLocation();
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Calculate angle around target
    FVector ToOwner = (OwnerLocation - TargetLocation).GetSafeNormal();
    float CurrentAngle = FMath::Atan2(ToOwner.Y, ToOwner.X);
    
    // Move clockwise around target
    float NewAngle = CurrentAngle + (90.0f * PI / 180.0f); // 90 degrees in radians
    
    FVector CirclePosition = TargetLocation + FVector(
        FMath::Cos(NewAngle) * CircleDistance,
        FMath::Sin(NewAngle) * CircleDistance,
        0.0f
    );
    
    // Project to navigation mesh
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(CirclePosition, NavLocation))
        {
            return NavLocation.Location;
        }
    }
    
    return CirclePosition;
}

bool UCombatBehaviorComponent::HasLineOfSight(AActor* Target) const
{
    if (!Target || !GetOwner())
        return false;
    
    FVector Start = GetOwner()->GetActorLocation() + FVector(0, 0, 50); // Eye level
    FVector End = Target->GetActorLocation() + FVector(0, 0, 50);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Target);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // No hit means clear line of sight
}

void UCombatBehaviorComponent::BroadcastIntimidation(float Radius)
{
    // This would integrate with the AI Perception system
    // For now, we'll use a simple radius check
    
    if (!GetOwner())
        return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Make noise that AI Perception can detect
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (OwnerPawn->GetController())
        {
            // Use PawnNoiseEmitter if available
            UPawnNoiseEmitterComponent* NoiseEmitter = OwnerPawn->FindComponentByClass<UPawnNoiseEmitterComponent>();
            if (NoiseEmitter)
            {
                NoiseEmitter->MakeNoise(OwnerPawn, 1.0f, OwnerLocation);
            }
        }
    }
}