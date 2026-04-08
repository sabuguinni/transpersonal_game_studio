#include "VFXSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UVFXSystemManager::UVFXSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = LODUpdateInterval;
    
    // Default performance settings
    MaxActiveVFX = 100;
    CurrentVFXQuality = 2; // High quality by default
    LODUpdateInterval = 0.5f;
    HighLODDistance = 1000.0f;
    MediumLODDistance = 3000.0f;
    
    LastLODUpdateTime = 0.0f;
    CachedPlayerController = nullptr;
}

void UVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache player controller for distance calculations
    CachedPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager initialized with quality level: %d"), CurrentVFXQuality);
}

void UVFXSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update LOD system periodically
    LastLODUpdateTime += DeltaTime;
    if (LastLODUpdateTime >= LODUpdateInterval)
    {
        UpdateAllVFXLOD();
        CleanupInactiveVFX();
        LastLODUpdateTime = 0.0f;
    }
}

UNiagaraComponent* UVFXSystemManager::SpawnVFX(const FString& EffectName, FVector Location, FRotator Rotation, AActor* AttachToActor)
{
    // Find effect data in library
    FVFXEffectData* EffectData = VFXLibrary.Find(EffectName);
    if (!EffectData || !EffectData->NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX effect not found: %s"), *EffectName);
        return nullptr;
    }
    
    // Check if we can spawn new VFX (performance limit)
    if (ActiveVFXComponents.Num() >= MaxActiveVFX)
    {
        // Try to cull low priority effects
        for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; --i)
        {
            if (ActiveVFXComponents[i].IsValid())
            {
                UNiagaraComponent* Component = ActiveVFXComponents[i].Get();
                if (ShouldCullVFX(Component))
                {
                    Component->DestroyComponent();
                    ActiveVFXComponents.RemoveAt(i);
                    break;
                }
            }
            else
            {
                ActiveVFXComponents.RemoveAt(i);
            }
        }
        
        // Still at limit after culling
        if (ActiveVFXComponents.Num() >= MaxActiveVFX)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX spawn limit reached, skipping: %s"), *EffectName);
            return nullptr;
        }
    }
    
    UNiagaraComponent* VFXComponent = nullptr;
    
    if (AttachToActor)
    {
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            EffectData->NiagaraSystem,
            AttachToActor->GetRootComponent(),
            NAME_None,
            Location,
            Rotation,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }
    else
    {
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            EffectData->NiagaraSystem,
            Location,
            Rotation,
            FVector::OneVector,
            true,
            true,
            ENCPoolMethod::None,
            true
        );
    }
    
    if (VFXComponent)
    {
        // Apply LOD based on distance
        if (CachedPlayerController && CachedPlayerController->GetPawn())
        {
            float Distance = FVector::Dist(Location, CachedPlayerController->GetPawn()->GetActorLocation());
            UpdateVFXLOD(VFXComponent, Distance);
        }
        
        ActiveVFXComponents.Add(VFXComponent);
        
        UE_LOG(LogTemp, Log, TEXT("VFX spawned: %s at location: %s"), 
               *EffectName, *Location.ToString());
    }
    
    return VFXComponent;
}

void UVFXSystemManager::StopVFX(UNiagaraComponent* VFXComponent)
{
    if (VFXComponent && IsValid(VFXComponent))
    {
        VFXComponent->DestroyComponent();
        ActiveVFXComponents.RemoveSingle(VFXComponent);
    }
}

void UVFXSystemManager::StopAllVFXByCategory(EVFXCategory Category)
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; --i)
    {
        if (ActiveVFXComponents[i].IsValid())
        {
            UNiagaraComponent* Component = ActiveVFXComponents[i].Get();
            // Note: Would need to store category info with component to implement this properly
            // For now, just log the request
            UE_LOG(LogTemp, Log, TEXT("Stopping VFX by category: %d"), (int32)Category);
        }
    }
}

void UVFXSystemManager::SetGlobalVFXQuality(float QualityLevel)
{
    CurrentVFXQuality = FMath::Clamp(FMath::RoundToInt(QualityLevel * 3), 0, 3);
    
    // Update all active VFX with new quality settings
    UpdateAllVFXLOD();
    
    UE_LOG(LogTemp, Log, TEXT("VFX Quality set to: %d"), CurrentVFXQuality);
}

void UVFXSystemManager::UpdateLODSystem(FVector ViewerLocation)
{
    for (TWeakObjectPtr<UNiagaraComponent>& WeakComponent : ActiveVFXComponents)
    {
        if (WeakComponent.IsValid())
        {
            UNiagaraComponent* Component = WeakComponent.Get();
            float Distance = FVector::Dist(ViewerLocation, Component->GetComponentLocation());
            UpdateVFXLOD(Component, Distance);
        }
    }
}

int32 UVFXSystemManager::GetActiveVFXCount() const
{
    int32 ValidCount = 0;
    for (const TWeakObjectPtr<UNiagaraComponent>& WeakComponent : ActiveVFXComponents)
    {
        if (WeakComponent.IsValid())
        {
            ValidCount++;
        }
    }
    return ValidCount;
}

void UVFXSystemManager::PlayCreatureBreathingEffect(AActor* Creature, float IntensityMultiplier)
{
    if (!Creature) return;
    
    FVector Location = Creature->GetActorLocation();
    FRotator Rotation = Creature->GetActorRotation();
    
    UNiagaraComponent* BreathingVFX = SpawnVFX(TEXT("CreatureBreathing"), Location, Rotation, Creature);
    if (BreathingVFX)
    {
        BreathingVFX->SetFloatParameter(TEXT("Intensity"), IntensityMultiplier);
    }
}

void UVFXSystemManager::PlayCreatureFootstepEffect(FVector Location, float CreatureSize)
{
    UNiagaraComponent* FootstepVFX = SpawnVFX(TEXT("CreatureFootstep"), Location);
    if (FootstepVFX)
    {
        FootstepVFX->SetFloatParameter(TEXT("CreatureSize"), CreatureSize);
    }
}

void UVFXSystemManager::PlayDomesticationProgressEffect(AActor* Creature, float TrustLevel)
{
    if (!Creature) return;
    
    FVector Location = Creature->GetActorLocation() + FVector(0, 0, 100); // Above creature
    
    UNiagaraComponent* DomesticationVFX = SpawnVFX(TEXT("DomesticationProgress"), Location, FRotator::ZeroRotator, Creature);
    if (DomesticationVFX)
    {
        DomesticationVFX->SetFloatParameter(TEXT("TrustLevel"), TrustLevel);
        
        // Change color based on trust level
        FLinearColor TrustColor = FLinearColor::LerpUsingHSV(
            FLinearColor::Red,    // Hostile
            FLinearColor::Green,  // Friendly
            TrustLevel
        );
        DomesticationVFX->SetColorParameter(TEXT("TrustColor"), TrustColor);
    }
}

void UVFXSystemManager::PlayWeatherEffect(const FString& WeatherType, float Intensity)
{
    UNiagaraComponent* WeatherVFX = SpawnVFX(WeatherType, FVector::ZeroVector);
    if (WeatherVFX)
    {
        WeatherVFX->SetFloatParameter(TEXT("Intensity"), Intensity);
    }
}

void UVFXSystemManager::PlayVegetationDisturbance(FVector Location, float Radius)
{
    UNiagaraComponent* VegetationVFX = SpawnVFX(TEXT("VegetationDisturbance"), Location);
    if (VegetationVFX)
    {
        VegetationVFX->SetFloatParameter(TEXT("Radius"), Radius);
    }
}

void UVFXSystemManager::PlayImpactEffect(FVector Location, FVector Normal, const FString& SurfaceType)
{
    FRotator ImpactRotation = FRotationMatrix::MakeFromZ(Normal).Rotator();
    
    FString EffectName = FString::Printf(TEXT("Impact_%s"), *SurfaceType);
    UNiagaraComponent* ImpactVFX = SpawnVFX(EffectName, Location, ImpactRotation);
    if (ImpactVFX)
    {
        ImpactVFX->SetVectorParameter(TEXT("ImpactNormal"), Normal);
    }
}

void UVFXSystemManager::PlayBloodEffect(FVector Location, FVector Direction, float Intensity)
{
    FRotator BloodRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
    
    UNiagaraComponent* BloodVFX = SpawnVFX(TEXT("BloodSpray"), Location, BloodRotation);
    if (BloodVFX)
    {
        BloodVFX->SetVectorParameter(TEXT("Direction"), Direction);
        BloodVFX->SetFloatParameter(TEXT("Intensity"), Intensity);
    }
}

void UVFXSystemManager::InitializeVFXLibrary()
{
    // Initialize default VFX library
    // This would typically be loaded from data assets or configuration files
    
    FVFXEffectData CreatureBreathing;
    CreatureBreathing.Category = EVFXCategory::Creature;
    CreatureBreathing.Priority = EVFXPriority::Medium;
    CreatureBreathing.MaxDistance = 2000.0f;
    CreatureBreathing.MaxInstances = 20;
    VFXLibrary.Add(TEXT("CreatureBreathing"), CreatureBreathing);
    
    FVFXEffectData CreatureFootstep;
    CreatureFootstep.Category = EVFXCategory::Creature;
    CreatureFootstep.Priority = EVFXPriority::High;
    CreatureFootstep.MaxDistance = 1500.0f;
    CreatureFootstep.MaxInstances = 30;
    VFXLibrary.Add(TEXT("CreatureFootstep"), CreatureFootstep);
    
    FVFXEffectData BloodSpray;
    BloodSpray.Category = EVFXCategory::Combat;
    BloodSpray.Priority = EVFXPriority::Critical;
    BloodSpray.MaxDistance = 3000.0f;
    BloodSpray.MaxInstances = 15;
    VFXLibrary.Add(TEXT("BloodSpray"), BloodSpray);
    
    FVFXEffectData DomesticationProgress;
    DomesticationProgress.Category = EVFXCategory::Interaction;
    DomesticationProgress.Priority = EVFXPriority::High;
    DomesticationProgress.MaxDistance = 1000.0f;
    DomesticationProgress.MaxInstances = 5;
    VFXLibrary.Add(TEXT("DomesticationProgress"), DomesticationProgress);
    
    UE_LOG(LogTemp, Log, TEXT("VFX Library initialized with %d effects"), VFXLibrary.Num());
}

void UVFXSystemManager::CleanupInactiveVFX()
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; --i)
    {
        if (!ActiveVFXComponents[i].IsValid())
        {
            ActiveVFXComponents.RemoveAt(i);
        }
        else
        {
            UNiagaraComponent* Component = ActiveVFXComponents[i].Get();
            if (!Component || !IsValid(Component) || !Component->IsActive())
            {
                ActiveVFXComponents.RemoveAt(i);
            }
        }
    }
}

void UVFXSystemManager::UpdateAllVFXLOD()
{
    if (!CachedPlayerController || !CachedPlayerController->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
    
    for (TWeakObjectPtr<UNiagaraComponent>& WeakComponent : ActiveVFXComponents)
    {
        if (WeakComponent.IsValid())
        {
            UNiagaraComponent* Component = WeakComponent.Get();
            float Distance = FVector::Dist(PlayerLocation, Component->GetComponentLocation());
            UpdateVFXLOD(Component, Distance);
        }
    }
}

void UVFXSystemManager::UpdateVFXLOD(UNiagaraComponent* VFXComponent, float DistanceToPlayer)
{
    if (!VFXComponent || !IsValid(VFXComponent))
    {
        return;
    }
    
    int32 LODLevel = GetVFXLODLevel(DistanceToPlayer);
    float QualityMultiplier = CalculateQualityMultiplier(DistanceToPlayer, EVFXPriority::Medium);
    
    // Apply LOD settings to the VFX component
    switch (LODLevel)
    {
        case 0: // High LOD
            VFXComponent->SetFloatParameter(TEXT("LODQuality"), 1.0f * QualityMultiplier);
            VFXComponent->SetIntParameter(TEXT("ParticleCount"), 100);
            break;
            
        case 1: // Medium LOD
            VFXComponent->SetFloatParameter(TEXT("LODQuality"), 0.6f * QualityMultiplier);
            VFXComponent->SetIntParameter(TEXT("ParticleCount"), 60);
            break;
            
        case 2: // Low LOD
            VFXComponent->SetFloatParameter(TEXT("LODQuality"), 0.3f * QualityMultiplier);
            VFXComponent->SetIntParameter(TEXT("ParticleCount"), 30);
            break;
            
        default: // Cull
            if (ShouldCullVFX(VFXComponent))
            {
                VFXComponent->SetVisibility(false);
            }
            break;
    }
}

int32 UVFXSystemManager::GetVFXLODLevel(float Distance) const
{
    if (Distance <= HighLODDistance)
    {
        return 0; // High LOD
    }
    else if (Distance <= MediumLODDistance)
    {
        return 1; // Medium LOD
    }
    else if (Distance <= 5000.0f) // Low LOD distance
    {
        return 2; // Low LOD
    }
    else
    {
        return 3; // Cull
    }
}

bool UVFXSystemManager::ShouldCullVFX(UNiagaraComponent* VFXComponent) const
{
    if (!VFXComponent || !CachedPlayerController || !CachedPlayerController->GetPawn())
    {
        return true;
    }
    
    float Distance = FVector::Dist(
        CachedPlayerController->GetPawn()->GetActorLocation(),
        VFXComponent->GetComponentLocation()
    );
    
    // Cull if too far away
    return Distance > 5000.0f;
}

float UVFXSystemManager::CalculateQualityMultiplier(float Distance, EVFXPriority Priority) const
{
    float BaseMultiplier = 1.0f;
    
    // Adjust based on global quality setting
    switch (CurrentVFXQuality)
    {
        case 0: BaseMultiplier = 0.25f; break; // Low
        case 1: BaseMultiplier = 0.5f; break;  // Medium
        case 2: BaseMultiplier = 0.75f; break; // High
        case 3: BaseMultiplier = 1.0f; break;  // Ultra
    }
    
    // Adjust based on priority
    switch (Priority)
    {
        case EVFXPriority::Critical:
            BaseMultiplier *= 1.0f;
            break;
        case EVFXPriority::High:
            BaseMultiplier *= 0.8f;
            break;
        case EVFXPriority::Medium:
            BaseMultiplier *= 0.6f;
            break;
        case EVFXPriority::Low:
            BaseMultiplier *= 0.4f;
            break;
        case EVFXPriority::Background:
            BaseMultiplier *= 0.2f;
            break;
    }
    
    return BaseMultiplier;
}