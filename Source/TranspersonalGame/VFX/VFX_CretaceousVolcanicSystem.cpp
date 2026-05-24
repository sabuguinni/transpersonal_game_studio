#include "VFX_CretaceousVolcanicSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

UVFX_CretaceousVolcanicSystem::UVFX_CretaceousVolcanicSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f;
    
    CurrentIntensity = EVFX_VolcanicIntensity::Dormant;
    VolcanicUpdateInterval = 2.0f;
    MaxVolcanicRange = 10000.0f;
    bIsErupting = false;

    // Initialize Niagara components
    LavaFlowComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LavaFlowComponent"));
    ThermalVentComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ThermalVentComponent"));
    VolcanicAshComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VolcanicAshComponent"));

    // Set default Niagara systems paths
    VolcanicEffects.LavaFlowSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Environment/NS_Lava_Flow")));
    VolcanicEffects.ThermalVentSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Environment/NS_Thermal_Vent")));
    VolcanicEffects.VolcanicAshSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Environment/NS_Volcanic_Ash")));
}

void UVFX_CretaceousVolcanicSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVolcanicSystems();
    
    // Start volcanic update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            VolcanicUpdateTimer,
            this,
            &UVFX_CretaceousVolcanicSystem::UpdateVolcanicEffects,
            VolcanicUpdateInterval,
            true
        );
    }
}

void UVFX_CretaceousVolcanicSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CleanupVolcanicEffects();
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(VolcanicUpdateTimer);
    }
    
    Super::EndPlay(EndPlayReason);
}

void UVFX_CretaceousVolcanicSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update volcanic effect intensities based on current state
    if (bIsErupting)
    {
        float IntensityMultiplier = static_cast<float>(CurrentIntensity) / 4.0f;
        
        if (LavaFlowComponent && LavaFlowComponent->GetAsset())
        {
            LavaFlowComponent->SetFloatParameter(TEXT("IntensityMultiplier"), IntensityMultiplier);
        }
        
        if (ThermalVentComponent && ThermalVentComponent->GetAsset())
        {
            ThermalVentComponent->SetFloatParameter(TEXT("SteamIntensity"), IntensityMultiplier * 2.0f);
        }
        
        if (VolcanicAshComponent && VolcanicAshComponent->GetAsset())
        {
            VolcanicAshComponent->SetFloatParameter(TEXT("AshDensity"), IntensityMultiplier * 1.5f);
        }
    }
}

void UVFX_CretaceousVolcanicSystem::SetVolcanicIntensity(EVFX_VolcanicIntensity NewIntensity)
{
    CurrentIntensity = NewIntensity;
    VolcanicEffects.EffectIntensity = static_cast<float>(NewIntensity) / 4.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Volcanic intensity set to: %d"), static_cast<int32>(NewIntensity));
}

void UVFX_CretaceousVolcanicSystem::TriggerLavaFlow(const FVector& StartLocation, const FVector& EndLocation)
{
    if (UNiagaraSystem* LavaSystem = VolcanicEffects.LavaFlowSystem.LoadSynchronous())
    {
        UNiagaraComponent* LavaEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            LavaSystem,
            StartLocation,
            FRotator::ZeroRotator
        );
        
        if (LavaEffect)
        {
            LavaEffect->SetVectorParameter(TEXT("FlowDirection"), (EndLocation - StartLocation).GetSafeNormal());
            LavaEffect->SetFloatParameter(TEXT("FlowDistance"), FVector::Dist(StartLocation, EndLocation));
            LavaEffect->SetFloatParameter(TEXT("LavaTemperature"), 1200.0f); // Realistic lava temperature
            
            ActiveVolcanicEffects.Add(LavaEffect);
            UE_LOG(LogTemp, Log, TEXT("Lava flow triggered from %s to %s"), *StartLocation.ToString(), *EndLocation.ToString());
        }
    }
}

void UVFX_CretaceousVolcanicSystem::ActivateThermalVent(const FVector& VentLocation, float SteamIntensity)
{
    if (UNiagaraSystem* VentSystem = VolcanicEffects.ThermalVentSystem.LoadSynchronous())
    {
        UNiagaraComponent* VentEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            VentSystem,
            VentLocation,
            FRotator(-90.0f, 0.0f, 0.0f) // Point upward
        );
        
        if (VentEffect)
        {
            VentEffect->SetFloatParameter(TEXT("SteamIntensity"), SteamIntensity);
            VentEffect->SetFloatParameter(TEXT("VentTemperature"), 95.0f); // Hot steam temperature
            VentEffect->SetVectorParameter(TEXT("SteamColor"), FVector(0.9f, 0.95f, 1.0f)); // Slightly blue-white steam
            
            ActiveVolcanicEffects.Add(VentEffect);
            UE_LOG(LogTemp, Log, TEXT("Thermal vent activated at %s with intensity %f"), *VentLocation.ToString(), SteamIntensity);
        }
    }
}

void UVFX_CretaceousVolcanicSystem::TriggerVolcanicAsh(const FVector& EruptionCenter, float AshRadius)
{
    if (UNiagaraSystem* AshSystem = VolcanicEffects.VolcanicAshSystem.LoadSynchronous())
    {
        UNiagaraComponent* AshEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            AshSystem,
            EruptionCenter,
            FRotator::ZeroRotator
        );
        
        if (AshEffect)
        {
            AshEffect->SetFloatParameter(TEXT("AshRadius"), AshRadius);
            AshEffect->SetFloatParameter(TEXT("AshDensity"), VolcanicEffects.EffectIntensity);
            AshEffect->SetVectorParameter(TEXT("WindDirection"), FVector(1.0f, 0.5f, 0.1f)); // Realistic wind pattern
            AshEffect->SetFloatParameter(TEXT("ParticleSize"), FMath::RandRange(0.5f, 2.0f));
            
            ActiveVolcanicEffects.Add(AshEffect);
            UE_LOG(LogTemp, Log, TEXT("Volcanic ash triggered at %s with radius %f"), *EruptionCenter.ToString(), AshRadius);
        }
    }
}

void UVFX_CretaceousVolcanicSystem::StartVolcanicEruption()
{
    bIsErupting = true;
    SetVolcanicIntensity(EVFX_VolcanicIntensity::Erupting);
    
    if (AActor* Owner = GetOwner())
    {
        FVector EruptionCenter = Owner->GetActorLocation();
        
        // Trigger multiple volcanic effects
        TriggerVolcanicAsh(EruptionCenter, 8000.0f);
        TriggerLavaFlow(EruptionCenter, EruptionCenter + FVector(2000.0f, 0.0f, -500.0f));
        ActivateThermalVent(EruptionCenter + FVector(500.0f, 500.0f, 0.0f), 2.0f);
        
        UE_LOG(LogTemp, Warning, TEXT("Volcanic eruption started at %s"), *EruptionCenter.ToString());
    }
}

void UVFX_CretaceousVolcanicSystem::StopVolcanicEruption()
{
    bIsErupting = false;
    SetVolcanicIntensity(EVFX_VolcanicIntensity::LowActivity);
    
    // Gradually reduce effects instead of stopping immediately
    for (UNiagaraComponent* Effect : ActiveVolcanicEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->SetFloatParameter(TEXT("IntensityMultiplier"), 0.1f);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Volcanic eruption stopped"));
}

bool UVFX_CretaceousVolcanicSystem::IsLocationAffectedByVolcanicActivity(const FVector& TestLocation) const
{
    if (!bIsErupting || !GetOwner())
    {
        return false;
    }
    
    float DistanceToVolcano = FVector::Dist(GetOwner()->GetActorLocation(), TestLocation);
    return DistanceToVolcano <= VolcanicEffects.EffectRadius;
}

float UVFX_CretaceousVolcanicSystem::GetVolcanicHeatAtLocation(const FVector& TestLocation) const
{
    if (!IsLocationAffectedByVolcanicActivity(TestLocation))
    {
        return 0.0f;
    }
    
    float DistanceToVolcano = FVector::Dist(GetOwner()->GetActorLocation(), TestLocation);
    float HeatFalloff = 1.0f - (DistanceToVolcano / VolcanicEffects.EffectRadius);
    
    return HeatFalloff * VolcanicEffects.EffectIntensity * 100.0f; // Temperature in Celsius
}

void UVFX_CretaceousVolcanicSystem::UpdateVolcanicEffects()
{
    // Clean up finished effects
    ActiveVolcanicEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !Effect || !IsValid(Effect) || !Effect->IsActive();
    });
    
    // Update effect parameters based on current intensity
    if (CurrentIntensity != EVFX_VolcanicIntensity::Dormant)
    {
        float RandomVariation = FMath::RandRange(0.8f, 1.2f);
        VolcanicEffects.EffectIntensity = (static_cast<float>(CurrentIntensity) / 4.0f) * RandomVariation;
    }
}

void UVFX_CretaceousVolcanicSystem::InitializeVolcanicSystems()
{
    // Load and assign Niagara systems to components
    if (UNiagaraSystem* LavaSystem = VolcanicEffects.LavaFlowSystem.LoadSynchronous())
    {
        LavaFlowComponent->SetAsset(LavaSystem);
        LavaFlowComponent->SetAutoActivate(false);
    }
    
    if (UNiagaraSystem* VentSystem = VolcanicEffects.ThermalVentSystem.LoadSynchronous())
    {
        ThermalVentComponent->SetAsset(VentSystem);
        ThermalVentComponent->SetAutoActivate(false);
    }
    
    if (UNiagaraSystem* AshSystem = VolcanicEffects.VolcanicAshSystem.LoadSynchronous())
    {
        VolcanicAshComponent->SetAsset(AshSystem);
        VolcanicAshComponent->SetAutoActivate(false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Cretaceous volcanic systems initialized"));
}

void UVFX_CretaceousVolcanicSystem::CleanupVolcanicEffects()
{
    // Stop all active effects
    for (UNiagaraComponent* Effect : ActiveVolcanicEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    
    ActiveVolcanicEffects.Empty();
    
    // Stop component effects
    if (LavaFlowComponent)
    {
        LavaFlowComponent->DeactivateImmediate();
    }
    
    if (ThermalVentComponent)
    {
        ThermalVentComponent->DeactivateImmediate();
    }
    
    if (VolcanicAshComponent)
    {
        VolcanicAshComponent->DeactivateImmediate();
    }
}

UNiagaraComponent* UVFX_CretaceousVolcanicSystem::CreateVolcanicEffect(UNiagaraSystem* System, const FVector& Location)
{
    if (!System || !GetWorld())
    {
        return nullptr;
    }
    
    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        FRotator::ZeroRotator
    );
    
    if (NewEffect)
    {
        ActiveVolcanicEffects.Add(NewEffect);
    }
    
    return NewEffect;
}