#include "Core_DinosaurPhysicsIntegrator.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"

UCore_DinosaurPhysicsIntegrator::UCore_DinosaurPhysicsIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Set reasonable default values
    BaseMassMultiplier = 1.0f;
    MovementForceMultiplier = 1.0f;
    CollisionResponseStrength = 1.0f;
    bEnableRealisticPhysics = true;
    bEnableRagdollPhysics = true;
    bRagdollActive = false;
    CachedMass = 100.0f;
    CurrentTerrainFriction = 1.0f;
    
    // Debug settings
    bDebugDrawForces = false;
    bDebugDrawCollisions = false;
}

void UCore_DinosaurPhysicsIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize physics components
    InitializePhysicsComponents();
    
    // Setup ragdoll constraints if enabled
    if (bEnableRagdollPhysics)
    {
        SetupRagdollConstraints();
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysicsIntegrator initialized for: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_DinosaurPhysicsIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableRealisticPhysics)
        return;
    
    // Update physics properties
    UpdatePhysicsProperties(DeltaTime);
    
    // Draw debug information if enabled
    if (bDebugDrawForces || bDebugDrawCollisions)
    {
        DrawDebugPhysics(DeltaTime);
    }
}

void UCore_DinosaurPhysicsIntegrator::ApplyMovementPhysics(FVector MovementDirection, float MovementSpeed)
{
    if (!ValidatePhysicsComponents() || !bEnableRealisticPhysics)
        return;
    
    // Calculate mass-based movement force
    FVector MovementForce = MovementDirection * MovementSpeed * MovementForceMultiplier;
    MovementForce = CalculateMassBasedForce(MovementForce);
    
    // Apply force to physics component
    if (OwnerMeshComponent)
    {
        OwnerMeshComponent->AddForce(MovementForce, NAME_None, true);
    }
    else if (OwnerStaticMeshComponent)
    {
        OwnerStaticMeshComponent->AddForce(MovementForce, NAME_None, true);
    }
    
    // Debug visualization
    if (bDebugDrawForces && GetWorld())
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        DrawDebugLine(GetWorld(), OwnerLocation, OwnerLocation + MovementForce * 0.01f, 
                     FColor::Green, false, 0.1f, 0, 2.0f);
    }
}

void UCore_DinosaurPhysicsIntegrator::ApplyTurningPhysics(float TurnInput, float DeltaTime)
{
    if (!ValidatePhysicsComponents() || !bEnableRealisticPhysics)
        return;
    
    // Calculate turning torque based on mass and size
    float TurningTorque = TurnInput * CachedMass * 0.1f * DeltaTime;
    FVector TorqueVector = FVector(0, 0, TurningTorque);
    
    // Apply torque for realistic turning
    if (OwnerMeshComponent)
    {
        OwnerMeshComponent->AddTorqueInRadians(TorqueVector, NAME_None, true);
    }
    else if (OwnerStaticMeshComponent)
    {
        OwnerStaticMeshComponent->AddTorqueInRadians(TorqueVector, NAME_None, true);
    }
}

void UCore_DinosaurPhysicsIntegrator::HandleSlopePhysics(FVector SlopeNormal, float SlopeAngle)
{
    if (!ValidatePhysicsComponents() || !bEnableRealisticPhysics)
        return;
    
    // Calculate slope resistance force
    float SlopeResistance = FMath::Sin(FMath::DegreesToRadians(SlopeAngle)) * CachedMass * 9.81f;
    FVector SlopeForce = -SlopeNormal * SlopeResistance * CurrentTerrainFriction;
    
    // Apply slope physics
    if (OwnerMeshComponent)
    {
        OwnerMeshComponent->AddForce(SlopeForce, NAME_None, true);
    }
    else if (OwnerStaticMeshComponent)
    {
        OwnerStaticMeshComponent->AddForce(SlopeForce, NAME_None, true);
    }
}

void UCore_DinosaurPhysicsIntegrator::HandleCollisionPhysics(AActor* OtherActor, FVector ImpactPoint, FVector ImpactNormal, float ImpactForce)
{
    if (!ValidatePhysicsComponents() || !bEnableRealisticPhysics || !OtherActor)
        return;
    
    // Calculate collision response based on masses
    float OtherMass = 100.0f; // Default mass
    if (UPrimitiveComponent* OtherPrimitive = OtherActor->FindComponentByClass<UPrimitiveComponent>())
    {
        OtherMass = OtherPrimitive->GetMass();
    }
    
    // Calculate response force
    float MassRatio = CachedMass / (CachedMass + OtherMass);
    FVector ResponseForce = ImpactNormal * ImpactForce * MassRatio * CollisionResponseStrength;
    
    // Apply collision response
    ApplyKnockbackForce(-ResponseForce, ImpactForce * 0.1f);
    
    // Debug visualization
    if (bDebugDrawCollisions && GetWorld())
    {
        DrawDebugSphere(GetWorld(), ImpactPoint, 50.0f, 12, FColor::Red, false, 2.0f);
        DrawDebugLine(GetWorld(), ImpactPoint, ImpactPoint + ImpactNormal * 200.0f, 
                     FColor::Yellow, false, 2.0f, 0, 3.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Collision with %s, Force: %f"), 
           *OtherActor->GetName(), ImpactForce);
}

void UCore_DinosaurPhysicsIntegrator::ApplyKnockbackForce(FVector ForceDirection, float ForceStrength)
{
    if (!ValidatePhysicsComponents() || !bEnableRealisticPhysics)
        return;
    
    // Calculate knockback force based on mass
    FVector KnockbackForce = ForceDirection.GetSafeNormal() * ForceStrength;
    KnockbackForce = CalculateMassBasedForce(KnockbackForce);
    
    // Apply knockback
    if (OwnerMeshComponent)
    {
        OwnerMeshComponent->AddImpulse(KnockbackForce, NAME_None, true);
    }
    else if (OwnerStaticMeshComponent)
    {
        OwnerStaticMeshComponent->AddImpulse(KnockbackForce, NAME_None, true);
    }
}

void UCore_DinosaurPhysicsIntegrator::HandleEnvironmentalCollision(FVector ImpactPoint, float ImpactMagnitude)
{
    if (!ValidatePhysicsComponents() || !bEnableRealisticPhysics)
        return;
    
    // Handle collision with environment (trees, rocks, terrain)
    if (ImpactMagnitude > CachedMass * 0.5f) // Significant impact
    {
        // Apply damage or effects based on impact magnitude
        float DamageAmount = CalculateImpactDamage(ImpactMagnitude, CachedMass);
        
        UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Environmental collision damage: %f"), DamageAmount);
        
        // Could trigger damage events or status effects here
        // For now, just log the impact
    }
}

void UCore_DinosaurPhysicsIntegrator::ActivateRagdoll()
{
    if (!bEnableRagdollPhysics || bRagdollActive || !OwnerMeshComponent)
        return;
    
    // Activate ragdoll physics
    OwnerMeshComponent->SetSimulatePhysics(true);
    OwnerMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    OwnerMeshComponent->SetAllBodiesSimulatePhysics(true);
    
    bRagdollActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Ragdoll activated for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_DinosaurPhysicsIntegrator::DeactivateRagdoll()
{
    if (!bRagdollActive || !OwnerMeshComponent)
        return;
    
    // Deactivate ragdoll physics
    OwnerMeshComponent->SetSimulatePhysics(false);
    OwnerMeshComponent->SetAllBodiesSimulatePhysics(false);
    
    // Reset to kinematic for animation
    OwnerMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    bRagdollActive = false;
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Ragdoll deactivated for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_DinosaurPhysicsIntegrator::ApplyAttackImpact(FVector ImpactLocation, FVector ImpactDirection, float ImpactForce)
{
    if (!ValidatePhysicsComponents() || !bEnableRealisticPhysics)
        return;
    
    // Apply localized impact force
    FVector LocalizedForce = ImpactDirection.GetSafeNormal() * ImpactForce;
    
    if (OwnerMeshComponent)
    {
        OwnerMeshComponent->AddImpulseAtLocation(LocalizedForce, ImpactLocation, NAME_None);
    }
    else if (OwnerStaticMeshComponent)
    {
        OwnerStaticMeshComponent->AddImpulseAtLocation(LocalizedForce, ImpactLocation, NAME_None);
    }
    
    // Visual feedback
    if (bDebugDrawCollisions && GetWorld())
    {
        DrawDebugSphere(GetWorld(), ImpactLocation, 30.0f, 8, FColor::Orange, false, 1.0f);
    }
}

void UCore_DinosaurPhysicsIntegrator::HandleFallImpact(float FallHeight, FVector ImpactVelocity)
{
    if (!ValidatePhysicsComponents() || !bEnableRealisticPhysics)
        return;
    
    // Calculate fall damage based on height and velocity
    float FallForce = ImpactVelocity.Size() * CachedMass * 0.1f;
    float FallDamage = CalculateImpactDamage(FallForce, CachedMass);
    
    // Apply fall effects
    if (FallHeight > 500.0f) // Significant fall
    {
        // Could trigger stumble animation or temporary movement impairment
        UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Significant fall impact - Height: %f, Damage: %f"), 
               FallHeight, FallDamage);
    }
}

float UCore_DinosaurPhysicsIntegrator::CalculateImpactDamage(float ImpactForce, float Mass) const
{
    // Simple damage calculation based on force and mass
    float BaseDamage = ImpactForce / (Mass * 0.1f);
    return FMath::Clamp(BaseDamage, 0.0f, 100.0f);
}

void UCore_DinosaurPhysicsIntegrator::AdaptToTerrain(FVector GroundNormal, float TerrainFriction)
{
    CurrentTerrainFriction = FMath::Clamp(TerrainFriction, 0.1f, 2.0f);
    
    // Adjust physics properties based on terrain
    if (ValidatePhysicsComponents())
    {
        if (OwnerMeshComponent)
        {
            // Adjust friction and restitution
            OwnerMeshComponent->SetPhysMaterialOverride(nullptr); // Could set terrain-specific materials
        }
        else if (OwnerStaticMeshComponent)
        {
            OwnerStaticMeshComponent->SetPhysMaterialOverride(nullptr);
        }
    }
}

void UCore_DinosaurPhysicsIntegrator::HandleFootPlacement(FVector FootLocation, FVector GroundNormal)
{
    // This would typically be used with IK systems for foot placement
    // For now, just store the ground normal for physics calculations
    if (!GroundNormal.IsNearlyZero())
    {
        // Could adjust stance or apply corrective forces based on ground normal
        UE_LOG(LogTemp, VeryVerbose, TEXT("DinosaurPhysics: Foot placement at %s, Normal: %s"), 
               *FootLocation.ToString(), *GroundNormal.ToString());
    }
}

void UCore_DinosaurPhysicsIntegrator::InitializePhysicsComponents()
{
    if (!GetOwner())
        return;
    
    // Find mesh components
    OwnerMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    if (!OwnerMeshComponent)
    {
        OwnerStaticMeshComponent = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    }
    
    // Cache mass from physics component
    if (OwnerMeshComponent && OwnerMeshComponent->GetBodyInstance())
    {
        CachedMass = OwnerMeshComponent->GetMass();
    }
    else if (OwnerStaticMeshComponent && OwnerStaticMeshComponent->GetBodyInstance())
    {
        CachedMass = OwnerStaticMeshComponent->GetMass();
    }
    
    // Apply mass multiplier
    CachedMass *= BaseMassMultiplier;
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Initialized with mass: %f"), CachedMass);
}

void UCore_DinosaurPhysicsIntegrator::UpdatePhysicsProperties(float DeltaTime)
{
    if (!ValidatePhysicsComponents())
        return;
    
    // Update mass if it has changed
    float CurrentMass = 0.0f;
    if (OwnerMeshComponent && OwnerMeshComponent->GetBodyInstance())
    {
        CurrentMass = OwnerMeshComponent->GetMass();
    }
    else if (OwnerStaticMeshComponent && OwnerStaticMeshComponent->GetBodyInstance())
    {
        CurrentMass = OwnerStaticMeshComponent->GetMass();
    }
    
    if (FMath::Abs(CurrentMass - CachedMass) > 1.0f)
    {
        CachedMass = CurrentMass * BaseMassMultiplier;
    }
}

FVector UCore_DinosaurPhysicsIntegrator::CalculateMassBasedForce(FVector InputForce) const
{
    // Scale force based on mass (heavier dinosaurs need more force for same acceleration)
    float MassScale = CachedMass / 100.0f; // Normalize to 100kg baseline
    return InputForce * MassScale;
}

void UCore_DinosaurPhysicsIntegrator::SetupRagdollConstraints()
{
    if (!OwnerMeshComponent)
        return;
    
    // This would set up physics constraints for ragdoll behavior
    // For now, just prepare the component for ragdoll activation
    OwnerMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    OwnerMeshComponent->SetSimulatePhysics(false);
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Ragdoll constraints setup complete"));
}

void UCore_DinosaurPhysicsIntegrator::CleanupRagdollConstraints()
{
    // Clean up any created constraint components
    for (UPhysicsConstraintComponent* Constraint : RagdollConstraints)
    {
        if (Constraint && IsValid(Constraint))
        {
            Constraint->DestroyComponent();
        }
    }
    RagdollConstraints.Empty();
}

bool UCore_DinosaurPhysicsIntegrator::ValidatePhysicsComponents() const
{
    return (OwnerMeshComponent && IsValid(OwnerMeshComponent)) || 
           (OwnerStaticMeshComponent && IsValid(OwnerStaticMeshComponent));
}

void UCore_DinosaurPhysicsIntegrator::DrawDebugPhysics(float DeltaTime)
{
    if (!GetWorld() || !GetOwner())
        return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Draw mass indicator
    if (bDebugDrawForces)
    {
        FString MassText = FString::Printf(TEXT("Mass: %.1f kg"), CachedMass);
        DrawDebugString(GetWorld(), OwnerLocation + FVector(0, 0, 200), MassText, 
                       nullptr, FColor::White, 0.0f);
    }
    
    // Draw terrain friction indicator
    if (bDebugDrawCollisions)
    {
        FString FrictionText = FString::Printf(TEXT("Friction: %.2f"), CurrentTerrainFriction);
        DrawDebugString(GetWorld(), OwnerLocation + FVector(0, 0, 150), FrictionText, 
                       nullptr, FColor::Yellow, 0.0f);
    }
}