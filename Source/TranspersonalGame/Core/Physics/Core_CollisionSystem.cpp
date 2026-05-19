#include "Core_CollisionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

UCore_CollisionSystem::UCore_CollisionSystem()
{
    bOptimizationEnabled = true;
    CollisionLODDistance = 5000.0f;
    MaxActiveCollisions = 1000;
    CollisionEventsThisFrame = 0;
    AverageCollisionProcessingTime = 0.0f;
    TotalCollisionEvents = 0;
    bDebugDrawEnabled = false;
    DebugDrawDuration = 1.0f;
    CollisionProcessingTimeAccumulator = 0.0f;
    CollisionProcessingFrameCount = 0;
}

void UCore_CollisionSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Initializing collision system"));
    
    InitializeDefaultProfiles();
    
    // Initialize performance tracking
    RecentCollisionTimes.Reserve(60); // Track last 60 frames
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Initialization complete"));
}

void UCore_CollisionSystem::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Deinitializing collision system"));
    
    CollisionProfiles.Empty();
    RecentCollisionTimes.Empty();
    
    Super::Deinitialize();
}

void UCore_CollisionSystem::Tick(float DeltaTime)
{
    if (!GetWorld())
    {
        return;
    }

    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Optimize collisions if enabled
    if (bOptimizationEnabled)
    {
        OptimizeCollisions();
    }
    
    // Reset frame counters
    CollisionEventsThisFrame = 0;
}

void UCore_CollisionSystem::RegisterCollisionProfile(const FCore_CollisionProfile& Profile)
{
    if (Profile.ProfileName == NAME_None)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CollisionSystem: Cannot register profile with empty name"));
        return;
    }
    
    CollisionProfiles.Add(Profile.ProfileName, Profile);
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Registered collision profile: %s"), *Profile.ProfileName.ToString());
}

bool UCore_CollisionSystem::ApplyCollisionProfile(UPrimitiveComponent* Component, const FName& ProfileName)
{
    if (!Component)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CollisionSystem: Cannot apply profile to null component"));
        return false;
    }
    
    const FCore_CollisionProfile* Profile = CollisionProfiles.Find(ProfileName);
    if (!Profile)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CollisionSystem: Profile not found: %s"), *ProfileName.ToString());
        return false;
    }
    
    // Apply collision settings
    Component->SetCollisionEnabled(Profile->CollisionEnabled);
    Component->SetCollisionObjectType(Profile->ObjectType);
    Component->SetGenerateOverlapEvents(Profile->bGenerateOverlapEvents);
    Component->SetCanEverAffectNavigation(Profile->bCanCharacterStepUpOn);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Applied profile %s to component %s"), 
           *ProfileName.ToString(), *Component->GetName());
    
    return true;
}

FCore_CollisionProfile UCore_CollisionSystem::GetCollisionProfile(const FName& ProfileName) const
{
    const FCore_CollisionProfile* Profile = CollisionProfiles.Find(ProfileName);
    if (Profile)
    {
        return *Profile;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_CollisionSystem: Profile not found: %s"), *ProfileName.ToString());
    return FCore_CollisionProfile();
}

bool UCore_CollisionSystem::LineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit,
                                       ECollisionChannel TraceChannel, const TArray<AActor*>& ActorsToIgnore) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Performance check
    if (bOptimizationEnabled && !ShouldProcessCollision(Start))
    {
        return false;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActors(ActorsToIgnore);
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = true;
    
    bool bHit = World->LineTraceSingleByChannel(OutHit, Start, End, TraceChannel, QueryParams);
    
    // Debug drawing
    if (bDebugDrawEnabled)
    {
        FColor DebugColor = bHit ? FColor::Red : FColor::Green;
        DrawDebugLine(World, Start, End, DebugColor, false, DebugDrawDuration, 0, 1.0f);
        
        if (bHit)
        {
            DrawDebugSphere(World, OutHit.ImpactPoint, 5.0f, 8, FColor::Red, false, DebugDrawDuration);
        }
    }
    
    return bHit;
}

bool UCore_CollisionSystem::SphereTrace(const FVector& Start, const FVector& End, float Radius,
                                         FHitResult& OutHit, ECollisionChannel TraceChannel,
                                         const TArray<AActor*>& ActorsToIgnore) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    if (bOptimizationEnabled && !ShouldProcessCollision(Start))
    {
        return false;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActors(ActorsToIgnore);
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = true;
    
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
    bool bHit = World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, TraceChannel, SphereShape, QueryParams);
    
    if (bDebugDrawEnabled)
    {
        FColor DebugColor = bHit ? FColor::Red : FColor::Green;
        DrawDebugCapsule(World, (Start + End) * 0.5f, FVector::Dist(Start, End) * 0.5f, Radius, 
                        FRotationMatrix::MakeFromZ(End - Start).ToQuat(), DebugColor, false, DebugDrawDuration);
    }
    
    return bHit;
}

bool UCore_CollisionSystem::BoxTrace(const FVector& Start, const FVector& End, const FVector& HalfSize,
                                      const FRotator& Orientation, FHitResult& OutHit,
                                      ECollisionChannel TraceChannel, const TArray<AActor*>& ActorsToIgnore) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    if (bOptimizationEnabled && !ShouldProcessCollision(Start))
    {
        return false;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActors(ActorsToIgnore);
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = true;
    
    FCollisionShape BoxShape = FCollisionShape::MakeBox(HalfSize);
    FQuat BoxRotation = Orientation.Quaternion();
    
    bool bHit = World->SweepSingleByChannel(OutHit, Start, End, BoxRotation, TraceChannel, BoxShape, QueryParams);
    
    if (bDebugDrawEnabled)
    {
        FColor DebugColor = bHit ? FColor::Red : FColor::Green;
        DrawDebugBox(World, (Start + End) * 0.5f, HalfSize, BoxRotation, DebugColor, false, DebugDrawDuration);
    }
    
    return bHit;
}

TArray<AActor*> UCore_CollisionSystem::GetOverlappingActors(const FVector& Location, float Radius,
                                                             ECollisionChannel ObjectType) const
{
    TArray<AActor*> OverlappingActors;
    UWorld* World = GetWorld();
    
    if (!World)
    {
        return OverlappingActors;
    }
    
    if (bOptimizationEnabled && !ShouldProcessCollision(Location))
    {
        return OverlappingActors;
    }
    
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    TArray<FOverlapResult> OverlapResults;
    bool bFoundOverlaps = World->OverlapMultiByChannel(OverlapResults, Location, FQuat::Identity,
                                                       ObjectType, SphereShape, QueryParams);
    
    if (bFoundOverlaps)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (Result.GetActor())
            {
                OverlappingActors.AddUnique(Result.GetActor());
            }
        }
    }
    
    if (bDebugDrawEnabled)
    {
        FColor DebugColor = OverlappingActors.Num() > 0 ? FColor::Orange : FColor::Blue;
        DrawDebugSphere(World, Location, Radius, 16, DebugColor, false, DebugDrawDuration);
    }
    
    return OverlappingActors;
}

void UCore_CollisionSystem::BroadcastCollisionEvent(const FCore_CollisionEvent& Event)
{
    CollisionEventsThisFrame++;
    TotalCollisionEvents++;
    
    OnCollisionDetected.Broadcast(Event);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_CollisionSystem: Collision event between %s and %s at %s"),
           Event.Actor1 ? *Event.Actor1->GetName() : TEXT("NULL"),
           Event.Actor2 ? *Event.Actor2->GetName() : TEXT("NULL"),
           *Event.ImpactPoint.ToString());
}

void UCore_CollisionSystem::EnableCollisionOptimization(bool bEnable)
{
    bOptimizationEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Collision optimization %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_CollisionSystem::SetCollisionLODDistance(float Distance)
{
    CollisionLODDistance = FMath::Max(Distance, 100.0f);
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Collision LOD distance set to %f"), CollisionLODDistance);
}

int32 UCore_CollisionSystem::GetActiveCollisionCount() const
{
    return CollisionEventsThisFrame;
}

float UCore_CollisionSystem::GetCollisionPerformanceMetric() const
{
    return AverageCollisionProcessingTime;
}

void UCore_CollisionSystem::DebugDrawCollisionShapes(bool bEnable)
{
    bDebugDrawEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Debug drawing %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_CollisionSystem::LogCollisionStats() const
{
    UE_LOG(LogTemp, Log, TEXT("=== Collision System Statistics ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Events: %d"), TotalCollisionEvents);
    UE_LOG(LogTemp, Log, TEXT("Events This Frame: %d"), CollisionEventsThisFrame);
    UE_LOG(LogTemp, Log, TEXT("Average Processing Time: %.4f ms"), AverageCollisionProcessingTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("Optimization Enabled: %s"), bOptimizationEnabled ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("LOD Distance: %.2f"), CollisionLODDistance);
    UE_LOG(LogTemp, Log, TEXT("Registered Profiles: %d"), CollisionProfiles.Num());
}

void UCore_CollisionSystem::InitializeDefaultProfiles()
{
    // Default character profile
    FCore_CollisionProfile CharacterProfile;
    CharacterProfile.ProfileName = FName("Character");
    CharacterProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    CharacterProfile.ObjectType = ECR_Pawn;
    CharacterProfile.bGenerateOverlapEvents = true;
    CharacterProfile.bCanCharacterStepUpOn = false;
    RegisterCollisionProfile(CharacterProfile);
    
    // Environment profile
    FCore_CollisionProfile EnvironmentProfile;
    EnvironmentProfile.ProfileName = FName("Environment");
    EnvironmentProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    EnvironmentProfile.ObjectType = ECR_WorldStatic;
    EnvironmentProfile.bGenerateOverlapEvents = false;
    EnvironmentProfile.bCanCharacterStepUpOn = true;
    RegisterCollisionProfile(EnvironmentProfile);
    
    // Projectile profile
    FCore_CollisionProfile ProjectileProfile;
    ProjectileProfile.ProfileName = FName("Projectile");
    ProjectileProfile.CollisionEnabled = ECollisionEnabled::QueryOnly;
    ProjectileProfile.ObjectType = ECR_WorldDynamic;
    ProjectileProfile.bGenerateOverlapEvents = true;
    ProjectileProfile.bCanCharacterStepUpOn = false;
    RegisterCollisionProfile(ProjectileProfile);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Default collision profiles initialized"));
}

void UCore_CollisionSystem::UpdatePerformanceMetrics(float DeltaTime)
{
    // Track collision processing time
    float CurrentTime = FPlatformTime::Seconds();
    CollisionProcessingTimeAccumulator += DeltaTime;
    CollisionProcessingFrameCount++;
    
    // Update average every 60 frames
    if (CollisionProcessingFrameCount >= 60)
    {
        AverageCollisionProcessingTime = CollisionProcessingTimeAccumulator / CollisionProcessingFrameCount;
        CollisionProcessingTimeAccumulator = 0.0f;
        CollisionProcessingFrameCount = 0;
    }
    
    // Track recent collision times for performance analysis
    RecentCollisionTimes.Add(DeltaTime);
    if (RecentCollisionTimes.Num() > 60)
    {
        RecentCollisionTimes.RemoveAt(0);
    }
}

void UCore_CollisionSystem::OptimizeCollisions()
{
    // Disable collision for distant objects if performance is poor
    if (AverageCollisionProcessingTime > 0.016f) // > 16ms (60fps threshold)
    {
        CollisionLODDistance *= 0.95f; // Reduce LOD distance by 5%
        CollisionLODDistance = FMath::Max(CollisionLODDistance, 1000.0f); // Minimum 1000 units
    }
    else if (AverageCollisionProcessingTime < 0.008f) // < 8ms (good performance)
    {
        CollisionLODDistance *= 1.02f; // Increase LOD distance by 2%
        CollisionLODDistance = FMath::Min(CollisionLODDistance, 10000.0f); // Maximum 10000 units
    }
}

bool UCore_CollisionSystem::ShouldProcessCollision(const FVector& Location) const
{
    if (!bOptimizationEnabled)
    {
        return true;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check distance from player
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (PlayerPawn)
    {
        float DistanceToPlayer = FVector::Dist(Location, PlayerPawn->GetActorLocation());
        return DistanceToPlayer <= CollisionLODDistance;
    }
    
    return true;
}