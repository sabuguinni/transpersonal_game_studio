#include "Core_PhysicsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCore_PhysicsManager::UCore_PhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default physics settings
    GlobalGravityScale = 1.0f;
    DefaultLinearDamping = 0.01f;
    DefaultAngularDamping = 0.01f;
    bEnableAdvancedPhysics = true;
    
    // Collision settings
    DefaultCollisionChannel = ECC_WorldStatic;
    CollisionTolerance = 0.1f;
    
    // Ragdoll settings
    bEnableRagdollSystem = true;
    RagdollImpulseThreshold = 500.0f;
    RagdollRecoveryTime = 3.0f;
}

void UCore_PhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    LogPhysicsState("Core_PhysicsManager initialized");
    
    // Apply global physics settings
    SetGlobalPhysicsSettings(GlobalGravityScale, DefaultLinearDamping, DefaultAngularDamping);
    
    // Initialize validation cache
    PhysicsValidationCache.Empty();
    
    // Clear ragdoll tracking arrays
    RagdollActors.Empty();
    RagdollTimers.Empty();
}

void UCore_PhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableRagdollSystem)
    {
        UpdateRagdollTimers(DeltaTime);
        CleanupInvalidActors();
    }
}

void UCore_PhysicsManager::SetGlobalPhysicsSettings(float NewGravityScale, float NewLinearDamping, float NewAngularDamping)
{
    GlobalGravityScale = NewGravityScale;
    DefaultLinearDamping = NewLinearDamping;
    DefaultAngularDamping = NewAngularDamping;
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Apply gravity scale
        World->GetWorldSettings()->GlobalGravityZ = -980.0f * GlobalGravityScale;
        
        LogPhysicsState(FString::Printf(TEXT("Physics settings updated - Gravity: %.2f, Linear: %.3f, Angular: %.3f"), 
                                       NewGravityScale, NewLinearDamping, NewAngularDamping));
    }
}

bool UCore_PhysicsManager::PerformLineTrace(FVector Start, FVector End, FHitResult& OutHit, bool bTraceComplex)
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = bTraceComplex;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = World->LineTraceSingleByChannel(
        OutHit,
        Start,
        End,
        DefaultCollisionChannel,
        QueryParams
    );
    
    return bHit;
}

bool UCore_PhysicsManager::PerformSphereTrace(FVector Start, FVector End, float Radius, FHitResult& OutHit)
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = World->SweepSingleByChannel(
        OutHit,
        Start,
        End,
        FQuat::Identity,
        DefaultCollisionChannel,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
    
    return bHit;
}

TArray<FHitResult> UCore_PhysicsManager::PerformMultiLineTrace(FVector Start, FVector End, bool bTraceComplex)
{
    TArray<FHitResult> HitResults;
    
    UWorld* World = GetWorld();
    if (!World) return HitResults;
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = bTraceComplex;
    QueryParams.AddIgnoredActor(GetOwner());
    
    World->LineTraceMultiByChannel(
        HitResults,
        Start,
        End,
        DefaultCollisionChannel,
        QueryParams
    );
    
    return HitResults;
}

void UCore_PhysicsManager::EnableRagdoll(AActor* TargetActor, FVector ImpulseDirection, float ImpulseStrength)
{
    if (!TargetActor || !bEnableRagdollSystem) return;
    
    USkeletalMeshComponent* SkelMesh = GetSkeletalMeshFromActor(TargetActor);
    if (!SkelMesh) return;
    
    // Enable ragdoll physics
    SkelMesh->SetSimulatePhysics(true);
    SkelMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkelMesh->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Apply impulse if specified
    if (!ImpulseDirection.IsZero() && ImpulseStrength > 0.0f)
    {
        FVector NormalizedImpulse = ImpulseDirection.GetSafeNormal() * ImpulseStrength;
        SkelMesh->AddImpulse(NormalizedImpulse);
    }
    
    // Track ragdoll state
    if (!RagdollActors.Contains(TargetActor))
    {
        RagdollActors.Add(TargetActor);
        RagdollTimers.Add(TargetActor, 0.0f);
    }
    
    LogPhysicsState(FString::Printf(TEXT("Ragdoll enabled for actor: %s"), *TargetActor->GetName()));
}

void UCore_PhysicsManager::DisableRagdoll(AActor* TargetActor)
{
    if (!TargetActor) return;
    
    USkeletalMeshComponent* SkelMesh = GetSkeletalMeshFromActor(TargetActor);
    if (!SkelMesh) return;
    
    // Disable ragdoll physics
    SkelMesh->SetSimulatePhysics(false);
    SkelMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Remove from tracking
    RagdollActors.Remove(TargetActor);
    RagdollTimers.Remove(TargetActor);
    
    LogPhysicsState(FString::Printf(TEXT("Ragdoll disabled for actor: %s"), *TargetActor->GetName()));
}

bool UCore_PhysicsManager::IsActorInRagdoll(AActor* TargetActor)
{
    return RagdollActors.Contains(TargetActor);
}

bool UCore_PhysicsManager::ValidatePhysicsSetup(AActor* TargetActor)
{
    if (!TargetActor) return false;
    
    // Check cache first
    if (PhysicsValidationCache.Contains(TargetActor))
    {
        return PhysicsValidationCache[TargetActor];
    }
    
    bool bIsValid = true;
    FString ValidationErrors;
    
    // Check for physics components
    UPrimitiveComponent* PrimComp = TargetActor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp)
    {
        ValidationErrors += "No PrimitiveComponent found; ";
        bIsValid = false;
    }
    else
    {
        // Check collision settings
        if (PrimComp->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
        {
            ValidationErrors += "Collision disabled; ";
        }
        
        // Check physics asset for skeletal mesh
        if (USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(PrimComp))
        {
            if (!SkelMesh->GetPhysicsAsset())
            {
                ValidationErrors += "No PhysicsAsset assigned; ";
                bIsValid = false;
            }
        }
    }
    
    // Cache result
    PhysicsValidationCache.Add(TargetActor, bIsValid);
    
    if (!bIsValid)
    {
        LogPhysicsState(FString::Printf(TEXT("Physics validation failed for %s: %s"), 
                                       *TargetActor->GetName(), *ValidationErrors));
    }
    
    return bIsValid;
}

void UCore_PhysicsManager::RunPhysicsPerformanceTest()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    float StartTime = FPlatformTime::Seconds();
    int32 TraceCount = 0;
    
    // Perform multiple traces to test performance
    for (int32 i = 0; i < 100; ++i)
    {
        FVector Start = FVector(FMath::RandRange(-1000.0f, 1000.0f), 
                               FMath::RandRange(-1000.0f, 1000.0f), 
                               1000.0f);
        FVector End = Start + FVector(0, 0, -2000.0f);
        
        FHitResult HitResult;
        if (PerformLineTrace(Start, End, HitResult))
        {
            TraceCount++;
        }
    }
    
    float EndTime = FPlatformTime::Seconds();
    float TestDuration = (EndTime - StartTime) * 1000.0f; // Convert to milliseconds
    
    LogPhysicsState(FString::Printf(TEXT("Physics Performance Test: %d hits in %.2fms (%.2f traces/ms)"), 
                                   TraceCount, TestDuration, 100.0f / TestDuration));
}

void UCore_PhysicsManager::ApplyDestructiveForce(AActor* TargetActor, FVector ForceLocation, float ForceRadius, float ForceStrength)
{
    if (!TargetActor) return;
    
    UPrimitiveComponent* PrimComp = TargetActor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp) return;
    
    // Apply radial force
    PrimComp->AddRadialForce(ForceLocation, ForceRadius, ForceStrength, ERadialImpulseFalloff::RIF_Linear, true);
    
    LogPhysicsState(FString::Printf(TEXT("Destructive force applied to %s at location %s"), 
                                   *TargetActor->GetName(), *ForceLocation.ToString()));
}

void UCore_PhysicsManager::CreatePhysicsExplosion(FVector ExplosionLocation, float ExplosionRadius, float ExplosionStrength)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find all actors within explosion radius
    TArray<AActor*> OverlappingActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), OverlappingActors);
    
    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor) continue;
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), ExplosionLocation);
        if (Distance <= ExplosionRadius)
        {
            ApplyDestructiveForce(Actor, ExplosionLocation, ExplosionRadius, ExplosionStrength);
        }
    }
    
    LogPhysicsState(FString::Printf(TEXT("Physics explosion created at %s (radius: %.1f, strength: %.1f)"), 
                                   *ExplosionLocation.ToString(), ExplosionRadius, ExplosionStrength));
}

void UCore_PhysicsManager::UpdateRagdollTimers(float DeltaTime)
{
    TArray<AActor*> ActorsToRemove;
    
    for (auto& RagdollPair : RagdollTimers)
    {
        AActor* Actor = RagdollPair.Key;
        float& Timer = RagdollPair.Value;
        
        if (!IsValid(Actor))
        {
            ActorsToRemove.Add(Actor);
            continue;
        }
        
        Timer += DeltaTime;
        
        // Auto-disable ragdoll after recovery time
        if (Timer >= RagdollRecoveryTime)
        {
            DisableRagdoll(Actor);
        }
    }
    
    // Clean up invalid actors
    for (AActor* Actor : ActorsToRemove)
    {
        RagdollActors.Remove(Actor);
        RagdollTimers.Remove(Actor);
    }
}

void UCore_PhysicsManager::CleanupInvalidActors()
{
    // Remove invalid actors from validation cache
    TArray<AActor*> InvalidActors;
    for (auto& ValidationPair : PhysicsValidationCache)
    {
        if (!IsValid(ValidationPair.Key))
        {
            InvalidActors.Add(ValidationPair.Key);
        }
    }
    
    for (AActor* Actor : InvalidActors)
    {
        PhysicsValidationCache.Remove(Actor);
    }
}

USkeletalMeshComponent* UCore_PhysicsManager::GetSkeletalMeshFromActor(AActor* Actor)
{
    if (!Actor) return nullptr;
    
    return Actor->FindComponentByClass<USkeletalMeshComponent>();
}

void UCore_PhysicsManager::LogPhysicsState(const FString& Message)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
                                        FString::Printf(TEXT("[PhysicsManager] %s"), *Message));
    }
    
    UE_LOG(LogTemp, Log, TEXT("[Core_PhysicsManager] %s"), *Message);
}