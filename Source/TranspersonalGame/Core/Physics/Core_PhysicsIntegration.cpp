#include "Core_PhysicsIntegration.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

UCore_PhysicsIntegration::UCore_PhysicsIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default physics properties
    Friction = 0.7f;
    Restitution = 0.3f;
    Density = 1.0f;
    
    // Initialize collision settings
    CollisionObjectType = ECC_WorldDynamic;
    CollisionResponseToWorld = ECR_Block;
    CollisionResponseToPawn = ECR_Block;
    CollisionResponseToVehicle = ECR_Block;
    
    // Initialize performance settings
    bOptimizePerformance = true;
    PhysicsLODLevel = 1;
    MaxSimulationDistance = 5000.0f;
    CullingDistance = 10000.0f;
    
    // Initialize physics state
    bPhysicsEnabled = true;
    bGravityEnabled = true;
    CustomGravity = FVector(0.0f, 0.0f, -980.0f);
    
    // Initialize force multipliers
    MaxForceMultiplier = 1.0f;
    MaxImpulseMultiplier = 1.0f;
    MaxTorqueMultiplier = 1.0f;
    
    // Initialize internal variables
    LastPerformanceCheck = 0.0f;
    PhysicsObjectCount = 0;
    AverageFrameTime = 0.016f; // 60 FPS baseline
}

void UCore_PhysicsIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Component initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_PhysicsIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bOptimizePerformance)
    {
        UpdatePhysicsLOD();
        
        // Performance monitoring every second
        LastPerformanceCheck += DeltaTime;
        if (LastPerformanceCheck >= 1.0f)
        {
            ValidatePhysicsState();
            LastPerformanceCheck = 0.0f;
        }
    }
    
    // Update average frame time for performance tracking
    AverageFrameTime = (AverageFrameTime * 0.9f) + (DeltaTime * 0.1f);
}

void UCore_PhysicsIntegration::SetPhysicsMaterial(UPhysicalMaterial* Material)
{
    if (!Material)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegration: Attempted to set null physics material"));
        return;
    }
    
    CurrentPhysicsMaterial = Material;
    
    // Apply material properties to owner's primitive component
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->SetPhysMaterialOverride(Material);
            
            // Update local properties from material
            Friction = Material->Friction;
            Restitution = Material->Restitution;
            Density = Material->Density;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Physics material set to %s"), *Material->GetName());
}

UPhysicalMaterial* UCore_PhysicsIntegration::GetCurrentPhysicsMaterial() const
{
    return CurrentPhysicsMaterial;
}

void UCore_PhysicsIntegration::ConfigureCollisionResponse(ECollisionResponse ResponseType)
{
    CollisionResponseToWorld = ResponseType;
    CollisionResponseToPawn = ResponseType;
    CollisionResponseToVehicle = ResponseType;
    
    // Apply to owner's primitive component
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->SetCollisionResponseToAllChannels(ResponseType);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Collision response configured"));
}

void UCore_PhysicsIntegration::SetCollisionObjectType(ECollisionChannel ObjectType)
{
    CollisionObjectType = ObjectType;
    
    // Apply to owner's primitive component
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->SetCollisionObjectType(ObjectType);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Collision object type set"));
}

void UCore_PhysicsIntegration::OptimizePhysicsPerformance(bool bEnable)
{
    bOptimizePerformance = bEnable;
    
    if (bEnable)
    {
        UpdatePhysicsLOD();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Physics optimization %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_PhysicsIntegration::SetPhysicsLOD(int32 LODLevel)
{
    PhysicsLODLevel = FMath::Clamp(LODLevel, 0, 3);
    
    if (bOptimizePerformance)
    {
        UpdatePhysicsLOD();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Physics LOD set to %d"), PhysicsLODLevel);
}

void UCore_PhysicsIntegration::EnablePhysicsSimulation(bool bEnable)
{
    bPhysicsEnabled = bEnable;
    
    // Apply to owner's primitive component
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->SetSimulatePhysics(bEnable);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Physics simulation %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

bool UCore_PhysicsIntegration::IsPhysicsSimulationEnabled() const
{
    return bPhysicsEnabled;
}

void UCore_PhysicsIntegration::ApplyForce(const FVector& Force, const FVector& Location)
{
    if (!bPhysicsEnabled)
    {
        return;
    }
    
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            FVector ScaledForce = Force * MaxForceMultiplier;
            PrimComp->AddForceAtLocation(ScaledForce, Location);
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsIntegration: Applied force %s at location %s"), 
                   *ScaledForce.ToString(), *Location.ToString());
        }
    }
}

void UCore_PhysicsIntegration::ApplyImpulse(const FVector& Impulse, const FVector& Location)
{
    if (!bPhysicsEnabled)
    {
        return;
    }
    
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            FVector ScaledImpulse = Impulse * MaxImpulseMultiplier;
            PrimComp->AddImpulseAtLocation(ScaledImpulse, Location);
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsIntegration: Applied impulse %s at location %s"), 
                   *ScaledImpulse.ToString(), *Location.ToString());
        }
    }
}

void UCore_PhysicsIntegration::ApplyTorque(const FVector& Torque)
{
    if (!bPhysicsEnabled)
    {
        return;
    }
    
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            FVector ScaledTorque = Torque * MaxTorqueMultiplier;
            PrimComp->AddTorqueInRadians(ScaledTorque);
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsIntegration: Applied torque %s"), 
                   *ScaledTorque.ToString());
        }
    }
}

bool UCore_PhysicsIntegration::LineTrace(const FVector& Start, const FVector& End, FHitResult& HitResult)
{
    if (UWorld* World = GetWorld())
    {
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        QueryParams.bTraceComplex = false;
        QueryParams.bReturnPhysicalMaterial = true;
        
        bool bHit = World->LineTraceSingleByChannel(
            HitResult,
            Start,
            End,
            ECC_WorldStatic,
            QueryParams
        );
        
        return bHit;
    }
    
    return false;
}

bool UCore_PhysicsIntegration::SphereTrace(const FVector& Start, const FVector& End, float Radius, FHitResult& HitResult)
{
    if (UWorld* World = GetWorld())
    {
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        QueryParams.bTraceComplex = false;
        QueryParams.bReturnPhysicalMaterial = true;
        
        bool bHit = World->SweepSingleByChannel(
            HitResult,
            Start,
            End,
            FQuat::Identity,
            ECC_WorldStatic,
            FCollisionShape::MakeSphere(Radius),
            QueryParams
        );
        
        return bHit;
    }
    
    return false;
}

bool UCore_PhysicsIntegration::BoxTrace(const FVector& Start, const FVector& End, const FVector& HalfSize, FHitResult& HitResult)
{
    if (UWorld* World = GetWorld())
    {
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        QueryParams.bTraceComplex = false;
        QueryParams.bReturnPhysicalMaterial = true;
        
        bool bHit = World->SweepSingleByChannel(
            HitResult,
            Start,
            End,
            FQuat::Identity,
            ECC_WorldStatic,
            FCollisionShape::MakeBox(HalfSize),
            QueryParams
        );
        
        return bHit;
    }
    
    return false;
}

void UCore_PhysicsIntegration::InitializePhysicsSettings()
{
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            // Apply initial physics settings
            PrimComp->SetSimulatePhysics(bPhysicsEnabled);
            PrimComp->SetEnableGravity(bGravityEnabled);
            PrimComp->SetCollisionObjectType(CollisionObjectType);
            PrimComp->SetCollisionResponseToAllChannels(CollisionResponseToWorld);
            
            // Apply custom gravity if set
            if (!CustomGravity.Equals(FVector(0.0f, 0.0f, -980.0f)))
            {
                // Note: Custom gravity requires world-level changes in UE5
                UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Custom gravity detected but requires world-level implementation"));
            }
        }
    }
}

void UCore_PhysicsIntegration::UpdatePhysicsLOD()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Calculate distance to player
    float DistanceToPlayer = 0.0f;
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
        {
            DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
        }
    }
    
    // Apply LOD based on distance and performance settings
    if (UPrimitiveComponent* PrimComp = GetOwner()->FindComponentByClass<UPrimitiveComponent>())
    {
        if (DistanceToPlayer > CullingDistance)
        {
            // Disable physics simulation at extreme distances
            PrimComp->SetSimulatePhysics(false);
        }
        else if (DistanceToPlayer > MaxSimulationDistance)
        {
            // Reduce physics complexity
            PrimComp->SetSimulatePhysics(false);
        }
        else
        {
            // Full physics simulation within range
            PrimComp->SetSimulatePhysics(bPhysicsEnabled);
        }
    }
}

void UCore_PhysicsIntegration::ValidatePhysicsState()
{
    // Count physics objects in the world for performance monitoring
    PhysicsObjectCount = 0;
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (AActor* Actor = *ActorItr)
            {
                if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        PhysicsObjectCount++;
                    }
                }
            }
        }
    }
    
    // Log performance warnings if needed
    if (AverageFrameTime > 0.033f) // Below 30 FPS
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegration: Performance warning - Average frame time: %f, Physics objects: %d"), 
               AverageFrameTime, PhysicsObjectCount);
    }
    
    // Auto-optimize if performance is poor
    if (AverageFrameTime > 0.05f && PhysicsObjectCount > 100) // Below 20 FPS with many objects
    {
        PhysicsLODLevel = FMath::Min(PhysicsLODLevel + 1, 3);
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Auto-increasing LOD level to %d due to performance"), PhysicsLODLevel);
    }
}