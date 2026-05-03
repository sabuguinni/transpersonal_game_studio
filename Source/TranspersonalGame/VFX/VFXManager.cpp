#include "VFXManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

UVFX_Manager::UVFX_Manager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check every 100ms
    
    MaxActiveVFXEffects = 50;
    VFXCullDistance = 10000.0f;
    bEnableVFXLOD = true;
}

void UVFX_Manager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVFXSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Manager initialized with %d VFX systems"), VFXSystems.Num());
}

void UVFX_Manager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up finished VFX effects
    CleanupFinishedVFX();
    
    // Cull distant VFX if LOD is enabled
    if (bEnableVFXLOD)
    {
        for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
        {
            if (ActiveVFXComponents[i] && ShouldCullVFXAtLocation(ActiveVFXComponents[i]->GetComponentLocation()))
            {
                ActiveVFXComponents[i]->DestroyComponent();
                ActiveVFXComponents.RemoveAt(i);
            }
        }
    }
}

void UVFX_Manager::InitializeVFXSystems()
{
    // Initialize default VFX systems - these would normally be set in Blueprint or config
    // For now, we'll register the types without specific Niagara assets
    
    VFXSystems.Add(EVFX_EffectType::DinosaurFootstep, nullptr);
    VFXSystems.Add(EVFX_EffectType::DinosaurBreath, nullptr);
    VFXSystems.Add(EVFX_EffectType::DinosaurImpact, nullptr);
    VFXSystems.Add(EVFX_EffectType::CampfireFire, nullptr);
    VFXSystems.Add(EVFX_EffectType::CampfireSmoke, nullptr);
    VFXSystems.Add(EVFX_EffectType::WeatherRain, nullptr);
    VFXSystems.Add(EVFX_EffectType::WeatherSnow, nullptr);
    VFXSystems.Add(EVFX_EffectType::WeatherFog, nullptr);
    VFXSystems.Add(EVFX_EffectType::CombatBlood, nullptr);
    VFXSystems.Add(EVFX_EffectType::CombatImpact, nullptr);
    VFXSystems.Add(EVFX_EffectType::EnvironmentDust, nullptr);
    VFXSystems.Add(EVFX_EffectType::EnvironmentWater, nullptr);
    VFXSystems.Add(EVFX_EffectType::CraftingSparks, nullptr);
}

UNiagaraComponent* UVFX_Manager::SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, FVector Scale)
{
    if (EffectType == EVFX_EffectType::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Cannot spawn VFX with type None"));
        return nullptr;
    }
    
    // Check if we've reached the maximum number of active VFX
    if (ActiveVFXComponents.Num() >= MaxActiveVFXEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Maximum VFX effects reached (%d), skipping spawn"), MaxActiveVFXEffects);
        return nullptr;
    }
    
    // Check distance culling
    if (ShouldCullVFXAtLocation(Location))
    {
        return nullptr;
    }
    
    UNiagaraSystem* VFXSystem = GetVFXSystemForType(EffectType);
    if (!VFXSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: No Niagara system found for effect type %d"), (int32)EffectType);
        return nullptr;
    }
    
    // Spawn the Niagara component
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        VFXSystem,
        Location,
        Rotation,
        Scale,
        true, // Auto destroy
        true, // Auto activate
        ENCPoolMethod::None
    );
    
    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        UE_LOG(LogTemp, Log, TEXT("VFX Manager: Spawned %s effect at %s"), 
               *UEnum::GetValueAsString(EffectType), *Location.ToString());
    }
    
    return VFXComponent;
}

void UVFX_Manager::SpawnDinosaurFootstepEffect(FVector Location, float DinosaurSize)
{
    // Adjust effect scale based on dinosaur size
    FVector EffectScale = FVector(DinosaurSize, DinosaurSize, DinosaurSize);
    
    // Spawn dust cloud effect
    SpawnVFXEffect(EVFX_EffectType::DinosaurFootstep, Location, FRotator::ZeroRotator, EffectScale);
    
    // Also spawn environment dust for larger dinosaurs
    if (DinosaurSize > 2.0f)
    {
        SpawnVFXEffect(EVFX_EffectType::EnvironmentDust, Location + FVector(0, 0, 50), FRotator::ZeroRotator, EffectScale * 0.5f);
    }
}

void UVFX_Manager::SpawnDinosaurBreathEffect(FVector Location, FRotator Direction)
{
    SpawnVFXEffect(EVFX_EffectType::DinosaurBreath, Location, Direction, FVector::OneVector);
}

void UVFX_Manager::SpawnCampfireEffect(FVector Location)
{
    // Spawn fire effect
    SpawnVFXEffect(EVFX_EffectType::CampfireFire, Location, FRotator::ZeroRotator, FVector::OneVector);
    
    // Spawn smoke effect slightly above
    SpawnVFXEffect(EVFX_EffectType::CampfireSmoke, Location + FVector(0, 0, 100), FRotator::ZeroRotator, FVector::OneVector);
}

void UVFX_Manager::SpawnCombatImpactEffect(FVector Location, FVector ImpactDirection)
{
    // Calculate rotation from impact direction
    FRotator ImpactRotation = ImpactDirection.Rotation();
    
    SpawnVFXEffect(EVFX_EffectType::CombatImpact, Location, ImpactRotation, FVector::OneVector);
    
    // Add blood effect for organic impacts
    SpawnVFXEffect(EVFX_EffectType::CombatBlood, Location, ImpactRotation, FVector(0.5f, 0.5f, 0.5f));
}

void UVFX_Manager::SpawnWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Intensity)
{
    if (WeatherType != EVFX_EffectType::WeatherRain && 
        WeatherType != EVFX_EffectType::WeatherSnow && 
        WeatherType != EVFX_EffectType::WeatherFog)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Invalid weather effect type"));
        return;
    }
    
    FVector WeatherScale = FVector(Intensity, Intensity, Intensity);
    SpawnVFXEffect(WeatherType, Location, FRotator::ZeroRotator, WeatherScale);
}

void UVFX_Manager::StopAllVFXEffects()
{
    for (UNiagaraComponent* VFXComponent : ActiveVFXComponents)
    {
        if (VFXComponent && IsValid(VFXComponent))
        {
            VFXComponent->DestroyComponent();
        }
    }
    
    ActiveVFXComponents.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Stopped all VFX effects"));
}

void UVFX_Manager::StopVFXEffectsByType(EVFX_EffectType EffectType)
{
    // Note: This is a simplified implementation
    // In a full system, we'd track VFX by type
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        if (ActiveVFXComponents[i] && IsValid(ActiveVFXComponents[i]))
        {
            // For now, just stop all since we don't track by type
            // This would be improved with a more sophisticated tracking system
            ActiveVFXComponents[i]->DestroyComponent();
            ActiveVFXComponents.RemoveAt(i);
        }
    }
}

int32 UVFX_Manager::GetActiveVFXCount() const
{
    return ActiveVFXComponents.Num();
}

void UVFX_Manager::CleanupFinishedVFX()
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* VFXComponent = ActiveVFXComponents[i];
        
        if (!VFXComponent || !IsValid(VFXComponent) || !VFXComponent->IsActive())
        {
            ActiveVFXComponents.RemoveAt(i);
        }
    }
}

UNiagaraSystem* UVFX_Manager::GetVFXSystemForType(EVFX_EffectType EffectType)
{
    if (VFXSystems.Contains(EffectType))
    {
        TSoftObjectPtr<UNiagaraSystem> SystemPtr = VFXSystems[EffectType];
        if (SystemPtr.IsValid())
        {
            return SystemPtr.Get();
        }
        else if (!SystemPtr.IsNull())
        {
            // Try to load the asset
            return SystemPtr.LoadSynchronous();
        }
    }
    
    return nullptr;
}

bool UVFX_Manager::ShouldCullVFXAtLocation(FVector Location)
{
    if (!bEnableVFXLOD)
    {
        return false;
    }
    
    // Get player location for distance calculation
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return false;
    }
    
    float DistanceToPlayer = FVector::Dist(Location, PlayerPawn->GetActorLocation());
    return DistanceToPlayer > VFXCullDistance;
}