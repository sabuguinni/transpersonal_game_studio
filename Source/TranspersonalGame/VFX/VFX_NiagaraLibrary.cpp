#include "VFX_NiagaraLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    PrimaryComponentTick.bCanEverTick = true;
    MaxActiveEffects = 50;
    EffectCullingDistance = 5000.0f;
    CurrentLODLevel = 0;
    RainEffect = nullptr;
    bIsRaining = false;
}

void UVFX_NiagaraLibrary::BeginPlay()
{
    Super::BeginPlay();
    InitializeEffectTemplates();
}

void UVFX_NiagaraLibrary::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Cleanup expired effects every 5 seconds
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= 5.0f)
    {
        CleanupExpiredEffects();
        CleanupTimer = 0.0f;
    }
}

void UVFX_NiagaraLibrary::InitializeEffectTemplates()
{
    // Initialize effect template paths (these would be actual Niagara assets in a real project)
    EffectTemplates.Add(EVFX_EffectType::Fire_Campfire, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Fire_Campfire"))));
    EffectTemplates.Add(EVFX_EffectType::Dust_FootstepImpact, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Dust_FootstepImpact"))));
    EffectTemplates.Add(EVFX_EffectType::Blood_Splatter, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Blood_Splatter"))));
    EffectTemplates.Add(EVFX_EffectType::Weather_Rain, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Weather_Rain"))));
    EffectTemplates.Add(EVFX_EffectType::Weather_Snow, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Weather_Snow"))));
    EffectTemplates.Add(EVFX_EffectType::Volcanic_Ash, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Volcanic_Ash"))));
    EffectTemplates.Add(EVFX_EffectType::Water_Splash, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Water_Splash"))));
    EffectTemplates.Add(EVFX_EffectType::Breath_Vapor, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Breath_Vapor"))));
    EffectTemplates.Add(EVFX_EffectType::Combat_Impact, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Combat_Impact"))));
    EffectTemplates.Add(EVFX_EffectType::Crafting_Sparks, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Crafting_Sparks"))));
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnEffect(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation, float Scale)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Max active effects reached (%d), skipping spawn"), MaxActiveEffects);
        return nullptr;
    }

    if (ShouldCullEffect(Location))
    {
        return nullptr;
    }

    UNiagaraSystem* EffectTemplate = GetEffectTemplate(EffectType);
    if (!EffectTemplate)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No template found for effect type %d"), (int32)EffectType);
        return nullptr;
    }

    UNiagaraComponent* EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectTemplate,
        Location,
        Rotation,
        FVector(Scale),
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (EffectComponent)
    {
        RegisterActiveEffect(EffectComponent);
        ApplyLODSettings(EffectComponent, CurrentLODLevel);
    }

    return EffectComponent;
}

void UVFX_NiagaraLibrary::SpawnEffectWithData(const FVFX_EffectData& EffectData)
{
    UNiagaraComponent* Effect = SpawnEffect(EffectData.EffectType, EffectData.Location, EffectData.Rotation, EffectData.Scale.X);
    
    if (Effect)
    {
        // Apply additional parameters
        Effect->SetFloatParameter(TEXT("Intensity"), EffectData.Intensity);
        Effect->SetFloatParameter(TEXT("Duration"), EffectData.Duration);
        Effect->SetBoolParameter(TEXT("Looping"), EffectData.bLooping);
    }
}

void UVFX_NiagaraLibrary::StopEffect(UNiagaraComponent* EffectComponent)
{
    if (EffectComponent && IsValid(EffectComponent))
    {
        EffectComponent->Deactivate();
        UnregisterActiveEffect(EffectComponent);
    }
}

void UVFX_NiagaraLibrary::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->Deactivate();
        }
    }
    ActiveEffects.Empty();
}

void UVFX_NiagaraLibrary::CreateFootstepImpact(const FVector& Location, float DinosaurSize)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Dust_FootstepImpact;
    EffectData.Location = Location;
    EffectData.Scale = FVector(DinosaurSize);
    EffectData.Duration = 2.0f;
    EffectData.Intensity = DinosaurSize;
    EffectData.bLooping = false;
    
    SpawnEffectWithData(EffectData);
}

void UVFX_NiagaraLibrary::CreateBloodSplatter(const FVector& Location, const FVector& Direction)
{
    FRotator SplatterRotation = Direction.Rotation();
    
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Blood_Splatter;
    EffectData.Location = Location;
    EffectData.Rotation = SplatterRotation;
    EffectData.Duration = 3.0f;
    EffectData.Intensity = 1.0f;
    EffectData.bLooping = false;
    
    SpawnEffectWithData(EffectData);
}

void UVFX_NiagaraLibrary::CreateCampfire(const FVector& Location)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Fire_Campfire;
    EffectData.Location = Location;
    EffectData.Duration = -1.0f; // Infinite
    EffectData.Intensity = 1.0f;
    EffectData.bLooping = true;
    
    SpawnEffectWithData(EffectData);
}

void UVFX_NiagaraLibrary::StartRain(float Intensity)
{
    if (bIsRaining)
    {
        StopRain();
    }

    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Weather_Rain;
    EffectData.Location = FVector(0, 0, 1000); // High in the sky
    EffectData.Duration = -1.0f; // Infinite
    EffectData.Intensity = Intensity;
    EffectData.bLooping = true;
    
    RainEffect = SpawnEffect(EffectData.EffectType, EffectData.Location, EffectData.Rotation, EffectData.Scale.X);
    if (RainEffect)
    {
        RainEffect->SetFloatParameter(TEXT("Intensity"), Intensity);
        bIsRaining = true;
    }
}

void UVFX_NiagaraLibrary::StopRain()
{
    if (RainEffect && IsValid(RainEffect))
    {
        StopEffect(RainEffect);
        RainEffect = nullptr;
    }
    bIsRaining = false;
}

void UVFX_NiagaraLibrary::CreateVolcanicAsh(const FVector& Location, float Intensity)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Volcanic_Ash;
    EffectData.Location = Location;
    EffectData.Duration = 10.0f;
    EffectData.Intensity = Intensity;
    EffectData.bLooping = true;
    
    SpawnEffectWithData(EffectData);
}

void UVFX_NiagaraLibrary::CreateBreathVapor(const FVector& Location)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Breath_Vapor;
    EffectData.Location = Location;
    EffectData.Duration = 1.5f;
    EffectData.Intensity = 0.8f;
    EffectData.bLooping = false;
    
    SpawnEffectWithData(EffectData);
}

void UVFX_NiagaraLibrary::CreateCraftingSparks(const FVector& Location)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Crafting_Sparks;
    EffectData.Location = Location;
    EffectData.Duration = 2.0f;
    EffectData.Intensity = 1.0f;
    EffectData.bLooping = false;
    
    SpawnEffectWithData(EffectData);
}

void UVFX_NiagaraLibrary::SetLODLevel(int32 LODLevel)
{
    CurrentLODLevel = FMath::Clamp(LODLevel, 0, 3);
    
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            ApplyLODSettings(Effect, CurrentLODLevel);
        }
    }
}

void UVFX_NiagaraLibrary::CleanupExpiredEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !Effect || !IsValid(Effect) || !Effect->IsActive();
    });
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetEffectTemplate(EVFX_EffectType EffectType)
{
    if (TSoftObjectPtr<UNiagaraSystem>* TemplatePtr = EffectTemplates.Find(EffectType))
    {
        return TemplatePtr->LoadSynchronous();
    }
    return nullptr;
}

void UVFX_NiagaraLibrary::RegisterActiveEffect(UNiagaraComponent* Effect)
{
    if (Effect && IsValid(Effect))
    {
        ActiveEffects.Add(Effect);
    }
}

void UVFX_NiagaraLibrary::UnregisterActiveEffect(UNiagaraComponent* Effect)
{
    ActiveEffects.Remove(Effect);
}

bool UVFX_NiagaraLibrary::ShouldCullEffect(const FVector& EffectLocation) const
{
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        float DistanceToPlayer = FVector::Dist(PlayerPawn->GetActorLocation(), EffectLocation);
        return DistanceToPlayer > EffectCullingDistance;
    }
    return false;
}

void UVFX_NiagaraLibrary::ApplyLODSettings(UNiagaraComponent* Effect, int32 LODLevel)
{
    if (!Effect || !IsValid(Effect))
    {
        return;
    }

    // Apply LOD-based quality settings
    switch (LODLevel)
    {
        case 0: // High Quality
            Effect->SetFloatParameter(TEXT("ParticleCount"), 1.0f);
            Effect->SetFloatParameter(TEXT("QualityMultiplier"), 1.0f);
            break;
        case 1: // Medium Quality
            Effect->SetFloatParameter(TEXT("ParticleCount"), 0.7f);
            Effect->SetFloatParameter(TEXT("QualityMultiplier"), 0.8f);
            break;
        case 2: // Low Quality
            Effect->SetFloatParameter(TEXT("ParticleCount"), 0.5f);
            Effect->SetFloatParameter(TEXT("QualityMultiplier"), 0.6f);
            break;
        case 3: // Very Low Quality
            Effect->SetFloatParameter(TEXT("ParticleCount"), 0.3f);
            Effect->SetFloatParameter(TEXT("QualityMultiplier"), 0.4f);
            break;
    }
}