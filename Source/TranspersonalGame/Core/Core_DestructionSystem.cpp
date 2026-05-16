#include "Core_DestructionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemActor.h"
#include "Field/FieldSystemComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize destruction parameters
    DestructionParams = FCore_DestructionParams();
    
    // Initialize runtime state
    bIsDestroyed = false;
    LastImpactTime = 0.0f;
    
    // Initialize component references
    TargetMeshComponent = nullptr;
    GeometryCollectionComponent = nullptr;
    FieldSystemComponent = nullptr;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Setup Chaos Physics integration
    SetupChaosPhysics();
    
    // Find target mesh component on owner
    if (AActor* Owner = GetOwner())
    {
        TargetMeshComponent = Owner->FindComponentByClass<UStaticMeshComponent>();
        if (TargetMeshComponent)
        {
            UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Found target mesh component on %s"), *Owner->GetName());
        }
    }
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update fragment cleanup if needed
    if (FragmentActors.Num() > 0)
    {
        // Remove null references
        FragmentActors.RemoveAll([](const TObjectPtr<AActor>& Actor) {
            return !IsValid(Actor.Get());
        });
    }
}

void UCore_DestructionSystem::InitializeDestruction(UStaticMeshComponent* MeshComponent, const FCore_DestructionParams& Params)
{
    if (!IsValid(MeshComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Invalid mesh component provided"));
        return;
    }
    
    TargetMeshComponent = MeshComponent;
    DestructionParams = Params;
    
    // Setup physics properties for destruction
    if (TargetMeshComponent)
    {
        TargetMeshComponent->SetSimulatePhysics(true);
        TargetMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        TargetMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        
        // Enable contact events for impact detection
        TargetMeshComponent->SetNotifyRigidBodyCollision(true);
    }
    
    // Create Geometry Collection if requested
    if (DestructionParams.bUseGeometryCollection && TargetMeshComponent && TargetMeshComponent->GetStaticMesh())
    {
        CreateGeometryCollection(TargetMeshComponent->GetStaticMesh());
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Initialized destruction for %s"), 
           TargetMeshComponent ? *TargetMeshComponent->GetName() : TEXT("Unknown"));
}

void UCore_DestructionSystem::TriggerDestruction(const FVector& ImpactLocation, const FVector& ImpactForce)
{
    if (bIsDestroyed || !CanBeDestroyed())
    {
        return;
    }
    
    float ImpactMagnitude = ImpactForce.Size();
    if (ImpactMagnitude < DestructionParams.ImpactThreshold)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Impact force %f below threshold %f"), 
               ImpactMagnitude, DestructionParams.ImpactThreshold);
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Triggering destruction type %d at location %s"), 
           (int32)DestructionParams.DestructionType, *ImpactLocation.ToString());
    
    // Execute destruction based on type
    switch (DestructionParams.DestructionType)
    {
        case ECore_DestructionType::Fracture:
            ExecuteFractureDestruction(ImpactLocation, ImpactForce);
            break;
        case ECore_DestructionType::Shatter:
            ExecuteShatterDestruction(ImpactLocation, ImpactForce);
            break;
        case ECore_DestructionType::Crumble:
            ExecuteCrumbleDestruction(ImpactLocation, ImpactForce);
            break;
        case ECore_DestructionType::Explode:
            ExecuteExplodeDestruction(ImpactLocation, ImpactForce);
            break;
        default:
            break;
    }
    
    bIsDestroyed = true;
    LastImpactTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Schedule fragment cleanup
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(FragmentCleanupTimer, this, 
            &UCore_DestructionSystem::CleanupFragments, DestructionParams.FragmentLifetime, false);
    }
}

void UCore_DestructionSystem::SetDestructionThreshold(float NewThreshold)
{
    DestructionParams.ImpactThreshold = FMath::Max(0.0f, NewThreshold);
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Destruction threshold set to %f"), NewThreshold);
}

bool UCore_DestructionSystem::CanBeDestroyed() const
{
    return !bIsDestroyed && IsValid(TargetMeshComponent);
}

void UCore_DestructionSystem::CreateGeometryCollection(UStaticMesh* SourceMesh)
{
    if (!IsValid(SourceMesh) || !GetOwner())
    {
        return;
    }
    
    // Create Geometry Collection Component if not exists
    if (!GeometryCollectionComponent)
    {
        GeometryCollectionComponent = NewObject<UGeometryCollectionComponent>(GetOwner());
        if (GeometryCollectionComponent)
        {
            GetOwner()->AddInstanceComponent(GeometryCollectionComponent);
            GeometryCollectionComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
                FAttachmentTransformRules::KeepWorldTransform);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Created Geometry Collection for %s"), *SourceMesh->GetName());
}

void UCore_DestructionSystem::ApplyFieldForce(const FVector& Location, float Magnitude, float Radius)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Create temporary field system actor for force application
    AFieldSystemActor* FieldActor = GetWorld()->SpawnActor<AFieldSystemActor>();
    if (FieldActor)
    {
        FieldSystemComponent = FieldActor->GetFieldSystemComponent();
        if (FieldSystemComponent)
        {
            // Apply radial force field
            ApplyChaosForceField(Location, Magnitude, Radius);
        }
        
        // Clean up field actor after a short delay
        FTimerHandle CleanupTimer;
        GetWorld()->GetTimerManager().SetTimer(CleanupTimer, [FieldActor]() {
            if (IsValid(FieldActor))
            {
                FieldActor->Destroy();
            }
        }, 2.0f, false);
    }
}

void UCore_DestructionSystem::CleanupFragments()
{
    int32 CleanedCount = 0;
    
    for (int32 i = FragmentActors.Num() - 1; i >= 0; i--)
    {
        if (IsValid(FragmentActors[i].Get()))
        {
            FragmentActors[i]->Destroy();
            CleanedCount++;
        }
        FragmentActors.RemoveAt(i);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Cleaned up %d fragments"), CleanedCount);
}

int32 UCore_DestructionSystem::GetActiveFragmentCount() const
{
    int32 ActiveCount = 0;
    for (const TObjectPtr<AActor>& Fragment : FragmentActors)
    {
        if (IsValid(Fragment.Get()))
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

void UCore_DestructionSystem::ExecuteFractureDestruction(const FVector& ImpactLocation, const FVector& ImpactForce)
{
    if (!TargetMeshComponent || !TargetMeshComponent->GetStaticMesh())
    {
        return;
    }
    
    UStaticMesh* OriginalMesh = TargetMeshComponent->GetStaticMesh();
    FVector OriginalLocation = TargetMeshComponent->GetComponentLocation();
    
    // Create 3-5 fracture pieces
    int32 NumFragments = FMath::RandRange(3, 5);
    
    for (int32 i = 0; i < NumFragments; i++)
    {
        FVector FragmentOffset = FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(0.0f, 50.0f)
        );
        
        FVector FragmentVelocity = (ImpactForce * 0.1f) + FVector(
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(100.0f, 300.0f)
        );
        
        AActor* Fragment = CreateFragment(OriginalLocation + FragmentOffset, FragmentVelocity, OriginalMesh);
        if (Fragment)
        {
            FragmentActors.Add(Fragment);
        }
    }
    
    // Hide original mesh
    if (TargetMeshComponent)
    {
        TargetMeshComponent->SetVisibility(false);
        TargetMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void UCore_DestructionSystem::ExecuteShatterDestruction(const FVector& ImpactLocation, const FVector& ImpactForce)
{
    if (!TargetMeshComponent || !TargetMeshComponent->GetStaticMesh())
    {
        return;
    }
    
    UStaticMesh* OriginalMesh = TargetMeshComponent->GetStaticMesh();
    FVector OriginalLocation = TargetMeshComponent->GetComponentLocation();
    
    // Create many small fragments
    int32 NumFragments = FMath::Min(DestructionParams.MaxFragments, 15);
    
    for (int32 i = 0; i < NumFragments; i++)
    {
        FVector FragmentOffset = FVector(
            FMath::RandRange(-150.0f, 150.0f),
            FMath::RandRange(-150.0f, 150.0f),
            FMath::RandRange(0.0f, 100.0f)
        );
        
        FVector FragmentVelocity = (ImpactForce * 0.15f) + FVector(
            FMath::RandRange(-400.0f, 400.0f),
            FMath::RandRange(-400.0f, 400.0f),
            FMath::RandRange(200.0f, 500.0f)
        );
        
        AActor* Fragment = CreateFragment(OriginalLocation + FragmentOffset, FragmentVelocity, OriginalMesh);
        if (Fragment)
        {
            FragmentActors.Add(Fragment);
            
            // Scale down fragments for shatter effect
            if (UStaticMeshComponent* FragmentMesh = Fragment->FindComponentByClass<UStaticMeshComponent>())
            {
                float Scale = FMath::RandRange(0.1f, 0.3f);
                FragmentMesh->SetWorldScale3D(FVector(Scale));
            }
        }
    }
    
    // Hide original mesh
    if (TargetMeshComponent)
    {
        TargetMeshComponent->SetVisibility(false);
        TargetMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void UCore_DestructionSystem::ExecuteCrumbleDestruction(const FVector& ImpactLocation, const FVector& ImpactForce)
{
    // Gradual crumble - create fragments over time
    ExecuteFractureDestruction(ImpactLocation, ImpactForce * 0.5f);
}

void UCore_DestructionSystem::ExecuteExplodeDestruction(const FVector& ImpactLocation, const FVector& ImpactForce)
{
    // Explosive destruction with radial force
    ApplyFieldForce(ImpactLocation, ImpactForce.Size() * 2.0f, 500.0f);
    ExecuteShatterDestruction(ImpactLocation, ImpactForce * 1.5f);
}

AActor* UCore_DestructionSystem::CreateFragment(const FVector& Location, const FVector& Velocity, UStaticMesh* FragmentMesh)
{
    if (!GetWorld() || !FragmentMesh)
    {
        return nullptr;
    }
    
    // Spawn static mesh actor for fragment
    AStaticMeshActor* Fragment = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (Fragment)
    {
        UStaticMeshComponent* MeshComp = Fragment->GetStaticMeshComponent();
        if (MeshComp)
        {
            MeshComp->SetStaticMesh(FragmentMesh);
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->AddImpulse(Velocity, NAME_None, true);
            
            // Add some random angular velocity
            FVector AngularVelocity = FVector(
                FMath::RandRange(-10.0f, 10.0f),
                FMath::RandRange(-10.0f, 10.0f),
                FMath::RandRange(-10.0f, 10.0f)
            );
            MeshComp->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
        }
        
        // Schedule individual fragment cleanup
        ScheduleFragmentCleanup(Fragment, DestructionParams.FragmentLifetime + FMath::RandRange(-2.0f, 2.0f));
    }
    
    return Fragment;
}

void UCore_DestructionSystem::ScheduleFragmentCleanup(AActor* Fragment, float Delay)
{
    if (!GetWorld() || !Fragment)
    {
        return;
    }
    
    FTimerHandle CleanupTimer;
    GetWorld()->GetTimerManager().SetTimer(CleanupTimer, [Fragment]() {
        if (IsValid(Fragment))
        {
            Fragment->Destroy();
        }
    }, Delay, false);
}

void UCore_DestructionSystem::SetupChaosPhysics()
{
    // Enable Chaos Physics if available
    if (GetWorld())
    {
        UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Chaos Physics setup initialized"));
    }
}

void UCore_DestructionSystem::ApplyChaosForceField(const FVector& Location, float Magnitude, float Radius)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Apply radial impulse to nearby physics objects
    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        Location,
        Location,
        FQuat::Identity,
        ECC_WorldDynamic,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
    
    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (UPrimitiveComponent* HitComponent = Hit.GetComponent())
            {
                if (HitComponent->IsSimulatingPhysics())
                {
                    FVector ForceDirection = (Hit.GetActor()->GetActorLocation() - Location).GetSafeNormal();
                    FVector Force = ForceDirection * Magnitude;
                    HitComponent->AddImpulseAtLocation(Force, Hit.Location);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Applied Chaos force field at %s with magnitude %f"), 
           *Location.ToString(), Magnitude);
}