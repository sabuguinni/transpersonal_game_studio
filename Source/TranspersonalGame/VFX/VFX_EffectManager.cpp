#include "VFX_EffectManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

UVFX_EffectManager::UVFX_EffectManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Tick every 100ms for performance
    
    MaxActiveEffects = 50;
    EffectCullDistance = 5000.0f;
}

void UVFX_EffectManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeEffectDatabase();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Initialized with %d effect types"), EffectDatabase.Num());
}

void UVFX_EffectManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Cleanup expired effects every tick
    CleanupExpiredEffects();
    
    // Cull distant effects for performance
    CullDistantEffects();
}

void UVFX_EffectManager::InitializeEffectDatabase()
{
    // Initialize campfire effect
    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::Fire_Campfire;
    CampfireData.Duration = -1.0f; // Infinite duration
    CampfireData.Scale = 1.0f;
    CampfireData.TintColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
    EffectDatabase.Add(EVFX_EffectType::Fire_Campfire, CampfireData);
    
    // Initialize footstep dust effect
    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::Dust_Footstep;
    FootstepData.Duration = 2.0f;
    FootstepData.Scale = 0.8f;
    FootstepData.TintColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);
    EffectDatabase.Add(EVFX_EffectType::Dust_Footstep, FootstepData);
    
    // Initialize impact dust effect
    FVFX_EffectData ImpactData;
    ImpactData.EffectType = EVFX_EffectType::Dust_Impact;
    ImpactData.Duration = 3.0f;
    ImpactData.Scale = 1.5f;
    ImpactData.TintColor = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
    EffectDatabase.Add(EVFX_EffectType::Dust_Impact, ImpactData);
    
    // Initialize volcanic smoke effect
    FVFX_EffectData VolcanicData;
    VolcanicData.EffectType = EVFX_EffectType::Smoke_Volcanic;
    VolcanicData.Duration = -1.0f; // Infinite duration
    VolcanicData.Scale = 2.0f;
    VolcanicData.TintColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);
    EffectDatabase.Add(EVFX_EffectType::Smoke_Volcanic, VolcanicData);
    
    // Initialize water spray effect
    FVFX_EffectData WaterData;
    WaterData.EffectType = EVFX_EffectType::Water_Spray;
    WaterData.Duration = 1.5f;
    WaterData.Scale = 1.2f;
    WaterData.TintColor = FLinearColor(0.8f, 0.9f, 1.0f, 0.7f);
    EffectDatabase.Add(EVFX_EffectType::Water_Spray, WaterData);
}

UNiagaraComponent* UVFX_EffectManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float ScaleOverride)
{
    if (!EffectDatabase.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Effect type %d not found in database"), (int32)EffectType);
        return nullptr;
    }
    
    // Check active effect limit
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Max active effects reached (%d)"), MaxActiveEffects);
        return nullptr;
    }
    
    const FVFX_EffectData& EffectData = EffectDatabase[EffectType];
    
    // For now, create a basic Niagara component without specific system
    // In production, this would load the actual Niagara system asset
    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        nullptr, // Would be EffectData.NiagaraSystem.LoadSynchronous() in production
        Location,
        Rotation,
        FVector(EffectData.Scale * ScaleOverride),
        true,
        true,
        ENCPoolMethod::None,
        true
    );
    
    if (NewEffect)
    {
        ActiveEffects.Add(NewEffect);
        
        // Set duration if not infinite
        if (EffectData.Duration > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NewEffect]()
            {
                if (IsValid(NewEffect))
                {
                    NewEffect->DestroyComponent();
                    ActiveEffects.Remove(NewEffect);
                }
            }, EffectData.Duration, false);
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Spawned effect %d at %s"), (int32)EffectType, *Location.ToString());
    }
    
    return NewEffect;
}

void UVFX_EffectManager::SpawnFootstepEffect(FVector Location, ESurfaceType SurfaceType)
{
    EVFX_EffectType EffectType = GetFootstepEffectType(SurfaceType);
    SpawnEffect(EffectType, Location, FRotator::ZeroRotator, 1.0f);
}

void UVFX_EffectManager::SpawnCampfireEffect(FVector Location, float IntensityScale)
{
    SpawnEffect(EVFX_EffectType::Fire_Campfire, Location, FRotator::ZeroRotator, IntensityScale);
}

void UVFX_EffectManager::SpawnDinosaurImpactEffect(FVector Location, float DinosaurMass)
{
    float ScaleMultiplier = FMath::Clamp(DinosaurMass / 1000.0f, 0.5f, 3.0f);
    SpawnEffect(EVFX_EffectType::Dust_Impact, Location, FRotator::ZeroRotator, ScaleMultiplier);
}

void UVFX_EffectManager::SpawnWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Coverage)
{
    float ScaleMultiplier = Coverage / 1000.0f;
    SpawnEffect(WeatherType, Location, FRotator::ZeroRotator, ScaleMultiplier);
}

void UVFX_EffectManager::CleanupExpiredEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveEffects[i]) || ActiveEffects[i]->IsBeingDestroyed())
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}

void UVFX_EffectManager::SetEffectQuality(int32 QualityLevel)
{
    // Adjust max active effects based on quality
    switch (QualityLevel)
    {
        case 0: // Low
            MaxActiveEffects = 20;
            EffectCullDistance = 3000.0f;
            break;
        case 1: // Medium
            MaxActiveEffects = 35;
            EffectCullDistance = 4000.0f;
            break;
        case 2: // High
            MaxActiveEffects = 50;
            EffectCullDistance = 5000.0f;
            break;
        case 3: // Ultra
            MaxActiveEffects = 75;
            EffectCullDistance = 7000.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Quality set to %d (MaxEffects: %d, CullDistance: %.0f)"), 
           QualityLevel, MaxActiveEffects, EffectCullDistance);
}

void UVFX_EffectManager::CullDistantEffects()
{
    if (!GetWorld()) return;
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (IsValid(ActiveEffects[i]))
        {
            float Distance = FVector::Dist(ActiveEffects[i]->GetComponentLocation(), PlayerLocation);
            if (Distance > EffectCullDistance)
            {
                ActiveEffects[i]->DestroyComponent();
                ActiveEffects.RemoveAt(i);
            }
        }
    }
}

EVFX_EffectType UVFX_EffectManager::GetFootstepEffectType(ESurfaceType SurfaceType)
{
    switch (SurfaceType)
    {
        case ESurfaceType::Dirt:
        case ESurfaceType::Grass:
        case ESurfaceType::Sand:
            return EVFX_EffectType::Dust_Footstep;
        case ESurfaceType::Water:
            return EVFX_EffectType::Water_Spray;
        default:
            return EVFX_EffectType::Dust_Footstep;
    }
}