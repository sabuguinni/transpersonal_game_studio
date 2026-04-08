#include "VFXSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AVFXSystemManager::AVFXSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms for performance
    
    // Default performance settings
    MaxActiveVFX = 50;
    PerformanceUpdateInterval = 0.1f;
    bEnableDistanceCulling = true;
    bEnableLODSystem = true;
    CurrentQualityLevel = 2; // Default to High quality
    LastPerformanceUpdate = 0.0f;
}

void AVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeVFXPool();
}

void AVFXSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastPerformanceUpdate += DeltaTime;
    if (LastPerformanceUpdate >= PerformanceUpdateInterval)
    {
        UpdateVFXPerformance();
        LastPerformanceUpdate = 0.0f;
    }
}

void AVFXSystemManager::InitializeVFXPool()
{
    // Initialize core survival VFX
    FVFXPoolEntry FireEntry;
    FireEntry.Category = EVFXCategory::Survival;
    FireEntry.Priority = EVFXPriority::Critical;
    FireEntry.MaxInstances = 15;
    FireEntry.CullDistance = 3000.0f;
    RegisterVFXSystem("Fire_Campfire", FireEntry);
    
    FVFXPoolEntry SmokeEntry;
    SmokeEntry.Category = EVFXCategory::Survival;
    SmokeEntry.Priority = EVFXPriority::High;
    SmokeEntry.MaxInstances = 20;
    SmokeEntry.CullDistance = 4000.0f;
    RegisterVFXSystem("Smoke_Campfire", SmokeEntry);
    
    // Initialize wildlife VFX
    FVFXPoolEntry FootstepEntry;
    FootstepEntry.Category = EVFXCategory::Wildlife;
    FootstepEntry.Priority = EVFXPriority::Medium;
    FootstepEntry.MaxInstances = 30;
    FootstepEntry.CullDistance = 2000.0f;
    RegisterVFXSystem("Footstep_Dust", FootstepEntry);
    
    // Initialize environmental VFX
    FVFXPoolEntry RainEntry;
    RainEntry.Category = EVFXCategory::Environmental;
    RainEntry.Priority = EVFXPriority::High;
    RainEntry.MaxInstances = 5;
    RainEntry.CullDistance = 10000.0f;
    RegisterVFXSystem("Rain_Heavy", RainEntry);
}

UNiagaraComponent* AVFXSystemManager::SpawnVFX(const FString& VFXName, const FVector& Location, const FRotator& Rotation, AActor* AttachToActor)
{
    if (!VFXPool.Contains(VFXName))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX System '%s' not found in pool"), *VFXName);
        return nullptr;
    }
    
    // Check if we've exceeded max active VFX
    if (ActiveVFXComponents.Num() >= MaxActiveVFX)
    {
        // Try to cull distant VFX first
        CullDistantVFX();
        
        // If still at limit, don't spawn low priority effects
        const FVFXPoolEntry& PoolEntry = VFXPool[VFXName];
        if (PoolEntry.Priority == EVFXPriority::Low && ActiveVFXComponents.Num() >= MaxActiveVFX)
        {
            return nullptr;
        }
    }
    
    UNiagaraComponent* VFXComponent = GetPooledVFX(VFXName);
    if (!VFXComponent)
    {
        // Create new component if pool is empty
        const FVFXPoolEntry& PoolEntry = VFXPool[VFXName];
        if (PoolEntry.NiagaraSystem.IsValid())
        {
            VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                PoolEntry.NiagaraSystem.LoadSynchronous(),
                Location,
                Rotation
            );
        }
    }
    
    if (VFXComponent)
    {
        VFXComponent->SetWorldLocation(Location);
        VFXComponent->SetWorldRotation(Rotation);
        
        if (AttachToActor)
        {
            VFXComponent->AttachToComponent(
                AttachToActor->GetRootComponent(),
                FAttachmentTransformRules::KeepWorldTransform
            );
        }
        
        VFXComponent->Activate();
        ActiveVFXComponents.Add(VFXComponent);
    }
    
    return VFXComponent;
}

void AVFXSystemManager::ReturnVFXToPool(UNiagaraComponent* VFXComponent)
{
    if (!VFXComponent) return;
    
    VFXComponent->Deactivate();
    VFXComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    
    ActiveVFXComponents.Remove(VFXComponent);
    PooledVFXComponents.Add(VFXComponent);
}

void AVFXSystemManager::RegisterVFXSystem(const FString& Name, const FVFXPoolEntry& PoolEntry)
{
    VFXPool.Add(Name, PoolEntry);
}

UNiagaraComponent* AVFXSystemManager::GetPooledVFX(const FString& VFXName)
{
    // Simple pooling - return first available component
    for (int32 i = PooledVFXComponents.Num() - 1; i >= 0; i--)
    {
        if (PooledVFXComponents[i] && !PooledVFXComponents[i]->IsActive())
        {
            UNiagaraComponent* Component = PooledVFXComponents[i];
            PooledVFXComponents.RemoveAt(i);
            return Component;
        }
    }
    return nullptr;
}

void AVFXSystemManager::UpdateVFXPerformance()
{
    if (bEnableDistanceCulling)
    {
        CullDistantVFX();
    }
    
    if (bEnableLODSystem)
    {
        UpdateVFXLOD();
    }
}

void AVFXSystemManager::CullDistantVFX()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn()) return;
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* VFXComp = ActiveVFXComponents[i];
        if (!VFXComp || !VFXComp->IsActive()) continue;
        
        float Distance = FVector::Dist(PlayerLocation, VFXComp->GetComponentLocation());
        
        // Find the cull distance for this VFX
        float CullDistance = 5000.0f; // Default
        for (const auto& PoolPair : VFXPool)
        {
            if (PoolPair.Value.NiagaraSystem.IsValid() && 
                VFXComp->GetAsset() == PoolPair.Value.NiagaraSystem.LoadSynchronous())
            {
                CullDistance = PoolPair.Value.CullDistance;
                break;
            }
        }
        
        if (Distance > CullDistance)
        {
            ReturnVFXToPool(VFXComp);
        }
    }
}

void AVFXSystemManager::UpdateVFXLOD()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn()) return;
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (!VFXComp || !VFXComp->IsActive()) continue;
        
        float Distance = FVector::Dist(PlayerLocation, VFXComp->GetComponentLocation());
        
        // Simple 3-tier LOD system
        if (Distance < 1000.0f)
        {
            // High detail
            VFXComp->SetFloatParameter("LOD_Multiplier", 1.0f);
        }
        else if (Distance < 3000.0f)
        {
            // Medium detail
            VFXComp->SetFloatParameter("LOD_Multiplier", 0.6f);
        }
        else
        {
            // Low detail
            VFXComp->SetFloatParameter("LOD_Multiplier", 0.3f);
        }
    }
}

void AVFXSystemManager::SetVFXQuality(int32 QualityLevel)
{
    CurrentQualityLevel = FMath::Clamp(QualityLevel, 0, 3);
    
    switch (CurrentQualityLevel)
    {
        case 0: // Low
            MaxActiveVFX = 25;
            PerformanceUpdateInterval = 0.2f;
            break;
        case 1: // Medium
            MaxActiveVFX = 35;
            PerformanceUpdateInterval = 0.15f;
            break;
        case 2: // High
            MaxActiveVFX = 50;
            PerformanceUpdateInterval = 0.1f;
            break;
        case 3: // Ultra
            MaxActiveVFX = 75;
            PerformanceUpdateInterval = 0.05f;
            break;
    }
}

// Survival VFX Functions
UNiagaraComponent* AVFXSystemManager::SpawnFireEffect(const FVector& Location, float Intensity)
{
    UNiagaraComponent* FireVFX = SpawnVFX("Fire_Campfire", Location);
    if (FireVFX)
    {
        FireVFX->SetFloatParameter("Intensity", Intensity);
        FireVFX->SetFloatParameter("Scale", FMath::Clamp(Intensity, 0.5f, 2.0f));
    }
    return FireVFX;
}

UNiagaraComponent* AVFXSystemManager::SpawnSmokeEffect(const FVector& Location, float Density)
{
    UNiagaraComponent* SmokeVFX = SpawnVFX("Smoke_Campfire", Location);
    if (SmokeVFX)
    {
        SmokeVFX->SetFloatParameter("Density", Density);
        SmokeVFX->SetFloatParameter("WindInfluence", 0.8f);
    }
    return SmokeVFX;
}

UNiagaraComponent* AVFXSystemManager::SpawnSparkEffect(const FVector& Location, const FVector& Direction)
{
    UNiagaraComponent* SparkVFX = SpawnVFX("Sparks_Tool", Location);
    if (SparkVFX)
    {
        SparkVFX->SetVectorParameter("SparkDirection", Direction);
        SparkVFX->SetFloatParameter("SparkCount", 15.0f);
    }
    return SparkVFX;
}

// Wildlife VFX Functions
UNiagaraComponent* AVFXSystemManager::SpawnFootstepEffect(const FVector& Location, float DinosaurSize)
{
    UNiagaraComponent* FootstepVFX = SpawnVFX("Footstep_Dust", Location);
    if (FootstepVFX)
    {
        FootstepVFX->SetFloatParameter("Size", DinosaurSize);
        FootstepVFX->SetFloatParameter("DustAmount", DinosaurSize * 10.0f);
    }
    return FootstepVFX;
}

UNiagaraComponent* AVFXSystemManager::SpawnBreathEffect(const FVector& Location, float Temperature)
{
    UNiagaraComponent* BreathVFX = SpawnVFX("Breath_Cold", Location);
    if (BreathVFX)
    {
        float Visibility = FMath::Clamp((20.0f - Temperature) / 20.0f, 0.0f, 1.0f);
        BreathVFX->SetFloatParameter("Visibility", Visibility);
    }
    return BreathVFX;
}

// Environmental VFX Functions
UNiagaraComponent* AVFXSystemManager::SpawnRainEffect(const FVector& Location, float Intensity)
{
    UNiagaraComponent* RainVFX = SpawnVFX("Rain_Heavy", Location);
    if (RainVFX)
    {
        RainVFX->SetFloatParameter("Intensity", Intensity);
        RainVFX->SetFloatParameter("DropSize", FMath::Lerp(0.8f, 1.5f, Intensity));
    }
    return RainVFX;
}

UNiagaraComponent* AVFXSystemManager::SpawnWindEffect(const FVector& Location, const FVector& WindDirection)
{
    UNiagaraComponent* WindVFX = SpawnVFX("Wind_Leaves", Location);
    if (WindVFX)
    {
        WindVFX->SetVectorParameter("WindDirection", WindDirection.GetSafeNormal());
        WindVFX->SetFloatParameter("WindStrength", WindDirection.Size());
    }
    return WindVFX;
}