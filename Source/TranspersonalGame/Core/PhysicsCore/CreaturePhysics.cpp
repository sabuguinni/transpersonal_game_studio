// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CreaturePhysics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodyInstance.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

// Console variable for debug visualization
static TAutoConsoleVariable<bool> CVarShowPhysicsDebug(
    TEXT("tp.ShowPhysicsDebug"),
    false,
    TEXT("Show debug visualization for creature physics"),
    ECVF_Default
);

UCreaturePhysics::UCreaturePhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values for medium-sized creature (human/velociraptor scale)
    CreatureSize = ECreatureSize::Medium;
    CurrentPhysicsState = ECreaturePhysicsState::Alive;
    CreatureMass = 100.0f;
    CreatureHeight = 180.0f;
    bIsGroundStable = true;
    MaxStableSlope = 45.0f;
    IKAdjustmentStrength = 0.8f;
    RagdollTransitionTime = 1.0f;
    CollisionDamageThreshold = 500.0f;
    GroundTraceDistance = 200.0f;
    
    // Initialize physics scaling data for different creature sizes
    PhysicsScalingData.Add(ECreatureSize::Small, {0.5f, 0.3f, 0.2f, 30.0f});
    PhysicsScalingData.Add(ECreatureSize::Medium, {1.0f, 1.0f, 1.0f, 90.0f});
    PhysicsScalingData.Add(ECreatureSize::Large, {5.0f, 3.0f, 3.0f, 250.0f});
    PhysicsScalingData.Add(ECreatureSize::Massive, {20.0f, 8.0f, 8.0f, 500.0f});
}

void UCreaturePhysics::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to creature's skeletal mesh
    CreatureMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    if (!CreatureMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("CreaturePhysics: No skeletal mesh component found on %s"), 
               *GetOwner()->GetName());
        return;
    }
    
    // Initialize physics settings
    InitializePhysicsForSize();
    ApplyPhysicsConstraints();
    
    UE_LOG(LogTemp, Log, TEXT("CreaturePhysics: Initialized for %s with mass %f kg"), 
           *GetOwner()->GetName(), CreatureMass);
}

void UCreaturePhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Only update ground stability for living creatures
    if (CurrentPhysicsState == ECreaturePhysicsState::Alive)
    {
        UpdateGroundStability();
    }
}

void UCreaturePhysics::ConfigureCreaturePhysics(ECreatureSize CreatureType, float Mass, float Height)
{
    CreatureSize = CreatureType;
    CreatureMass = FMath::Clamp(Mass, 1.0f, 50000.0f);
    CreatureHeight = FMath::Clamp(Height, 10.0f, 2000.0f);
    
    // Update ground trace distance based on height
    GroundTraceDistance = CreatureHeight * 1.2f;
    
    // Reinitialize physics with new parameters
    if (CreatureMesh)
    {
        InitializePhysicsForSize();
        ApplyPhysicsConstraints();
    }
    
    UE_LOG(LogTemp, Log, TEXT("CreaturePhysics: Reconfigured creature physics - Size: %d, Mass: %f kg, Height: %f cm"), 
           static_cast<int32>(CreatureType), Mass, Height);
}

void UCreaturePhysics::ApplyGroundAdaptation(FVector GroundNormal, const TArray<FVector>& FootPositions)
{
    if (!CreatureMesh || CurrentPhysicsState != ECreaturePhysicsState::Alive)
    {
        return;
    }
    
    // Calculate ground slope angle
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GroundNormal, FVector::UpVector)));
    
    // Check if ground is stable
    bIsGroundStable = SlopeAngle <= MaxStableSlope;
    
    // Apply IK adjustments for foot placement
    if (bIsGroundStable && FootPositions.Num() > 0)
    {
        // In a full implementation, this would communicate with the animation system
        // to adjust foot IK targets based on ground surface
        
        for (int32 i = 0; i < FootPositions.Num(); i++)
        {
            // Trace down from each foot position to find ground contact
            FVector TraceStart = FootPositions[i] + FVector::UpVector * 50.0f;
            FVector TraceEnd = FootPositions[i] - FVector::UpVector * 100.0f;
            
            FHitResult HitResult;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(GetOwner());
            
            bool bHit = GetWorld()->LineTraceSingleByChannel(
                HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams);
            
            if (bHit)
            {
                // Calculate IK adjustment (this would be passed to animation system)
                float IKOffset = (FootPositions[i].Z - HitResult.Location.Z) * IKAdjustmentStrength;
                
                // Debug visualization
                if (CVarShowPhysicsDebug.GetValueOnGameThread())
                {
                    DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, FColor::Green, false, 0.1f);
                    DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Yellow, false, 0.1f);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("CreaturePhysics: Ground adaptation - Slope: %f°, Stable: %s"), 
           SlopeAngle, bIsGroundStable ? TEXT("Yes") : TEXT("No"));
}

void UCreaturePhysics::TransitionToDeathState(FVector DeathImpulse, FVector DeathLocation)
{
    if (CurrentPhysicsState == ECreaturePhysicsState::Dead)
    {
        return; // Already dead
    }
    
    // Transition to dying state first
    HandleStateTransition(ECreaturePhysicsState::Dying);
    
    if (!CreatureMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("CreaturePhysics: Cannot transition to death state without skeletal mesh"));
        return;
    }
    
    // Start ragdoll transition timer
    GetWorld()->GetTimerManager().SetTimer(
        RagdollTransitionTimer,
        [this, DeathImpulse, DeathLocation]()
        {
            // Enable full ragdoll physics
            CreatureMesh->SetSimulatePhysics(true);
            CreatureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            CreatureMesh->SetCollisionResponseToAllChannels(ECR_Block);
            CreatureMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
            
            // Apply death impulse with size-appropriate scaling
            const FCreaturePhysicsScaling& Scaling = PhysicsScalingData[CreatureSize];
            FVector ScaledImpulse = DeathImpulse * Scaling.ForceMultiplier;
            
            CreatureMesh->AddImpulseAtLocation(ScaledImpulse, DeathLocation);
            
            // Transition to final dead state
            HandleStateTransition(ECreaturePhysicsState::Dead);
            
            UE_LOG(LogTemp, Log, TEXT("CreaturePhysics: %s transitioned to ragdoll death state with impulse %s"), 
                   *GetOwner()->GetName(), *ScaledImpulse.ToString());
        },
        RagdollTransitionTime,
        false
    );
    
    UE_LOG(LogTemp, Log, TEXT("CreaturePhysics: %s beginning death transition"), *GetOwner()->GetName());
}

void UCreaturePhysics::HandleCreatureCollision(AActor* OtherCreature, float CollisionImpulse, FVector CollisionLocation)
{
    if (!OtherCreature || CurrentPhysicsState == ECreaturePhysicsState::Dead)
    {
        return;
    }
    
    // Get size scaling for damage calculation
    const FCreaturePhysicsScaling& MyScaling = PhysicsScalingData[CreatureSize];
    
    // Calculate collision damage based on impulse and size
    float CollisionDamage = CollisionImpulse / MyScaling.DamageResistance;
    
    if (CollisionDamage > CollisionDamageThreshold)
    {
        // Significant collision - apply stun effect
        if (CurrentPhysicsState == ECreaturePhysicsState::Alive)
        {
            HandleStateTransition(ECreaturePhysicsState::Stunned);
            
            // Auto-recover from stun after a delay
            GetWorld()->GetTimerManager().SetTimer(
                FTimerHandle(),
                [this]()
                {
                    if (CurrentPhysicsState == ECreaturePhysicsState::Stunned)
                    {
                        HandleStateTransition(ECreaturePhysicsState::Alive);
                    }
                },
                2.0f, // 2 second stun duration
                false
            );
        }
        
        // Apply collision response force
        if (CreatureMesh)
        {
            FVector CollisionForce = (GetOwner()->GetActorLocation() - CollisionLocation).GetSafeNormal();
            CollisionForce *= CollisionImpulse * 0.5f; // Reduce force for realism
            
            CreatureMesh->AddImpulseAtLocation(CollisionForce, CollisionLocation);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("CreaturePhysics: %s collision with %s - Impulse: %f, Damage: %f"), 
           *GetOwner()->GetName(), *OtherCreature->GetName(), CollisionImpulse, CollisionDamage);
}

void UCreaturePhysics::InitializePhysicsForSize()
{
    if (!CreatureMesh)
    {
        return;
    }
    
    const FCreaturePhysicsScaling& Scaling = PhysicsScalingData[CreatureSize];
    
    // Set mass based on creature size
    float ScaledMass = CreatureMass * Scaling.MassMultiplier;
    
    // Apply mass to all physics bodies
    if (UPhysicsAsset* PhysicsAsset = CreatureMesh->GetPhysicsAsset())
    {
        for (int32 i = 0; i < PhysicsAsset->SkeletalBodySetups.Num(); i++)
        {
            if (UBodySetup* BodySetup = PhysicsAsset->SkeletalBodySetups[i])
            {
                // Set mass properties based on creature size
                BodySetup->PhysicsType = EPhysicsType::PhysType_Simulated;
                
                // Calculate mass distribution - larger creatures have proportionally more mass in torso
                float BodyMassRatio = 1.0f;
                FString BoneName = BodySetup->BoneName.ToString().ToLower();
                
                if (BoneName.Contains(TEXT("spine")) || BoneName.Contains(TEXT("pelvis")) || BoneName.Contains(TEXT("chest")))
                {
                    BodyMassRatio = 3.0f; // Torso carries most mass
                }
                else if (BoneName.Contains(TEXT("head")) || BoneName.Contains(TEXT("skull")))
                {
                    BodyMassRatio = 1.5f; // Head is heavier for brain
                }
                else if (BoneName.Contains(TEXT("leg")) || BoneName.Contains(TEXT("thigh")))
                {
                    BodyMassRatio = 2.0f; // Legs need mass for stability
                }
                
                // Apply calculated mass
                float FinalMass = (ScaledMass / PhysicsAsset->SkeletalBodySetups.Num()) * BodyMassRatio;
                
                // Configure physics material properties
                BodySetup->PhysMaterial = nullptr; // Use default for now
                
                UE_LOG(LogTemp, VeryVerbose, TEXT("CreaturePhysics: Set mass %f for bone %s"), 
                       FinalMass, *BodySetup->BoneName.ToString());
            }
        }
    }
    
    // Set collision properties based on creature size
    CreatureMesh->SetCollisionObjectType(ECC_Pawn);
    CreatureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CreatureMesh->SetCollisionResponseToAllChannels(ECR_Block);
    CreatureMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    
    UE_LOG(LogTemp, Log, TEXT("CreaturePhysics: Physics initialized for %s creature with scaled mass %f kg"), 
           *UEnum::GetValueAsString(CreatureSize), ScaledMass);
}

void UCreaturePhysics::UpdateGroundStability()
{
    if (!CreatureMesh || !GetWorld())
    {
        return;
    }
    
    // Trace downward to check ground surface
    FVector TraceStart = GetOwner()->GetActorLocation();
    FVector TraceEnd = TraceStart - FVector::UpVector * GroundTraceDistance;
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = false; // Use simple collision for performance
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams);
    
    if (bHit)
    {
        // Calculate slope angle
        FVector GroundNormal = HitResult.Normal;
        float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GroundNormal, FVector::UpVector)));
        
        // Update stability
        bool bWasStable = bIsGroundStable;
        bIsGroundStable = SlopeAngle <= MaxStableSlope;
        
        // Log stability changes
        if (bWasStable != bIsGroundStable)
        {
            UE_LOG(LogTemp, Log, TEXT("CreaturePhysics: %s ground stability changed to %s (slope: %f°)"), 
                   *GetOwner()->GetName(), 
                   bIsGroundStable ? TEXT("stable") : TEXT("unstable"), 
                   SlopeAngle);
        }
        
        // Debug visualization
        if (CVarShowPhysicsDebug.GetValueOnGameThread())
        {
            FColor DebugColor = bIsGroundStable ? FColor::Green : FColor::Red;
            DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, DebugColor, false, 0.1f);
            DrawDebugString(GetWorld(), HitResult.Location, 
                           FString::Printf(TEXT("Slope: %.1f°"), SlopeAngle), 
                           nullptr, DebugColor, 0.1f);
        }
    }
    else
    {
        // No ground detected - creature is falling
        bIsGroundStable = false;
        
        if (CVarShowPhysicsDebug.GetValueOnGameThread())
        {
            DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Orange, false, 0.1f);
        }
    }
}

void UCreaturePhysics::ApplyPhysicsConstraints()
{
    if (!CreatureMesh)
    {
        return;
    }
    
    const FCreaturePhysicsScaling& Scaling = PhysicsScalingData[CreatureSize];
    
    // Apply size-appropriate physics constraints
    // Larger creatures should be more stable but less agile
    
    if (UPhysicsAsset* PhysicsAsset = CreatureMesh->GetPhysicsAsset())
    {
        // Configure constraints based on creature size
        for (int32 i = 0; i < PhysicsAsset->ConstraintSetup.Num(); i++)
        {
            if (UPhysicsConstraintTemplate* Constraint = PhysicsAsset->ConstraintSetup[i])
            {
                // Larger creatures have stronger but less flexible joints
                float ConstraintStrength = FMath::Lerp(1000.0f, 10000.0f, Scaling.MassMultiplier / 20.0f);
                float AngularLimits = FMath::Lerp(90.0f, 30.0f, Scaling.MassMultiplier / 20.0f);
                
                // Apply constraint modifications
                // Note: In a full implementation, these would be set through the constraint template
                UE_LOG(LogTemp, VeryVerbose, TEXT("CreaturePhysics: Constraint %s - Strength: %f, Limits: %f°"), 
                       *Constraint->DefaultInstance.ConstraintBone1.ToString(), 
                       ConstraintStrength, AngularLimits);
            }
        }
    }
    
    // Set physics simulation properties
    CreatureMesh->SetNotifyRigidBodyCollision(true);
    CreatureMesh->SetGenerateOverlapEvents(true);
    
    UE_LOG(LogTemp, Log, TEXT("CreaturePhysics: Applied physics constraints for %s creature"), 
           *UEnum::GetValueAsString(CreatureSize));
}

void UCreaturePhysics::HandleStateTransition(ECreaturePhysicsState NewState)
{
    if (CurrentPhysicsState == NewState)
    {
        return;
    }
    
    ECreaturePhysicsState OldState = CurrentPhysicsState;
    CurrentPhysicsState = NewState;
    
    // Handle state-specific logic
    switch (NewState)
    {
        case ECreaturePhysicsState::Alive:
            // Restore normal physics simulation
            if (CreatureMesh)
            {
                CreatureMesh->SetSimulatePhysics(false);
                CreatureMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            }
            break;
            
        case ECreaturePhysicsState::Stunned:
            // Reduce movement capability but maintain collision
            if (CreatureMesh)
            {
                CreatureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
            break;
            
        case ECreaturePhysicsState::Dying:
            // Begin transition to ragdoll
            if (CreatureMesh)
            {
                CreatureMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
            }
            break;
            
        case ECreaturePhysicsState::Dead:
            // Full ragdoll physics enabled
            if (CreatureMesh)
            {
                CreatureMesh->SetSimulatePhysics(true);
                CreatureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                CreatureMesh->SetCollisionResponseToAllChannels(ECR_Block);
                CreatureMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
            }
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("CreaturePhysics: %s state transition from %s to %s"), 
           *GetOwner()->GetName(),
           *UEnum::GetValueAsString(OldState),
           *UEnum::GetValueAsString(NewState));
}