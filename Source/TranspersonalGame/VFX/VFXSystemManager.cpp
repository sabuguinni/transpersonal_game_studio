#include "VFXSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

UVFXSystemManager::UVFXSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms for performance
}

void UVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeVFXDatabase();
}

void UVFXSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CleanupInactiveVFX();
    UpdateLODDistances();
}

UNiagaraComponent* UVFXSystemManager::SpawnVFXAtLocation(const FString& EffectName, const FVector& Location, const FRotator& Rotation)
{
    if (!VFXDatabase.Contains(EffectName))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect '%s' not found in database"), *EffectName);
        return nullptr;
    }

    const FVFXEffectData& EffectData = VFXDatabase[EffectName];
    
    // Check distance culling
    if (ShouldCullVFXByDistance(Location))
    {
        return nullptr;
    }

    // Check instance limits
    int32 ActiveInstancesOfType = 0;
    for (UNiagaraComponent* Component : ActiveVFXComponents)
    {
        if (Component && Component->GetAsset() == EffectData.NiagaraSystem.LoadSynchronous())
        {
            ActiveInstancesOfType++;
        }
    }

    if (ActiveInstancesOfType >= EffectData.MaxActiveInstances)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX instance limit reached for effect: %s"), *EffectName);
        return nullptr;
    }

    UNiagaraSystem* NiagaraSystem = EffectData.NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        return nullptr;
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        NiagaraSystem,
        Location,
        Rotation
    );

    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        
        // Apply LOD based on distance
        int32 LODLevel = GetLODLevel(FVector::Dist(Location, GetOwner()->GetActorLocation()));
        VFXComponent->SetVariableInt(TEXT("LODLevel"), LODLevel);
    }

    return VFXComponent;
}

UNiagaraComponent* UVFXSystemManager::AttachVFXToActor(const FString& EffectName, AActor* TargetActor, const FName& SocketName)
{
    if (!TargetActor || !VFXDatabase.Contains(EffectName))
    {
        return nullptr;
    }

    const FVFXEffectData& EffectData = VFXDatabase[EffectName];
    UNiagaraSystem* NiagaraSystem = EffectData.NiagaraSystem.LoadSynchronous();
    
    if (!NiagaraSystem)
    {
        return nullptr;
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        NiagaraSystem,
        TargetActor->GetRootComponent(),
        SocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        true
    );

    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
    }

    return VFXComponent;
}

void UVFXSystemManager::PlayCreatureBreathingEffect(AActor* Creature, float IntensityMultiplier)
{
    if (!Creature)
    {
        return;
    }

    UNiagaraComponent* BreathVFX = AttachVFXToActor(TEXT("CreatureBreath"), Creature, TEXT("head"));
    if (BreathVFX)
    {
        BreathVFX->SetVariableFloat(TEXT("Intensity"), IntensityMultiplier);
        BreathVFX->SetVariableFloat(TEXT("Temperature"), 37.0f); // Body temperature effect
    }
}

void UVFXSystemManager::PlayCreatureFootstepEffect(AActor* Creature, const FVector& ImpactLocation, float CreatureWeight)
{
    FString EffectName = TEXT("FootstepDust");
    
    // Choose effect based on creature weight
    if (CreatureWeight > 1000.0f)
    {
        EffectName = TEXT("FootstepHeavy");
    }
    else if (CreatureWeight > 100.0f)
    {
        EffectName = TEXT("FootstepMedium");
    }

    UNiagaraComponent* FootstepVFX = SpawnVFXAtLocation(EffectName, ImpactLocation);
    if (FootstepVFX)
    {
        FootstepVFX->SetVariableFloat(TEXT("Weight"), CreatureWeight);
        FootstepVFX->SetVariableVector(TEXT("ImpactVelocity"), Creature->GetVelocity());
    }
}

void UVFXSystemManager::PlayDomesticationProgressEffect(AActor* Creature, float TrustLevel)
{
    if (!Creature)
    {
        return;
    }

    UNiagaraComponent* TrustVFX = AttachVFXToActor(TEXT("DomesticationAura"), Creature);
    if (TrustVFX)
    {
        // Trust level affects color and intensity
        FLinearColor TrustColor = FLinearColor::LerpUsingHSV(
            FLinearColor::Red,      // 0% trust - red/hostile
            FLinearColor::Green,    // 100% trust - green/friendly
            TrustLevel
        );
        
        VFXComponent->SetVariableLinearColor(TEXT("TrustColor"), TrustColor);
        VFXComponent->SetVariableFloat(TEXT("TrustLevel"), TrustLevel);
        VFXComponent->SetVariableFloat(TEXT("Opacity"), TrustLevel * 0.3f); // Subtle effect
    }
}

void UVFXSystemManager::PlayTemporalGemEffect(const FVector& GemLocation, bool bIsActivating)
{
    FString EffectName = bIsActivating ? TEXT("TemporalGemActivation") : TEXT("TemporalGemIdle");
    
    UNiagaraComponent* GemVFX = SpawnVFXAtLocation(EffectName, GemLocation);
    if (GemVFX)
    {
        GemVFX->SetVariableBool(TEXT("IsActivating"), bIsActivating);
        
        if (bIsActivating)
        {
            // Temporal distortion effect
            PlayTimeDistortionEffect(GemLocation, 1000.0f, 5.0f);
        }
    }
}

void UVFXSystemManager::PlayTimeDistortionEffect(const FVector& Location, float Radius, float Duration)
{
    UNiagaraComponent* DistortionVFX = SpawnVFXAtLocation(TEXT("TimeDistortion"), Location);
    if (DistortionVFX)
    {
        DistortionVFX->SetVariableFloat(TEXT("DistortionRadius"), Radius);
        DistortionVFX->SetVariableFloat(TEXT("Duration"), Duration);
        
        // Auto-destroy after duration
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [DistortionVFX]() { if (DistortionVFX) DistortionVFX->DestroyComponent(); },
            Duration,
            false
        );
    }
}

void UVFXSystemManager::SetAtmosphericTension(float TensionLevel)
{
    // Global atmospheric effects based on tension
    for (UNiagaraComponent* Component : ActiveVFXComponents)
    {
        if (Component && Component->GetAsset()->GetName().Contains(TEXT("Atmospheric")))
        {
            Component->SetVariableFloat(TEXT("TensionLevel"), TensionLevel);
        }
    }
}

void UVFXSystemManager::PlayPredatorPresenceEffect(const FVector& PredatorLocation, float ThreatRadius)
{
    UNiagaraComponent* ThreatVFX = SpawnVFXAtLocation(TEXT("PredatorPresence"), PredatorLocation);
    if (ThreatVFX)
    {
        ThreatVFX->SetVariableFloat(TEXT("ThreatRadius"), ThreatRadius);
        ThreatVFX->SetVariableFloat(TEXT("IntensityFalloff"), 2.0f);
    }
}

void UVFXSystemManager::InitializeVFXDatabase()
{
    // Environmental Effects
    VFXDatabase.Add(TEXT("FootstepDust"), FVFXEffectData());
    VFXDatabase.Add(TEXT("FootstepMedium"), FVFXEffectData());
    VFXDatabase.Add(TEXT("FootstepHeavy"), FVFXEffectData());
    
    // Creature Effects
    VFXDatabase.Add(TEXT("CreatureBreath"), FVFXEffectData());
    VFXDatabase.Add(TEXT("DomesticationAura"), FVFXEffectData());
    
    // Temporal Effects
    VFXDatabase.Add(TEXT("TemporalGemIdle"), FVFXEffectData());
    VFXDatabase.Add(TEXT("TemporalGemActivation"), FVFXEffectData());
    VFXDatabase.Add(TEXT("TimeDistortion"), FVFXEffectData());
    
    // Atmospheric Effects
    VFXDatabase.Add(TEXT("PredatorPresence"), FVFXEffectData());
    VFXDatabase.Add(TEXT("AtmosphericTension"), FVFXEffectData());

    // Set priorities
    VFXDatabase[TEXT("TemporalGemActivation")].Priority = EVFXPriority::Critical;
    VFXDatabase[TEXT("PredatorPresence")].Priority = EVFXPriority::High;
}

void UVFXSystemManager::CleanupInactiveVFX()
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        if (!ActiveVFXComponents[i] || !IsValid(ActiveVFXComponents[i]))
        {
            ActiveVFXComponents.RemoveAt(i);
        }
    }
}

bool UVFXSystemManager::ShouldCullVFXByDistance(const FVector& VFXLocation) const
{
    if (!GetOwner())
    {
        return false;
    }

    float Distance = FVector::Dist(VFXLocation, GetOwner()->GetActorLocation());
    return Distance > MaxVFXDistance;
}

int32 UVFXSystemManager::GetLODLevel(float Distance) const
{
    if (Distance < 1000.0f)
    {
        return 0; // High quality
    }
    else if (Distance < 2500.0f)
    {
        return 1; // Medium quality
    }
    else
    {
        return 2; // Low quality
    }
}