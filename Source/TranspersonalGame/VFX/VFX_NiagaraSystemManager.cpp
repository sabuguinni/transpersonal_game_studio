#include "VFX_NiagaraSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/SceneComponent.h"

AVFX_NiagaraSystemManager::AVFX_NiagaraSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for VFX updates

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("VFXManagerRoot"));

    // Initialize default values
    GlobalVFXIntensity = 1.0f;
    bVFXEnabled = true;
    MaxActiveVFXSystems = 10;
    VFXCullingDistance = 5000.0f;
    CurrentThreatLevel = EThreatLevel::Safe;
    CurrentBiome = EBiomeType::Forest;
}

void AVFX_NiagaraSystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize VFX database with prehistoric effects
    InitializeVFXDatabase();

    // Start cleanup timer
    GetWorldTimerManager().SetTimer(VFXCleanupTimer, this, &AVFX_NiagaraSystemManager::CleanupExpiredVFX, 5.0f, true);

    // Start VFX update timer
    GetWorldTimerManager().SetTimer(VFXUpdateTimer, this, &AVFX_NiagaraSystemManager::UpdateVFXBasedOnThreatLevel, 1.0f, true);

    UE_LOG(LogTemp, Log, TEXT("VFX Niagara System Manager initialized"));
}

void AVFX_NiagaraSystemManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Stop all active VFX
    StopAllVFXEffects();

    // Clear timers
    GetWorldTimerManager().ClearTimer(VFXCleanupTimer);
    GetWorldTimerManager().ClearTimer(VFXUpdateTimer);

    Super::EndPlay(EndPlayReason);
}

void AVFX_NiagaraSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update VFX intensity based on current conditions
    if (bVFXEnabled)
    {
        UpdateVFXBasedOnBiome();
    }
}

UNiagaraComponent* AVFX_NiagaraSystemManager::SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    if (!bVFXEnabled)
    {
        return nullptr;
    }

    // Check if we're at max capacity
    if (ActiveVFXComponents.Num() >= MaxActiveVFXSystems)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX System at maximum capacity, cannot spawn new effect"));
        return nullptr;
    }

    // Check culling distance
    if (!IsVFXWithinCullingDistance(Location))
    {
        return nullptr;
    }

    // Get effect data
    const FVFX_EffectData* EffectData = VFXEffectDatabase.Find(EffectType);
    if (!EffectData || !EffectData->NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect data not found or invalid Niagara system for type: %d"), (int32)EffectType);
        return nullptr;
    }

    // Stop existing effect of same type if running
    if (ActiveVFXComponents.Contains(EffectType))
    {
        StopVFXEffect(EffectType);
    }

    // Spawn Niagara component
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectData->NiagaraSystem.Get(),
        Location,
        Rotation,
        FVector(1.0f),
        EffectData->bAutoActivate,
        true, // Auto destroy
        ENCPoolMethod::None,
        EffectData->bLooping
    );

    if (NiagaraComp)
    {
        // Set intensity parameter
        NiagaraComp->SetFloatParameter(TEXT("Intensity"), EffectData->Intensity * GlobalVFXIntensity);
        
        // Store active component
        ActiveVFXComponents.Add(EffectType, NiagaraComp);

        UE_LOG(LogTemp, Log, TEXT("Spawned VFX Effect: %d at location %s"), (int32)EffectType, *Location.ToString());
    }

    return NiagaraComp;
}

void AVFX_NiagaraSystemManager::StopVFXEffect(EVFX_EffectType EffectType)
{
    if (UNiagaraComponent** FoundComp = ActiveVFXComponents.Find(EffectType))
    {
        if (*FoundComp && IsValid(*FoundComp))
        {
            (*FoundComp)->Deactivate();
            (*FoundComp)->DestroyComponent();
        }
        ActiveVFXComponents.Remove(EffectType);
        
        UE_LOG(LogTemp, Log, TEXT("Stopped VFX Effect: %d"), (int32)EffectType);
    }
}

void AVFX_NiagaraSystemManager::StopAllVFXEffects()
{
    for (auto& VFXPair : ActiveVFXComponents)
    {
        if (VFXPair.Value && IsValid(VFXPair.Value))
        {
            VFXPair.Value->Deactivate();
            VFXPair.Value->DestroyComponent();
        }
    }
    ActiveVFXComponents.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Stopped all VFX effects"));
}

void AVFX_NiagaraSystemManager::SetVFXIntensity(EVFX_EffectType EffectType, float Intensity)
{
    if (UNiagaraComponent** FoundComp = ActiveVFXComponents.Find(EffectType))
    {
        if (*FoundComp && IsValid(*FoundComp))
        {
            (*FoundComp)->SetFloatParameter(TEXT("Intensity"), Intensity * GlobalVFXIntensity);
        }
    }

    // Update database
    if (FVFX_EffectData* EffectData = VFXEffectDatabase.Find(EffectType))
    {
        EffectData->Intensity = Intensity;
    }
}

void AVFX_NiagaraSystemManager::SpawnCampfireVFX(FVector Location)
{
    UNiagaraComponent* CampfireComp = SpawnVFXEffect(EVFX_EffectType::Campfire, Location);
    if (CampfireComp)
    {
        // Set campfire-specific parameters
        CampfireComp->SetFloatParameter(TEXT("FlameHeight"), 200.0f);
        CampfireComp->SetFloatParameter(TEXT("SmokeIntensity"), 0.8f);
        CampfireComp->SetVectorParameter(TEXT("FlameColor"), FVector(1.0f, 0.4f, 0.1f));
    }
}

void AVFX_NiagaraSystemManager::SpawnWeatherRainVFX(FVector Location, float Intensity)
{
    UNiagaraComponent* RainComp = SpawnVFXEffect(EVFX_EffectType::WeatherRain, Location);
    if (RainComp)
    {
        RainComp->SetFloatParameter(TEXT("RainIntensity"), Intensity);
        RainComp->SetFloatParameter(TEXT("DropSize"), 1.0f + Intensity * 0.5f);
        RainComp->SetVectorParameter(TEXT("WindDirection"), FVector(0.0f, 1.0f, -1.0f));
    }
}

void AVFX_NiagaraSystemManager::SpawnWaterSplashVFX(FVector Location, float SplashSize)
{
    UNiagaraComponent* SplashComp = SpawnVFXEffect(EVFX_EffectType::WaterSplash, Location);
    if (SplashComp)
    {
        SplashComp->SetFloatParameter(TEXT("SplashSize"), SplashSize);
        SplashComp->SetFloatParameter(TEXT("ParticleCount"), 50.0f * SplashSize);
    }
}

void AVFX_NiagaraSystemManager::SpawnDinoFootstepVFX(FVector Location, float DinoSize)
{
    UNiagaraComponent* FootstepComp = SpawnVFXEffect(EVFX_EffectType::DinoFootstep, Location);
    if (FootstepComp)
    {
        FootstepComp->SetFloatParameter(TEXT("ImpactSize"), DinoSize);
        FootstepComp->SetFloatParameter(TEXT("DustAmount"), 100.0f * DinoSize);
        FootstepComp->SetVectorParameter(TEXT("ImpactDirection"), FVector(0.0f, 0.0f, -1.0f));
    }
}

void AVFX_NiagaraSystemManager::SpawnBloodImpactVFX(FVector Location, FVector ImpactDirection)
{
    UNiagaraComponent* BloodComp = SpawnVFXEffect(EVFX_EffectType::BloodImpact, Location);
    if (BloodComp)
    {
        BloodComp->SetVectorParameter(TEXT("ImpactDirection"), ImpactDirection.GetSafeNormal());
        BloodComp->SetFloatParameter(TEXT("BloodAmount"), 1.0f);
        BloodComp->SetVectorParameter(TEXT("BloodColor"), FVector(0.8f, 0.1f, 0.1f));
    }
}

void AVFX_NiagaraSystemManager::SpawnDustCloudVFX(FVector Location, float CloudSize)
{
    UNiagaraComponent* DustComp = SpawnVFXEffect(EVFX_EffectType::DustCloud, Location);
    if (DustComp)
    {
        DustComp->SetFloatParameter(TEXT("CloudSize"), CloudSize);
        DustComp->SetFloatParameter(TEXT("ParticleLifetime"), 3.0f + CloudSize);
    }
}

void AVFX_NiagaraSystemManager::SpawnSparksVFX(FVector Location, FVector Direction)
{
    UNiagaraComponent* SparksComp = SpawnVFXEffect(EVFX_EffectType::Sparks, Location);
    if (SparksComp)
    {
        SparksComp->SetVectorParameter(TEXT("SparkDirection"), Direction.GetSafeNormal());
        SparksComp->SetFloatParameter(TEXT("SparkCount"), 25.0f);
        SparksComp->SetFloatParameter(TEXT("SparkSpeed"), 500.0f);
    }
}

void AVFX_NiagaraSystemManager::SyncWithAudioThreatLevel(EThreatLevel ThreatLevel)
{
    CurrentThreatLevel = ThreatLevel;
    UpdateVFXBasedOnThreatLevel();
}

void AVFX_NiagaraSystemManager::SyncWithBiomeAudio(EBiomeType BiomeType)
{
    CurrentBiome = BiomeType;
    UpdateVFXBasedOnBiome();
}

void AVFX_NiagaraSystemManager::InitializeVFXDatabase()
{
    // Initialize prehistoric VFX effect database
    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::Campfire;
    CampfireData.LifeTime = -1.0f; // Infinite
    CampfireData.Intensity = 1.0f;
    CampfireData.bLooping = true;
    VFXEffectDatabase.Add(EVFX_EffectType::Campfire, CampfireData);

    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::DinoFootstep;
    FootstepData.LifeTime = 2.0f;
    FootstepData.Intensity = 1.5f;
    FootstepData.bLooping = false;
    VFXEffectDatabase.Add(EVFX_EffectType::DinoFootstep, FootstepData);

    FVFX_EffectData RainData;
    RainData.EffectType = EVFX_EffectType::WeatherRain;
    RainData.LifeTime = -1.0f; // Infinite
    RainData.Intensity = 1.0f;
    RainData.bLooping = true;
    VFXEffectDatabase.Add(EVFX_EffectType::WeatherRain, RainData);

    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::BloodImpact;
    BloodData.LifeTime = 3.0f;
    BloodData.Intensity = 1.2f;
    BloodData.bLooping = false;
    VFXEffectDatabase.Add(EVFX_EffectType::BloodImpact, BloodData);

    FVFX_EffectData WaterData;
    WaterData.EffectType = EVFX_EffectType::WaterSplash;
    WaterData.LifeTime = 1.5f;
    WaterData.Intensity = 1.0f;
    WaterData.bLooping = false;
    VFXEffectDatabase.Add(EVFX_EffectType::WaterSplash, WaterData);

    UE_LOG(LogTemp, Log, TEXT("VFX Database initialized with %d effects"), VFXEffectDatabase.Num());
}

void AVFX_NiagaraSystemManager::CleanupExpiredVFX()
{
    TArray<EVFX_EffectType> ToRemove;

    for (auto& VFXPair : ActiveVFXComponents)
    {
        if (!VFXPair.Value || !IsValid(VFXPair.Value) || !VFXPair.Value->IsActive())
        {
            ToRemove.Add(VFXPair.Key);
        }
    }

    for (EVFX_EffectType EffectType : ToRemove)
    {
        ActiveVFXComponents.Remove(EffectType);
    }
}

bool AVFX_NiagaraSystemManager::IsVFXWithinCullingDistance(FVector VFXLocation) const
{
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), VFXLocation);
        return Distance <= VFXCullingDistance;
    }
    return true; // Default to allowing VFX if no player found
}

void AVFX_NiagaraSystemManager::UpdateVFXBasedOnThreatLevel()
{
    switch (CurrentThreatLevel)
    {
    case EThreatLevel::Safe:
        GlobalVFXIntensity = 0.8f;
        break;
    case EThreatLevel::Caution:
        GlobalVFXIntensity = 1.0f;
        break;
    case EThreatLevel::Danger:
        GlobalVFXIntensity = 1.3f;
        break;
    case EThreatLevel::Combat:
        GlobalVFXIntensity = 1.8f;
        break;
    }

    // Update all active VFX with new intensity
    for (auto& VFXPair : ActiveVFXComponents)
    {
        if (VFXPair.Value && IsValid(VFXPair.Value))
        {
            const FVFX_EffectData* EffectData = VFXEffectDatabase.Find(VFXPair.Key);
            if (EffectData)
            {
                VFXPair.Value->SetFloatParameter(TEXT("Intensity"), EffectData->Intensity * GlobalVFXIntensity);
            }
        }
    }
}

void AVFX_NiagaraSystemManager::UpdateVFXBasedOnBiome()
{
    // Adjust VFX parameters based on current biome
    switch (CurrentBiome)
    {
    case EBiomeType::Forest:
        // More particle density for forest atmosphere
        for (auto& VFXPair : ActiveVFXComponents)
        {
            if (VFXPair.Value && IsValid(VFXPair.Value))
            {
                VFXPair.Value->SetFloatParameter(TEXT("BiomeMultiplier"), 1.2f);
            }
        }
        break;
    case EBiomeType::Plains:
        // Windier effects for open plains
        for (auto& VFXPair : ActiveVFXComponents)
        {
            if (VFXPair.Value && IsValid(VFXPair.Value))
            {
                VFXPair.Value->SetFloatParameter(TEXT("BiomeMultiplier"), 1.0f);
                VFXPair.Value->SetVectorParameter(TEXT("WindStrength"), FVector(2.0f, 1.0f, 0.0f));
            }
        }
        break;
    case EBiomeType::River:
        // Water-based effects enhanced
        for (auto& VFXPair : ActiveVFXComponents)
        {
            if (VFXPair.Value && IsValid(VFXPair.Value))
            {
                VFXPair.Value->SetFloatParameter(TEXT("BiomeMultiplier"), 0.8f);
                VFXPair.Value->SetFloatParameter(TEXT("Humidity"), 1.5f);
            }
        }
        break;
    }
}