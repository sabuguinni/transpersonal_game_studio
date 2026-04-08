#include "VFXSystemArchitecture.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UVFXManagerComponent::UVFXManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms for performance
    
    // Default performance settings
    CurrentPerformanceTier = EVFXPerformanceTier::Medium;
    MaxActiveEffects = 50;
    EffectCullingDistance = 5000.0f; // 50 meters
}

void UVFXManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize effect registry with core atmospheric effects
    InitializeDefaultEffects();
    
    // Set initial performance tier based on platform
    DetermineOptimalPerformanceTier();
}

void UVFXManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Performance management
    CullDistantEffects();
    
    // Update effect scaling based on current performance tier
    UpdatePerformanceScaling();
}

UNiagaraComponent* UVFXManagerComponent::SpawnEffect(const FString& EffectName, const FVector& Location, const FRotator& Rotation, AActor* AttachToActor)
{
    // Check if effect exists in registry
    if (!EffectRegistry.Contains(EffectName))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect '%s' not found in registry"), *EffectName);
        return nullptr;
    }
    
    const FVFXEffectDefinition& EffectDef = EffectRegistry[EffectName];
    
    // Performance check
    if (!ShouldSpawnEffect(EffectDef, Location))
    {
        return nullptr;
    }
    
    // Load Niagara system
    UNiagaraSystem* NiagaraSystem = EffectDef.NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load Niagara system for effect '%s'"), *EffectName);
        return nullptr;
    }
    
    // Spawn the effect
    UNiagaraComponent* EffectComponent = nullptr;
    
    if (AttachToActor)
    {
        EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            NiagaraSystem,
            AttachToActor->GetRootComponent(),
            NAME_None,
            Location,
            Rotation,
            EAttachLocation::KeepWorldPosition,
            true
        );
    }
    else
    {
        EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            NiagaraSystem,
            Location,
            Rotation
        );
    }
    
    if (EffectComponent)
    {
        // Apply performance scaling
        float QualityScale = EffectDef.QualityScaling.Contains(CurrentPerformanceTier) 
            ? EffectDef.QualityScaling[CurrentPerformanceTier] 
            : 1.0f;
            
        EffectComponent->SetFloatParameter(TEXT("QualityScale"), QualityScale);
        
        // Track active effect
        ActiveEffects.Add(EffectComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned VFX effect '%s' at location %s"), *EffectName, *Location.ToString());
    }
    
    return EffectComponent;
}

void UVFXManagerComponent::StopEffect(UNiagaraComponent* EffectComponent)
{
    if (EffectComponent && IsValid(EffectComponent))
    {
        EffectComponent->DestroyComponent();
        ActiveEffects.Remove(EffectComponent);
    }
}

void UVFXManagerComponent::StopAllEffectsOfCategory(EVFXCategory Category)
{
    TArray<UNiagaraComponent*> EffectsToRemove;
    
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            // Get effect category from component tags or custom data
            // This would need to be set when spawning the effect
            FString CategoryTag = FString::Printf(TEXT("VFXCategory_%d"), (int32)Category);
            if (Effect->ComponentHasTag(FName(*CategoryTag)))
            {
                Effect->DestroyComponent();
                EffectsToRemove.Add(Effect);
            }
        }
    }
    
    for (UNiagaraComponent* Effect : EffectsToRemove)
    {
        ActiveEffects.Remove(Effect);
    }
}

void UVFXManagerComponent::SetPerformanceTier(EVFXPerformanceTier NewTier)
{
    if (CurrentPerformanceTier != NewTier)
    {
        CurrentPerformanceTier = NewTier;
        UpdatePerformanceScaling();
        
        UE_LOG(LogTemp, Log, TEXT("VFX Performance tier changed to: %d"), (int32)NewTier);
    }
}

void UVFXManagerComponent::SpawnAtmosphericEffect(const FString& EffectName, const FVector& Location, float Duration)
{
    UNiagaraComponent* Effect = SpawnEffect(EffectName, Location);
    
    if (Effect && Duration > 0.0f)
    {
        // Set auto-destroy timer
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Effect]()
        {
            StopEffect(Effect);
        }, Duration, false);
    }
}

void UVFXManagerComponent::TriggerDinosaurPresenceHint(const FVector& Location, float Intensity)
{
    // Spawn subtle effects that suggest dinosaur presence
    // Examples: rustling leaves, disturbed dust, broken branches
    
    FString EffectName = TEXT("DinosaurPresence_Subtle");
    if (Intensity > 0.7f)
    {
        EffectName = TEXT("DinosaurPresence_Strong");
    }
    else if (Intensity > 0.4f)
    {
        EffectName = TEXT("DinosaurPresence_Moderate");
    }
    
    SpawnAtmosphericEffect(EffectName, Location, 3.0f);
}

void UVFXManagerComponent::UpdatePerformanceScaling()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            // Apply current performance tier scaling to all active effects
            float QualityScale = 1.0f;
            
            switch (CurrentPerformanceTier)
            {
                case EVFXPerformanceTier::Low:
                    QualityScale = 0.3f;
                    break;
                case EVFXPerformanceTier::Medium:
                    QualityScale = 0.6f;
                    break;
                case EVFXPerformanceTier::High:
                    QualityScale = 1.0f;
                    break;
                case EVFXPerformanceTier::Cinematic:
                    QualityScale = 1.5f;
                    break;
            }
            
            Effect->SetFloatParameter(TEXT("QualityScale"), QualityScale);
        }
    }
}

void UVFXManagerComponent::CullDistantEffects()
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
        return;
        
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
        return;
        
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    TArray<UNiagaraComponent*> EffectsToRemove;
    
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            float Distance = FVector::Dist(Effect->GetComponentLocation(), PlayerLocation);
            if (Distance > EffectCullingDistance)
            {
                Effect->DestroyComponent();
                EffectsToRemove.Add(Effect);
            }
        }
        else
        {
            EffectsToRemove.Add(Effect);
        }
    }
    
    for (UNiagaraComponent* Effect : EffectsToRemove)
    {
        ActiveEffects.Remove(Effect);
    }
}

bool UVFXManagerComponent::ShouldSpawnEffect(const FVFXEffectDefinition& EffectDef, const FVector& Location)
{
    // Check maximum active effects limit
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        return false;
    }
    
    // Check distance from player
    if (GetWorld() && GetWorld()->GetFirstPlayerController())
    {
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        if (PlayerPawn)
        {
            float Distance = FVector::Dist(Location, PlayerPawn->GetActorLocation());
            if (Distance > EffectCullingDistance)
            {
                return false;
            }
        }
    }
    
    return true;
}

void UVFXManagerComponent::InitializeDefaultEffects()
{
    // This would typically load from a data table or configuration file
    // For now, we'll define some core effects programmatically
    
    FVFXEffectDefinition AtmosphericDust;
    AtmosphericDust.EffectName = TEXT("Atmospheric_Dust");
    AtmosphericDust.Category = EVFXCategory::Atmospheric;
    AtmosphericDust.Intensity = EVFXIntensity::Subtle;
    AtmosphericDust.EmotionalIntent = TEXT("Create sense of age and abandonment");
    AtmosphericDust.UsageContexts.Add(TEXT("Ambient environment"));
    EffectRegistry.Add(AtmosphericDust.EffectName, AtmosphericDust);
    
    FVFXEffectDefinition DinosaurFootstep;
    DinosaurFootstep.EffectName = TEXT("DinosaurPresence_Footstep");
    DinosaurFootstep.Category = EVFXCategory::DinosaurPresence;
    DinosaurFootstep.Intensity = EVFXIntensity::Moderate;
    DinosaurFootstep.EmotionalIntent = TEXT("Suggest nearby dinosaur without showing it");
    DinosaurFootstep.UsageContexts.Add(TEXT("Off-screen dinosaur movement"));
    EffectRegistry.Add(DinosaurFootstep.EffectName, DinosaurFootstep);
}

void UVFXManagerComponent::DetermineOptimalPerformanceTier()
{
    // This would analyze system specs and set appropriate tier
    // For now, default to Medium
    CurrentPerformanceTier = EVFXPerformanceTier::Medium;
}