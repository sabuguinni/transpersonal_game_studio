#include "Core_CollisionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "CollisionQueryParams.h"

UCore_CollisionSystem::UCore_CollisionSystem()
{
    MaxCollisionDistance = 10000.0f;
    MaxCollisionObjects = 1000;
    bEnableCollisionOptimization = true;
}

void UCore_CollisionSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeCollisionProfiles();
    SetupCustomCollisionChannels();
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Initialized with %d collision profiles"), 
           CollisionProfiles.Num());
}

void UCore_CollisionSystem::Deinitialize()
{
    CollisionCallbacks.Empty();
    TrackedActors.Empty();
    
    Super::Deinitialize();
}

void UCore_CollisionSystem::InitializeCollisionProfiles()
{
    // Character collision profile
    FCore_CollisionProfile CharacterProfile;
    CharacterProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    CharacterProfile.ObjectType = ECC_Pawn;
    CharacterProfile.bGenerateOverlapEvents = true;
    CharacterProfile.bCanCharacterStepUpOn = false;
    CollisionProfiles.Add(ECore_CollisionType::Character, CharacterProfile);
    
    // Dinosaur collision profile
    FCore_CollisionProfile DinosaurProfile;
    DinosaurProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    DinosaurProfile.ObjectType = ECC_Pawn;
    DinosaurProfile.bGenerateOverlapEvents = true;
    DinosaurProfile.bCanCharacterStepUpOn = false;
    CollisionProfiles.Add(ECore_CollisionType::Dinosaur, DinosaurProfile);
    
    // Environment collision profile
    FCore_CollisionProfile EnvironmentProfile;
    EnvironmentProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    EnvironmentProfile.ObjectType = ECC_WorldStatic;
    EnvironmentProfile.bGenerateOverlapEvents = false;
    EnvironmentProfile.bCanCharacterStepUpOn = true;
    CollisionProfiles.Add(ECore_CollisionType::Environment, EnvironmentProfile);
    
    // Projectile collision profile
    FCore_CollisionProfile ProjectileProfile;
    ProjectileProfile.CollisionEnabled = ECollisionEnabled::QueryOnly;
    ProjectileProfile.ObjectType = ECC_WorldDynamic;
    ProjectileProfile.bGenerateOverlapEvents = true;
    ProjectileProfile.bCanCharacterStepUpOn = false;
    CollisionProfiles.Add(ECore_CollisionType::Projectile, ProjectileProfile);
    
    // Trigger collision profile
    FCore_CollisionProfile TriggerProfile;
    TriggerProfile.CollisionEnabled = ECollisionEnabled::QueryOnly;
    TriggerProfile.ObjectType = ECC_WorldDynamic;
    TriggerProfile.bGenerateOverlapEvents = true;
    TriggerProfile.bCanCharacterStepUpOn = false;
    CollisionProfiles.Add(ECore_CollisionType::Trigger, TriggerProfile);
    
    // Water collision profile
    FCore_CollisionProfile WaterProfile;
    WaterProfile.CollisionEnabled = ECollisionEnabled::QueryOnly;
    WaterProfile.ObjectType = ECC_WorldStatic;
    WaterProfile.bGenerateOverlapEvents = true;
    WaterProfile.bCanCharacterStepUpOn = false;
    CollisionProfiles.Add(ECore_CollisionType::Water, WaterProfile);
}

void UCore_CollisionSystem::SetupCustomCollisionChannels()
{
    // This would normally set up custom collision channels
    // For now, we use the default UE5 channels
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Custom collision channels configured"));
}

FCore_CollisionProfile UCore_CollisionSystem::GetCollisionProfile(ECore_CollisionType CollisionType) const
{
    if (const FCore_CollisionProfile* Profile = CollisionProfiles.Find(CollisionType))
    {
        return *Profile;
    }
    
    // Return default profile if not found
    return FCore_CollisionProfile();
}

void UCore_CollisionSystem::ApplyCollisionProfile(UPrimitiveComponent* Component, ECore_CollisionType CollisionType)
{
    if (!Component)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CollisionSystem: Cannot apply collision profile to null component"));
        return;
    }
    
    FCore_CollisionProfile Profile = GetCollisionProfile(CollisionType);
    
    Component->SetCollisionEnabled(Profile.CollisionEnabled);
    Component->SetCollisionObjectType(Profile.ObjectType);
    Component->SetGenerateOverlapEvents(Profile.bGenerateOverlapEvents);
    Component->SetCanEverAffectNavigation(Profile.bCanCharacterStepUpOn);
    
    // Set collision responses based on type
    switch (CollisionType)
    {
        case ECore_CollisionType::Character:
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            break;
            
        case ECore_CollisionType::Dinosaur:
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            break;
            
        case ECore_CollisionType::Environment:
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            Component->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
            break;
            
        case ECore_CollisionType::Projectile:
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            Component->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
            break;
            
        case ECore_CollisionType::Trigger:
            Component->SetCollisionResponseToAllChannels(ECR_Overlap);
            break;
            
        case ECore_CollisionType::Water:
            Component->SetCollisionResponseToAllChannels(ECR_Overlap);
            Component->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Applied %s collision profile to component"), 
           *UEnum::GetValueAsString(CollisionType));
}

void UCore_CollisionSystem::SetupCollisionChannels()
{
    SetupCustomCollisionChannels();
    OptimizeCollisionSettings();
}

bool UCore_CollisionSystem::LineTraceByChannel(const FVector& Start, const FVector& End, 
                                               ECollisionChannel TraceChannel, FHitResult& HitResult, 
                                               bool bTraceComplex)
{
    if (UWorld* World = GetWorld())
    {
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = bTraceComplex;
        QueryParams.bReturnPhysicalMaterial = true;
        
        return World->LineTraceSingleByChannel(HitResult, Start, End, TraceChannel, QueryParams);
    }
    
    return false;
}

bool UCore_CollisionSystem::SphereTraceByChannel(const FVector& Start, const FVector& End, float Radius, 
                                                 ECollisionChannel TraceChannel, FHitResult& HitResult)
{
    if (UWorld* World = GetWorld())
    {
        FCollisionQueryParams QueryParams;
        QueryParams.bReturnPhysicalMaterial = true;
        
        return World->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, TraceChannel, 
                                          FCollisionShape::MakeSphere(Radius), QueryParams);
    }
    
    return false;
}

TArray<AActor*> UCore_CollisionSystem::GetOverlappingActors(const FVector& Location, float Radius, 
                                                           ECollisionChannel QueryChannel)
{
    TArray<AActor*> OverlappingActors;
    
    if (UWorld* World = GetWorld())
    {
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        
        if (World->OverlapMultiByChannel(OverlapResults, Location, FQuat::Identity, QueryChannel, 
                                        FCollisionShape::MakeSphere(Radius), QueryParams))
        {
            for (const FOverlapResult& Result : OverlapResults)
            {
                if (Result.GetActor())
                {
                    OverlappingActors.Add(Result.GetActor());
                }
            }
        }
    }
    
    return OverlappingActors;
}

void UCore_CollisionSystem::RegisterCollisionCallback(AActor* Actor, const FCore_OnCollisionHit& Callback)
{
    if (Actor)
    {
        CollisionCallbacks.Add(Actor, Callback);
        TrackedActors.Add(Actor);
        
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Registered collision callback for %s"), 
               *Actor->GetName());
    }
}

void UCore_CollisionSystem::HandleCollisionHit(AActor* HitActor, AActor* OtherActor, const FHitResult& HitResult)
{
    if (!HitActor || !OtherActor)
    {
        return;
    }
    
    // Broadcast global collision event
    OnCollisionHit.Broadcast(HitActor, OtherActor, HitResult);
    
    // Call specific actor callback if registered
    if (FCore_OnCollisionHit* Callback = CollisionCallbacks.Find(HitActor))
    {
        Callback->Broadcast(HitActor, OtherActor, HitResult);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Collision between %s and %s"), 
           *HitActor->GetName(), *OtherActor->GetName());
}

void UCore_CollisionSystem::OptimizeCollisionForDistance(AActor* Actor, float DistanceToPlayer)
{
    if (!Actor || !bEnableCollisionOptimization)
    {
        return;
    }
    
    // Disable collision for very distant objects
    if (DistanceToPlayer > MaxCollisionDistance)
    {
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    else
    {
        // Re-enable collision for nearby objects
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
}

void UCore_CollisionSystem::ValidateCollisionSetup()
{
    int32 ValidatedActors = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
            {
                // Check if collision is properly configured
                if (PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                {
                    ValidatedActors++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Validated %d actors with collision"), ValidatedActors);
}

void UCore_CollisionSystem::OptimizeCollisionSettings()
{
    // Clean up tracked actors that may have been destroyed
    TrackedActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
    {
        return !ActorPtr.IsValid();
    });
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystem: Optimized collision settings, tracking %d actors"), 
           TrackedActors.Num());
}