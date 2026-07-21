#include "VFX_CombatEffects.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UVFX_CombatEffects::UVFX_CombatEffects()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default combat settings
    CombatSettings.EffectType = EVFX_CombatType::BloodSplatter;
    CombatSettings.Intensity = 1.0f;
    CombatSettings.Scale = FVector(1.0f, 1.0f, 1.0f);
    CombatSettings.Duration = 2.0f;
    CombatSettings.TintColor = FLinearColor::Red;
}

void UVFX_CombatEffects::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize active effects array
    ActiveEffects.Empty();
}

void UVFX_CombatEffects::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up finished effects periodically
    CleanupFinishedEffects();
}

void UVFX_CombatEffects::TriggerBloodSplatter(const FVector& Location, const FVector& Normal, float Intensity)
{
    if (!BloodSplatterSystem.IsNull())
    {
        UNiagaraSystem* LoadedSystem = BloodSplatterSystem.LoadSynchronous();
        if (LoadedSystem)
        {
            // Calculate rotation from normal
            FRotator Rotation = FRotationMatrix::MakeFromZ(Normal).Rotator();
            
            // Spawn blood splatter effect
            UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                LoadedSystem,
                Location,
                Rotation,
                CombatSettings.Scale * Intensity,
                true,
                true,
                ENCPoolMethod::None,
                true
            );
            
            if (Effect)
            {
                // Set blood color and intensity
                Effect->SetNiagaraVariableLinearColor(FString("BloodColor"), CombatSettings.TintColor);
                Effect->SetNiagaraVariableFloat(FString("Intensity"), Intensity * CombatSettings.Intensity);
                
                ActiveEffects.Add(Effect);
            }
        }
    }
}

void UVFX_CombatEffects::TriggerImpactSpark(const FVector& Location, const FVector& Direction, float Intensity)
{
    if (!ImpactSparkSystem.IsNull())
    {
        UNiagaraSystem* LoadedSystem = ImpactSparkSystem.LoadSynchronous();
        if (LoadedSystem)
        {
            // Calculate rotation from direction
            FRotator Rotation = Direction.Rotation();
            
            // Spawn impact spark effect
            UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                LoadedSystem,
                Location,
                Rotation,
                CombatSettings.Scale * Intensity,
                true,
                true,
                ENCPoolMethod::None,
                true
            );
            
            if (Effect)
            {
                // Set spark parameters
                Effect->SetNiagaraVariableFloat(FString("SparkIntensity"), Intensity * CombatSettings.Intensity);
                Effect->SetNiagaraVariableVector(FString("Direction"), Direction);
                
                ActiveEffects.Add(Effect);
            }
        }
    }
}

void UVFX_CombatEffects::TriggerDustCloud(const FVector& Location, float Radius)
{
    if (!DustCloudSystem.IsNull())
    {
        UNiagaraSystem* LoadedSystem = DustCloudSystem.LoadSynchronous();
        if (LoadedSystem)
        {
            // Spawn dust cloud effect
            UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                LoadedSystem,
                Location,
                FRotator::ZeroRotator,
                CombatSettings.Scale,
                true,
                true,
                ENCPoolMethod::None,
                true
            );
            
            if (Effect)
            {
                // Set dust cloud parameters
                Effect->SetNiagaraVariableFloat(FString("CloudRadius"), Radius);
                Effect->SetNiagaraVariableFloat(FString("Intensity"), CombatSettings.Intensity);
                
                ActiveEffects.Add(Effect);
            }
        }
    }
}

void UVFX_CombatEffects::TriggerWeaponTrail(const FVector& StartLocation, const FVector& EndLocation)
{
    if (!WeaponTrailSystem.IsNull())
    {
        UNiagaraSystem* LoadedSystem = WeaponTrailSystem.LoadSynchronous();
        if (LoadedSystem)
        {
            // Calculate trail direction and rotation
            FVector TrailDirection = (EndLocation - StartLocation).GetSafeNormal();
            FRotator TrailRotation = TrailDirection.Rotation();
            
            // Spawn weapon trail effect
            UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                LoadedSystem,
                StartLocation,
                TrailRotation,
                CombatSettings.Scale,
                true,
                true,
                ENCPoolMethod::None,
                true
            );
            
            if (Effect)
            {
                // Set trail parameters
                Effect->SetNiagaraVariableVector(FString("StartLocation"), StartLocation);
                Effect->SetNiagaraVariableVector(FString("EndLocation"), EndLocation);
                Effect->SetNiagaraVariableFloat(FString("TrailLength"), FVector::Dist(StartLocation, EndLocation));
                
                ActiveEffects.Add(Effect);
            }
        }
    }
}

void UVFX_CombatEffects::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
}

void UVFX_CombatEffects::SetEffectIntensity(float NewIntensity)
{
    CombatSettings.Intensity = FMath::Clamp(NewIntensity, 0.1f, 5.0f);
    
    // Update active effects
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->SetNiagaraVariableFloat(FString("Intensity"), CombatSettings.Intensity);
        }
    }
}

void UVFX_CombatEffects::SetEffectScale(const FVector& NewScale)
{
    CombatSettings.Scale = NewScale;
    
    // Update active effects
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->SetWorldScale3D(NewScale);
        }
    }
}

void UVFX_CombatEffects::SpawnNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation)
{
    if (System && GetWorld())
    {
        UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            System,
            Location,
            Rotation,
            CombatSettings.Scale,
            true,
            true,
            ENCPoolMethod::None,
            true
        );
        
        if (Effect)
        {
            ActiveEffects.Add(Effect);
        }
    }
}

void UVFX_CombatEffects::CleanupFinishedEffects()
{
    // Remove null or destroyed effects from the array
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !Effect || !IsValid(Effect) || !Effect->IsActive();
    });
}