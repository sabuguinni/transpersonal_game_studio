#include "VFX_NiagaraManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/PointLightComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"

AVFX_NiagaraManager::AVFX_NiagaraManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize VFX settings
    EffectUpdateInterval = 0.1f;
    MaxActiveEffects = 50;
    
    // LOD distances for performance optimization
    LODDistance1 = 2000.0f;  // High quality
    LODDistance2 = 5000.0f;  // Medium quality
    LODDistance3 = 10000.0f; // Low quality

    LastUpdateTime = 0.0f;
}

void AVFX_NiagaraManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Niagara Manager started - prehistoric effects system active"));
}

void AVFX_NiagaraManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= EffectUpdateInterval)
    {
        UpdateLODLevels();
        CleanupExpiredEffects();
        LastUpdateTime = 0.0f;
    }
}

void AVFX_NiagaraManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float Intensity, float Duration)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX: Max effects reached, skipping new effect"));
        return;
    }

    FVFX_EffectData NewEffect;
    NewEffect.EffectType = EffectType;
    NewEffect.Location = Location;
    NewEffect.Rotation = Rotation;
    NewEffect.Intensity = Intensity;
    NewEffect.Duration = Duration;

    ActiveEffects.Add(NewEffect);

    // Create the appropriate effect based on type
    switch (EffectType)
    {
        case EVFX_EffectType::Fire_Campfire:
            CreateFireEffect(NewEffect);
            break;
        case EVFX_EffectType::Dust_Footstep:
            CreateDustEffect(NewEffect);
            break;
        case EVFX_EffectType::Blood_Impact:
            CreateBloodEffect(NewEffect);
            break;
        case EVFX_EffectType::Water_Splash:
            CreateWaterEffect(NewEffect);
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("VFX: Unhandled effect type"));
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("VFX: Spawned effect at location %s"), *Location.ToString());
}

void AVFX_NiagaraManager::StopEffect(EVFX_EffectType EffectType, FVector Location)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        const FVFX_EffectData& Effect = ActiveEffects[i];
        if (Effect.EffectType == EffectType && FVector::Dist(Effect.Location, Location) < 100.0f)
        {
            ActiveEffects.RemoveAt(i);
            break;
        }
    }
}

void AVFX_NiagaraManager::StopAllEffects()
{
    ActiveEffects.Empty();
    
    // Stop all particle components
    for (UParticleSystemComponent* ParticleComp : ParticleComponents)
    {
        if (ParticleComp && IsValid(ParticleComp))
        {
            ParticleComp->DestroyComponent();
        }
    }
    ParticleComponents.Empty();

    // Stop all light components
    for (UPointLightComponent* LightComp : LightComponents)
    {
        if (LightComp && IsValid(LightComp))
        {
            LightComp->DestroyComponent();
        }
    }
    LightComponents.Empty();

    UE_LOG(LogTemp, Log, TEXT("VFX: All effects stopped"));
}

void AVFX_NiagaraManager::UpdateLODLevels()
{
    float DistanceToPlayer = GetDistanceToPlayer();
    
    // Adjust effect quality based on distance
    for (UParticleSystemComponent* ParticleComp : ParticleComponents)
    {
        if (ParticleComp && IsValid(ParticleComp))
        {
            if (DistanceToPlayer > LODDistance3)
            {
                // Very far - disable
                ParticleComp->SetVisibility(false);
            }
            else if (DistanceToPlayer > LODDistance2)
            {
                // Far - low quality
                ParticleComp->SetVisibility(true);
                // Reduce particle count (would need Niagara-specific calls)
            }
            else if (DistanceToPlayer > LODDistance1)
            {
                // Medium distance - medium quality
                ParticleComp->SetVisibility(true);
            }
            else
            {
                // Close - full quality
                ParticleComp->SetVisibility(true);
            }
        }
    }
}

float AVFX_NiagaraManager::GetDistanceToPlayer() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                return FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
            }
        }
    }
    return 10000.0f; // Default far distance
}

void AVFX_NiagaraManager::CleanupExpiredEffects()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        // Simple duration-based cleanup (would need proper timing implementation)
        if (ActiveEffects.Num() > MaxActiveEffects * 0.8f)
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}

void AVFX_NiagaraManager::CreateFireEffect(const FVFX_EffectData& EffectData)
{
    // Create point light for fire glow
    UPointLightComponent* FireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FireLight"));
    if (FireLight)
    {
        FireLight->SetupAttachment(RootComponent);
        FireLight->SetWorldLocation(EffectData.Location);
        FireLight->SetIntensity(2000.0f * EffectData.Intensity);
        FireLight->SetLightColor(FLinearColor(1.0f, 0.4f, 0.1f, 1.0f)); // Orange fire color
        FireLight->SetAttenuationRadius(1000.0f);
        
        LightComponents.Add(FireLight);
        
        UE_LOG(LogTemp, Log, TEXT("VFX: Created fire effect with light at %s"), *EffectData.Location.ToString());
    }
}

void AVFX_NiagaraManager::CreateDustEffect(const FVFX_EffectData& EffectData)
{
    // Placeholder for dust particle effect
    // In a full implementation, this would spawn a Niagara system
    UE_LOG(LogTemp, Log, TEXT("VFX: Created dust effect at %s"), *EffectData.Location.ToString());
}

void AVFX_NiagaraManager::CreateBloodEffect(const FVFX_EffectData& EffectData)
{
    // Placeholder for blood particle effect
    UE_LOG(LogTemp, Log, TEXT("VFX: Created blood effect at %s"), *EffectData.Location.ToString());
}

void AVFX_NiagaraManager::CreateWaterEffect(const FVFX_EffectData& EffectData)
{
    // Placeholder for water splash effect
    UE_LOG(LogTemp, Log, TEXT("VFX: Created water effect at %s"), *EffectData.Location.ToString());
}