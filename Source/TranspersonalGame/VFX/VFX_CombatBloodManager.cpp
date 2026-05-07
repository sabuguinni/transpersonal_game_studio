#include "VFX_CombatBloodManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

UVFX_CombatBloodManager::UVFX_CombatBloodManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check every 100ms
    
    // Default blood intensity
    CurrentBloodIntensity = 0.7f;
    MaxBloodEffectDistance = 1000.0f;
    MaxActiveBloodEffects = 10;
    LastBloodEffectTime = 0.0f;
    
    // Initialize soft object pointers (will be set via Blueprint or content)
    BloodSplatterSystem = nullptr;
    BloodDripSystem = nullptr;
    BloodPoolSystem = nullptr;
}

void UVFX_CombatBloodManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize blood effect arrays
    ActiveBloodEffects.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_CombatBloodManager: Blood system initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UVFX_CombatBloodManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Cleanup old blood effects periodically
    CleanupOldBloodEffects();
}

void UVFX_CombatBloodManager::TriggerBloodSplatter(const FVector& ImpactLocation, const FVector& ImpactNormal, float Damage)
{
    if (!ShouldSpawnBloodEffect(ImpactLocation))
    {
        return;
    }
    
    // Calculate blood intensity based on damage
    float BloodIntensity = CalculateBloodIntensityFromDamage(Damage);
    
    // Load and spawn blood splatter effect
    if (BloodSplatterSystem.IsValid())
    {
        UNiagaraSystem* LoadedSystem = BloodSplatterSystem.LoadSynchronous();
        if (LoadedSystem)
        {
            SpawnBloodEffect(LoadedSystem, ImpactLocation, ImpactNormal, BloodIntensity);
        }
    }
    else
    {
        // Fallback: Create basic blood effect without Niagara system
        UE_LOG(LogTemp, Warning, TEXT("VFX_CombatBloodManager: No blood splatter system assigned, using fallback"));
        
        // Spawn a simple particle effect or decal here
        // For now, just log the blood effect
        UE_LOG(LogTemp, Log, TEXT("Blood splatter at location: %s, intensity: %f"), 
               *ImpactLocation.ToString(), BloodIntensity);
    }
    
    // Broadcast event
    OnBloodEffectTriggered.Broadcast(ImpactLocation, BloodIntensity);
    LastBloodEffectTime = GetWorld()->GetTimeSeconds();
}

void UVFX_CombatBloodManager::TriggerBloodDrip(const FVector& StartLocation, float Duration)
{
    if (!ShouldSpawnBloodEffect(StartLocation))
    {
        return;
    }
    
    if (BloodDripSystem.IsValid())
    {
        UNiagaraSystem* LoadedSystem = BloodDripSystem.LoadSynchronous();
        if (LoadedSystem)
        {
            // Calculate drip end location (gravity effect)
            FVector EndLocation = StartLocation + FVector(0, 0, -200.0f);
            
            UNiagaraComponent* DripEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                LoadedSystem,
                StartLocation,
                FRotator::ZeroRotator,
                FVector(1.0f),
                true,
                true,
                ENCPoolMethod::None,
                true
            );
            
            if (DripEffect)
            {
                ActiveBloodEffects.Add(DripEffect);
                
                // Set drip parameters
                DripEffect->SetFloatParameter(TEXT("LifeTime"), Duration);
                DripEffect->SetVectorParameter(TEXT("EndLocation"), EndLocation);
                
                UE_LOG(LogTemp, Log, TEXT("Blood drip effect spawned at %s"), *StartLocation.ToString());
            }
        }
    }
}

void UVFX_CombatBloodManager::CreateBloodPool(const FVector& GroundLocation, float PoolSize)
{
    if (!ShouldSpawnBloodEffect(GroundLocation))
    {
        return;
    }
    
    if (BloodPoolSystem.IsValid())
    {
        UNiagaraSystem* LoadedSystem = BloodPoolSystem.LoadSynchronous();
        if (LoadedSystem)
        {
            UNiagaraComponent* PoolEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                LoadedSystem,
                GroundLocation,
                FRotator::ZeroRotator,
                FVector(PoolSize / 100.0f), // Scale based on pool size
                true,
                true,
                ENCPoolMethod::None,
                true
            );
            
            if (PoolEffect)
            {
                ActiveBloodEffects.Add(PoolEffect);
                
                // Set pool parameters
                PoolEffect->SetFloatParameter(TEXT("PoolSize"), PoolSize);
                PoolEffect->SetFloatParameter(TEXT("Intensity"), CurrentBloodIntensity);
                
                UE_LOG(LogTemp, Log, TEXT("Blood pool created at %s, size: %f"), 
                       *GroundLocation.ToString(), PoolSize);
            }
        }
    }
}

void UVFX_CombatBloodManager::SetBloodIntensity(float NewIntensity)
{
    CurrentBloodIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    
    // Update all active blood effects with new intensity
    for (UNiagaraComponent* BloodEffect : ActiveBloodEffects)
    {
        if (IsValid(BloodEffect))
        {
            BloodEffect->SetFloatParameter(TEXT("Intensity"), CurrentBloodIntensity);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Blood intensity set to: %f"), CurrentBloodIntensity);
}

void UVFX_CombatBloodManager::CleanupBloodEffects()
{
    for (UNiagaraComponent* BloodEffect : ActiveBloodEffects)
    {
        if (IsValid(BloodEffect))
        {
            BloodEffect->DestroyComponent();
        }
    }
    
    ActiveBloodEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("All blood effects cleaned up"));
}

void UVFX_CombatBloodManager::SpawnBloodEffect(UNiagaraSystem* System, const FVector& Location, const FVector& Normal, float Scale)
{
    if (!System || !GetWorld())
    {
        return;
    }
    
    // Calculate rotation from normal
    FRotator Rotation = FRotationMatrix::MakeFromZ(Normal).Rotator();
    
    UNiagaraComponent* BloodEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        Rotation,
        FVector(Scale),
        true,
        true,
        ENCPoolMethod::None,
        true
    );
    
    if (BloodEffect)
    {
        ActiveBloodEffects.Add(BloodEffect);
        
        // Set effect parameters
        BloodEffect->SetFloatParameter(TEXT("Intensity"), CurrentBloodIntensity);
        BloodEffect->SetVectorParameter(TEXT("ImpactNormal"), Normal);
        
        UE_LOG(LogTemp, Log, TEXT("Blood effect spawned at %s with scale %f"), 
               *Location.ToString(), Scale);
    }
}

void UVFX_CombatBloodManager::CleanupOldBloodEffects()
{
    // Remove null or inactive effects
    ActiveBloodEffects.RemoveAll([](UNiagaraComponent* Effect) {
        return !IsValid(Effect) || !Effect->IsActive();
    });
    
    // Limit number of active effects
    while (ActiveBloodEffects.Num() > MaxActiveBloodEffects)
    {
        UNiagaraComponent* OldestEffect = ActiveBloodEffects[0];
        if (IsValid(OldestEffect))
        {
            OldestEffect->DestroyComponent();
        }
        ActiveBloodEffects.RemoveAt(0);
    }
}

bool UVFX_CombatBloodManager::ShouldSpawnBloodEffect(const FVector& Location) const
{
    if (!GetOwner() || !GetWorld())
    {
        return false;
    }
    
    // Check distance from owner
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Location);
    if (Distance > MaxBloodEffectDistance)
    {
        return false;
    }
    
    // Check if too many effects are already active
    if (ActiveBloodEffects.Num() >= MaxActiveBloodEffects)
    {
        return false;
    }
    
    // Rate limiting: don't spawn effects too frequently
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastBloodEffectTime < 0.1f) // 100ms minimum between effects
    {
        return false;
    }
    
    return true;
}

float UVFX_CombatBloodManager::CalculateBloodIntensityFromDamage(float Damage) const
{
    // Scale blood intensity based on damage (0-100 damage -> 0.1-1.0 intensity)
    float NormalizedDamage = FMath::Clamp(Damage / 100.0f, 0.0f, 1.0f);
    return FMath::Lerp(0.1f, CurrentBloodIntensity, NormalizedDamage);
}