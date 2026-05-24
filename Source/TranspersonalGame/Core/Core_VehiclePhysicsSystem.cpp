#include "Core_VehiclePhysicsSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Vehicles/WheeledVehicle.h"
#include "Vehicles/WheeledVehicleMovementComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UCore_VehiclePhysicsSystem::UCore_VehiclePhysicsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    VehicleConfig = FCore_VehiclePhysicsConfig();
    DamageInfo = FCore_VehicleDamageInfo();
    CurrentSurfaceType = ECore_VehicleSurfaceType::Dirt;
    
    GravityScale = 1.0f;
    LinearDamping = 0.01f;
    AngularDamping = 0.05f;
    bEnableAdvancedPhysics = true;
    
    PhysicsUpdateRate = 60.0f;
    bUseAsyncPhysics = true;
    
    // Initialize internal state
    OwnerVehicle = nullptr;
    VehicleMovement = nullptr;
    LastVelocity = FVector::ZeroVector;
    LastAngularVelocity = FVector::ZeroVector;
    LastUpdateTime = 0.0f;
    AccumulatedDeltaTime = 0.0f;
    
    PhysicsUpdateTimer = 0.0f;
    PhysicsCalculationsThisFrame = 0;
    AveragePhysicsTime = 0.0f;
    
    bDebugPhysics = false;
    DebugUpdateInterval = 0.1f;
    LastDebugUpdate = 0.0f;
}

void UCore_VehiclePhysicsSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the owner vehicle
    OwnerVehicle = Cast<AWheeledVehicle>(GetOwner());
    if (OwnerVehicle)
    {
        VehicleMovement = OwnerVehicle->GetVehicleMovementComponent();
        InitializeVehiclePhysics(OwnerVehicle);
        
        UE_LOG(LogTemp, Log, TEXT("Core_VehiclePhysicsSystem: Initialized for vehicle %s"), 
               *OwnerVehicle->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_VehiclePhysicsSystem: Owner is not a WheeledVehicle"));
    }
}

void UCore_VehiclePhysicsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerVehicle || !VehicleMovement)
    {
        return;
    }
    
    // Accumulate delta time for physics updates
    AccumulatedDeltaTime += DeltaTime;
    PhysicsUpdateTimer += DeltaTime;
    
    // Update physics at specified rate
    float PhysicsUpdateInterval = 1.0f / PhysicsUpdateRate;
    if (AccumulatedDeltaTime >= PhysicsUpdateInterval)
    {
        UpdateVehiclePhysics(AccumulatedDeltaTime);
        AccumulatedDeltaTime = 0.0f;
        PhysicsCalculationsThisFrame++;
    }
    
    // Update performance metrics
    if (PhysicsUpdateTimer >= 1.0f)
    {
        AveragePhysicsTime = PhysicsCalculationsThisFrame / PhysicsUpdateTimer;
        PhysicsCalculationsThisFrame = 0;
        PhysicsUpdateTimer = 0.0f;
    }
    
    // Debug rendering
    if (bDebugPhysics && (GetWorld()->GetTimeSeconds() - LastDebugUpdate) >= DebugUpdateInterval)
    {
        LastDebugUpdate = GetWorld()->GetTimeSeconds();
        
        FVector VehicleLocation = OwnerVehicle->GetActorLocation();
        FVector Velocity = VehicleMovement->GetVelocity();
        
        // Draw velocity vector
        DrawDebugLine(GetWorld(), VehicleLocation, VehicleLocation + Velocity * 0.1f, 
                     FColor::Green, false, DebugUpdateInterval, 0, 2.0f);
        
        // Draw surface type indicator
        FColor SurfaceColor = FColor::White;
        switch (CurrentSurfaceType)
        {
            case ECore_VehicleSurfaceType::Road: SurfaceColor = FColor::Black; break;
            case ECore_VehicleSurfaceType::Dirt: SurfaceColor = FColor::Orange; break;
            case ECore_VehicleSurfaceType::Grass: SurfaceColor = FColor::Green; break;
            case ECore_VehicleSurfaceType::Sand: SurfaceColor = FColor::Yellow; break;
            case ECore_VehicleSurfaceType::Mud: SurfaceColor = FColor::Cyan; break;
            case ECore_VehicleSurfaceType::Water: SurfaceColor = FColor::Blue; break;
            default: SurfaceColor = FColor::White; break;
        }
        
        DrawDebugSphere(GetWorld(), VehicleLocation + FVector(0, 0, 100), 50.0f, 
                       12, SurfaceColor, false, DebugUpdateInterval);
    }
}

void UCore_VehiclePhysicsSystem::InitializeVehiclePhysics(AWheeledVehicle* Vehicle)
{
    if (!Vehicle || !VehicleMovement)
    {
        return;
    }
    
    // Apply vehicle configuration to movement component
    if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Vehicle->GetRootComponent()))
    {
        RootPrimitive->SetMassOverrideInKg(NAME_None, VehicleConfig.Mass, true);
        RootPrimitive->SetLinearDamping(LinearDamping);
        RootPrimitive->SetAngularDamping(AngularDamping);
        
        // Apply gravity scale
        RootPrimitive->SetEnableGravity(true);
        if (RootPrimitive->GetBodyInstance())
        {
            RootPrimitive->GetBodyInstance()->bOverrideGravityScale = true;
            RootPrimitive->GetBodyInstance()->GravityScale = GravityScale;
        }
    }
    
    // Configure vehicle movement settings
    VehicleMovement->MaxEngineRPM = VehicleConfig.MaxEngineForce / 10.0f;
    VehicleMovement->EngineSetup.MaxRPM = VehicleConfig.MaxEngineForce / 10.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle physics initialized with mass: %.2f"), VehicleConfig.Mass);
}

void UCore_VehiclePhysicsSystem::UpdateVehiclePhysics(float DeltaTime)
{
    if (!OwnerVehicle || !VehicleMovement)
    {
        return;
    }
    
    // Update surface detection
    DetectSurfaceType();
    
    // Update physics subsystems
    UpdateSuspensionPhysics(DeltaTime);
    UpdateTirePhysics(DeltaTime);
    UpdateEnginePhysics(DeltaTime);
    UpdateAerodynamics(DeltaTime);
    UpdateDamageEffects(DeltaTime);
    
    // Apply surface effects
    ApplySurfaceEffects();
    
    // Store current state for next frame
    LastVelocity = VehicleMovement->GetVelocity();
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UCore_VehiclePhysicsSystem::ApplyEngineForce(float ForceAmount)
{
    if (!VehicleMovement || !IsVehicleOperational())
    {
        return;
    }
    
    // Apply damage modifier
    float EffectiveForce = ForceAmount * (DamageInfo.EngineHealth / 100.0f);
    
    // Apply surface resistance
    float SurfaceMultiplier = GetSurfaceGripMultiplier();
    EffectiveForce *= SurfaceMultiplier;
    
    // Apply to vehicle movement
    VehicleMovement->SetThrottleInput(EffectiveForce);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Applied engine force: %.2f (modified from %.2f)"), 
           EffectiveForce, ForceAmount);
}

void UCore_VehiclePhysicsSystem::ApplyBrakeForce(float BrakeAmount)
{
    if (!VehicleMovement)
    {
        return;
    }
    
    // Apply brake force with surface grip consideration
    float EffectiveBrake = BrakeAmount * GetSurfaceGripMultiplier();
    VehicleMovement->SetBrakeInput(EffectiveBrake);
}

void UCore_VehiclePhysicsSystem::ApplySteeringInput(float SteerAmount)
{
    if (!VehicleMovement)
    {
        return;
    }
    
    // Apply steering with surface grip consideration
    float EffectiveSteer = SteerAmount * GetSurfaceGripMultiplier();
    VehicleMovement->SetSteeringInput(EffectiveSteer);
}

void UCore_VehiclePhysicsSystem::SetSurfaceType(ECore_VehicleSurfaceType SurfaceType)
{
    CurrentSurfaceType = SurfaceType;
    UE_LOG(LogTemp, Log, TEXT("Surface type changed to: %d"), (int32)SurfaceType);
}

float UCore_VehiclePhysicsSystem::GetSurfaceGripMultiplier() const
{
    switch (CurrentSurfaceType)
    {
        case ECore_VehicleSurfaceType::Road: return 1.0f;
        case ECore_VehicleSurfaceType::Dirt: return 0.8f;
        case ECore_VehicleSurfaceType::Grass: return 0.7f;
        case ECore_VehicleSurfaceType::Sand: return 0.6f;
        case ECore_VehicleSurfaceType::Mud: return 0.4f;
        case ECore_VehicleSurfaceType::Snow: return 0.5f;
        case ECore_VehicleSurfaceType::Ice: return 0.2f;
        case ECore_VehicleSurfaceType::Water: return 0.1f;
        case ECore_VehicleSurfaceType::Rock: return 0.9f;
        default: return 1.0f;
    }
}

float UCore_VehiclePhysicsSystem::GetSurfaceResistanceMultiplier() const
{
    switch (CurrentSurfaceType)
    {
        case ECore_VehicleSurfaceType::Road: return 1.0f;
        case ECore_VehicleSurfaceType::Dirt: return 1.2f;
        case ECore_VehicleSurfaceType::Grass: return 1.3f;
        case ECore_VehicleSurfaceType::Sand: return 1.8f;
        case ECore_VehicleSurfaceType::Mud: return 2.5f;
        case ECore_VehicleSurfaceType::Snow: return 1.6f;
        case ECore_VehicleSurfaceType::Ice: return 0.8f;
        case ECore_VehicleSurfaceType::Water: return 5.0f;
        case ECore_VehicleSurfaceType::Rock: return 1.1f;
        default: return 1.0f;
    }
}

void UCore_VehiclePhysicsSystem::ApplyDamage(float DamageAmount, const FString& DamageType)
{
    if (DamageType == "Engine")
    {
        DamageInfo.EngineHealth = FMath::Max(0.0f, DamageInfo.EngineHealth - DamageAmount);
        if (DamageInfo.EngineHealth <= 0.0f)
        {
            DamageInfo.bIsEngineDestroyed = true;
        }
    }
    else if (DamageType == "Transmission")
    {
        DamageInfo.TransmissionHealth = FMath::Max(0.0f, DamageInfo.TransmissionHealth - DamageAmount);
    }
    else if (DamageType == "Suspension")
    {
        DamageInfo.SuspensionHealth = FMath::Max(0.0f, DamageInfo.SuspensionHealth - DamageAmount);
    }
    else if (DamageType == "Tire")
    {
        DamageInfo.TireCondition = FMath::Max(0.0f, DamageInfo.TireCondition - DamageAmount);
    }
    
    // Check if vehicle is immobilized
    if (DamageInfo.EngineHealth <= 20.0f || DamageInfo.TransmissionHealth <= 10.0f)
    {
        DamageInfo.bIsImmobilized = true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Vehicle damaged: %s (%.2f damage)"), *DamageType, DamageAmount);
}

void UCore_VehiclePhysicsSystem::RepairVehicle(float RepairAmount)
{
    DamageInfo.EngineHealth = FMath::Min(100.0f, DamageInfo.EngineHealth + RepairAmount);
    DamageInfo.TransmissionHealth = FMath::Min(100.0f, DamageInfo.TransmissionHealth + RepairAmount);
    DamageInfo.SuspensionHealth = FMath::Min(100.0f, DamageInfo.SuspensionHealth + RepairAmount);
    DamageInfo.TireCondition = FMath::Min(100.0f, DamageInfo.TireCondition + RepairAmount);
    
    // Reset flags if sufficiently repaired
    if (DamageInfo.EngineHealth > 20.0f && DamageInfo.TransmissionHealth > 10.0f)
    {
        DamageInfo.bIsImmobilized = false;
    }
    
    if (DamageInfo.EngineHealth > 0.0f)
    {
        DamageInfo.bIsEngineDestroyed = false;
    }
}

bool UCore_VehiclePhysicsSystem::IsVehicleOperational() const
{
    return !DamageInfo.bIsEngineDestroyed && !DamageInfo.bIsImmobilized;
}

void UCore_VehiclePhysicsSystem::EnablePhysicsDebug(bool bEnable)
{
    bDebugPhysics = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Physics debug %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

FString UCore_VehiclePhysicsSystem::GetVehiclePhysicsStatus() const
{
    if (!OwnerVehicle || !VehicleMovement)
    {
        return TEXT("No vehicle");
    }
    
    FVector Velocity = VehicleMovement->GetVelocity();
    float Speed = Velocity.Size();
    
    return FString::Printf(TEXT("Speed: %.1f | Surface: %d | Engine: %.1f%% | Operational: %s"),
                          Speed, (int32)CurrentSurfaceType, DamageInfo.EngineHealth,
                          IsVehicleOperational() ? TEXT("Yes") : TEXT("No"));
}

float UCore_VehiclePhysicsSystem::GetPhysicsPerformanceMetric() const
{
    return AveragePhysicsTime;
}

void UCore_VehiclePhysicsSystem::OptimizePhysicsSettings()
{
    // Adjust physics update rate based on performance
    if (AveragePhysicsTime > 120.0f)
    {
        PhysicsUpdateRate = FMath::Max(30.0f, PhysicsUpdateRate - 10.0f);
    }
    else if (AveragePhysicsTime < 60.0f)
    {
        PhysicsUpdateRate = FMath::Min(120.0f, PhysicsUpdateRate + 5.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics update rate optimized to: %.1f Hz"), PhysicsUpdateRate);
}

void UCore_VehiclePhysicsSystem::UpdateSuspensionPhysics(float DeltaTime)
{
    // Apply suspension damage effects
    if (DamageInfo.SuspensionHealth < 100.0f)
    {
        float DamageMultiplier = DamageInfo.SuspensionHealth / 100.0f;
        // Reduce suspension effectiveness based on damage
    }
}

void UCore_VehiclePhysicsSystem::UpdateTirePhysics(float DeltaTime)
{
    // Apply tire wear and condition effects
    if (DamageInfo.TireCondition < 100.0f)
    {
        float ConditionMultiplier = DamageInfo.TireCondition / 100.0f;
        // Reduce grip based on tire condition
    }
}

void UCore_VehiclePhysicsSystem::UpdateEnginePhysics(float DeltaTime)
{
    // Apply engine damage effects
    if (DamageInfo.EngineHealth < 100.0f)
    {
        float HealthMultiplier = DamageInfo.EngineHealth / 100.0f;
        // Reduce engine power based on health
    }
}

void UCore_VehiclePhysicsSystem::UpdateAerodynamics(float DeltaTime)
{
    if (!OwnerVehicle || !VehicleMovement)
    {
        return;
    }
    
    FVector Velocity = VehicleMovement->GetVelocity();
    float Speed = Velocity.Size();
    
    // Apply air resistance
    float AirResistanceForce = VehicleConfig.AirResistance * Speed * Speed;
    FVector ResistanceVector = -Velocity.GetSafeNormal() * AirResistanceForce;
    
    if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(OwnerVehicle->GetRootComponent()))
    {
        RootPrimitive->AddForce(ResistanceVector);
    }
}

void UCore_VehiclePhysicsSystem::UpdateDamageEffects(float DeltaTime)
{
    // Gradually worsen damage over time under stress
    if (VehicleMovement && VehicleMovement->GetVelocity().Size() > 1000.0f)
    {
        float StressDamage = DeltaTime * 0.1f; // Very slow degradation
        DamageInfo.EngineHealth = FMath::Max(0.0f, DamageInfo.EngineHealth - StressDamage);
        DamageInfo.TireCondition = FMath::Max(0.0f, DamageInfo.TireCondition - StressDamage * 0.5f);
    }
}

void UCore_VehiclePhysicsSystem::DetectSurfaceType()
{
    if (!OwnerVehicle)
    {
        return;
    }
    
    // Simple surface detection via line trace
    FVector StartLocation = OwnerVehicle->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerVehicle);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, 
                                           ECC_WorldStatic, QueryParams))
    {
        // Determine surface type based on hit material or actor tags
        if (HitResult.GetActor())
        {
            if (HitResult.GetActor()->Tags.Contains("Road"))
            {
                CurrentSurfaceType = ECore_VehicleSurfaceType::Road;
            }
            else if (HitResult.GetActor()->Tags.Contains("Water"))
            {
                CurrentSurfaceType = ECore_VehicleSurfaceType::Water;
            }
            else if (HitResult.GetActor()->Tags.Contains("Sand"))
            {
                CurrentSurfaceType = ECore_VehicleSurfaceType::Sand;
            }
            else
            {
                CurrentSurfaceType = ECore_VehicleSurfaceType::Dirt; // Default
            }
        }
    }
}

void UCore_VehiclePhysicsSystem::ApplySurfaceEffects()
{
    // Apply surface-specific physics modifications
    float GripMultiplier = GetSurfaceGripMultiplier();
    float ResistanceMultiplier = GetSurfaceResistanceMultiplier();
    
    // Modify vehicle movement parameters based on surface
    if (VehicleMovement)
    {
        // Apply resistance as additional drag
        FVector Velocity = VehicleMovement->GetVelocity();
        float AdditionalDrag = (ResistanceMultiplier - 1.0f) * 100.0f;
        
        if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(OwnerVehicle->GetRootComponent()))
        {
            RootPrimitive->SetLinearDamping(LinearDamping + AdditionalDrag * 0.01f);
        }
    }
}

void UCore_VehiclePhysicsSystem::OptimizePhysicsLOD()
{
    // Implement distance-based LOD for physics calculations
    if (GetWorld() && GetWorld()->GetFirstPlayerController())
    {
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        if (PlayerPawn && OwnerVehicle)
        {
            float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), OwnerVehicle->GetActorLocation());
            UpdatePhysicsLOD(Distance);
        }
    }
}

void UCore_VehiclePhysicsSystem::UpdatePhysicsLOD(float DistanceToPlayer)
{
    // Adjust physics fidelity based on distance
    if (DistanceToPlayer > 5000.0f)
    {
        PhysicsUpdateRate = 15.0f; // Low detail
        bEnableAdvancedPhysics = false;
    }
    else if (DistanceToPlayer > 2000.0f)
    {
        PhysicsUpdateRate = 30.0f; // Medium detail
        bEnableAdvancedPhysics = true;
    }
    else
    {
        PhysicsUpdateRate = 60.0f; // High detail
        bEnableAdvancedPhysics = true;
    }
}