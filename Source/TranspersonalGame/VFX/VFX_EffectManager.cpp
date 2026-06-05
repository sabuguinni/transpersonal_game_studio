#include "VFX_EffectManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

AVFX_EffectManager::AVFX_EffectManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for performance

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    MaxActiveEffects = 50;
    EffectCullingDistance = 5000.0f;
    UpdateFrequency = 0.1f;
    LastUpdateTime = 0.0f;
    CachedPlayerController = nullptr;

    // Initialize effect library
    InitializeEffectLibrary();
}

void AVFX_EffectManager::BeginPlay()
{
    Super::BeginPlay();

    // Cache player controller
    CachedPlayerController = GetWorld()->GetFirstPlayerController();

    // Start performance timers
    GetWorldTimerManager().SetTimer(
        EffectCleanupTimer,
        this,
        &AVFX_EffectManager::CleanupFinishedEffects,
        1.0f, // Every second
        true
    );

    GetWorldTimerManager().SetTimer(
        PerformanceCullTimer,
        this,
        &AVFX_EffectManager::CullDistantEffects,
        2.0f, // Every 2 seconds
        true
    );

    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Initialized with %d effect types"), EffectLibrary.Num());
}

void AVFX_EffectManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateFrequency)
    {
        // Update active effects performance
        for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
        {
            if (!ActiveEffects[i] || !IsValid(ActiveEffects[i]))
            {
                ActiveEffects.RemoveAt(i);
            }
        }
        LastUpdateTime = 0.0f;
    }
}

void AVFX_EffectManager::InitializeEffectLibrary()
{
    // Initialize default effect data
    FVFX_EffectData BloodData;
    BloodData.Duration = 2.0f;
    BloodData.Intensity = 1.0f;
    BloodData.bLooping = false;
    EffectLibrary.Add(EVFX_EffectType::BloodImpact, BloodData);

    FVFX_EffectData DustData;
    DustData.Duration = 3.0f;
    DustData.Intensity = 0.8f;
    DustData.bLooping = false;
    EffectLibrary.Add(EVFX_EffectType::DustCloud, DustData);

    FVFX_EffectData SparksData;
    SparksData.Duration = 1.5f;
    SparksData.Intensity = 1.2f;
    SparksData.bLooping = false;
    EffectLibrary.Add(EVFX_EffectType::Sparks, SparksData);

    FVFX_EffectData FireData;
    FireData.Duration = 0.0f; // Continuous
    FireData.Intensity = 1.0f;
    FireData.bLooping = true;
    EffectLibrary.Add(EVFX_EffectType::Fire, FireData);

    FVFX_EffectData WaterData;
    WaterData.Duration = 2.5f;
    WaterData.Intensity = 0.9f;
    WaterData.bLooping = false;
    EffectLibrary.Add(EVFX_EffectType::Water, WaterData);

    FVFX_EffectData FootstepData;
    FootstepData.Duration = 1.0f;
    FootstepData.Intensity = 0.7f;
    FootstepData.bLooping = false;
    EffectLibrary.Add(EVFX_EffectType::FootstepDust, FootstepData);
}

void AVFX_EffectManager::PlayEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float Scale)
{
    if (!EffectLibrary.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Effect type not found in library"));
        return;
    }

    // Check performance limits
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Max active effects reached, skipping new effect"));
        return;
    }

    // Check distance culling
    if (!IsLocationVisible(Location))
    {
        return;
    }

    // Create and configure effect
    UNiagaraComponent* EffectComponent = CreateEffectComponent(EffectType, Location, Rotation, Scale);
    if (EffectComponent)
    {
        ActiveEffects.Add(EffectComponent);
        
        // Play associated sound
        const FVFX_EffectData& EffectData = EffectLibrary[EffectType];
        if (EffectData.SoundEffect)
        {
            UGameplayStatics::PlaySoundAtLocation(this, EffectData.SoundEffect, Location);
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Played effect at location %s"), *Location.ToString());
    }
}

void AVFX_EffectManager::PlayEffectAtActor(EVFX_EffectType EffectType, AActor* TargetActor, FVector Offset)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Target actor is null"));
        return;
    }

    FVector EffectLocation = TargetActor->GetActorLocation() + Offset;
    FRotator EffectRotation = TargetActor->GetActorRotation();
    
    PlayEffect(EffectType, EffectLocation, EffectRotation);
}

void AVFX_EffectManager::PlayDinosaurFootstep(FVector Location, float DinosaurSize)
{
    // Scale dust effect based on dinosaur size
    float EffectScale = FMath::Clamp(DinosaurSize, 0.5f, 3.0f);
    PlayEffect(EVFX_EffectType::FootstepDust, Location, FRotator::ZeroRotator, EffectScale);
    
    // Add additional dust for large dinosaurs
    if (DinosaurSize > 2.0f)
    {
        FVector OffsetLocation1 = Location + FVector(50, 50, 0);
        FVector OffsetLocation2 = Location + FVector(-50, -50, 0);
        PlayEffect(EVFX_EffectType::DustCloud, OffsetLocation1, FRotator::ZeroRotator, 0.5f);
        PlayEffect(EVFX_EffectType::DustCloud, OffsetLocation2, FRotator::ZeroRotator, 0.5f);
    }
}

void AVFX_EffectManager::PlayDinosaurRoarEffect(FVector Location, float Intensity)
{
    // Create visual distortion effect for roar
    PlayEffect(EVFX_EffectType::EnvironmentalMist, Location, FRotator::ZeroRotator, Intensity);
    
    // Add dust particles around roar location
    for (int32 i = 0; i < 3; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-200, 200),
            FMath::RandRange(-200, 200),
            FMath::RandRange(0, 100)
        );
        PlayEffect(EVFX_EffectType::DustCloud, Location + RandomOffset, FRotator::ZeroRotator, 0.3f);
    }
}

void AVFX_EffectManager::PlayBloodImpact(FVector ImpactLocation, FVector ImpactNormal, float Damage)
{
    // Scale blood effect based on damage
    float EffectScale = FMath::Clamp(Damage / 50.0f, 0.2f, 2.0f);
    
    // Calculate rotation from impact normal
    FRotator ImpactRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
    
    PlayEffect(EVFX_EffectType::BloodImpact, ImpactLocation, ImpactRotation, EffectScale);
    
    // Add sparks for high damage impacts
    if (Damage > 75.0f)
    {
        PlayEffect(EVFX_EffectType::Sparks, ImpactLocation, ImpactRotation, 0.5f);
    }
}

void AVFX_EffectManager::PlayWeaponImpact(FVector ImpactLocation, FVector ImpactNormal, EWeaponType WeaponType)
{
    FRotator ImpactRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
    
    switch (WeaponType)
    {
        case EWeaponType::Stone:
            PlayEffect(EVFX_EffectType::Sparks, ImpactLocation, ImpactRotation, 0.8f);
            PlayEffect(EVFX_EffectType::DustCloud, ImpactLocation, ImpactRotation, 0.3f);
            break;
            
        case EWeaponType::Spear:
            PlayEffect(EVFX_EffectType::BloodImpact, ImpactLocation, ImpactRotation, 1.0f);
            break;
            
        case EWeaponType::Club:
            PlayEffect(EVFX_EffectType::DustCloud, ImpactLocation, ImpactRotation, 1.2f);
            break;
            
        default:
            PlayEffect(EVFX_EffectType::DustCloud, ImpactLocation, ImpactRotation, 0.5f);
            break;
    }
}

void AVFX_EffectManager::PlayCampfireEffect(FVector Location)
{
    PlayEffect(EVFX_EffectType::Fire, Location, FRotator::ZeroRotator, 1.0f);
    
    // Add smoke effect
    FVector SmokeLocation = Location + FVector(0, 0, 100);
    PlayEffect(EVFX_EffectType::Smoke, SmokeLocation, FRotator::ZeroRotator, 0.8f);
}

void AVFX_EffectManager::PlayWaterSplash(FVector Location, float Intensity)
{
    float EffectScale = FMath::Clamp(Intensity, 0.3f, 2.0f);
    PlayEffect(EVFX_EffectType::Water, Location, FRotator::ZeroRotator, EffectScale);
}

UNiagaraComponent* AVFX_EffectManager::CreateEffectComponent(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float Scale)
{
    if (!EffectLibrary.Contains(EffectType))
    {
        return nullptr;
    }

    const FVFX_EffectData& EffectData = EffectLibrary[EffectType];
    if (!EffectData.NiagaraEffect)
    {
        // Create a basic placeholder component for now
        UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            nullptr, // Will be replaced with actual Niagara system
            Location,
            Rotation,
            FVector(Scale),
            true, // Auto destroy
            true, // Auto activate
            ENCPoolMethod::None,
            true // Pre cull check
        );
        
        return Component;
    }

    return nullptr;
}

void AVFX_EffectManager::CleanupFinishedEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (!ActiveEffects[i] || !IsValid(ActiveEffects[i]) || !ActiveEffects[i]->IsActive())
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}

void AVFX_EffectManager::CullDistantEffects()
{
    if (!CachedPlayerController || !CachedPlayerController->GetPawn())
    {
        return;
    }

    FVector PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
    
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (ActiveEffects[i] && IsValid(ActiveEffects[i]))
        {
            float Distance = FVector::Dist(PlayerLocation, ActiveEffects[i]->GetComponentLocation());
            if (Distance > EffectCullingDistance)
            {
                ActiveEffects[i]->DestroyComponent();
                ActiveEffects.RemoveAt(i);
            }
        }
    }
}

bool AVFX_EffectManager::IsLocationVisible(FVector Location) const
{
    if (!CachedPlayerController || !CachedPlayerController->GetPawn())
    {
        return true; // Default to visible if no player
    }

    FVector PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, Location);
    
    return Distance <= EffectCullingDistance;
}

int32 AVFX_EffectManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void AVFX_EffectManager::SetEffectQuality(float QualityLevel)
{
    QualityLevel = FMath::Clamp(QualityLevel, 0.1f, 1.0f);
    
    // Adjust max effects based on quality
    MaxActiveEffects = FMath::RoundToInt(50 * QualityLevel);
    
    // Adjust culling distance
    EffectCullingDistance = 5000.0f * QualityLevel;
    
    UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Quality set to %f, MaxEffects: %d"), QualityLevel, MaxActiveEffects);
}

void AVFX_EffectManager::StopEffect(EVFX_EffectType EffectType)
{
    // Implementation for stopping specific effect types
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (ActiveEffects[i] && IsValid(ActiveEffects[i]))
        {
            ActiveEffects[i]->Deactivate();
        }
    }
}

void AVFX_EffectManager::StopAllEffects()
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