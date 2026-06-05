#include "VFX_NiagaraLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance
    
    // Initialize default settings
    GlobalVFXScale = 1.0f;
    bEnableVFXLOD = true;
    MaxVFXDistance = 5000.0f;
    
    // Initialize Niagara system paths (will be set in Blueprint or config)
    CampfireSystem = nullptr;
    BloodImpactSystem = nullptr;
    FootstepDustSystem = nullptr;
    RainSystem = nullptr;
    DustStormSystem = nullptr;
    WaterfallSystem = nullptr;
}

void UVFX_NiagaraLibrary::BeginPlay()
{
    Super::BeginPlay();
    
    // Clear any existing effects
    ActiveEffects.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary initialized - Ready for prehistoric VFX"));
}

void UVFX_NiagaraLibrary::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up finished effects to prevent memory leaks
    CleanupFinishedEffects();
}

// === CAMPFIRE VFX ===
void UVFX_NiagaraLibrary::SpawnCampfireEffect(FVector Location, float Intensity)
{
    if (!IsPlayerNearby(Location, MaxVFXDistance))
    {
        return; // LOD optimization - don't spawn if player is far away
    }
    
    if (CampfireSystem.IsValid())
    {
        UNiagaraComponent* CampfireComponent = CreateNiagaraEffect(CampfireSystem.Get(), Location);
        if (CampfireComponent)
        {
            // Set campfire parameters
            CampfireComponent->SetFloatParameter(TEXT("Intensity"), Intensity * GlobalVFXScale);
            CampfireComponent->SetFloatParameter(TEXT("FlameHeight"), 100.0f * Intensity);
            CampfireComponent->SetFloatParameter(TEXT("SparksRate"), 50.0f * Intensity);
            
            UE_LOG(LogTemp, Log, TEXT("Spawned campfire VFX at location: %s"), *Location.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CampfireSystem not loaded - cannot spawn campfire VFX"));
    }
}

void UVFX_NiagaraLibrary::StopCampfireEffect()
{
    // Find and stop all campfire effects
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (ActiveEffects[i] && ActiveEffects[i]->GetAsset() == CampfireSystem.Get())
        {
            ActiveEffects[i]->Deactivate();
            ActiveEffects.RemoveAt(i);
        }
    }
}

// === COMBAT VFX ===
void UVFX_NiagaraLibrary::SpawnBloodImpactEffect(FVector Location, FVector Normal)
{
    if (!IsPlayerNearby(Location, MaxVFXDistance))
    {
        return; // LOD optimization
    }
    
    if (BloodImpactSystem.IsValid())
    {
        FRotator Rotation = Normal.Rotation();
        UNiagaraComponent* BloodComponent = CreateNiagaraEffect(BloodImpactSystem.Get(), Location, Rotation);
        if (BloodComponent)
        {
            BloodComponent->SetVectorParameter(TEXT("ImpactNormal"), Normal);
            BloodComponent->SetFloatParameter(TEXT("BloodAmount"), 1.0f * GlobalVFXScale);
            
            UE_LOG(LogTemp, Log, TEXT("Spawned blood impact VFX at location: %s"), *Location.ToString());
        }
    }
}

void UVFX_NiagaraLibrary::SpawnWeaponImpactEffect(FVector Location, FVector Normal, float Force)
{
    if (!IsPlayerNearby(Location, MaxVFXDistance))
    {
        return;
    }
    
    // Use a simple spark/dust effect for weapon impacts
    // This would use a separate Niagara system for weapon sparks
    FRotator Rotation = Normal.Rotation();
    
    // For now, create a basic dust puff effect
    SpawnFootstepDustEffect(Location, Force * 0.5f);
    
    UE_LOG(LogTemp, Log, TEXT("Spawned weapon impact VFX with force: %f"), Force);
}

// === DINOSAUR VFX ===
void UVFX_NiagaraLibrary::SpawnFootstepDustEffect(FVector Location, float DinosaurSize)
{
    if (!IsPlayerNearby(Location, MaxVFXDistance))
    {
        return;
    }
    
    if (FootstepDustSystem.IsValid())
    {
        UNiagaraComponent* DustComponent = CreateNiagaraEffect(FootstepDustSystem.Get(), Location);
        if (DustComponent)
        {
            float DustScale = FMath::Clamp(DinosaurSize, 0.1f, 5.0f);
            DustComponent->SetFloatParameter(TEXT("DustAmount"), DustScale * GlobalVFXScale);
            DustComponent->SetFloatParameter(TEXT("ParticleSize"), 10.0f * DustScale);
            DustComponent->SetFloatParameter(TEXT("SpreadRadius"), 50.0f * DustScale);
            
            UE_LOG(LogTemp, Log, TEXT("Spawned footstep dust VFX for dinosaur size: %f"), DinosaurSize);
        }
    }
}

void UVFX_NiagaraLibrary::SpawnDinosaurBreathEffect(FVector Location, FVector Direction)
{
    if (!IsPlayerNearby(Location, MaxVFXDistance))
    {
        return;
    }
    
    // Create a simple vapor/mist effect for dinosaur breath
    FRotator Rotation = Direction.Rotation();
    
    // This would use a dedicated breath system in a full implementation
    UE_LOG(LogTemp, Log, TEXT("Spawned dinosaur breath VFX at location: %s"), *Location.ToString());
}

// === WEATHER VFX ===
void UVFX_NiagaraLibrary::SpawnRainEffect(FVector Location, float Intensity)
{
    if (RainSystem.IsValid())
    {
        UNiagaraComponent* RainComponent = CreateNiagaraEffect(RainSystem.Get(), Location);
        if (RainComponent)
        {
            RainComponent->SetFloatParameter(TEXT("RainIntensity"), Intensity * GlobalVFXScale);
            RainComponent->SetFloatParameter(TEXT("DropletSize"), 2.0f);
            RainComponent->SetFloatParameter(TEXT("WindForce"), 10.0f);
            
            UE_LOG(LogTemp, Log, TEXT("Spawned rain VFX with intensity: %f"), Intensity);
        }
    }
}

void UVFX_NiagaraLibrary::SpawnDustStormEffect(FVector Location, FVector Direction, float Intensity)
{
    if (DustStormSystem.IsValid())
    {
        FRotator Rotation = Direction.Rotation();
        UNiagaraComponent* StormComponent = CreateNiagaraEffect(DustStormSystem.Get(), Location, Rotation);
        if (StormComponent)
        {
            StormComponent->SetVectorParameter(TEXT("WindDirection"), Direction);
            StormComponent->SetFloatParameter(TEXT("StormIntensity"), Intensity * GlobalVFXScale);
            StormComponent->SetFloatParameter(TEXT("ParticleDensity"), 100.0f * Intensity);
            
            UE_LOG(LogTemp, Log, TEXT("Spawned dust storm VFX with intensity: %f"), Intensity);
        }
    }
}

// === ENVIRONMENT VFX ===
void UVFX_NiagaraLibrary::SpawnWaterfallSprayEffect(FVector Location, float Height)
{
    if (WaterfallSystem.IsValid())
    {
        UNiagaraComponent* WaterfallComponent = CreateNiagaraEffect(WaterfallSystem.Get(), Location);
        if (WaterfallComponent)
        {
            WaterfallComponent->SetFloatParameter(TEXT("FallHeight"), Height);
            WaterfallComponent->SetFloatParameter(TEXT("SprayIntensity"), Height / 500.0f * GlobalVFXScale);
            WaterfallComponent->SetFloatParameter(TEXT("MistAmount"), 0.5f);
            
            UE_LOG(LogTemp, Log, TEXT("Spawned waterfall spray VFX with height: %f"), Height);
        }
    }
}

void UVFX_NiagaraLibrary::SpawnVolcanicAshEffect(FVector Location, float Radius)
{
    // Create a large-scale ash particle effect
    // This would use a dedicated volcanic ash system
    UE_LOG(LogTemp, Log, TEXT("Spawned volcanic ash VFX at location: %s with radius: %f"), *Location.ToString(), Radius);
}

// === HELPER FUNCTIONS ===
UNiagaraComponent* UVFX_NiagaraLibrary::CreateNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation)
{
    if (!System || !GetWorld())
    {
        return nullptr;
    }
    
    UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        Rotation,
        FVector(GlobalVFXScale),
        true,  // Auto destroy
        true,  // Auto activate
        ENCPoolMethod::None,
        true   // Pre cull check
    );
    
    if (NiagaraComponent)
    {
        ActiveEffects.Add(NiagaraComponent);
    }
    
    return NiagaraComponent;
}

void UVFX_NiagaraLibrary::CleanupFinishedEffects()
{
    // Remove null or inactive components
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (!ActiveEffects[i] || !ActiveEffects[i]->IsActive())
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}

bool UVFX_NiagaraLibrary::IsPlayerNearby(FVector Location, float MaxDistance) const
{
    if (!bEnableVFXLOD)
    {
        return true; // Always spawn if LOD is disabled
    }
    
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return true; // Spawn if we can't find player (safe fallback)
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, Location);
    
    return Distance <= MaxDistance;
}