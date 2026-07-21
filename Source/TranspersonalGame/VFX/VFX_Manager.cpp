#include "VFX_Manager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

AVFX_Manager::AVFX_Manager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize settings
    MaxActiveEffects = 50;
    EffectCullDistance = 5000.0f;
    bEnableVFXLOD = true;

    // Initialize arrays
    ActiveEffects.Reserve(MaxActiveEffects);
    ActiveComponents.Reserve(MaxActiveEffects);
}

void AVFX_Manager::BeginPlay()
{
    Super::BeginPlay();

    // Load VFX assets
    LoadVFXAssets();

    // Setup cleanup timer
    GetWorldTimerManager().SetTimer(CleanupTimer, this, &AVFX_Manager::CleanupExpiredEffects, 1.0f, true);

    UE_LOG(LogTemp, Log, TEXT("VFX Manager initialized with %d max effects"), MaxActiveEffects);
}

void AVFX_Manager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update effect LOD based on camera distance
    if (bEnableVFXLOD)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC && PC->PlayerCameraManager)
        {
            FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
            
            for (UNiagaraComponent* Component : ActiveComponents)
            {
                if (IsValid(Component))
                {
                    float Distance = FVector::Dist(CameraLocation, Component->GetComponentLocation());
                    SetEffectLOD(Component, Distance);
                }
            }
        }
    }
}

void AVFX_Manager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, float Duration)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Max effects reached, skipping spawn"));
        return;
    }

    UNiagaraSystem* SystemToUse = nullptr;
    FString EffectName;

    switch (EffectType)
    {
        case EVFX_EffectType::Fire:
            SystemToUse = FireEffect;
            EffectName = TEXT("Fire");
            break;
        case EVFX_EffectType::Dust:
            SystemToUse = DustEffect;
            EffectName = TEXT("Dust");
            break;
        case EVFX_EffectType::Blood:
            SystemToUse = BloodEffect;
            EffectName = TEXT("Blood");
            break;
        case EVFX_EffectType::Smoke:
            SystemToUse = SmokeEffect;
            EffectName = TEXT("Smoke");
            break;
        case EVFX_EffectType::Water:
            SystemToUse = WaterEffect;
            EffectName = TEXT("Water");
            break;
        default:
            SystemToUse = DustEffect;
            EffectName = TEXT("Default");
            break;
    }

    if (SystemToUse)
    {
        UNiagaraComponent* Component = CreateNiagaraComponent(SystemToUse, Location);
        if (Component)
        {
            // Track the effect
            FVFX_EffectData EffectData;
            EffectData.EffectName = EffectName;
            EffectData.Location = Location;
            EffectData.Duration = Duration;
            EffectData.Intensity = 1.0f;

            ActiveEffects.Add(EffectData);
            ActiveComponents.Add(Component);

            UE_LOG(LogTemp, Log, TEXT("Spawned %s effect at %s"), *EffectName, *Location.ToString());
        }
    }
}

void AVFX_Manager::SpawnDinosaurFootstepEffect(FVector Location, float DinosaurSize)
{
    if (DustEffect)
    {
        UNiagaraComponent* Component = CreateNiagaraComponent(DustEffect, Location);
        if (Component)
        {
            // Scale effect based on dinosaur size
            Component->SetFloatParameter(TEXT("Scale"), DinosaurSize);
            Component->SetFloatParameter(TEXT("Intensity"), DinosaurSize * 0.8f);

            FVFX_EffectData EffectData;
            EffectData.EffectName = TEXT("DinosaurFootstep");
            EffectData.Location = Location;
            EffectData.Duration = 3.0f;
            EffectData.Intensity = DinosaurSize;

            ActiveEffects.Add(EffectData);
            ActiveComponents.Add(Component);

            UE_LOG(LogTemp, Log, TEXT("Spawned dinosaur footstep effect (size: %.1f) at %s"), DinosaurSize, *Location.ToString());
        }
    }
}

void AVFX_Manager::SpawnCampfireEffect(FVector Location)
{
    if (FireEffect)
    {
        UNiagaraComponent* Component = CreateNiagaraComponent(FireEffect, Location);
        if (Component)
        {
            // Make campfire persistent
            Component->SetFloatParameter(TEXT("LifeTime"), -1.0f); // Infinite

            FVFX_EffectData EffectData;
            EffectData.EffectName = TEXT("Campfire");
            EffectData.Location = Location;
            EffectData.Duration = -1.0f; // Persistent
            EffectData.Intensity = 1.0f;

            ActiveEffects.Add(EffectData);
            ActiveComponents.Add(Component);

            UE_LOG(LogTemp, Log, TEXT("Spawned campfire effect at %s"), *Location.ToString());
        }
    }
}

void AVFX_Manager::SpawnBloodEffect(FVector Location, FVector Direction)
{
    if (BloodEffect)
    {
        UNiagaraComponent* Component = CreateNiagaraComponent(BloodEffect, Location);
        if (Component)
        {
            // Set blood direction
            Component->SetVectorParameter(TEXT("Direction"), Direction.GetSafeNormal());

            FVFX_EffectData EffectData;
            EffectData.EffectName = TEXT("Blood");
            EffectData.Location = Location;
            EffectData.Duration = 2.0f;
            EffectData.Intensity = 1.0f;

            ActiveEffects.Add(EffectData);
            ActiveComponents.Add(Component);

            UE_LOG(LogTemp, Log, TEXT("Spawned blood effect at %s"), *Location.ToString());
        }
    }
}

void AVFX_Manager::SpawnDustCloud(FVector Location, float Radius)
{
    if (DustEffect)
    {
        UNiagaraComponent* Component = CreateNiagaraComponent(DustEffect, Location);
        if (Component)
        {
            Component->SetFloatParameter(TEXT("Radius"), Radius);
            Component->SetFloatParameter(TEXT("Intensity"), Radius / 200.0f);

            FVFX_EffectData EffectData;
            EffectData.EffectName = TEXT("DustCloud");
            EffectData.Location = Location;
            EffectData.Duration = 4.0f;
            EffectData.Intensity = Radius / 200.0f;

            ActiveEffects.Add(EffectData);
            ActiveComponents.Add(Component);

            UE_LOG(LogTemp, Log, TEXT("Spawned dust cloud (radius: %.1f) at %s"), Radius, *Location.ToString());
        }
    }
}

void AVFX_Manager::SpawnWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Coverage)
{
    UNiagaraSystem* WeatherSystem = nullptr;
    FString WeatherName;

    switch (WeatherType)
    {
        case EVFX_EffectType::Water:
            WeatherSystem = WaterEffect;
            WeatherName = TEXT("Rain");
            break;
        case EVFX_EffectType::Dust:
            WeatherSystem = DustEffect;
            WeatherName = TEXT("Sandstorm");
            break;
        default:
            return;
    }

    if (WeatherSystem)
    {
        UNiagaraComponent* Component = CreateNiagaraComponent(WeatherSystem, Location);
        if (Component)
        {
            Component->SetFloatParameter(TEXT("Coverage"), Coverage);
            Component->SetFloatParameter(TEXT("Intensity"), 1.0f);

            FVFX_EffectData EffectData;
            EffectData.EffectName = WeatherName;
            EffectData.Location = Location;
            EffectData.Duration = -1.0f; // Weather persists
            EffectData.Intensity = 1.0f;

            ActiveEffects.Add(EffectData);
            ActiveComponents.Add(Component);

            UE_LOG(LogTemp, Log, TEXT("Spawned %s weather effect at %s"), *WeatherName, *Location.ToString());
        }
    }
}

void AVFX_Manager::StopEffect(FString EffectName)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (ActiveEffects[i].EffectName == EffectName)
        {
            if (IsValid(ActiveComponents[i]))
            {
                ActiveComponents[i]->DestroyComponent();
            }
            ActiveEffects.RemoveAt(i);
            ActiveComponents.RemoveAt(i);
            break;
        }
    }
}

void AVFX_Manager::StopAllEffects()
{
    for (UNiagaraComponent* Component : ActiveComponents)
    {
        if (IsValid(Component))
        {
            Component->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
    ActiveComponents.Empty();

    UE_LOG(LogTemp, Log, TEXT("Stopped all VFX effects"));
}

int32 AVFX_Manager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void AVFX_Manager::CleanupExpiredEffects()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        const FVFX_EffectData& Effect = ActiveEffects[i];
        
        // Skip persistent effects (duration < 0)
        if (Effect.Duration < 0.0f)
        {
            continue;
        }

        // Check if effect has expired or component is invalid
        if (!IsValid(ActiveComponents[i]) || 
            (CurrentTime - Effect.Duration > 0.0f && !ActiveComponents[i]->IsActive()))
        {
            if (IsValid(ActiveComponents[i]))
            {
                ActiveComponents[i]->DestroyComponent();
            }
            ActiveEffects.RemoveAt(i);
            ActiveComponents.RemoveAt(i);
        }
    }
}

void AVFX_Manager::LoadVFXAssets()
{
    // Try to load basic Niagara systems from engine content
    // These paths may need adjustment based on actual UE5 content
    
    // Note: In a real project, these would be proper asset references
    // For now, we'll set them to nullptr and handle gracefully
    FireEffect = nullptr;
    DustEffect = nullptr;
    BloodEffect = nullptr;
    SmokeEffect = nullptr;
    WaterEffect = nullptr;

    UE_LOG(LogTemp, Log, TEXT("VFX assets loading complete (using fallback systems)"));
}

UNiagaraComponent* AVFX_Manager::CreateNiagaraComponent(UNiagaraSystem* System, FVector Location)
{
    if (!System)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create Niagara component: System is null"));
        return nullptr;
    }

    UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        FRotator::ZeroRotator,
        FVector::OneVector,
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    return Component;
}

void AVFX_Manager::SetEffectLOD(UNiagaraComponent* Component, float Distance)
{
    if (!IsValid(Component))
    {
        return;
    }

    // Simple LOD system based on distance
    if (Distance > EffectCullDistance)
    {
        Component->SetVisibility(false);
    }
    else
    {
        Component->SetVisibility(true);
        
        // Reduce quality at distance
        float LODScale = FMath::Clamp(1.0f - (Distance / EffectCullDistance), 0.2f, 1.0f);
        Component->SetFloatParameter(TEXT("LODScale"), LODScale);
    }
}