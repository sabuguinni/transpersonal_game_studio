#include "VFX_NiagaraLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second for cleanup
    
    MaxActiveEffects = 50;
    EffectCullDistance = 5000.0f;
    CurrentQualityLevel = EVFX_IntensityLevel::Medium;
}

void UVFX_NiagaraLibrary::BeginPlay()
{
    Super::BeginPlay();
    InitializeEffectLibrary();
}

void UVFX_NiagaraLibrary::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic cleanup of expired effects
    RemoveInactiveEffects();
}

void UVFX_NiagaraLibrary::InitializeEffectLibrary()
{
    // Initialize prehistoric VFX effect library
    FVFX_EffectData FootstepData;
    FootstepData.EffectName = TEXT("TRexFootstep");
    FootstepData.Category = EVFX_EffectCategory::Dinosaur;
    FootstepData.Duration = 3.0f;
    FootstepData.IntensityLevel = EVFX_IntensityLevel::High;
    EffectLibrary.Add(TEXT("TRexFootstep"), FootstepData);

    FVFX_EffectData CampfireData;
    CampfireData.EffectName = TEXT("Campfire");
    CampfireData.Category = EVFX_EffectCategory::Fire;
    CampfireData.Duration = -1.0f; // Infinite
    CampfireData.bLooping = true;
    CampfireData.IntensityLevel = EVFX_IntensityLevel::Medium;
    EffectLibrary.Add(TEXT("Campfire"), CampfireData);

    FVFX_EffectData BloodData;
    BloodData.EffectName = TEXT("BloodSplatter");
    BloodData.Category = EVFX_EffectCategory::Combat;
    BloodData.Duration = 2.0f;
    BloodData.IntensityLevel = EVFX_IntensityLevel::High;
    EffectLibrary.Add(TEXT("BloodSplatter"), BloodData);

    FVFX_EffectData DustData;
    DustData.EffectName = TEXT("DustCloud");
    DustData.Category = EVFX_EffectCategory::Environment;
    DustData.Duration = 4.0f;
    DustData.IntensityLevel = EVFX_IntensityLevel::Medium;
    EffectLibrary.Add(TEXT("DustCloud"), DustData);

    FVFX_EffectData RainData;
    RainData.EffectName = TEXT("HeavyRain");
    RainData.Category = EVFX_EffectCategory::Weather;
    RainData.Duration = -1.0f; // Infinite
    RainData.bLooping = true;
    RainData.IntensityLevel = EVFX_IntensityLevel::High;
    EffectLibrary.Add(TEXT("HeavyRain"), RainData);

    UE_LOG(LogTemp, Warning, TEXT("VFX Library initialized with %d effects"), EffectLibrary.Num());
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnVFXAtLocation(const FVector& Location, EVFX_EffectCategory Category, const FString& EffectName)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("VFX_NiagaraLibrary: No valid world context"));
        return nullptr;
    }

    // Check if we should cull this effect due to distance
    if (ShouldCullEffect(Location))
    {
        return nullptr;
    }

    // Find effect data
    FVFX_EffectData* EffectData = EffectLibrary.Find(EffectName);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX effect not found: %s"), *EffectName);
        return nullptr;
    }

    // Check active effect limit
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        CleanupExpiredEffects();
        if (ActiveEffects.Num() >= MaxActiveEffects)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX effect limit reached, skipping: %s"), *EffectName);
            return nullptr;
        }
    }

    // Create Niagara component
    UNiagaraComponent* NiagaraComp = CreateNiagaraComponent(*EffectData);
    if (NiagaraComp)
    {
        NiagaraComp->SetWorldLocation(Location);
        NiagaraComp->Activate();
        RegisterActiveEffect(NiagaraComp);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned VFX: %s at location %s"), *EffectName, *Location.ToString());
    }

    return NiagaraComp;
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnVFXAttached(USceneComponent* AttachComponent, EVFX_EffectCategory Category, const FString& EffectName)
{
    if (!AttachComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX_NiagaraLibrary: Invalid attach component"));
        return nullptr;
    }

    return SpawnVFXAtLocation(AttachComponent->GetComponentLocation(), Category, EffectName);
}

void UVFX_NiagaraLibrary::PlayFootstepEffect(const FVector& Location, float DinosaurSize)
{
    FString EffectName = DinosaurSize > 5.0f ? TEXT("TRexFootstep") : TEXT("DustCloud");
    SpawnVFXAtLocation(Location, EVFX_EffectCategory::Dinosaur, EffectName);
}

void UVFX_NiagaraLibrary::PlayRoarEffect(const FVector& Location, float Intensity)
{
    // Create air distortion effect for roar
    SpawnVFXAtLocation(Location, EVFX_EffectCategory::Dinosaur, TEXT("DustCloud"));
}

void UVFX_NiagaraLibrary::PlayBreathingEffect(USceneComponent* AttachPoint, bool bColdWeather)
{
    if (bColdWeather)
    {
        SpawnVFXAttached(AttachPoint, EVFX_EffectCategory::Dinosaur, TEXT("DustCloud"));
    }
}

void UVFX_NiagaraLibrary::PlayBloodImpact(const FVector& Location, const FVector& ImpactDirection)
{
    SpawnVFXAtLocation(Location, EVFX_EffectCategory::Combat, TEXT("BloodSplatter"));
}

void UVFX_NiagaraLibrary::PlayWeaponImpact(const FVector& Location, const FString& WeaponType)
{
    SpawnVFXAtLocation(Location, EVFX_EffectCategory::Combat, TEXT("DustCloud"));
}

void UVFX_NiagaraLibrary::PlayCampfireEffect(const FVector& Location)
{
    SpawnVFXAtLocation(Location, EVFX_EffectCategory::Fire, TEXT("Campfire"));
}

void UVFX_NiagaraLibrary::PlayDustCloud(const FVector& Location, float Size)
{
    SpawnVFXAtLocation(Location, EVFX_EffectCategory::Environment, TEXT("DustCloud"));
}

void UVFX_NiagaraLibrary::StartRainEffect(float Intensity)
{
    if (GetWorld())
    {
        FVector WorldCenter = FVector::ZeroVector;
        SpawnVFXAtLocation(WorldCenter + FVector(0, 0, 2000), EVFX_EffectCategory::Weather, TEXT("HeavyRain"));
    }
}

void UVFX_NiagaraLibrary::StopRainEffect()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && Effect->IsValidLowLevel())
        {
            FVFX_EffectData* Data = EffectLibrary.Find(TEXT("HeavyRain"));
            if (Data && Data->Category == EVFX_EffectCategory::Weather)
            {
                Effect->Deactivate();
            }
        }
    }
}

void UVFX_NiagaraLibrary::PlayLightningEffect(const FVector& Location)
{
    SpawnVFXAtLocation(Location, EVFX_EffectCategory::Weather, TEXT("DustCloud"));
}

void UVFX_NiagaraLibrary::SetGlobalVFXQuality(EVFX_IntensityLevel QualityLevel)
{
    CurrentQualityLevel = QualityLevel;
    
    // Adjust max effects based on quality
    switch (QualityLevel)
    {
        case EVFX_IntensityLevel::Low:
            MaxActiveEffects = 20;
            EffectCullDistance = 3000.0f;
            break;
        case EVFX_IntensityLevel::Medium:
            MaxActiveEffects = 50;
            EffectCullDistance = 5000.0f;
            break;
        case EVFX_IntensityLevel::High:
            MaxActiveEffects = 100;
            EffectCullDistance = 8000.0f;
            break;
        case EVFX_IntensityLevel::Epic:
            MaxActiveEffects = 200;
            EffectCullDistance = 12000.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX Quality set to level %d, Max Effects: %d"), (int32)QualityLevel, MaxActiveEffects);
}

void UVFX_NiagaraLibrary::CleanupExpiredEffects()
{
    RemoveInactiveEffects();
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateNiagaraComponent(const FVFX_EffectData& EffectData)
{
    if (!GetWorld())
    {
        return nullptr;
    }

    // Create a temporary actor to hold the Niagara component
    AActor* TempActor = GetWorld()->SpawnActor<AActor>();
    if (!TempActor)
    {
        return nullptr;
    }

    UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(TempActor);
    if (NiagaraComp)
    {
        TempActor->SetRootComponent(NiagaraComp);
        NiagaraComp->RegisterComponent();
        
        // Set auto-destroy if not looping
        if (!EffectData.bLooping && EffectData.Duration > 0)
        {
            TempActor->SetLifeSpan(EffectData.Duration + 1.0f);
        }
    }

    return NiagaraComp;
}

void UVFX_NiagaraLibrary::RegisterActiveEffect(UNiagaraComponent* Effect)
{
    if (Effect)
    {
        ActiveEffects.Add(Effect);
    }
}

void UVFX_NiagaraLibrary::RemoveInactiveEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !Effect || !Effect->IsValidLowLevel() || !Effect->IsActive();
    });
}

bool UVFX_NiagaraLibrary::ShouldCullEffect(const FVector& EffectLocation) const
{
    if (!GetWorld())
    {
        return true;
    }

    // Get player pawn location for distance culling
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), EffectLocation);
        return Distance > EffectCullDistance;
    }

    return false;
}