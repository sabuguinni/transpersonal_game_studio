#include "VFXSystemArchitecture.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"

UVFXSystemManager::UVFXSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick 10 times per second for performance
}

void UVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize performance settings based on platform
    if (GEngine)
    {
        // Adjust settings based on platform capabilities
        FString PlatformName = FPlatformProperties::PlatformName();
        
        if (PlatformName.Contains(TEXT("Windows")) || PlatformName.Contains(TEXT("Mac")))
        {
            // Desktop - High quality settings
            PerformanceSettings.MaxParticlesPerSystem = 2000;
            PerformanceSettings.bEnableGPUSimulation = true;
            PerformanceSettings.bEnableAsyncCompute = true;
            CurrentLODLevel = EVFXLODLevel::LOD1_High;
        }
        else
        {
            // Console/Mobile - Conservative settings
            PerformanceSettings.MaxParticlesPerSystem = 1000;
            PerformanceSettings.bEnableGPUSimulation = false;
            PerformanceSettings.bEnableAsyncCompute = false;
            CurrentLODLevel = EVFXLODLevel::LOD2_Medium;
        }
    }
}

void UVFXSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastPerformanceCheck += DeltaTime;
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        PerformanceOptimizationTick();
        LastPerformanceCheck = 0.0f;
    }
}

UNiagaraComponent* UVFXSystemManager::SpawnVFXAtLocation(
    UNiagaraSystem* VFXSystem,
    FVector Location,
    FRotator Rotation,
    FVector Scale,
    EVFXPriority Priority)
{
    if (!VFXSystem || !GetWorld())
    {
        return nullptr;
    }

    // Check if we should spawn based on priority and performance
    if (ActiveVFXComponents.Num() > PerformanceSettings.MaxParticlesPerSystem && Priority > EVFXPriority::High)
    {
        return nullptr; // Skip low priority effects when at capacity
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        VFXSystem,
        Location,
        Rotation,
        Scale,
        true, // Auto destroy
        true, // Auto activate
        ENCPoolMethod::None,
        true  // Pre cull check
    );

    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        
        // Apply LOD settings
        ApplyLODToComponent(VFXComponent, Priority);
    }

    return VFXComponent;
}

UNiagaraComponent* UVFXSystemManager::SpawnVFXAttached(
    UNiagaraSystem* VFXSystem,
    USceneComponent* AttachToComponent,
    FName AttachPointName,
    FVector Location,
    FRotator Rotation,
    FVector Scale,
    EVFXPriority Priority)
{
    if (!VFXSystem || !AttachToComponent || !GetWorld())
    {
        return nullptr;
    }

    // Check performance constraints
    if (ActiveVFXComponents.Num() > PerformanceSettings.MaxParticlesPerSystem && Priority > EVFXPriority::High)
    {
        return nullptr;
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        VFXSystem,
        AttachToComponent,
        AttachPointName,
        Location,
        Rotation,
        Scale,
        EAttachLocation::KeepRelativeOffset,
        true, // Auto destroy
        ENCPoolMethod::None,
        true  // Pre cull check
    );

    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        ApplyLODToComponent(VFXComponent, Priority);
    }

    return VFXComponent;
}

void UVFXSystemManager::SetVFXQualityLevel(EVFXLODLevel QualityLevel)
{
    CurrentLODLevel = QualityLevel;
    
    // Apply new quality settings to all active VFX
    for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (IsValid(VFXComp))
        {
            ApplyLODToComponent(VFXComp, EVFXPriority::Medium);
        }
    }
}

void UVFXSystemManager::OptimizeVFXForDistance(float ViewerDistance)
{
    EVFXLODLevel NewLODLevel = CurrentLODLevel;
    
    if (ViewerDistance > PerformanceSettings.LODDistanceThreshold_Low)
    {
        NewLODLevel = EVFXLODLevel::LOD3_Low;
    }
    else if (ViewerDistance > PerformanceSettings.LODDistanceThreshold_Medium)
    {
        NewLODLevel = EVFXLODLevel::LOD2_Medium;
    }
    else if (ViewerDistance > PerformanceSettings.LODDistanceThreshold_High)
    {
        NewLODLevel = EVFXLODLevel::LOD1_High;
    }
    else
    {
        NewLODLevel = EVFXLODLevel::LOD0_Ultra;
    }
    
    if (NewLODLevel != CurrentLODLevel)
    {
        SetVFXQualityLevel(NewLODLevel);
    }
}

void UVFXSystemManager::PlayCreatureBreathEffect(AActor* Creature, float IntensityMultiplier)
{
    if (!Creature || !CreatureBreathSystem)
    {
        return;
    }

    // Find breath attachment point (usually head or mouth)
    USceneComponent* AttachPoint = Creature->GetRootComponent();
    
    // Try to find a more specific attachment point
    if (USceneComponent* MeshComp = Creature->FindComponentByClass<USceneComponent>())
    {
        AttachPoint = MeshComp;
    }

    UNiagaraComponent* BreathVFX = SpawnVFXAttached(
        CreatureBreathSystem,
        AttachPoint,
        FName("head"), // Assuming creatures have a head socket
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        FVector::OneVector,
        EVFXPriority::Medium
    );

    if (BreathVFX)
    {
        // Set intensity parameter
        BreathVFX->SetFloatParameter(FName("Intensity"), IntensityMultiplier);
        
        // Set creature size parameter for scaling
        FVector CreatureScale = Creature->GetActorScale3D();
        float ScaleFactor = FMath::Max3(CreatureScale.X, CreatureScale.Y, CreatureScale.Z);
        BreathVFX->SetFloatParameter(FName("CreatureScale"), ScaleFactor);
    }
}

void UVFXSystemManager::PlayCreatureFootstepEffect(AActor* Creature, FVector ImpactLocation, float CreatureWeight)
{
    if (!Creature || !CreatureFootstepSystem)
    {
        return;
    }

    UNiagaraComponent* FootstepVFX = SpawnVFXAtLocation(
        CreatureFootstepSystem,
        ImpactLocation,
        FRotator::ZeroRotator,
        FVector::OneVector,
        EVFXPriority::High // Footsteps are important for player awareness
    );

    if (FootstepVFX)
    {
        // Set weight parameter for dust/debris amount
        FootstepVFX->SetFloatParameter(FName("CreatureWeight"), CreatureWeight);
        
        // Set surface type parameter (would be determined by ground material)
        FootstepVFX->SetIntParameter(FName("SurfaceType"), 0); // Default to dirt/ground
    }
}

void UVFXSystemManager::PlayCreatureImpactEffect(AActor* Creature, FVector ImpactLocation, FVector ImpactNormal, float ImpactForce)
{
    if (!Creature)
    {
        return;
    }

    // Use appropriate system based on impact force
    UNiagaraSystem* ImpactSystem = (ImpactForce > 10.0f) ? CriticalHitSystem : HitEffectSystem;
    
    if (!ImpactSystem)
    {
        return;
    }

    FRotator ImpactRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
    
    UNiagaraComponent* ImpactVFX = SpawnVFXAtLocation(
        ImpactSystem,
        ImpactLocation,
        ImpactRotation,
        FVector::OneVector,
        EVFXPriority::Critical // Combat effects are always critical
    );

    if (ImpactVFX)
    {
        ImpactVFX->SetFloatParameter(FName("ImpactForce"), ImpactForce);
        ImpactVFX->SetVectorParameter(FName("ImpactNormal"), ImpactNormal);
    }
}

void UVFXSystemManager::PlayHitEffect(FVector HitLocation, FVector HitNormal, AActor* HitActor, float Damage)
{
    if (!HitEffectSystem)
    {
        return;
    }

    FRotator HitRotation = FRotationMatrix::MakeFromZ(HitNormal).Rotator();
    
    UNiagaraComponent* HitVFX = SpawnVFXAtLocation(
        HitEffectSystem,
        HitLocation,
        HitRotation,
        FVector::OneVector,
        EVFXPriority::Critical
    );

    if (HitVFX)
    {
        HitVFX->SetFloatParameter(FName("Damage"), Damage);
        HitVFX->SetVectorParameter(FName("HitNormal"), HitNormal);
        
        // Set color based on damage type or actor type
        if (HitActor)
        {
            // Could determine blood color, spark color, etc. based on actor type
            FLinearColor EffectColor = FLinearColor::Red; // Default to blood
            HitVFX->SetColorParameter(FName("EffectColor"), EffectColor);
        }
    }
}

void UVFXSystemManager::PlayCriticalHitEffect(FVector HitLocation, FVector HitNormal, AActor* HitActor)
{
    PlayHitEffect(HitLocation, HitNormal, HitActor, 100.0f); // High damage value for critical
}

void UVFXSystemManager::PlayDestructionEffect(FVector Location, UStaticMesh* DestroyedMesh, float DestructionScale)
{
    if (!DestructionSystem)
    {
        return;
    }

    UNiagaraComponent* DestructionVFX = SpawnVFXAtLocation(
        DestructionSystem,
        Location,
        FRotator::ZeroRotator,
        FVector(DestructionScale),
        EVFXPriority::High
    );

    if (DestructionVFX)
    {
        DestructionVFX->SetFloatParameter(FName("DestructionScale"), DestructionScale);
        
        // Set debris parameters based on destroyed mesh
        if (DestroyedMesh)
        {
            FBox MeshBounds = DestroyedMesh->GetBounds().GetBox();
            float MeshSize = MeshBounds.GetSize().GetMax();
            DestructionVFX->SetFloatParameter(FName("MeshSize"), MeshSize);
        }
    }
}

void UVFXSystemManager::PerformanceOptimizationTick()
{
    // Clean up destroyed components
    ActiveVFXComponents.RemoveAll([](UNiagaraComponent* Comp)
    {
        return !IsValid(Comp) || !Comp->IsActive();
    });

    // Cull distant VFX
    CullDistantVFX();
    
    // Adjust quality based on performance
    AdjustVFXQualityBasedOnPerformance();
}

void UVFXSystemManager::CullDistantVFX()
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
    {
        return;
    }

    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* VFXComp = ActiveVFXComponents[i];
        if (!IsValid(VFXComp))
        {
            ActiveVFXComponents.RemoveAt(i);
            continue;
        }

        float Distance = FVector::Dist(VFXComp->GetComponentLocation(), PlayerLocation);
        
        // Cull very distant low-priority effects
        if (Distance > PerformanceSettings.LODDistanceThreshold_Low * 2.0f)
        {
            VFXComp->DestroyComponent();
            ActiveVFXComponents.RemoveAt(i);
        }
    }
}

void UVFXSystemManager::AdjustVFXQualityBasedOnPerformance()
{
    // Simple performance heuristic - if we have too many active VFX, reduce quality
    if (ActiveVFXComponents.Num() > PerformanceSettings.MaxParticlesPerSystem * 0.8f)
    {
        if (CurrentLODLevel < EVFXLODLevel::LOD3_Low)
        {
            SetVFXQualityLevel(static_cast<EVFXLODLevel>(static_cast<int32>(CurrentLODLevel) + 1));
        }
    }
    else if (ActiveVFXComponents.Num() < PerformanceSettings.MaxParticlesPerSystem * 0.5f)
    {
        if (CurrentLODLevel > EVFXLODLevel::LOD0_Ultra)
        {
            SetVFXQualityLevel(static_cast<EVFXLODLevel>(static_cast<int32>(CurrentLODLevel) - 1));
        }
    }
}

void UVFXSystemManager::ApplyLODToComponent(UNiagaraComponent* VFXComponent, EVFXPriority Priority)
{
    if (!VFXComponent)
    {
        return;
    }

    // Apply LOD settings based on current LOD level and priority
    float QualityMultiplier = 1.0f;
    
    switch (CurrentLODLevel)
    {
        case EVFXLODLevel::LOD0_Ultra:
            QualityMultiplier = 1.0f;
            break;
        case EVFXLODLevel::LOD1_High:
            QualityMultiplier = 0.8f;
            break;
        case EVFXLODLevel::LOD2_Medium:
            QualityMultiplier = 0.6f;
            break;
        case EVFXLODLevel::LOD3_Low:
            QualityMultiplier = 0.4f;
            break;
    }

    // Critical effects maintain higher quality
    if (Priority == EVFXPriority::Critical)
    {
        QualityMultiplier = FMath::Max(QualityMultiplier, 0.7f);
    }

    // Apply quality settings to the VFX component
    VFXComponent->SetFloatParameter(FName("QualityMultiplier"), QualityMultiplier);
}