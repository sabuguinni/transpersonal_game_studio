#include "VFXManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"

AVFXManager::AVFXManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Tick every 100ms for performance

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    CurrentActiveEffects = 0;
    
    // Initialize default performance settings
    PerformanceSettings.MaxActiveEffects = 50;
    PerformanceSettings.CullingDistance = 3000.0f;
    PerformanceSettings.QualityLevel = EVFXQuality::High;
    PerformanceSettings.bEnableLODSystem = true;
    PerformanceSettings.LODNearDistance = 500.0f;
    PerformanceSettings.LODFarDistance = 2000.0f;
}

void AVFXManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultVFXSystems();
    
    UE_LOG(LogTemp, Log, TEXT("VFX Manager initialized with %d registered systems"), RegisteredVFXSystems.Num());
}

void AVFXManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CleanupFinishedEffects();
    
    // Performance culling based on player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (PlayerPawn && PerformanceSettings.bEnableLODSystem)
    {
        CullDistantEffects(PlayerPawn->GetActorLocation());
    }
}

void AVFXManager::InitializeDefaultVFXSystems()
{
    // Register default VFX systems for prehistoric game
    
    // Environmental Effects
    FVFXSystemData FireData;
    FireData.EffectName = TEXT("Fire");
    FireData.Category = EVFXCategory::Environmental;
    FireData.MaxDistance = 2000.0f;
    FireData.MaxParticles = 2000;
    FireData.bAutoDestroy = false;
    FireData.LifeTime = 0.0f; // Continuous
    RegisteredVFXSystems.Add(TEXT("Fire"), FireData);

    FVFXSystemData SmokeData;
    SmokeData.EffectName = TEXT("Smoke");
    SmokeData.Category = EVFXCategory::Environmental;
    SmokeData.MaxDistance = 1500.0f;
    SmokeData.MaxParticles = 1500;
    SmokeData.bAutoDestroy = false;
    SmokeData.LifeTime = 0.0f;
    RegisteredVFXSystems.Add(TEXT("Smoke"), SmokeData);

    // Combat Effects
    FVFXSystemData ExplosionData;
    ExplosionData.EffectName = TEXT("Explosion");
    ExplosionData.Category = EVFXCategory::Combat;
    ExplosionData.MaxDistance = 3000.0f;
    ExplosionData.MaxParticles = 5000;
    ExplosionData.bAutoDestroy = true;
    ExplosionData.LifeTime = 3.0f;
    RegisteredVFXSystems.Add(TEXT("Explosion"), ExplosionData);

    FVFXSystemData BloodData;
    BloodData.EffectName = TEXT("Blood");
    BloodData.Category = EVFXCategory::Combat;
    BloodData.MaxDistance = 1000.0f;
    BloodData.MaxParticles = 500;
    BloodData.bAutoDestroy = true;
    BloodData.LifeTime = 2.0f;
    RegisteredVFXSystems.Add(TEXT("Blood"), BloodData);

    // Mystical Effects
    FVFXSystemData ConsciousnessData;
    ConsciousnessData.EffectName = TEXT("Consciousness");
    ConsciousnessData.Category = EVFXCategory::Mystical;
    ConsciousnessData.MaxDistance = 2500.0f;
    ConsciousnessData.MaxParticles = 3000;
    ConsciousnessData.bAutoDestroy = false;
    ConsciousnessData.LifeTime = 0.0f;
    RegisteredVFXSystems.Add(TEXT("Consciousness"), ConsciousnessData);

    FVFXSystemData HealingData;
    HealingData.EffectName = TEXT("Healing");
    HealingData.Category = EVFXCategory::Mystical;
    HealingData.MaxDistance = 1500.0f;
    HealingData.MaxParticles = 1000;
    HealingData.bAutoDestroy = true;
    HealingData.LifeTime = 4.0f;
    RegisteredVFXSystems.Add(TEXT("Healing"), HealingData);

    // Natural Effects
    FVFXSystemData LightningData;
    LightningData.EffectName = TEXT("Lightning");
    LightningData.Category = EVFXCategory::Natural;
    LightningData.MaxDistance = 5000.0f;
    LightningData.MaxParticles = 1000;
    LightningData.bAutoDestroy = true;
    LightningData.LifeTime = 1.0f;
    RegisteredVFXSystems.Add(TEXT("Lightning"), LightningData);

    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Initialized %d default VFX systems"), RegisteredVFXSystems.Num());
}

UNiagaraComponent* AVFXManager::SpawnVFXEffect(const FString& EffectName, const FVector& Location, const FRotator& Rotation, AActor* AttachToActor)
{
    if (CurrentActiveEffects >= PerformanceSettings.MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Max active effects reached (%d), skipping spawn"), PerformanceSettings.MaxActiveEffects);
        return nullptr;
    }

    FVFXSystemData* SystemData = GetVFXSystemData(EffectName);
    if (!SystemData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Effect '%s' not found in registered systems"), *EffectName);
        return nullptr;
    }

    UNiagaraSystem* NiagaraSystem = SystemData->NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Failed to load Niagara system for effect '%s'"), *EffectName);
        return nullptr;
    }

    UNiagaraComponent* VFXComponent = nullptr;

    if (AttachToActor)
    {
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            NiagaraSystem,
            AttachToActor->GetRootComponent(),
            NAME_None,
            Location,
            Rotation,
            EAttachLocation::KeepWorldPosition,
            SystemData->bAutoDestroy
        );
    }
    else
    {
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            NiagaraSystem,
            Location,
            Rotation,
            FVector::OneVector,
            SystemData->bAutoDestroy
        );
    }

    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        CurrentActiveEffects++;

        // Apply performance settings
        if (PerformanceSettings.bEnableLODSystem)
        {
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
            if (PlayerPawn)
            {
                float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
                ApplyLODSettings(VFXComponent, Distance);
            }
        }

        // Set auto-destroy timer if specified
        if (SystemData->bAutoDestroy && SystemData->LifeTime > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, VFXComponent]()
            {
                StopVFXEffect(VFXComponent);
            }, SystemData->LifeTime, false);
        }

        UE_LOG(LogTemp, Log, TEXT("VFX Manager: Spawned effect '%s' at location %s"), *EffectName, *Location.ToString());
    }

    return VFXComponent;
}

void AVFXManager::StopVFXEffect(UNiagaraComponent* VFXComponent)
{
    if (!VFXComponent || !IsValid(VFXComponent))
    {
        return;
    }

    VFXComponent->Deactivate();
    ActiveVFXComponents.Remove(VFXComponent);
    CurrentActiveEffects = FMath::Max(0, CurrentActiveEffects - 1);

    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Stopped VFX effect. Active effects: %d"), CurrentActiveEffects);
}

void AVFXManager::StopAllVFXEffects()
{
    for (UNiagaraComponent* VFXComponent : ActiveVFXComponents)
    {
        if (IsValid(VFXComponent))
        {
            VFXComponent->Deactivate();
        }
    }

    ActiveVFXComponents.Empty();
    CurrentActiveEffects = 0;

    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Stopped all VFX effects"));
}

void AVFXManager::RegisterVFXSystem(const FString& EffectName, UNiagaraSystem* NiagaraSystem, EVFXCategory Category)
{
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Cannot register null Niagara system for effect '%s'"), *EffectName);
        return;
    }

    FVFXSystemData SystemData;
    SystemData.EffectName = EffectName;
    SystemData.NiagaraSystem = NiagaraSystem;
    SystemData.Category = Category;

    RegisteredVFXSystems.Add(EffectName, SystemData);

    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Registered VFX system '%s'"), *EffectName);
}

void AVFXManager::UnregisterVFXSystem(const FString& EffectName)
{
    if (RegisteredVFXSystems.Remove(EffectName) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX Manager: Unregistered VFX system '%s'"), *EffectName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Failed to unregister VFX system '%s' - not found"), *EffectName);
    }
}

void AVFXManager::SetVFXQuality(EVFXQuality NewQuality)
{
    PerformanceSettings.QualityLevel = NewQuality;

    // Adjust performance settings based on quality
    switch (NewQuality)
    {
        case EVFXQuality::Low:
            PerformanceSettings.MaxActiveEffects = 20;
            PerformanceSettings.CullingDistance = 1500.0f;
            break;
        case EVFXQuality::Medium:
            PerformanceSettings.MaxActiveEffects = 35;
            PerformanceSettings.CullingDistance = 2500.0f;
            break;
        case EVFXQuality::High:
            PerformanceSettings.MaxActiveEffects = 50;
            PerformanceSettings.CullingDistance = 3000.0f;
            break;
        case EVFXQuality::Ultra:
            PerformanceSettings.MaxActiveEffects = 100;
            PerformanceSettings.CullingDistance = 5000.0f;
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Quality set to %d, Max Effects: %d"), (int32)NewQuality, PerformanceSettings.MaxActiveEffects);
}

void AVFXManager::UpdatePerformanceSettings(const FVFXPerformanceSettings& NewSettings)
{
    PerformanceSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Performance settings updated"));
}

void AVFXManager::CullDistantEffects(const FVector& ViewerLocation)
{
    TArray<UNiagaraComponent*> ComponentsToRemove;

    for (UNiagaraComponent* VFXComponent : ActiveVFXComponents)
    {
        if (!IsValid(VFXComponent))
        {
            ComponentsToRemove.Add(VFXComponent);
            continue;
        }

        float Distance = FVector::Dist(ViewerLocation, VFXComponent->GetComponentLocation());
        
        if (Distance > PerformanceSettings.CullingDistance)
        {
            VFXComponent->Deactivate();
            ComponentsToRemove.Add(VFXComponent);
        }
        else if (PerformanceSettings.bEnableLODSystem)
        {
            ApplyLODSettings(VFXComponent, Distance);
        }
    }

    for (UNiagaraComponent* ComponentToRemove : ComponentsToRemove)
    {
        ActiveVFXComponents.Remove(ComponentToRemove);
        CurrentActiveEffects = FMath::Max(0, CurrentActiveEffects - 1);
    }
}

void AVFXManager::CleanupFinishedEffects()
{
    TArray<UNiagaraComponent*> ComponentsToRemove;

    for (UNiagaraComponent* VFXComponent : ActiveVFXComponents)
    {
        if (!IsValid(VFXComponent) || !VFXComponent->IsActive())
        {
            ComponentsToRemove.Add(VFXComponent);
        }
    }

    for (UNiagaraComponent* ComponentToRemove : ComponentsToRemove)
    {
        ActiveVFXComponents.Remove(ComponentToRemove);
        CurrentActiveEffects = FMath::Max(0, CurrentActiveEffects - 1);
    }
}

void AVFXManager::ApplyLODSettings(UNiagaraComponent* VFXComponent, float Distance)
{
    if (!VFXComponent)
    {
        return;
    }

    float LODLevel = 0.0f;
    
    if (Distance > PerformanceSettings.LODFarDistance)
    {
        LODLevel = 2.0f; // Low detail
    }
    else if (Distance > PerformanceSettings.LODNearDistance)
    {
        LODLevel = 1.0f; // Medium detail
    }
    else
    {
        LODLevel = 0.0f; // High detail
    }

    // Apply LOD scaling
    float ScaleFactor = 1.0f - (LODLevel * 0.3f); // Reduce scale by 30% per LOD level
    VFXComponent->SetRelativeScale3D(FVector(ScaleFactor));
}

FVFXSystemData* AVFXManager::GetVFXSystemData(const FString& EffectName)
{
    return RegisteredVFXSystems.Find(EffectName);
}

// Preset VFX Effects Implementation
UNiagaraComponent* AVFXManager::SpawnFireEffect(const FVector& Location, float Scale)
{
    UNiagaraComponent* FireComponent = SpawnVFXEffect(TEXT("Fire"), Location);
    if (FireComponent)
    {
        FireComponent->SetRelativeScale3D(FVector(Scale));
    }
    return FireComponent;
}

UNiagaraComponent* AVFXManager::SpawnSmokeEffect(const FVector& Location, float Scale)
{
    UNiagaraComponent* SmokeComponent = SpawnVFXEffect(TEXT("Smoke"), Location);
    if (SmokeComponent)
    {
        SmokeComponent->SetRelativeScale3D(FVector(Scale));
    }
    return SmokeComponent;
}

UNiagaraComponent* AVFXManager::SpawnExplosionEffect(const FVector& Location, float Scale)
{
    UNiagaraComponent* ExplosionComponent = SpawnVFXEffect(TEXT("Explosion"), Location);
    if (ExplosionComponent)
    {
        ExplosionComponent->SetRelativeScale3D(FVector(Scale));
    }
    return ExplosionComponent;
}

UNiagaraComponent* AVFXManager::SpawnConsciousnessEffect(const FVector& Location, float Scale)
{
    UNiagaraComponent* ConsciousnessComponent = SpawnVFXEffect(TEXT("Consciousness"), Location);
    if (ConsciousnessComponent)
    {
        ConsciousnessComponent->SetRelativeScale3D(FVector(Scale));
    }
    return ConsciousnessComponent;
}

UNiagaraComponent* AVFXManager::SpawnHealingEffect(AActor* TargetActor, float Scale)
{
    if (!TargetActor)
    {
        return nullptr;
    }

    UNiagaraComponent* HealingComponent = SpawnVFXEffect(TEXT("Healing"), TargetActor->GetActorLocation(), FRotator::ZeroRotator, TargetActor);
    if (HealingComponent)
    {
        HealingComponent->SetRelativeScale3D(FVector(Scale));
    }
    return HealingComponent;
}