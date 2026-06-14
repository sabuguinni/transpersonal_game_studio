#include "Core_CollisionSystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"

UCore_CollisionSystem::UCore_CollisionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Default collision settings
    CollisionData.CollisionType = ECore_CollisionType::Dynamic;
    CollisionData.Mass = 1.0f;
    CollisionData.Friction = 0.7f;
    CollisionData.Restitution = 0.3f;
    CollisionData.bCanBlock = true;
    CollisionData.bCanOverlap = false;
    
    // Default object types for collision
    CollisionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    CollisionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
    CollisionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
}

void UCore_CollisionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find primary collision component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        // Try to find the main collision component
        PrimaryCollisionComponent = Owner->GetRootComponent() ? Cast<UPrimitiveComponent>(Owner->GetRootComponent()) : nullptr;
        
        if (!PrimaryCollisionComponent)
        {
            // Look for static mesh component
            PrimaryCollisionComponent = Owner->FindComponentByClass<UStaticMeshComponent>();
        }
        
        if (!PrimaryCollisionComponent)
        {
            // Look for skeletal mesh component
            PrimaryCollisionComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        }
        
        if (!PrimaryCollisionComponent)
        {
            // Look for capsule component
            PrimaryCollisionComponent = Owner->FindComponentByClass<UCapsuleComponent>();
        }
        
        // Bind collision events if we found a component
        if (PrimaryCollisionComponent)
        {
            Owner->OnActorHit.AddDynamic(this, &UCore_CollisionSystem::HandleActorHit);
            Owner->OnActorBeginOverlap.AddDynamic(this, &UCore_CollisionSystem::HandleActorBeginOverlap);
            Owner->OnActorEndOverlap.AddDynamic(this, &UCore_CollisionSystem::HandleActorEndOverlap);
            
            UpdateCollisionProfile();
        }
    }
}

void UCore_CollisionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update collision state if needed
    if (PrimaryCollisionComponent && bCollisionEnabled)
    {
        // Perform any continuous collision checks here
        LastCollisionTime += DeltaTime;
    }
}

bool UCore_CollisionSystem::LineTrace(const FVector& Start, const FVector& End, FHitResult& HitResult)
{
    if (!GetWorld()) return false;
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = true;
    
    return GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
}

bool UCore_CollisionSystem::SphereTrace(const FVector& Start, const FVector& End, float Radius, FHitResult& HitResult)
{
    if (!GetWorld()) return false;
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = true;
    
    return GetWorld()->SweepSingleByChannel(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        ECollisionChannel::ECC_Visibility,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
}

bool UCore_CollisionSystem::BoxTrace(const FVector& Start, const FVector& End, const FVector& HalfSize, FHitResult& HitResult)
{
    if (!GetWorld()) return false;
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = true;
    
    return GetWorld()->SweepSingleByChannel(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        ECollisionChannel::ECC_Visibility,
        FCollisionShape::MakeBox(HalfSize),
        QueryParams
    );
}

TArray<FHitResult> UCore_CollisionSystem::MultiLineTrace(const FVector& Start, const FVector& End)
{
    TArray<FHitResult> HitResults;
    
    if (!GetWorld()) return HitResults;
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = true;
    
    GetWorld()->LineTraceMultiByChannel(
        HitResults,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return HitResults;
}

bool UCore_CollisionSystem::SphereOverlap(const FVector& Location, float Radius, TArray<AActor*>& OverlappingActors)
{
    if (!GetWorld()) return false;
    
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        Location,
        FQuat::Identity,
        ECollisionChannel::ECC_WorldDynamic,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
    
    OverlappingActors.Empty();
    for (const FOverlapResult& Result : OverlapResults)
    {
        if (Result.GetActor())
        {
            OverlappingActors.Add(Result.GetActor());
        }
    }
    
    return bHasOverlaps;
}

bool UCore_CollisionSystem::BoxOverlap(const FVector& Location, const FVector& HalfSize, TArray<AActor*>& OverlappingActors)
{
    if (!GetWorld()) return false;
    
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        Location,
        FQuat::Identity,
        ECollisionChannel::ECC_WorldDynamic,
        FCollisionShape::MakeBox(HalfSize),
        QueryParams
    );
    
    OverlappingActors.Empty();
    for (const FOverlapResult& Result : OverlapResults)
    {
        if (Result.GetActor())
        {
            OverlappingActors.Add(Result.GetActor());
        }
    }
    
    return bHasOverlaps;
}

void UCore_CollisionSystem::SetCollisionEnabled(bool bEnabled)
{
    bCollisionEnabled = bEnabled;
    
    if (PrimaryCollisionComponent)
    {
        if (bEnabled)
        {
            PrimaryCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
        else
        {
            PrimaryCollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
}

void UCore_CollisionSystem::SetCollisionType(ECore_CollisionType NewType)
{
    CollisionData.CollisionType = NewType;
    UpdateCollisionProfile();
}

void UCore_CollisionSystem::UpdateCollisionProfile()
{
    if (!PrimaryCollisionComponent) return;
    
    switch (CollisionData.CollisionType)
    {
        case ECore_CollisionType::Static:
            PrimaryCollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
            PrimaryCollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
            break;
            
        case ECore_CollisionType::Dynamic:
            PrimaryCollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            PrimaryCollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
            break;
            
        case ECore_CollisionType::Character:
            PrimaryCollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
            PrimaryCollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
            PrimaryCollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Ignore);
            break;
            
        case ECore_CollisionType::Projectile:
            PrimaryCollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            PrimaryCollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
            PrimaryCollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Block);
            break;
            
        case ECore_CollisionType::Trigger:
            PrimaryCollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            PrimaryCollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
            break;
            
        case ECore_CollisionType::None:
        default:
            PrimaryCollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
    }
    
    // Apply physical material properties if this is a physics object
    if (PrimaryCollisionComponent->IsSimulatingPhysics())
    {
        PrimaryCollisionComponent->SetMassOverrideInKg(NAME_None, CollisionData.Mass);
        
        // Note: Friction and Restitution would typically be set via Physical Materials
        // For now, we'll store them in our data structure for potential use
    }
}

void UCore_CollisionSystem::HandleActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!bCollisionEnabled) return;
    
    // Broadcast the collision event
    OnCollisionDetected(Hit);
    
    // Log collision for debugging
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Collision: %s hit %s"), 
            SelfActor ? *SelfActor->GetName() : TEXT("Unknown"),
            OtherActor ? *OtherActor->GetName() : TEXT("Unknown"));
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, Message);
    }
}

void UCore_CollisionSystem::HandleActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!bCollisionEnabled || !OtherActor) return;
    
    // Track overlapping actors
    CurrentOverlaps.AddUnique(OtherActor);
    
    // Broadcast overlap event
    OnOverlapBegin(OtherActor);
}

void UCore_CollisionSystem::HandleActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!bCollisionEnabled || !OtherActor) return;
    
    // Remove from tracking
    CurrentOverlaps.Remove(OtherActor);
    
    // Broadcast overlap end event
    OnOverlapEnd(OtherActor);
}