#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    CurrentPhysicsMode = ECore_PhysicsMode::Survival;
    
    // Initialize default physics settings
    PhysicsSettings.GravityScale = 1.0f;
    PhysicsSettings.LinearDamping = 0.01f;
    PhysicsSettings.AngularDamping = 0.0f;
    PhysicsSettings.Restitution = 0.3f;
    PhysicsSettings.Friction = 0.7f;
    PhysicsSettings.bEnableRagdoll = true;
    PhysicsSettings.bEnableDestruction = true;
    
    // Initialize ragdoll settings
    RagdollSettings.BlendTime = 0.2f;
    RagdollSettings.RecoveryTime = 2.0f;
    RagdollSettings.ImpactThreshold = 500.0f;
    RagdollSettings.bAutoRecover = true;
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Initializing physics system"));
    
    InitializePhysicsSettings();
    ApplySurvivalPhysics(); // Default to survival physics mode
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdatePhysicsSimulation(DeltaTime);
    ProcessRagdollActors(DeltaTime);
    ProcessDestructibleActors(DeltaTime);
}

void UCore_PhysicsSystemManager::SetPhysicsMode(ECore_PhysicsMode NewMode)
{
    if (CurrentPhysicsMode == NewMode)
    {
        return;
    }
    
    CurrentPhysicsMode = NewMode;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Switching to physics mode: %d"), (int32)NewMode);
    
    switch (CurrentPhysicsMode)
    {
        case ECore_PhysicsMode::Realistic:
            ApplyRealisticPhysics();
            break;
        case ECore_PhysicsMode::Arcade:
            ApplyArcadePhysics();
            break;
        case ECore_PhysicsMode::Cinematic:
            ApplyCinematicPhysics();
            break;
        case ECore_PhysicsMode::Survival:
            ApplySurvivalPhysics();
            break;
    }
}

void UCore_PhysicsSystemManager::ApplyPhysicsSettings(const FCore_PhysicsSettings& Settings)
{
    PhysicsSettings = Settings;
    
    // Apply gravity scale to world
    UWorld* World = GetWorld();
    if (World && World->GetPhysicsScene())
    {
        World->GetGravityZ() * PhysicsSettings.GravityScale;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Applied physics settings - Gravity: %f, Damping: %f"), 
           PhysicsSettings.GravityScale, PhysicsSettings.LinearDamping);
}

void UCore_PhysicsSystemManager::SetCollisionPreset(AActor* Actor, ECore_CollisionPreset Preset)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: SetCollisionPreset - Actor is null"));
        return;
    }
    
    UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
    
    UPrimitiveComponent* PrimComp = StaticMeshComp ? Cast<UPrimitiveComponent>(StaticMeshComp) : Cast<UPrimitiveComponent>(SkeletalMeshComp);
    
    if (!PrimComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: No primitive component found on actor %s"), *Actor->GetName());
        return;
    }
    
    switch (Preset)
    {
        case ECore_CollisionPreset::Character:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
            break;
            
        case ECore_CollisionPreset::Dinosaur:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            PrimComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
            break;
            
        case ECore_CollisionPreset::Environment:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            break;
            
        case ECore_CollisionPreset::Projectile:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            PrimComp->SetCollisionResponseToAllChannels(ECR_Ignore);
            PrimComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            break;
            
        case ECore_CollisionPreset::Debris:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
            break;
            
        default:
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Applied collision preset %d to actor %s"), 
           (int32)Preset, *Actor->GetName());
}

void UCore_PhysicsSystemManager::EnablePhysicsSimulation(AActor* Actor, bool bEnable)
{
    if (!Actor)
    {
        return;
    }
    
    UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (StaticMeshComp)
    {
        StaticMeshComp->SetSimulatePhysics(bEnable);
        
        if (bEnable)
        {
            StaticMeshComp->SetLinearDamping(PhysicsSettings.LinearDamping);
            StaticMeshComp->SetAngularDamping(PhysicsSettings.AngularDamping);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: %s physics simulation for %s"), 
               bEnable ? TEXT("Enabled") : TEXT("Disabled"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::EnableRagdoll(AActor* Character, const FCore_RagdollSettings& Settings)
{
    if (!Character)
    {
        return;
    }
    
    ACharacter* CharacterActor = Cast<ACharacter>(Character);
    if (!CharacterActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Actor %s is not a Character"), *Character->GetName());
        return;
    }
    
    USkeletalMeshComponent* SkeletalMeshComp = CharacterActor->GetMesh();
    if (!SkeletalMeshComp)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: No skeletal mesh component found on character %s"), *Character->GetName());
        return;
    }
    
    // Enable ragdoll physics
    SkeletalMeshComp->SetSimulatePhysics(true);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Add to ragdoll tracking
    if (!RagdollActors.Contains(Character))
    {
        RagdollActors.Add(Character);
    }
    
    RagdollSettings = Settings;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Enabled ragdoll for character %s"), *Character->GetName());
}

void UCore_PhysicsSystemManager::DisableRagdoll(AActor* Character)
{
    if (!Character)
    {
        return;
    }
    
    ACharacter* CharacterActor = Cast<ACharacter>(Character);
    if (!CharacterActor)
    {
        return;
    }
    
    USkeletalMeshComponent* SkeletalMeshComp = CharacterActor->GetMesh();
    if (SkeletalMeshComp)
    {
        SkeletalMeshComp->SetSimulatePhysics(false);
        SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    
    RagdollActors.Remove(Character);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Disabled ragdoll for character %s"), *Character->GetName());
}

bool UCore_PhysicsSystemManager::IsRagdollActive(AActor* Character) const
{
    return RagdollActors.Contains(Character);
}

void UCore_PhysicsSystemManager::TriggerDestruction(AActor* Actor, FVector ImpactLocation, float ImpactForce)
{
    if (!Actor || !PhysicsSettings.bEnableDestruction)
    {
        return;
    }
    
    // Apply destruction force
    ApplyForceAtLocation(Actor, FVector(0, 0, -ImpactForce), ImpactLocation);
    
    // Add to destructible tracking
    if (!DestructibleActors.Contains(Actor))
    {
        DestructibleActors.Add(Actor);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Triggered destruction on %s with force %f"), 
           *Actor->GetName(), ImpactForce);
}

void UCore_PhysicsSystemManager::SetDestructible(AActor* Actor, bool bDestructible)
{
    if (!Actor)
    {
        return;
    }
    
    if (bDestructible && !DestructibleActors.Contains(Actor))
    {
        DestructibleActors.Add(Actor);
    }
    else if (!bDestructible)
    {
        DestructibleActors.Remove(Actor);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Set destructible %s for %s"), 
           bDestructible ? TEXT("true") : TEXT("false"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::ApplyForceAtLocation(AActor* Actor, FVector Force, FVector Location)
{
    if (!Actor)
    {
        return;
    }
    
    UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (StaticMeshComp && StaticMeshComp->IsSimulatingPhysics())
    {
        StaticMeshComp->AddForceAtLocation(Force, Location);
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Applied force %s at location %s to %s"), 
               *Force.ToString(), *Location.ToString(), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::ApplyImpulse(AActor* Actor, FVector Impulse)
{
    if (!Actor)
    {
        return;
    }
    
    UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (StaticMeshComp && StaticMeshComp->IsSimulatingPhysics())
    {
        StaticMeshComp->AddImpulse(Impulse);
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Applied impulse %s to %s"), 
               *Impulse.ToString(), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::ApplyRadialForce(FVector Origin, float Radius, float Strength, bool bVelChange)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> ActorsInRadius = GetActorsInRadius(Origin, Radius);
    
    for (AActor* Actor : ActorsInRadius)
    {
        if (!Actor)
        {
            continue;
        }
        
        UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (StaticMeshComp && StaticMeshComp->IsSimulatingPhysics())
        {
            FVector Direction = (Actor->GetActorLocation() - Origin).GetSafeNormal();
            FVector Force = Direction * Strength;
            
            if (bVelChange)
            {
                StaticMeshComp->AddImpulse(Force);
            }
            else
            {
                StaticMeshComp->AddForce(Force);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Applied radial force - Origin: %s, Radius: %f, Strength: %f"), 
           *Origin.ToString(), Radius, Strength);
}

bool UCore_PhysicsSystemManager::LineTrace(FVector Start, FVector End, FHitResult& HitResult, bool bTraceComplex)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = bTraceComplex;
    QueryParams.AddIgnoredActor(GetOwner());
    
    return World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams);
}

bool UCore_PhysicsSystemManager::SphereTrace(FVector Start, FVector End, float Radius, FHitResult& HitResult)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    return World->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_WorldStatic, 
                                       FCollisionShape::MakeSphere(Radius), QueryParams);
}

TArray<AActor*> UCore_PhysicsSystemManager::GetActorsInRadius(FVector Center, float Radius)
{
    TArray<AActor*> ActorsInRadius;
    UWorld* World = GetWorld();
    
    if (!World)
    {
        return ActorsInRadius;
    }
    
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = World->OverlapMultiByChannel(OverlapResults, Center, FQuat::Identity, ECC_WorldStatic,
                                             FCollisionShape::MakeSphere(Radius), QueryParams);
    
    if (bHit)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (Result.GetActor())
            {
                ActorsInRadius.Add(Result.GetActor());
            }
        }
    }
    
    return ActorsInRadius;
}

void UCore_PhysicsSystemManager::ValidatePhysicsSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Validating physics system..."));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: No world found for validation"));
        return;
    }
    
    int32 PhysicsActorCount = 0;
    int32 RagdollCount = RagdollActors.Num();
    int32 DestructibleCount = DestructibleActors.Num();
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
            if (StaticMeshComp && StaticMeshComp->IsSimulatingPhysics())
            {
                PhysicsActorCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Validation complete - Physics Actors: %d, Ragdolls: %d, Destructibles: %d"), 
           PhysicsActorCount, RagdollCount, DestructibleCount);
}

void UCore_PhysicsSystemManager::RunPhysicsTests()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Running physics tests..."));
    
    // Test basic physics functionality
    ValidatePhysicsSystem();
    
    // Test line tracing
    FHitResult HitResult;
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Start + FVector(0, 0, -1000);
    
    bool bHit = LineTrace(Start, End, HitResult);
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Line trace test - Hit: %s"), bHit ? TEXT("true") : TEXT("false"));
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics tests complete"));
}

void UCore_PhysicsSystemManager::InitializePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (World)
    {
        // Apply initial physics settings to world
        ApplyPhysicsSettings(PhysicsSettings);
    }
}

void UCore_PhysicsSystemManager::UpdatePhysicsSimulation(float DeltaTime)
{
    // Update physics simulation based on current mode
    // This is called every tick to maintain physics consistency
}

void UCore_PhysicsSystemManager::ProcessRagdollActors(float DeltaTime)
{
    // Process ragdoll recovery and state management
    for (int32 i = RagdollActors.Num() - 1; i >= 0; i--)
    {
        AActor* RagdollActor = RagdollActors[i];
        if (!RagdollActor || !IsValid(RagdollActor))
        {
            RagdollActors.RemoveAt(i);
            continue;
        }
        
        // Handle auto-recovery if enabled
        if (RagdollSettings.bAutoRecover)
        {
            // Implementation for auto-recovery logic would go here
        }
    }
}

void UCore_PhysicsSystemManager::ProcessDestructibleActors(float DeltaTime)
{
    // Process destructible actor states and cleanup
    for (int32 i = DestructibleActors.Num() - 1; i >= 0; i--)
    {
        AActor* DestructibleActor = DestructibleActors[i];
        if (!DestructibleActor || !IsValid(DestructibleActor))
        {
            DestructibleActors.RemoveAt(i);
            continue;
        }
        
        // Process destruction states and effects
    }
}

void UCore_PhysicsSystemManager::ApplyRealisticPhysics()
{
    PhysicsSettings.GravityScale = 1.0f;
    PhysicsSettings.LinearDamping = 0.01f;
    PhysicsSettings.AngularDamping = 0.0f;
    PhysicsSettings.Restitution = 0.2f;
    PhysicsSettings.Friction = 0.8f;
    
    ApplyPhysicsSettings(PhysicsSettings);
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Applied realistic physics settings"));
}

void UCore_PhysicsSystemManager::ApplyArcadePhysics()
{
    PhysicsSettings.GravityScale = 0.7f;
    PhysicsSettings.LinearDamping = 0.05f;
    PhysicsSettings.AngularDamping = 0.1f;
    PhysicsSettings.Restitution = 0.5f;
    PhysicsSettings.Friction = 0.5f;
    
    ApplyPhysicsSettings(PhysicsSettings);
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Applied arcade physics settings"));
}

void UCore_PhysicsSystemManager::ApplyCinematicPhysics()
{
    PhysicsSettings.GravityScale = 0.5f;
    PhysicsSettings.LinearDamping = 0.1f;
    PhysicsSettings.AngularDamping = 0.2f;
    PhysicsSettings.Restitution = 0.3f;
    PhysicsSettings.Friction = 0.6f;
    
    ApplyPhysicsSettings(PhysicsSettings);
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Applied cinematic physics settings"));
}

void UCore_PhysicsSystemManager::ApplySurvivalPhysics()
{
    PhysicsSettings.GravityScale = 1.2f;
    PhysicsSettings.LinearDamping = 0.02f;
    PhysicsSettings.AngularDamping = 0.01f;
    PhysicsSettings.Restitution = 0.25f;
    PhysicsSettings.Friction = 0.9f;
    
    ApplyPhysicsSettings(PhysicsSettings);
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Applied survival physics settings"));
}