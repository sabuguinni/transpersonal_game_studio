#include "Core_DestructionSystem.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialInterface.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    bIsDestroyed = false;
    AccumulatedDamage = 0.0f;
    FragmentScale = 1.0f;
    FragmentImpulse = 500.0f;
    
    // Initialize default destruction settings
    DestructionSettings.DestructionType = ECore_DestructionType::Fracture;
    DestructionSettings.DestructionThreshold = 100.0f;
    DestructionSettings.FragmentCount = 8;
    DestructionSettings.FragmentLifetime = 5.0f;
    DestructionSettings.bCreateDebris = true;
    DestructionSettings.bPlaySound = true;
    DestructionSettings.bSpawnParticles = true;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMaterialThresholds();
    
    // Set cleanup timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            CleanupTimer,
            this,
            &UCore_DestructionSystem::CleanupFragments,
            DestructionSettings.FragmentLifetime,
            true
        );
    }
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Decay accumulated damage over time
    if (AccumulatedDamage > 0.0f)
    {
        AccumulatedDamage = FMath::Max(0.0f, AccumulatedDamage - (DeltaTime * 10.0f));
    }
}

void UCore_DestructionSystem::TriggerDestruction(FVector ImpactLocation, float ImpactForce, FVector ImpactDirection)
{
    if (bIsDestroyed || !CanBeDestroyed(ImpactForce))
    {
        return;
    }
    
    AccumulatedDamage += ImpactForce;
    
    if (AccumulatedDamage >= DestructionSettings.DestructionThreshold)
    {
        bIsDestroyed = true;
        
        // Broadcast destruction event
        OnDestructionTriggered.Broadcast(GetOwner(), ImpactLocation);
        
        // Create fragments based on destruction type
        if (DestructionSettings.bCreateDebris)
        {
            CreateFragments(ImpactLocation, ImpactDirection);
        }
        
        // Play effects
        PlayDestructionEffects(ImpactLocation);
        
        // Hide or destroy the original actor
        if (GetOwner())
        {
            GetOwner()->SetActorHiddenInGame(true);
            GetOwner()->SetActorEnableCollision(false);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Destruction triggered for %s at location %s"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               *ImpactLocation.ToString());
    }
}

void UCore_DestructionSystem::SetDestructionType(ECore_DestructionType NewType)
{
    DestructionSettings.DestructionType = NewType;
}

void UCore_DestructionSystem::SetDestructionThreshold(float NewThreshold)
{
    DestructionSettings.DestructionThreshold = FMath::Max(0.0f, NewThreshold);
}

bool UCore_DestructionSystem::CanBeDestroyed(float AppliedForce) const
{
    if (bIsDestroyed)
    {
        return false;
    }
    
    // Check material-specific thresholds
    FString MaterialName = GetOwnerMaterialName();
    if (MaterialThresholds.Contains(MaterialName))
    {
        return (AccumulatedDamage + AppliedForce) >= MaterialThresholds[MaterialName];
    }
    
    return (AccumulatedDamage + AppliedForce) >= DestructionSettings.DestructionThreshold;
}

void UCore_DestructionSystem::CreateFragments(FVector ImpactLocation, FVector ImpactDirection)
{
    switch (DestructionSettings.DestructionType)
    {
        case ECore_DestructionType::Fracture:
            CreateFractureFragments(ImpactLocation, ImpactDirection);
            break;
        case ECore_DestructionType::Shatter:
            CreateShatterFragments(ImpactLocation);
            break;
        case ECore_DestructionType::Crumble:
            CreateCrumbleFragments(ImpactLocation);
            break;
        case ECore_DestructionType::Explode:
            CreateExplosionFragments(ImpactLocation);
            break;
        case ECore_DestructionType::Dissolve:
            // Dissolve doesn't create physical fragments
            break;
        default:
            CreateFractureFragments(ImpactLocation, ImpactDirection);
            break;
    }
}

void UCore_DestructionSystem::PlayDestructionEffects(FVector Location)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Spawn particles
    if (DestructionSettings.bSpawnParticles && DestructionParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            DestructionParticles,
            Location,
            FRotator::ZeroRotator,
            FVector(1.0f),
            true
        );
    }
    
    // Play sound
    if (DestructionSettings.bPlaySound && DestructionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            DestructionSound,
            Location,
            1.0f,
            1.0f
        );
    }
}

void UCore_DestructionSystem::CleanupFragments()
{
    for (int32 i = CreatedFragments.Num() - 1; i >= 0; i--)
    {
        if (CreatedFragments[i] && IsValid(CreatedFragments[i]))
        {
            // Check if fragment is old enough to be cleaned up
            float FragmentAge = GetWorld()->GetTimeSeconds() - CreatedFragments[i]->GetActorTimestamp();
            if (FragmentAge > DestructionSettings.FragmentLifetime)
            {
                CreatedFragments[i]->Destroy();
                CreatedFragments.RemoveAt(i);
            }
        }
        else
        {
            CreatedFragments.RemoveAt(i);
        }
    }
}

void UCore_DestructionSystem::SetMaterialThreshold(const FString& MaterialName, float Threshold)
{
    MaterialThresholds.Add(MaterialName, Threshold);
}

float UCore_DestructionSystem::GetMaterialThreshold(const FString& MaterialName) const
{
    if (MaterialThresholds.Contains(MaterialName))
    {
        return MaterialThresholds[MaterialName];
    }
    return DestructionSettings.DestructionThreshold;
}

void UCore_DestructionSystem::CreateRadialDestruction(FVector Center, float Radius, float Force)
{
    if (!GetWorld())
    {
        return;
    }
    
    TArray<AActor*> OverlappingActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        Center,
        Radius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        nullptr,
        TArray<AActor*>(),
        OverlappingActors
    );
    
    for (AActor* Actor : OverlappingActors)
    {
        if (UCore_DestructionSystem* DestructionComp = Actor->FindComponentByClass<UCore_DestructionSystem>())
        {
            FVector Direction = (Actor->GetActorLocation() - Center).GetSafeNormal();
            float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
            float AdjustedForce = Force * (1.0f - (Distance / Radius));
            
            DestructionComp->TriggerDestruction(Actor->GetActorLocation(), AdjustedForce, Direction);
        }
    }
}

void UCore_DestructionSystem::CreateDirectionalDestruction(FVector Start, FVector End, float Width, float Force)
{
    if (!GetWorld())
    {
        return;
    }
    
    FVector Direction = (End - Start).GetSafeNormal();
    float Length = FVector::Dist(Start, End);
    FVector Center = Start + (Direction * Length * 0.5f);
    
    TArray<AActor*> OverlappingActors;
    UKismetSystemLibrary::BoxOverlapActors(
        GetWorld(),
        Center,
        FVector(Length * 0.5f, Width * 0.5f, Width * 0.5f),
        nullptr,
        TArray<AActor*>(),
        OverlappingActors
    );
    
    for (AActor* Actor : OverlappingActors)
    {
        if (UCore_DestructionSystem* DestructionComp = Actor->FindComponentByClass<UCore_DestructionSystem>())
        {
            DestructionComp->TriggerDestruction(Actor->GetActorLocation(), Force, Direction);
        }
    }
}

void UCore_DestructionSystem::ScheduleDelayedDestruction(float Delay, FVector ImpactLocation, float ImpactForce)
{
    if (GetWorld())
    {
        // Store parameters for delayed execution
        GetWorld()->GetTimerManager().SetTimer(
            DelayedDestructionTimer,
            this,
            &UCore_DestructionSystem::HandleDelayedDestruction,
            Delay,
            false
        );
    }
}

void UCore_DestructionSystem::InitializeMaterialThresholds()
{
    // Set default material thresholds
    MaterialThresholds.Add(TEXT("Wood"), 75.0f);
    MaterialThresholds.Add(TEXT("Stone"), 150.0f);
    MaterialThresholds.Add(TEXT("Metal"), 200.0f);
    MaterialThresholds.Add(TEXT("Glass"), 25.0f);
    MaterialThresholds.Add(TEXT("Bone"), 100.0f);
    MaterialThresholds.Add(TEXT("Ice"), 50.0f);
}

void UCore_DestructionSystem::CreateFractureFragments(FVector ImpactLocation, FVector ImpactDirection)
{
    if (!GetWorld() || !FragmentMesh)
    {
        return;
    }
    
    for (int32 i = 0; i < DestructionSettings.FragmentCount; i++)
    {
        FVector RandomOffset = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(10.0f, 50.0f);
        FVector SpawnLocation = ImpactLocation + RandomOffset;
        FRotator RandomRotation = UKismetMathLibrary::RandomRotator();
        
        AStaticMeshActor* Fragment = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnLocation, RandomRotation);
        if (Fragment)
        {
            Fragment->GetStaticMeshComponent()->SetStaticMesh(FragmentMesh);
            Fragment->GetStaticMeshComponent()->SetWorldScale3D(FVector(FragmentScale * FMath::RandRange(0.5f, 1.5f)));
            
            ApplyFragmentPhysics(Fragment, ImpactDirection, FragmentImpulse);
            CreatedFragments.Add(Fragment);
        }
    }
}

void UCore_DestructionSystem::CreateShatterFragments(FVector ImpactLocation)
{
    FVector RandomDirection = UKismetMathLibrary::RandomUnitVector();
    CreateFractureFragments(ImpactLocation, RandomDirection);
}

void UCore_DestructionSystem::CreateCrumbleFragments(FVector ImpactLocation)
{
    FVector DownDirection = FVector(0, 0, -1);
    CreateFractureFragments(ImpactLocation, DownDirection);
}

void UCore_DestructionSystem::CreateExplosionFragments(FVector ImpactLocation)
{
    if (!GetWorld() || !FragmentMesh)
    {
        return;
    }
    
    for (int32 i = 0; i < DestructionSettings.FragmentCount; i++)
    {
        FVector RadialDirection = UKismetMathLibrary::RandomUnitVector();
        FVector SpawnLocation = ImpactLocation + (RadialDirection * FMath::RandRange(20.0f, 100.0f));
        FRotator RandomRotation = UKismetMathLibrary::RandomRotator();
        
        AStaticMeshActor* Fragment = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnLocation, RandomRotation);
        if (Fragment)
        {
            Fragment->GetStaticMeshComponent()->SetStaticMesh(FragmentMesh);
            Fragment->GetStaticMeshComponent()->SetWorldScale3D(FVector(FragmentScale * FMath::RandRange(0.3f, 1.0f)));
            
            ApplyFragmentPhysics(Fragment, RadialDirection, FragmentImpulse * 2.0f);
            CreatedFragments.Add(Fragment);
        }
    }
}

void UCore_DestructionSystem::ApplyFragmentPhysics(AActor* Fragment, FVector ImpactDirection, float Force)
{
    if (!Fragment)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Fragment->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        
        FVector Impulse = ImpactDirection * Force * FMath::RandRange(0.5f, 1.5f);
        MeshComp->AddImpulse(Impulse, NAME_None, true);
    }
}

void UCore_DestructionSystem::HandleDelayedDestruction()
{
    // This would be called by the timer for delayed destruction
    // Parameters would need to be stored as member variables
    UE_LOG(LogTemp, Log, TEXT("Delayed destruction executed"));
}

FString UCore_DestructionSystem::GetOwnerMaterialName() const
{
    if (!GetOwner())
    {
        return TEXT("Unknown");
    }
    
    UStaticMeshComponent* MeshComp = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp && MeshComp->GetMaterial(0))
    {
        return MeshComp->GetMaterial(0)->GetName();
    }
    
    return TEXT("Default");
}