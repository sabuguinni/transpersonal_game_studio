#include "VFX_FootstepEffectManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"

UVFX_FootstepEffectManager::UVFX_FootstepEffectManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick rate for performance

    // Initialize default effect data
    FVFX_FootstepEffectData DirtEffect;
    DirtEffect.EffectScale = 1.0f;
    DirtEffect.DustIntensity = 1.0f;
    DirtEffect.DustColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f); // Brown earth
    SurfaceEffects.Add(EVFX_FootstepSurfaceType::Dirt, DirtEffect);

    FVFX_FootstepEffectData RockEffect;
    RockEffect.EffectScale = 0.8f;
    RockEffect.DustIntensity = 0.6f;
    RockEffect.DustColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f); // Gray stone
    SurfaceEffects.Add(EVFX_FootstepSurfaceType::Rock, RockEffect);

    FVFX_FootstepEffectData GrassEffect;
    GrassEffect.EffectScale = 0.5f;
    GrassEffect.DustIntensity = 0.3f;
    GrassEffect.DustColor = FLinearColor(0.4f, 0.7f, 0.3f, 1.0f); // Green vegetation
    SurfaceEffects.Add(EVFX_FootstepSurfaceType::Grass, GrassEffect);

    FVFX_FootstepEffectData WaterEffect;
    WaterEffect.EffectScale = 1.2f;
    WaterEffect.DustIntensity = 0.8f;
    WaterEffect.DustColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f); // Muddy brown
    SurfaceEffects.Add(EVFX_FootstepSurfaceType::Water, WaterEffect);

    FVFX_FootstepEffectData SandEffect;
    SandEffect.EffectScale = 1.1f;
    SandEffect.DustIntensity = 1.2f;
    SandEffect.DustColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f); // Sandy yellow
    SurfaceEffects.Add(EVFX_FootstepSurfaceType::Sand, SandEffect);

    // Initialize creature size multipliers
    CreatureSizeMultipliers.Add(EVFX_CreatureSize::Small, 0.3f);
    CreatureSizeMultipliers.Add(EVFX_CreatureSize::Medium, 1.0f);
    CreatureSizeMultipliers.Add(EVFX_CreatureSize::Large, 2.5f);
    CreatureSizeMultipliers.Add(EVFX_CreatureSize::Massive, 4.0f);

    // Set default asset paths
    DefaultDustSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Particles/NS_Dino_FootstepDust")));
    DefaultDebrisSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Particles/NS_Dino_DebrisImpact")));
    BloodImpactSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Particles/NS_Combat_BloodImpact")));

    // Performance defaults
    MaxEffectDistance = 5000.0f;
    EffectCooldown = 0.1f;
    MaxConcurrentEffects = 20;
    bSyncWithAudio = true;
}

void UVFX_FootstepEffectManager::BeginPlay()
{
    Super::BeginPlay();
    
    LastEffectTime = 0.0f;
    ActiveEffectCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("VFX_FootstepEffectManager initialized for actor: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UVFX_FootstepEffectManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update effect cooldown timer
    if (LastEffectTime > 0.0f)
    {
        LastEffectTime -= DeltaTime;
        if (LastEffectTime <= 0.0f)
        {
            LastEffectTime = 0.0f;
        }
    }
    
    // Decay active effect count (effects naturally expire)
    if (ActiveEffectCount > 0)
    {
        // Assume effects last ~2 seconds on average
        float DecayRate = DeltaTime * 0.5f;
        ActiveEffectCount = FMath::Max(0, ActiveEffectCount - FMath::RoundToInt(DecayRate));
    }
}

void UVFX_FootstepEffectManager::TriggerFootstepEffect(const FVector& Location, EVFX_FootstepSurfaceType SurfaceType, EVFX_CreatureSize CreatureSize)
{
    if (!CanSpawnEffect())
    {
        return;
    }

    // Get effect data for surface type
    FVFX_FootstepEffectData EffectData = GetEffectDataForSurface(SurfaceType);
    
    // Apply creature size multiplier
    float SizeMultiplier = GetCreatureSizeMultiplier(CreatureSize);
    EffectData.EffectScale *= SizeMultiplier;
    EffectData.DustIntensity *= SizeMultiplier;

    // Spawn dust effect
    if (DefaultDustSystem.IsValid())
    {
        UNiagaraSystem* DustSystem = DefaultDustSystem.LoadSynchronous();
        if (DustSystem)
        {
            SpawnNiagaraEffect(DustSystem, Location, EffectData);
        }
    }

    // Spawn debris for larger creatures
    if (CreatureSize == EVFX_CreatureSize::Large || CreatureSize == EVFX_CreatureSize::Massive)
    {
        if (DefaultDebrisSystem.IsValid())
        {
            UNiagaraSystem* DebrisSystem = DefaultDebrisSystem.LoadSynchronous();
            if (DebrisSystem)
            {
                SpawnNiagaraEffect(DebrisSystem, Location, EffectData);
            }
        }
    }

    // Update cooldown and effect count
    LastEffectTime = EffectCooldown;
    ActiveEffectCount++;

    UE_LOG(LogTemp, Log, TEXT("Footstep effect triggered at %s for %s creature on %s surface"), 
           *Location.ToString(), 
           *UEnum::GetValueAsString(CreatureSize),
           *UEnum::GetValueAsString(SurfaceType));
}

void UVFX_FootstepEffectManager::TriggerDinosaurFootstep(const FVector& Location, const FString& DinosaurType, float SizeMultiplier)
{
    // Map dinosaur types to creature sizes
    EVFX_CreatureSize CreatureSize = EVFX_CreatureSize::Medium;
    
    if (DinosaurType.Contains(TEXT("TRex")) || DinosaurType.Contains(TEXT("Tyrannosaurus")))
    {
        CreatureSize = EVFX_CreatureSize::Large;
    }
    else if (DinosaurType.Contains(TEXT("Brachio")) || DinosaurType.Contains(TEXT("Sauropod")))
    {
        CreatureSize = EVFX_CreatureSize::Massive;
    }
    else if (DinosaurType.Contains(TEXT("Compy")) || DinosaurType.Contains(TEXT("Small")))
    {
        CreatureSize = EVFX_CreatureSize::Small;
    }
    
    // Detect surface type from location
    EVFX_FootstepSurfaceType SurfaceType = DetectSurfaceType(Location);
    
    TriggerFootstepEffect(Location, SurfaceType, CreatureSize);
}

void UVFX_FootstepEffectManager::TriggerPlayerFootstep(const FVector& Location, EVFX_FootstepSurfaceType SurfaceType)
{
    // Player is always small size
    TriggerFootstepEffect(Location, SurfaceType, EVFX_CreatureSize::Small);
}

void UVFX_FootstepEffectManager::SetEffectIntensity(float NewIntensity)
{
    NewIntensity = FMath::Clamp(NewIntensity, 0.1f, 5.0f);
    
    for (auto& SurfaceEffect : SurfaceEffects)
    {
        SurfaceEffect.Value.DustIntensity = NewIntensity;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX effect intensity set to: %f"), NewIntensity);
}

void UVFX_FootstepEffectManager::SetSurfaceEffectData(EVFX_FootstepSurfaceType SurfaceType, const FVFX_FootstepEffectData& EffectData)
{
    SurfaceEffects.Add(SurfaceType, EffectData);
    UE_LOG(LogTemp, Log, TEXT("Surface effect data updated for: %s"), *UEnum::GetValueAsString(SurfaceType));
}

void UVFX_FootstepEffectManager::SyncWithScreenShake(bool bEnableSync)
{
    bSyncWithAudio = bEnableSync;
    UE_LOG(LogTemp, Log, TEXT("VFX-Audio sync %s"), bEnableSync ? TEXT("enabled") : TEXT("disabled"));
}

FVFX_FootstepEffectData UVFX_FootstepEffectManager::GetEffectDataForSurface(EVFX_FootstepSurfaceType SurfaceType) const
{
    if (const FVFX_FootstepEffectData* FoundData = SurfaceEffects.Find(SurfaceType))
    {
        return *FoundData;
    }
    
    // Return default dirt effect if surface type not found
    if (const FVFX_FootstepEffectData* DefaultData = SurfaceEffects.Find(EVFX_FootstepSurfaceType::Dirt))
    {
        return *DefaultData;
    }
    
    return FVFX_FootstepEffectData();
}

float UVFX_FootstepEffectManager::GetCreatureSizeMultiplier(EVFX_CreatureSize CreatureSize) const
{
    if (const float* Multiplier = CreatureSizeMultipliers.Find(CreatureSize))
    {
        return *Multiplier;
    }
    
    return 1.0f; // Default multiplier
}

EVFX_FootstepSurfaceType UVFX_FootstepEffectManager::DetectSurfaceType(const FVector& Location) const
{
    // Simple surface detection based on location and world context
    // In a full implementation, this would use line traces to detect material types
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return EVFX_FootstepSurfaceType::Dirt;
    }
    
    // For now, use height-based detection as placeholder
    float Height = Location.Z;
    
    if (Height < 100.0f)
    {
        return EVFX_FootstepSurfaceType::Water; // Low areas = water/mud
    }
    else if (Height > 800.0f)
    {
        return EVFX_FootstepSurfaceType::Rock; // High areas = rock
    }
    else if (Height > 400.0f)
    {
        return EVFX_FootstepSurfaceType::Grass; // Mid-high = grass
    }
    else
    {
        return EVFX_FootstepSurfaceType::Dirt; // Default = dirt
    }
}

void UVFX_FootstepEffectManager::SpawnNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FVFX_FootstepEffectData& EffectData)
{
    if (!System || !GetWorld())
    {
        return;
    }
    
    // Check distance from player for performance
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController && PlayerController->GetPawn())
    {
        float Distance = FVector::Dist(PlayerController->GetPawn()->GetActorLocation(), Location);
        if (Distance > MaxEffectDistance)
        {
            return; // Too far from player, skip effect
        }
    }
    
    // Spawn Niagara effect
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        FRotator::ZeroRotator,
        FVector(EffectData.EffectScale),
        true,
        true,
        ENCPoolMethod::AutoRelease
    );
    
    if (NiagaraComp)
    {
        // Set effect parameters
        NiagaraComp->SetFloatParameter(TEXT("DustIntensity"), EffectData.DustIntensity);
        NiagaraComp->SetColorParameter(TEXT("DustColor"), EffectData.DustColor);
        NiagaraComp->SetFloatParameter(TEXT("EffectScale"), EffectData.EffectScale);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Spawned Niagara effect: %s at %s"), 
               *System->GetName(), *Location.ToString());
    }
}

bool UVFX_FootstepEffectManager::CanSpawnEffect() const
{
    // Check cooldown
    if (LastEffectTime > 0.0f)
    {
        return false;
    }
    
    // Check max concurrent effects
    if (ActiveEffectCount >= MaxConcurrentEffects)
    {
        return false;
    }
    
    return true;
}