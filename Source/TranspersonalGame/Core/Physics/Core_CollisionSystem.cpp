#include "Core_CollisionSystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UCore_CollisionSystem::UCore_CollisionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS

    // Initialize default settings
    CollisionSettings = FCore_CollisionSettings();
    bEnableAdvancedCollision = true;
    CollisionCheckFrequency = 60.0f;
    MaxCollisionEvents = 50;

    LastCollisionCheckTime = 0.0f;
    CollisionEventCount = 0;

    RecentCollisions.Reserve(MaxCollisionEvents);
}

void UCore_CollisionSystem::BeginPlay()
{
    Super::BeginPlay();

    // Apply initial collision settings
    ApplyCollisionSettings();

    // Bind collision events
    UPrimitiveComponent* PrimComp = GetOwnerPrimitiveComponent();
    if (PrimComp)
    {
        PrimComp->OnComponentHit.AddDynamic(this, &UCore_CollisionSystem::OnComponentHit);
        PrimComp->OnComponentBeginOverlap.AddDynamic(this, &UCore_CollisionSystem::OnComponentBeginOverlap);
    }

    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_CollisionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableAdvancedCollision)
    {
        UpdateCollisionDetection(DeltaTime);
    }
}

void UCore_CollisionSystem::SetCollisionProfile(ECore_CollisionProfile NewProfile)
{
    CollisionSettings.CollisionProfile = NewProfile;
    ApplyCollisionSettings();
}

void UCore_CollisionSystem::SetCollisionEnabled(bool bEnabled)
{
    CollisionSettings.bEnableCollision = bEnabled;
    ApplyCollisionSettings();
}

void UCore_CollisionSystem::SetCollisionSettings(const FCore_CollisionSettings& NewSettings)
{
    CollisionSettings = NewSettings;
    ApplyCollisionSettings();
}

bool UCore_CollisionSystem::IsCollisionEnabled() const
{
    return CollisionSettings.bEnableCollision;
}

ECore_CollisionProfile UCore_CollisionSystem::GetCurrentCollisionProfile() const
{
    return CollisionSettings.CollisionProfile;
}

TArray<FCore_CollisionEvent> UCore_CollisionSystem::GetRecentCollisions() const
{
    return RecentCollisions;
}

bool UCore_CollisionSystem::PerformLineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit)
{
    if (!GetWorld())
    {
        return false;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = true;

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        OutHit,
        Start,
        End,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        ProcessCollisionEvent(OutHit, (End - Start).Size());
    }

    return bHit;
}

bool UCore_CollisionSystem::PerformSphereTrace(const FVector& Start, const FVector& End, float Radius, FHitResult& OutHit)
{
    if (!GetWorld())
    {
        return false;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = true;

    bool bHit = GetWorld()->SweepSingleByChannel(
        OutHit,
        Start,
        End,
        FQuat::Identity,
        ECC_Visibility,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );

    if (bHit)
    {
        ProcessCollisionEvent(OutHit, (End - Start).Size() * Radius);
    }

    return bHit;
}

TArray<AActor*> UCore_CollisionSystem::GetOverlappingActors(float SearchRadius)
{
    TArray<AActor*> OverlappingActors;
    
    if (!GetWorld() || !GetOwner())
    {
        return OverlappingActors;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        OwnerLocation,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(SearchRadius),
        QueryParams
    );

    if (bHasOverlaps)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (Result.GetActor())
            {
                OverlappingActors.AddUnique(Result.GetActor());
            }
        }
    }

    return OverlappingActors;
}

void UCore_CollisionSystem::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!CollisionSettings.bEnableCollision)
    {
        return;
    }

    float ImpactForce = NormalImpulse.Size();
    ProcessCollisionEvent(Hit, ImpactForce);

    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Hit detected with %s, Force: %f"), 
           OtherActor ? *OtherActor->GetName() : TEXT("Unknown"), ImpactForce);
}

void UCore_CollisionSystem::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!CollisionSettings.bEnableCollision || !CollisionSettings.bGenerateOverlapEvents)
    {
        return;
    }

    ProcessCollisionEvent(SweepResult, 0.0f); // Overlap events have no impact force

    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Overlap detected with %s"), 
           OtherActor ? *OtherActor->GetName() : TEXT("Unknown"));
}

void UCore_CollisionSystem::ClearCollisionHistory()
{
    RecentCollisions.Empty();
    CollisionEventCount = 0;
}

void UCore_CollisionSystem::ApplyCollisionSettings()
{
    UPrimitiveComponent* PrimComp = GetOwnerPrimitiveComponent();
    if (!PrimComp)
    {
        return;
    }

    // Apply collision enabled state
    PrimComp->SetCollisionEnabled(CollisionSettings.bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);

    // Apply overlap events
    PrimComp->SetGenerateOverlapEvents(CollisionSettings.bGenerateOverlapEvents);

    // Apply step up capability
    PrimComp->SetCanCharacterStepUpOn(CollisionSettings.bCanCharacterStepUpOn ? ECB_Yes : ECB_No);

    // Apply collision profile based on enum
    FName ProfileName;
    switch (CollisionSettings.CollisionProfile)
    {
        case ECore_CollisionProfile::Character:
            ProfileName = TEXT("Pawn");
            break;
        case ECore_CollisionProfile::Dinosaur:
            ProfileName = TEXT("Pawn");
            break;
        case ECore_CollisionProfile::Environment:
            ProfileName = TEXT("WorldStatic");
            break;
        case ECore_CollisionProfile::Projectile:
            ProfileName = TEXT("Projectile");
            break;
        case ECore_CollisionProfile::Trigger:
            ProfileName = TEXT("Trigger");
            break;
        case ECore_CollisionProfile::Destructible:
            ProfileName = TEXT("Destructible");
            break;
        default:
            ProfileName = TEXT("BlockAll");
            break;
    }

    PrimComp->SetCollisionProfileName(ProfileName);

    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Applied collision profile %s"), *ProfileName.ToString());
}

float UCore_CollisionSystem::GetCollisionIntensity(const FCore_CollisionEvent& Event) const
{
    // Calculate intensity based on impact force and other factors
    float BaseIntensity = Event.ImpactForce / 1000.0f; // Normalize to 0-1 range roughly
    
    // Factor in the type of collision
    float TypeMultiplier = 1.0f;
    if (Event.HitActor)
    {
        // Increase intensity for certain actor types
        if (Event.HitActor->GetClass()->GetName().Contains(TEXT("Dinosaur")))
        {
            TypeMultiplier = 2.0f;
        }
        else if (Event.HitActor->GetClass()->GetName().Contains(TEXT("Character")))
        {
            TypeMultiplier = 1.5f;
        }
    }

    return FMath::Clamp(BaseIntensity * TypeMultiplier, 0.0f, 10.0f);
}

void UCore_CollisionSystem::UpdateCollisionDetection(float DeltaTime)
{
    LastCollisionCheckTime += DeltaTime;
    
    // Perform periodic collision checks based on frequency
    if (LastCollisionCheckTime >= (1.0f / CollisionCheckFrequency))
    {
        LastCollisionCheckTime = 0.0f;
        
        // Perform environmental collision check
        if (GetOwner())
        {
            FVector OwnerLocation = GetOwner()->GetActorLocation();
            FVector DownVector = OwnerLocation + FVector(0, 0, -200.0f);
            
            FHitResult GroundHit;
            if (PerformLineTrace(OwnerLocation, DownVector, GroundHit))
            {
                // Ground collision detected - could be used for footstep effects, etc.
            }
        }
    }
}

void UCore_CollisionSystem::ProcessCollisionEvent(const FHitResult& Hit, float ImpactForce)
{
    FCore_CollisionEvent NewEvent;
    NewEvent.HitActor = Hit.GetActor();
    NewEvent.HitComponent = Hit.GetComponent();
    NewEvent.ImpactPoint = Hit.ImpactPoint;
    NewEvent.ImpactNormal = Hit.ImpactNormal;
    NewEvent.ImpactForce = ImpactForce;
    NewEvent.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    RegisterCollisionEvent(NewEvent);

    // Broadcast the collision event
    OnCollisionDetected.Broadcast(NewEvent);
}

void UCore_CollisionSystem::RegisterCollisionEvent(const FCore_CollisionEvent& Event)
{
    // Add to recent collisions array
    RecentCollisions.Add(Event);
    CollisionEventCount++;

    // Maintain maximum size
    if (RecentCollisions.Num() > MaxCollisionEvents)
    {
        RecentCollisions.RemoveAt(0);
    }
}

UPrimitiveComponent* UCore_CollisionSystem::GetOwnerPrimitiveComponent() const
{
    if (!GetOwner())
    {
        return nullptr;
    }

    // Try to find the main collision component
    UPrimitiveComponent* PrimComp = GetOwner()->FindComponentByClass<UCapsuleComponent>();
    if (!PrimComp)
    {
        PrimComp = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    }
    if (!PrimComp)
    {
        PrimComp = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    }

    return PrimComp;
}