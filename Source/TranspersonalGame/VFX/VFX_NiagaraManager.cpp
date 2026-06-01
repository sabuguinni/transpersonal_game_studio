#include "VFX_NiagaraManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

AVFX_NiagaraManager::AVFX_NiagaraManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize VFX settings
    bEnableDustEffects = true;
    bEnableFireEffects = true;
    bEnableWeatherEffects = true;
    bEnableBloodEffects = true;

    // Performance defaults
    VFXQualityScale = 1.0f;
    MaxVFXDistance = 3000.0f;
    MaxActiveVFXSystems = 50;

    // Biome intensity defaults
    SavanaDustIntensity = 1.2f;
    SwampMistIntensity = 0.8f;
    ForestParticleIntensity = 1.0f;
    DesertSandstormIntensity = 1.5f;
    MountainSnowIntensity = 0.9f;

    LastCleanupTime = 0.0f;
}

void AVFX_NiagaraManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVFXSystems();
    
    UE_LOG(LogTemp, Log, TEXT("VFX Niagara Manager initialized"));
}

void AVFX_NiagaraManager::InitializeVFXSystems()
{
    // Clear any existing VFX actors
    ActiveVFXActors.Empty();
    
    // Set up performance monitoring
    LastCleanupTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("VFX systems initialized with quality scale: %f"), VFXQualityScale);
}

void AVFX_NiagaraManager::TriggerFootstepDust(FVector Location, float Intensity)
{
    if (!bEnableDustEffects || !IsLocationValid(Location))
    {
        return;
    }

    // Check distance to player
    float DistanceToPlayer = CalculateDistanceToPlayer(Location);
    if (DistanceToPlayer > MaxVFXDistance)
    {
        return;
    }

    // Get biome-specific intensity
    EVFX_BiomeType BiomeType = GetBiomeAtLocation(Location);
    float BiomeIntensity = GetBiomeVFXIntensity(BiomeType);
    float FinalIntensity = Intensity * BiomeIntensity * VFXQualityScale;

    // Create dust effect (placeholder implementation)
    UE_LOG(LogTemp, Log, TEXT("Triggered footstep dust at %s with intensity %f"), 
           *Location.ToString(), FinalIntensity);
    
    // Cleanup if needed
    if (ActiveVFXActors.Num() > MaxActiveVFXSystems)
    {
        CleanupDistantVFX();
    }
}

void AVFX_NiagaraManager::TriggerBloodImpact(FVector Location, FVector Direction, float Amount)
{
    if (!bEnableBloodEffects || !IsLocationValid(Location))
    {
        return;
    }

    float DistanceToPlayer = CalculateDistanceToPlayer(Location);
    if (DistanceToPlayer > MaxVFXDistance)
    {
        return;
    }

    float FinalAmount = Amount * VFXQualityScale;
    
    UE_LOG(LogTemp, Log, TEXT("Triggered blood impact at %s with amount %f"), 
           *Location.ToString(), FinalAmount);
}

void AVFX_NiagaraManager::CreateCampfire(FVector Location)
{
    if (!bEnableFireEffects || !IsLocationValid(Location))
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Created campfire VFX at %s"), *Location.ToString());
}

void AVFX_NiagaraManager::TriggerWeatherEffect(EVFX_WeatherType WeatherType, FVector Location, float Intensity)
{
    if (!bEnableWeatherEffects || !IsLocationValid(Location))
    {
        return;
    }

    EVFX_BiomeType BiomeType = GetBiomeAtLocation(Location);
    float BiomeIntensity = GetBiomeVFXIntensity(BiomeType);
    float FinalIntensity = Intensity * BiomeIntensity * VFXQualityScale;

    UE_LOG(LogTemp, Log, TEXT("Triggered weather effect type %d at %s with intensity %f"), 
           (int32)WeatherType, *Location.ToString(), FinalIntensity);
}

void AVFX_NiagaraManager::SetVFXQuality(float NewQuality)
{
    VFXQualityScale = FMath::Clamp(NewQuality, 0.1f, 2.0f);
    
    UE_LOG(LogTemp, Log, TEXT("VFX quality set to %f"), VFXQualityScale);
}

void AVFX_NiagaraManager::CleanupDistantVFX()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Only cleanup every 5 seconds to avoid performance hits
    if (CurrentTime - LastCleanupTime < 5.0f)
    {
        return;
    }

    int32 RemovedCount = 0;
    
    for (int32 i = ActiveVFXActors.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveVFXActors[i]))
        {
            ActiveVFXActors.RemoveAt(i);
            RemovedCount++;
            continue;
        }

        float Distance = CalculateDistanceToPlayer(ActiveVFXActors[i]->GetActorLocation());
        if (Distance > MaxVFXDistance * 1.5f) // Extra margin for cleanup
        {
            ActiveVFXActors[i]->Destroy();
            ActiveVFXActors.RemoveAt(i);
            RemovedCount++;
        }
    }

    LastCleanupTime = CurrentTime;
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Cleaned up %d distant VFX actors"), RemovedCount);
    }
}

int32 AVFX_NiagaraManager::GetActiveVFXCount() const
{
    return ActiveVFXActors.Num();
}

EVFX_BiomeType AVFX_NiagaraManager::GetBiomeAtLocation(FVector Location) const
{
    // Biome detection based on coordinates
    float X = Location.X;
    float Y = Location.Y;

    // Pantano: around (-50000, -45000)
    if (X < -30000 && Y < -25000)
    {
        return EVFX_BiomeType::Swamp;
    }
    // Floresta: around (-45000, 40000)
    else if (X < -25000 && Y > 20000)
    {
        return EVFX_BiomeType::Forest;
    }
    // Deserto: around (55000, 0)
    else if (X > 35000 && FMath::Abs(Y) < 20000)
    {
        return EVFX_BiomeType::Desert;
    }
    // Montanha: around (40000, 50000)
    else if (X > 20000 && Y > 30000)
    {
        return EVFX_BiomeType::Mountain;
    }
    // Default: Savana around (0, 0)
    else
    {
        return EVFX_BiomeType::Savanna;
    }
}

float AVFX_NiagaraManager::GetBiomeVFXIntensity(EVFX_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EVFX_BiomeType::Savanna:
            return SavanaDustIntensity;
        case EVFX_BiomeType::Swamp:
            return SwampMistIntensity;
        case EVFX_BiomeType::Forest:
            return ForestParticleIntensity;
        case EVFX_BiomeType::Desert:
            return DesertSandstormIntensity;
        case EVFX_BiomeType::Mountain:
            return MountainSnowIntensity;
        default:
            return 1.0f;
    }
}

void AVFX_NiagaraManager::UpdateVFXPerformance()
{
    // Performance monitoring and adjustment
    int32 ActiveCount = GetActiveVFXCount();
    
    if (ActiveCount > MaxActiveVFXSystems * 0.8f)
    {
        // Reduce quality slightly when approaching limit
        VFXQualityScale = FMath::Max(0.5f, VFXQualityScale * 0.95f);
    }
    else if (ActiveCount < MaxActiveVFXSystems * 0.3f)
    {
        // Restore quality when load is low
        VFXQualityScale = FMath::Min(1.0f, VFXQualityScale * 1.02f);
    }
}

bool AVFX_NiagaraManager::IsLocationValid(FVector Location) const
{
    // Basic validation - check if location is within reasonable bounds
    return FMath::Abs(Location.X) < 100000.0f && 
           FMath::Abs(Location.Y) < 100000.0f && 
           Location.Z > -1000.0f && 
           Location.Z < 10000.0f;
}

float AVFX_NiagaraManager::CalculateDistanceToPlayer(FVector Location) const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                return FVector::Dist(PlayerPawn->GetActorLocation(), Location);
            }
        }
    }
    
    // Default fallback distance
    return 1000.0f;
}